#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char *argv[]) {
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell &smash = SmallShell::getInstance();
    while (true) {
        std::cout << smash.prompt_line;
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        char cmd[COMMAND_MAX_LENGTH+1];
        smash.executeCommand(strcpy(cmd, cmd_line.c_str()));

    }
    return 0;
}