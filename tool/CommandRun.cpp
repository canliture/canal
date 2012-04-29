#include "CommandRun.h"
#include "Commands.h"
#include "State.h"
#include <cstdio>

CommandRun::CommandRun(Commands &commands)
    : Command("run",
              "",
              "Start program interpretation",
              "Start program interpretation.",
              commands)
{
}

void
CommandRun::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState())
    {
        puts("No program specified.  Use the \"file\" command.");
        return;
    }

    mCommands.getState()->addMainFrame();
    mCommands.getState()->run();
}
