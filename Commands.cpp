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

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
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
Command *SmallShell::CreateCommand(const char *cmd_line) {

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
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
      return new  JobsCommand(cmd_line, this->jobsList);
  }
  //else {
    //return new ExternalCommand(cmd_line);
  //}
    return nullptr;
}

void SmallShell::executeCommand(char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    /** parse command.
     * check for '&'.
     *
     */
    bool isBg = _isBackgroundComamnd(cmd_line);
    if(isBg){
        _removeBackgroundSign(cmd_line);
    }
    char *argv[COMMAND_MAX_ARGS];
    int argc = _parseCommandLine(cmd_line, argv);
    Command* cmd = CreateCommand(cmd_line);
    if (cmd){
        cmd->execute();
    }
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}


SmallShell::SmallShell() {
    memset(last_path, 0, COMMAND_MAX_LENGTH);
    memset(prompt_line, 0, COMMAND_MAX_LENGTH);
    ::strcpy(prompt_line,DEFAULT_PROMPT_LINE);
}


void updatePrompt(const char *new_prompt, char* promptLine) {
    memset(promptLine, 0, COMMAND_MAX_LENGTH);
    strcpy(promptLine, new_prompt);
}

void getCWD(char* buff) {
    if (!getcwd(buff, COMMAND_MAX_LENGTH)) {
        ::perror("smash error: getcwd() failed");
    }
}

void GetCurrDirCommand::execute() {
    char buff[COMMAND_MAX_LENGTH];
    getCWD(buff);
    std::cout << buff << endl; //stays smash even with chprompt @54
}
void changePrompt::execute() {

    char *arguments[COMMAND_MAX_LENGTH];
    int argc= _parseCommandLine(this->commandString,arguments);
    if (argc==1){ //reset
        memset(pPromptLine, 0, COMMAND_MAX_LENGTH);
        strcpy(pPromptLine,DEFAULT_PROMPT_LINE);
    }
    else { //we can ignore other parameters @pdf
        memset(pPromptLine, 0, COMMAND_MAX_LENGTH);
        strcpy(pPromptLine,arguments[1]);
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
    char *arguments[COMMAND_MAX_LENGTH];
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
        } else {
            if (chdir(plast_cwd) != 0) {
                perror("smash error: chdir() failed");
                return;
            } else {
                //TODO handle - logic after they answer in piazza @179
                return;
            }
        }
    }
    if (chdir(first_arg.c_str()) != 0) {
        perror("smash error: chdir() failed");
        return;
    } else {
        updateLastPWD(plast_cwd, buff_cwd);
    }
}

/* LISTDIR FUNCTIONS*/
#define DIR_BUF_MAX_LENGTH (2000) //to do check piazza for actual size|| i dont get what is this for
#define BUFF_SIZE 1024

struct linux_dirent {
    unsigned long  d_ino;    // Inode number
    unsigned long  d_off;    // Offset to next linux_dirent
    unsigned short d_reclen; // Length of this linux_dirent
    char           d_name[]; // Filename (null-terminated)
};


void ListDirCommand::execute(){

} 




/** JOBS FUNCS **/
void JobsCommand::execute() {
   // j_list->printJobsList();
}
