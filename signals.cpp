#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C"<<endl;
    SmallShell &smash=SmallShell::getInstance();
    if (smash.currentProcess!=NO_PROCESS_RUNNING) {
        if(kill(smash.currentProcess,SIGINT)==-1) {
            perror("smash error: kill failed");
        }
        cout << "smash: process " << smash.currentProcess << " was killed." <<endl;
        smash.currentProcess=NO_PROCESS_RUNNING;
    }
}
