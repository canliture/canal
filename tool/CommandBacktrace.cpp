#include "CommandBacktrace.h"
#include "Commands.h"
#include "State.h"
#include "../lib/Stack.h"
#include <cstdio>

CommandBacktrace::CommandBacktrace(Commands &commands)
    : Command("backtrace",
              "Print backtrace of all stack frames",
              "",
              commands)
{
}

void
CommandBacktrace::run(const std::vector<std::string> &args)
{
    if (!mCommands.mState || !mCommands.mState->isInterpreting())
    {
        puts("No stack.");
        return;
    }

    const std::vector<Canal::StackFrame> &frames = mCommands.mState->mStack->getFrames();
    for (std::vector<Canal::StackFrame>::const_iterator it = frames.begin(); it != frames.end(); ++it)
    {
        printf("#%d\t%s\n", it - frames.begin(), it->mFunction->getName().data());
    }
}