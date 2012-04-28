#include "Pointer.h"
#include "Array.h"
#include "Utils.h"
#include "State.h"
#include "SlotTracker.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/BasicBlock.h>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace Canal {
namespace Pointer {

Target::Target() : mType(Target::Uninitialized),
                   mArrayOffset(NULL)
{
}

Target::Target(const Target &target) : mType(target.mType),
                                       mConstant(target.mConstant),
                                       mInstruction(target.mInstruction),
                                       mArrayOffset(target.mArrayOffset)
{
    if (mArrayOffset)
        mArrayOffset = mArrayOffset->clone();
}

Target::~Target()
{
    delete mArrayOffset;
}

bool
Target::operator==(const Target &target) const
{
    if (mType != target.mType)
        return false;

    switch (mType)
    {
    case Uninitialized:
        return true;
    case Constant:
        return mConstant == target.mConstant;
    case MemoryBlock:
        // Check array offset.
        if (!mArrayOffset xor !target.mArrayOffset)
            return false;

        if (mArrayOffset && *mArrayOffset != *target.mArrayOffset)
            return false;

        return mInstruction == target.mInstruction;
    default:
        CANAL_DIE();
    }

    return true;
}

bool
Target::operator!=(const Target &target) const
{
    return !(*this == target);
}

void
Target::merge(const Target &target)
{
    CANAL_ASSERT(mType == target.mType);
    switch (mType)
    {
    case Uninitialized:
        break;
    case Constant:
        // TODO: mConstant can be abstract value.
        CANAL_ASSERT(mConstant == target.mConstant);
        break;
    case MemoryBlock:
        if (mArrayOffset)
        {
            if (target.mArrayOffset)
                mArrayOffset->merge(*target.mArrayOffset);
            else
                CANAL_NOT_IMPLEMENTED(); // only one pointer is array offset
        }
        else if (target.mArrayOffset)
            CANAL_NOT_IMPLEMENTED(); // only one pointer is array offset

        CANAL_ASSERT(mInstruction == target.mInstruction);
        break;
    default:
        CANAL_DIE();
    }
}

size_t
Target::memoryUsage() const
{
    return sizeof(Target) + (mArrayOffset ? mArrayOffset->memoryUsage() : 0);
}

std::string
Target::toString(SlotTracker &slotTracker) const
{
    std::stringstream ss;
    ss << "Pointer::Target: ";
    if (mArrayOffset)
    {
        ss << "{" << std::endl;
        ss << "    array offset: " << mArrayOffset->toString() << std::endl;
        ss << "    target: ";
    }

    switch (mType)
    {
    case Uninitialized:
        ss << "uninitialized";
        break;
    case Constant:
        ss << mConstant;
        break;
    case MemoryBlock:
    {
        const llvm::Instruction &instruction = llvm::cast<llvm::Instruction>(*mInstruction);
        slotTracker.setActiveFunction(*instruction.getParent()->getParent());
        std::string name(Canal::getName(instruction, slotTracker));
        if (name.empty())
            name = "<failed to name the location>";

        ss << name;
        break;
    }
    default:
        CANAL_DIE();
    }

    if (mArrayOffset)
        ss << std::endl << "}";

    return ss.str();
}

Value *
Target::dereference(const State &state) const
{
    switch (mType)
    {
    case Uninitialized:
    case Constant:
        return NULL;
    case MemoryBlock:
    {
        Value *variable = state.findVariable(*mInstruction);
        if (!variable)
            return NULL;
        if (!mArrayOffset)
            return variable;
        // Do not care about offsets when lacking better array.
        const Array::SingleItem &array = dynamic_cast<const Array::SingleItem&>(*variable);
        return array.mValue;
    }
    default:
        CANAL_DIE();
    }
}

InclusionBased::InclusionBased(const llvm::Module &module) : mModule(module)
{
}

InclusionBased *
InclusionBased::clone() const
{
    return new InclusionBased(*this);
}

bool
InclusionBased::operator==(const Value &value) const
{
    // Check if the value has the same type.
    const InclusionBased *pointer = dynamic_cast<const InclusionBased*>(&value);
    if (!pointer)
        return false;

    // Check if it has the same number of targets.
    if (pointer->mTargets.size() != mTargets.size())
        return false;

    // Check the targets.
    PlaceTargetMap::const_iterator it1 = pointer->mTargets.begin(),
        it2 = mTargets.begin();
    for (; it2 != mTargets.end(); ++it1, ++it2)
    {
        if (it1->first != it2->first || it1->second != it2->second)
            return false;
    }

    return true;
}

void
InclusionBased::merge(const Value &value)
{
    const InclusionBased &vv = dynamic_cast<const InclusionBased&>(value);
    PlaceTargetMap::const_iterator valueit = vv.mTargets.begin();
    for (; valueit != vv.mTargets.end(); ++valueit)
    {
        PlaceTargetMap::iterator it = mTargets.find(valueit->first);
        if (it == mTargets.end())
            mTargets.insert(*valueit);
        else
            it->second.merge(valueit->second);
    }
}

size_t
InclusionBased::memoryUsage() const
{
    size_t size = sizeof(InclusionBased);
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        size += it->second.memoryUsage();
    return size;
}

std::string
InclusionBased::toString() const
{
    SlotTracker slotTracker(mModule);
    std::stringstream ss;
    ss << "Pointer::InclusionBased: [" << std::endl;
    for (PlaceTargetMap::const_iterator it = mTargets.begin(); it != mTargets.end(); ++it)
    {
        const llvm::Instruction &instruction = llvm::cast<llvm::Instruction>(*it->first);
        slotTracker.setActiveFunction(*instruction.getParent()->getParent());
        std::string name(Canal::getName(*it->first, slotTracker));
        if (name.empty())
            name = "<failed to name the location>";

        ss << "    { assigned: " << name << std::endl;
        ss << "      target: " << indentExceptFirstLine(it->second.toString(slotTracker), 14) << " }" << std::endl;
    }
    ss << "]";
    return ss.str();
}

void
InclusionBased::addConstantTarget(const llvm::Value *instruction, size_t constant)
{
    CANAL_NOT_IMPLEMENTED();
}

void
InclusionBased::addMemoryTarget(const llvm::Value *instruction, const llvm::Value *target, Value *arrayOffset /*= NULL*/)
{
    Target newTarget;
    newTarget.mType = Target::MemoryBlock;
    newTarget.mInstruction = target;
    newTarget.mArrayOffset = arrayOffset;

    PlaceTargetMap::iterator it = mTargets.find(instruction);
    if (it != mTargets.end())
        it->second.merge(newTarget);
    else
        mTargets.insert(PlaceTargetMap::value_type(instruction, newTarget));
}

} // namespace Pointer
} // namespace Canal
