#include "CommandPrint.h"
#include "Commands.h"
#include "State.h"
#include "SlotTracker.h"
#include "Utils.h"
#include "../lib/Stack.h"
#include "../lib/Value.h"
#include "../lib/Utils.h"
#include <llvm/ValueSymbolTable.h>
#include <llvm/Module.h>
#include <cstdio>
#include <sstream>

CommandPrint::CommandPrint(Commands &commands)
    : Command("print",
              "Print value of a variable",
              "",
              commands)
{
}

std::vector<std::string>
CommandPrint::getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const
{
    std::vector<std::string> result;
    if (!mCommands.mState || !mCommands.mState->isInterpreting())
        return result;

    std::string arg(args[pointArg].substr(0, pointArgOffset));
    const llvm::Function &function = mCommands.mState->mStack->getCurrentFunction();
    mCommands.mState->mSlotTracker->setActiveFunction(function);

    // Check function arguments.
    llvm::Function::ArgumentListType::const_iterator it = function.getArgumentList().begin();
    for (; it != function.getArgumentList().end(); ++it)
        considerCompletionMatch(*it, result, arg);

    // Check every instruction in the current function.
    llvm::Function::const_iterator fit = function.begin(), fitend = function.end();
    for (; fit != fitend; ++fit)
    {
        llvm::BasicBlock::const_iterator bit = fit->begin(), bitend = fit->end();
        for (; bit != bitend; ++bit)
            considerCompletionMatch(*bit, result, arg);
    }

    // Check named and unnamed module-level variables.
    const llvm::Module &module = *mCommands.mState->mModule;
    for (llvm::Module::const_global_iterator it = module.global_begin(); it != module.global_end(); ++it)
    {
        considerCompletionMatch(*it, result, arg);
    }

    return result;
}

void
CommandPrint::run(const std::vector<std::string> &args)
{
    if (!mCommands.mState || !mCommands.mState->isInterpreting())
    {
        puts("No module is being interpreted.");
        return;
    }

    if (args.size() <= 1)
    {
        puts("Provide variable or constant name as \"print\" argument.");
        return;
    }

    Canal::State &state = mCommands.mState->mStack->getCurrentState();
    const llvm::Function &function = mCommands.mState->mStack->getCurrentFunction();
    mCommands.mState->mSlotTracker->setActiveFunction(function);

    for (std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); ++it)
    {
        if (it->size() == 0)
            continue;
        if (((*it)[0] != '%' && (*it)[0] != '@') || it->size() == 1)
        {
            printf("Parameter \"%s\": invalid format.\n", it->c_str());
            continue;
        }

        const llvm::Value *position;
        bool isNumber;
        std::string name(it->substr(1));
        unsigned pos = stringToUnsigned(name.c_str(), isNumber);
        if (isNumber)
        {
            if ((*it)[0] == '%')
                position = mCommands.mState->mSlotTracker->getLocalSlot(pos);
            else
                position = mCommands.mState->mSlotTracker->getGlobalSlot(pos);
        }
        else
        {
            if ((*it)[0] == '%')
                position = function.getValueSymbolTable().lookup(name);
            else
                position = mCommands.mState->mModule->getValueSymbolTable().lookup(name);
        }

        if (!position)
        {
            printf("Parameter \"%s\": does not exist in current scope.\n", it->c_str());
            continue;
        }

        Canal::Value *value = state.findVariable(*position);
        if (!value)
        {
            printf("%s = uninitialized\n", it->c_str());
            continue;
        }

        printf("%s = %s\n", it->c_str(), Canal::indentExceptFirstLine(value->toString(), it->size() + 3).c_str());
    }
}

void
CommandPrint::considerCompletionMatch(const llvm::Value &value, std::vector<std::string> &result, const std::string &prefix) const
{
    bool isGlobal = llvm::isa<llvm::GlobalValue>(value);
    std::stringstream ss;
    ss << (isGlobal ? "@" : "%");
    if (value.hasName())
        ss << value.getName().data();
    else
    {
        int id;
        if (isGlobal)
            id = mCommands.mState->mSlotTracker->getGlobalSlot(value);
        else
            id = mCommands.mState->mSlotTracker->getLocalSlot(value);
        if (id >= 0)
            ss << id;
        else
            return;
    }
    std::string name = ss.str();
    if (0 == strncmp(name.c_str(), prefix.c_str(), prefix.size()))
        result.push_back(name);
}