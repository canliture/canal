#include "Commands.h"
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <argp.h>
#include <sysexits.h>
}

// List of all commands.
Commands gCommands;

static struct argp_option gOptions[] = {
    {"eval", 'e', "COMMAND", 0, "Execute a single Canal command. May be used multiple times."},
    { 0 }
};

class Arguments
{
public:
    std::vector<std::string> mEvalCommands;
    std::string mFileName;
};

static error_t
parseArgument(int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    struct Arguments *arguments = (struct Arguments*)state->input;

    switch (key)
    {
    case 'e':
        arguments->mEvalCommands.push_back(arg);
        break;
    case ARGP_KEY_ARG:
        if (!arguments->mFileName.empty())
        {
            argp_usage(state);
            exit(EX_USAGE);
        }
        arguments->mFileName = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

// Our argp parser.
static struct argp gArgumentParser = {
    gOptions,
    parseArgument,
    "[FILE]",
    "Canal -- Abstract Interpreter"
};

static char *
completeEntry(const char *text, int state)
{
    static std::vector<std::string> matches;
    if (state == 0)
    {
        std::string buf(rl_line_buffer);
        if (rl_point >= buf.size())
            buf += std::string(text);
        matches = gCommands.getCompletionMatches(buf, rl_point);
    }

    if (state < matches.size())
        return strdup(matches[state].c_str());
    else
        return NULL;
}

int
main(int argc, char **argv)
{
    // Initialize the readline library.
    // Allow conditional parsing of the ~/.inputrc file.
    rl_readline_name = (char*)"Canal";
    // Tell the completer that we want a crack first.
    rl_completion_entry_function = completeEntry;
    rl_terminal_name = getenv("TERM");
    rl_completer_word_break_characters = (char*)" \t\n";
    // Stifle the history list, remembering only the last 256 entries.
    stifle_history(256);

    // Parse command arguments.
    Arguments arguments;
    argp_parse(&gArgumentParser, argc, argv, 0, 0, &arguments);

    // Loop reading and executing lines until the user quits.
    while (true)
    {
        char *line = readline("(canal) ");
        if (!line)
            break;

        char *expansion;
        int result = history_expand(line, &expansion);
        if (result < 0 || result == 2)
            fprintf(stderr, "%s\n", expansion);
        else
        {
            add_history(expansion);
            gCommands.executeLine(expansion);
        }

        free(expansion);
        free(line);
    }

    return 0;
}
