#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unordered_map>

#define HANDLE_ERROR(syscall) do { \
    perror("smash error: " #syscall " failed"); \
} while (0)
#define FAIL (-1)

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
        int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char*) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line, int argc, char** argv, bool isBg) {

    //1. dealis first word
    //2. check for alias
    //3. check for watch
    //4. pipe
    //5. redirection
    //6. built in
    //7. external
    string firstWord = argv[0];
    //TODO remove & and check it later so it doesnt screw up built in commands
    //also decode aliases
    if (firstWord == "pwd") {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord == "showpid") {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord == "cd") {
        return new ChangeDirCommand(cmd_line, this->last_path);
    }
    else if (firstWord == "chprompt") {
        return new changePrompt(cmd_line, this->prompt_line);
    }
    else if (firstWord == "jobs") {
        return new JobsCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line, argc, argv, isBg);
    }
    // return nullptr;
}

void SmallShell::executeCommand(const char* cmd_line) {
    // TODO: Add your implementation here
    // for example:
    /** parse command.
     *
     *
     */
    char cmdCopy[COMMAND_MAX_LENGTH];
    memset(cmdCopy, 0, COMMAND_MAX_LENGTH);
    strcpy(cmdCopy, cmd_line);

    bool isBg = _isBackgroundComamnd(cmd_line);
    if (isBg) {
        _removeBackgroundSign(cmdCopy);
    }
    //TODO:dealis!!!!!!
    char* argv[COMMAND_MAX_ARGS];
    memset(argv, 0, COMMAND_MAX_ARGS * sizeof(argv[0]));
    int argc = _parseCommandLine(cmdCopy, argv);
    if (argc == 0) {
        return;
    }
    Command* cmd = CreateCommand(cmd_line, argc, argv, isBg);
    if (cmd) {
        cmd->execute();
    }
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}


SmallShell::SmallShell() {
    memset(last_path, 0, COMMAND_MAX_LENGTH);
    memset(prompt_line, 0, COMMAND_MAX_LENGTH);
    ::strcpy(prompt_line, DEFAULT_PROMPT_LINE);
}


void updatePrompt(const char* new_prompt, char* promptLine) {
    memset(promptLine, 0, COMMAND_MAX_LENGTH);
    strcpy(promptLine, new_prompt);
}

void getCWD(char* buff) {
    if (!getcwd(buff, COMMAND_MAX_LENGTH)) {
        perror("smash error: getcwd() failed");
    }
}

void GetCurrDirCommand::execute() {
    char buff[COMMAND_MAX_LENGTH];
    getCWD(buff);
    std::cout << buff << endl; //stays smash even with chprompt @54
}
void changePrompt::execute() {

    char* arguments[COMMAND_MAX_LENGTH];
    int argc = _parseCommandLine(this->commandString, arguments);
    if (argc == 1) { //reset
        memset(pPromptLine, 0, COMMAND_MAX_LENGTH);
        strcpy(pPromptLine, DEFAULT_PROMPT_LINE);
    }
    else { //we can ignore other parameters @pdf
        memset(pPromptLine, 0, COMMAND_MAX_LENGTH);
        strcpy(pPromptLine, arguments[1]);
        ::strcat(pPromptLine, "> ");
    }
}

void ShowPidCommand::execute() {
    std::cout << "smash pid is " << getppid() << endl; //stays smash even with chprompt @54
}

void updateLastPWD(char* last_pwd, char* current_pwd) {
    memset(last_pwd, 0, COMMAND_MAX_LENGTH);
    strcpy(last_pwd, current_pwd);
}


void ChangeDirCommand::execute() {
    char buff_cwd[COMMAND_MAX_LENGTH];
    getCWD(buff_cwd);
    char* arguments[COMMAND_MAX_LENGTH];
    int argc = _parseCommandLine(this->commandString, arguments);

    if (argc > 2) {
        ::perror("smash error: cd: too many arguments\n");
        return;
    }
    if (argc == 1) {
        updateLastPWD(plast_cwd, buff_cwd);
        return;
    }
    string first_arg = arguments[1];

    if (first_arg.compare("-") == 0) {
        if (plast_cwd[0] == '\0') {
            ::perror("smash error: cd: OLDPWD not set");
            return;
        }
        else {
            if (chdir(plast_cwd) != 0) {
                perror("smash error: chdir() failed");
                return;
            }
            else {
                //TODO handle - logic after they answer in piazza @179
                return;
            }
        }
    }
    if (chdir(first_arg.c_str()) != 0) {
        perror("smash error: chdir() failed");
        return;
    }
    else {
        updateLastPWD(plast_cwd, buff_cwd);
    }
}

/* LISTDIR FUNCTIONS*/
#define BUFF_SIZE (2048)

struct linux_dirent {
    unsigned long  d_ino;    // Inode number
    unsigned long  d_off;    // Offset to next linux_dirent from the beginning of the dir
    unsigned short d_reclen; // Length of this linux_dirent
    char           d_name[]; // Filename (null-terminated)
};


void ListDirCommand::execute() {

}
//TODO: move these to static const class members
#define BASH_PATH "/bin/bash"
#define BASH_FLAG "-c"
void ExternalCommand::execute() {
    //assume cmd is parsed
    //check for '*', '?'
    //check for '&' flag (should be a field in the externalcommand class)
    //fork, execv, and wait according to logic
    //documnent running process
    bool wildcard = false;
    char** newargv = argv;
    if (strstr(commandString, "*") || strstr(commandString, "?")) {
        //means it has a wildcard and we need bash
        newargv = new char* [4];
        string command;
        for (int i = 0; i < argc; i++) {
            command += argv[i];
            if (i != argc - 1) {
                command += " ";
            }
        }
        newargv[0] = BASH_PATH;
        newargv[1] = BASH_FLAG;
        newargv[2] = (char*) command.c_str();   //not best practice but i know what im doing
        newargv[3] = nullptr;
        wildcard = true;
    }
    pid_t processPid = fork();

    if (processPid < 0) {
        HANDLE_ERROR(fork);
    }
    else if (processPid == 0) { //we are in son
        if (setpgrp() == FAIL) {
            HANDLE_ERROR(setpgrp);
            return;
        }
        if (execvp(newargv[0], newargv) == FAIL) {
            HANDLE_ERROR(execv);
            return;
        }
    }
    else {   //parent
        SmallShell& shell = SmallShell::getInstance();
        if (isBg) {
            //enter to job list and return
            shell.jobsList.removeFinishedJobs();    //is this needed?
            shell.jobsList.addJob(this, processPid);
        }
        else {
            //document as currenly running and wait for it to finish
            shell.currentProcess = processPid;
            if (waitpid(processPid, nullptr, 0) == FAIL) {
                HANDLE_ERROR(waitpid);
            }
        }
        if (wildcard) {
            delete[] newargv;
        }
        shell.currentProcess = -1;
    }
}


/** JOBS FUNCS **/

void JobsCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    shell.jobsList.removeFinishedJobs();
    shell.jobsList.printJobsList();
}

// JOBS LIST
bool JobsList::JobEntry::isFinished() {
    int status;
    int ret = waitpid(pid, &status, WNOHANG);
    if (ret == FAIL) {
        HANDLE_ERROR(waitpid);
        return false;
    }
    else if (ret == 0) {
        return false;
    }
    return true;
}

void JobsList::addJob(Command* cmd, pid_t pid) {
    int maxJobID = entries.rbegin()->first;
    JobEntry* newEntry = new JobEntry(maxJobID + 1, pid, cmd->commandString);
    entries[maxJobID + 1] = newEntry;
}
void JobsList::printJobsList() {
    for (auto it = entries.begin(); it != entries.end(); it++) {
        cout << "[" << it->first << "] " << it->second->cmd_str << endl;
    }
}

void JobsList::killAllJobs() {
    for (auto it = entries.begin(); it != entries.end(); it++) {
        if (kill(it->second->pid, SIGKILL) == FAIL) {
            HANDLE_ERROR(kill);
        }
    }
}

void JobsList::removeFinishedJobs() {
    for (auto it = entries.begin(); it != entries.end(); it++) {
        if (it->second->isFinished()) {
            delete it->second;
            entries.erase(it);
        }
    }
}
//**returns nullptr if theres no job with jobId */
JobsList::JobEntry* JobsList::getJobById(int jobId) {
    auto it = entries.find(jobId);
    if (it != entries.end()) {
        return it->second;
    }
    else {
        return nullptr;
    }
}

void JobsList::removeJobById(int jobId) {
    if (entries.find(jobId) == entries.end()) {
        return;
    }
    delete entries[jobId];
    entries.erase(jobId);
}