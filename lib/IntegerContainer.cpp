#include "IntegerContainer.h"
#include "IntegerBits.h"
#include "IntegerEnumeration.h"
#include "IntegerRange.h"
#include "Constant.h"
#include "Utils.h"
#include "StringUtils.h"
#include "APIntUtils.h"
#include "Pointer.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Container::Container(const Environment &environment,
                     unsigned bitWidth)
    : Domain(environment)
{
    mValues.push_back(new Bits(environment, bitWidth));
    mValues.push_back(new Enumeration(environment, bitWidth));
    mValues.push_back(new Range(environment, bitWidth));
}

Container::Container(const Environment &environment,
                     const llvm::APInt &number)
    : Domain(environment)
{
    mValues.push_back(new Bits(environment, number));
    mValues.push_back(new Enumeration(environment, number));
    mValues.push_back(new Range(environment, number));
}

Container::Container(const Container &container)
    : Domain(container.mEnvironment)
{
    mValues = container.mValues;
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = (*it)->clone();
}

Container::~Container()
{
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        delete *it;
}

unsigned
Container::getBitWidth() const
{
    return getEnumeration().getBitWidth();
}

Bits &
Container::getBits()
{
    return dynCast<Bits&>(*mValues[0]);
}

const Bits &
Container::getBits() const
{
    return dynCast<const Bits&>(*mValues[0]);
}

Enumeration &
Container::getEnumeration()
{
    return dynCast<Enumeration&>(*mValues[1]);
}

const Enumeration &
Container::getEnumeration() const
{
    return dynCast<const Enumeration&>(*mValues[1]);
}

Range &
Container::getRange()
{
    return dynCast<Range&>(*mValues[2]);
}

const Range &
Container::getRange() const
{
    return dynCast<const Range&>(*mValues[2]);
}

bool
Container::signedMin(llvm::APInt &result) const
{
    if (!getEnumeration().signedMin(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getRange().signedMin(temp))
        return false;

    if (result.sgt(temp))
        result = temp;

    if (!getBits().signedMin(temp))
        return false;

    if (result.sgt(temp))
        result = temp;

    return true;
}

bool
Container::signedMax(llvm::APInt &result) const
{
    if (!getEnumeration().signedMax(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getRange().signedMax(temp))
        return false;

    if (result.slt(temp))
        result = temp;

    if (!getBits().signedMax(temp))
        return false;

    if (result.slt(temp))
        result = temp;

    return true;
}

bool
Container::unsignedMin(llvm::APInt &result) const
{
    if (!getEnumeration().unsignedMin(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getRange().unsignedMin(temp))
        return false;

    if (result.ugt(temp))
        result = temp;

    if (!getBits().unsignedMin(temp))
        return false;

    if (result.ugt(temp))
        result = temp;

    return true;
}

bool
Container::unsignedMax(llvm::APInt &result) const
{
    if (!getEnumeration().unsignedMax(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getRange().unsignedMax(temp))
        return false;

    if (result.ult(temp))
        result = temp;

    if (!getBits().unsignedMax(temp))
        return false;

    if (result.ult(temp))
        result = temp;

    return true;
}

Container *
Container::clone() const
{
    return new Container(*this);
}

Container *
Container::cloneCleaned() const
{
    return new Container(mEnvironment, getBitWidth());
}

bool
Container::operator==(const Domain &value) const
{
    const Container *container = dynCast<const Container*>(&value);
    if (!container)
        return false;

    CANAL_ASSERT(mValues.size() == container->mValues.size());
    std::vector<Domain*>::const_iterator ita(mValues.begin()),
        itb(container->mValues.begin());
    for (; ita != mValues.end(); ++ita, ++itb)
    {
        if (**ita != **itb)
            return false;
    }

    return true;
}

void
Container::merge(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    if (const Constant *constant = dynCast<const Constant*>(&value))
    {
        for (; it != mValues.end(); ++it)
            (*it)->merge(*constant);
        return;
    }

    const Container &container = dynCast<const Container&>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->merge(**it2);
}

size_t
Container::memoryUsage() const
{
    size_t size(0);
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();
    return size;
}

std::string
Container::toString() const
{
    std::stringstream ss;
    ss << "integerContainer" << std::endl;
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << indent((*it)->toString(), 4);
    return ss.str();
}

bool
Container::matchesString(const std::string &text,
                         std::string &rationale) const
{
    const char *input = text.c_str();
    int count = StringUtils::skipString(&input, "integer");
    if (0 == count)
    {
        rationale = "No 'integer' keyword.";
        return false;
    }

    CANAL_NOT_IMPLEMENTED();
}

// Converts value to container.  If the value is a constant, it is
// converted to container and deleteAfter is set to true.
static const Container *
asContainer(const Domain &value, bool &deleteAfter)
{
    deleteAfter = false;
    const Container *container = dynCast<const Container*>(&value);
    if (!container)
    {
        const Constant *constant = dynCast<const Constant*>(&value);
        CANAL_ASSERT_MSG(constant,
                         "Unsupported type cannot be converted "
                         "to integer container. (" << typeid(value).name() << ")");

        container = new Container(value.mEnvironment, constant->getAPInt());
        deleteAfter = true;
    }

    return container;
}

static void
binaryOperation(Container &result,
                const Domain &a,
                const Domain &b,
                Domain::BinaryOperation operation)
{
    bool deleteAA, deleteBB;
    const Container *aa = asContainer(a, deleteAA),
        *bb = asContainer(b, deleteBB);

    std::vector<Domain*>::iterator it(result.mValues.begin());
    std::vector<Domain*>::const_iterator ita = aa->mValues.begin(),
        itb = bb->mValues.begin();

    for (; it != result.mValues.end(); ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb);

    if (deleteAA)
        delete aa;
    if (deleteBB)
        delete bb;
}

void
Container::add(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::add);
}

void
Container::sub(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::sub);
}

void
Container::mul(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::mul);
}

void
Container::udiv(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::udiv);
}

void
Container::sdiv(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::sdiv);
}

void
Container::urem(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::urem);
}

void
Container::srem(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::srem);
}

void
Container::shl(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::shl);
}

void
Container::lshr(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::lshr);
}

void
Container::ashr(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::ashr);
}

void
Container::and_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::and_);
}

void
Container::or_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::or_);
}

void
Container::xor_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::xor_);
}

static void
cmpOperation(Container &result,
             const Domain &a,
             const Domain &b,
             llvm::CmpInst::Predicate predicate,
             Domain::CmpOperation operation)
{
    const Canal::Pointer::InclusionBased* aPointer =
            dynCast<const Canal::Pointer::InclusionBased*>(&a),
            *bPointer = dynCast<const Canal::Pointer::InclusionBased*>(&b);

    bool deletePtrA = false, deletePtrB = false;
    const Constant* aConstant = dynCast<const Constant*>(&a),
            *bConstant = dynCast<const Constant*>(&b);
    if ( (aPointer || aConstant && aConstant->isNullPtr()) && (bPointer || bConstant && bConstant->isNullPtr()) ) {
        CANAL_ASSERT(operation == &Domain::icmp);
        if (aConstant)
        {
            aPointer = dynCast<const Canal::Pointer::InclusionBased*>(aConstant->toModifiableValue());
            deletePtrA = true;
        }
        if (bConstant)
        {
            bPointer = dynCast<const Canal::Pointer::InclusionBased*>(bConstant->toModifiableValue());
            deletePtrB = true;
        }

        bool cmpSingle = aPointer->isSingleTarget() && bPointer->isSingleTarget(),
            cmpeq = (*aPointer == *bPointer);

        result.setBottom();
        switch (predicate)
        {
        case llvm::CmpInst::ICMP_EQ:
        case llvm::CmpInst::ICMP_UGE:
        case llvm::CmpInst::ICMP_ULE:
        case llvm::CmpInst::ICMP_SGE:
        case llvm::CmpInst::ICMP_SLE:
            if (cmpeq && cmpSingle)
                result.merge(Container(result.mEnvironment, llvm::APInt(1, 1, false)));
            else
            {
                if (predicate == llvm::CmpInst::ICMP_EQ && cmpSingle)
                    result.merge(Container(result.mEnvironment, llvm::APInt(1, 0, false)));
                else result.setTop();
            }
            break;
        case llvm::CmpInst::ICMP_NE:
            if (cmpSingle)
                result.merge(Container(result.mEnvironment, llvm::APInt(1, (cmpeq ? 0 : 1), false)));
            else
                result.setTop();
            break;
        default:
            result.setTop();
        }
        if (deletePtrA)
            delete aPointer;
        if (deletePtrB)
            delete bPointer;
        return;
    }

    bool deleteAA, deleteBB;
    const Container *aa = asContainer(a, deleteAA),
        *bb = asContainer(b, deleteBB);

    std::vector<Domain*>::iterator it(result.mValues.begin());
    std::vector<Domain*>::const_iterator ita = aa->mValues.begin(),
        itb = bb->mValues.begin();

    for (; it != result.mValues.end(); ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb, predicate);

    if (deleteAA)
        delete aa;
    if (deleteBB)
        delete bb;
}


void
Container::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    cmpOperation(*this, a, b, predicate, &Domain::icmp);
}

void
Container::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    cmpOperation(*this, a, b, predicate, &Domain::fcmp);
}

float
Container::accuracy() const
{
    float accuracy = 0;
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyDomain *accuracyValue =
            dynCast<const AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        float localAccuracy = accuracyValue->accuracy();
        if (localAccuracy > accuracy)
            accuracy = localAccuracy;
    }
    return accuracy;
}

bool
Container::isBottom() const
{
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyDomain *accuracyValue =
            dynCast<const AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        if (!accuracyValue->isBottom())
            return false;
    }
    return true;
}

void
Container::setBottom()
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        AccuracyDomain *accuracyValue =
            dynCast<AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        accuracyValue->setBottom();
    }
}

bool
Container::isTop() const
{
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyDomain *accuracyValue =
            dynCast<const AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        if (!accuracyValue->isTop())
            return false;
    }
    return true;
}

void
Container::setTop()
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        AccuracyDomain *accuracyValue = dynCast<AccuracyDomain*>(*it);
        if (!accuracyValue)
            continue;

        accuracyValue->setTop();
    }
}

bool
Container::isSingleValue() const
{
    return getBits().isSingleValue()
        && getEnumeration().isSingleValue()
        && getRange().isSingleValue();
}

} // namespace Integer
} // namespace Canal
