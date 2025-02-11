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
#include <cassert>
#include <regex>

#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/syscall.h>

#define HANDLE_ERROR(syscall) do { \
    perror("smash error: "#syscall" failed"); \
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

int stringToInt(const string& str) {
    try {
        size_t pos;
        int result = stoi(str, &pos);

        // Check if the entire string was converted
        if (pos != str.size()) {
            return -1;
        }

        return result;
    }
    catch (const invalid_argument& e) {
        return -1;
    }
    catch (const out_of_range& e) {
        return -1;
    }
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

//moved it here to avoid circular dependency
PipeCommand::PipeCommand(const char* cmd_line, char* cmdCopy, int argc, char** argv) :Command(cmd_line, argc, argv) {
    char* delimiter = strchr(cmdCopy, '|');
    pipeToErr = (*(delimiter + 1) == '&');
    *delimiter = '\0';
    SmallShell& shell = SmallShell::getInstance();
    in = shell.CreateCommand(cmdCopy, cmdCopy, _parseCommandLine(cmdCopy, inargv), inargv, false);  //we trust ourselves not to modify cmdCopy, unless its a pipe/redirect
    char* outCmd = pipeToErr ? delimiter + 2 : delimiter + 1;
    out = shell.CreateCommand(outCmd, outCmd, _parseCommandLine(outCmd, outargv), outargv, false);
};

RedirectionCommand::RedirectionCommand(const char* cmd_line, char* cmdCopy, int argc, char** argv) :Command(cmd_line, argc, argv) {
    char* delimiter = strchr(cmdCopy, '>');
    overwrite = (*(delimiter + 1) == '>');
    *delimiter = '\0';
    filePath = overwrite ? string(delimiter + 2) : string(delimiter + 1);
    filePath = _trim(filePath);    //feels right (and examples in pdf agrees)
    SmallShell& shell = SmallShell::getInstance();
    in = shell.CreateCommand(cmdCopy, cmdCopy, _parseCommandLine(cmdCopy, inargv), inargv, false);
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line, char* cmdCopy, int argc, char** argv, bool isBg) {

    //1. dealis first word
    //2. check for alias
    //3. check for watch
    //4. pipe
    //5. redirection
    //6. built in
    //7. external
    string firstWord = argv[0];
    if (strchr(cmdCopy, '|') != nullptr) {
        //if cmdCopy contains '|' then we have a pipe   
        return new PipeCommand(cmd_line, cmdCopy, argc, argv);
    }
    else if (strchr(cmdCopy, '>') != nullptr) {
        //if cmdCopy contains '>' then we have a redirection
        return new RedirectionCommand(cmd_line, cmdCopy, argc, argv);
    }
    else if (firstWord == "pwd") {
        return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord == "showpid") {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord == "cd") {
        return new ChangeDirCommand(cmd_line, argc, argv, this->last_path);
    }
    else if (firstWord == "chprompt") {
        return new changePrompt(cmd_line, argc, argv);
    }
    else if (firstWord == "jobs") {
        return new JobsCommand(cmd_line);
    }
    else if (firstWord == "quit") {
        return new QuitCommand(cmd_line, argc, argv);
    }
    else if (firstWord == "alias") {
        return new aliasCommand(cmd_line, cmdCopy, argc, argv);
    }
    else if (firstWord == "unalias") {
        return new unaliasCommand(cmd_line, argc, argv);
    }
    else if (firstWord == "kill") {
        return new KillCommand(cmd_line, argc, argv);
    }
    else if (firstWord == "fg") {
        return new ForegroundCommand(cmd_line, argc, argv);
    }
    else if (firstWord == "listdir") {
        return new ListDirCommand(cmd_line, argc, argv);
    }
    else if (firstWord == "getuser") {
        return new GetUserCommand(cmd_line, argc, argv);
    }
    else {
        return new ExternalCommand(cmd_line, argc, argv, isBg);
    }
    return nullptr; //TODO: handle this case
}

void SmallShell::executeCommand(const char* cmd_line) {


    char cmdCopy[COMMAND_MAX_LENGTH];
    memset(cmdCopy, 0, COMMAND_MAX_LENGTH);
    strcpy(cmdCopy, cmd_line);
    aliases.deAlias(cmdCopy);

    bool isBg = _isBackgroundComamnd(cmd_line);
    if (isBg) {
        _removeBackgroundSign(cmdCopy);
    }
    char* argv[COMMAND_MAX_ARGS];
    memset(argv, 0, COMMAND_MAX_ARGS * sizeof(argv[0]));
    int argc = _parseCommandLine(cmdCopy, argv);
    if (argc == 0) {
        return;
    }
    Command* cmd = CreateCommand(cmd_line, cmdCopy, argc, argv, isBg);
    if (cmd) {
        cmd->execute();
    }
    delete cmd;
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
        perror("smash error: getcwd failed");
    }
}

void GetCurrDirCommand::execute() {
    char buff[COMMAND_MAX_LENGTH];
    getCWD(buff);
    std::cout << buff << endl; //stays smash even with chprompt @54
}
void changePrompt::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if (argc == 1) { //reset
        memset(smash.prompt_line, 0, COMMAND_MAX_LENGTH);
        strcpy(smash.prompt_line, DEFAULT_PROMPT_LINE);
    }
    else { //we can ignore other parameters @pdf
        memset(smash.prompt_line, 0, COMMAND_MAX_LENGTH);
        strcpy(smash.prompt_line, argv[1]);
        ::strcat(smash.prompt_line, "> ");
    }
}

void ShowPidCommand::execute() {
    std::cout << "smash pid is " << getpid() << endl; //stays smash even with chprompt @54, getpid cannot fail
}


void KillCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    if (argc < 3) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    if (shell.jobsList.getJobById(stringToInt(argv[2])) == nullptr) {
        cerr << "smash error: kill: job-id " << argv[2] << " does not exist" << endl;
        return;
    }
    if (argv[1][0] != '-') {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    int signal = stringToInt(argv[1] + 1); //skip the '-'
    //send signal to process
    if (kill(shell.jobsList.getJobById(stringToInt(argv[2]))->pid, signal) == FAIL) {
        HANDLE_ERROR(kill);
        return;
    }
    //print according to pdf
    cout << "signal number " << signal << " was sent to pid " << shell.jobsList.getJobById(stringToInt(argv[2]))->pid << endl;
}

void QuitCommand::execute() {
    if (isKill) {
        SmallShell& shell = SmallShell::getInstance();
        shell.jobsList.removeFinishedJobs();
        shell.jobsList.killAllJobs();
        exit(0);
    }
    //TODO: handle kill
    exit(0);
}
void ForegroundCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    if (argc == 1) {//no job id
        if (shell.jobsList.entries.empty()) {
            cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        shell.jobsList.bringJobToForeground(-1);
    }
    else {
        int jobId = stringToInt(argv[1]);
           if (jobId == -1 || argc > 2) {
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
        if (shell.jobsList.getJobById(jobId) == nullptr) {
            cerr << "smash error: fg: job-id " << jobId << " does not exist" << endl;
            return;
        }

        shell.jobsList.bringJobToForeground(jobId);
    }
}







void updateLastPWD(char* newpwd) {
    SmallShell& smash = SmallShell::getInstance();
    memset(smash.last_path, 0, COMMAND_MAX_LENGTH);
    strcpy(smash.last_path, newpwd);
}

void ChangeDirCommand::execute() {
    char buff_cwd[COMMAND_MAX_LENGTH];
    getcwd(buff_cwd, COMMAND_MAX_LENGTH);
    if (!buff_cwd) {
        HANDLE_ERROR(getcwd);
        return;
    }
    if (argc > 2) {
        cerr << ("smash error: cd: too many arguments") << endl;
        return;
    }
    if (argc == 1) {
        updateLastPWD(buff_cwd);
        return;
    }
    string first_arg = argv[1];

    if (first_arg == "-") {
        if (plast_cwd[0] == '\0') {
            cerr << ("smash error: cd: OLDPWD not set") << endl;
            return;
        }
        if (chdir(plast_cwd) != 0) {
            HANDLE_ERROR(chdir);
            return;
        }
        updateLastPWD(buff_cwd);
        return;
    }
    //from here chdir should eat it
    if (chdir(first_arg.c_str()) != 0) {
        HANDLE_ERROR(chdir);
        return;
    }
    updateLastPWD(buff_cwd);
}
/** execute pipe commands */
void PipeCommand::execute() {
    //two ready to go commands
    //create pipe
    int pipefd[2];
    if (pipe(pipefd) == FAIL) {
        HANDLE_ERROR(pipe);
        return;
    }
    pid_t pid1 = fork();
    if (pid1 == FAIL) {
        HANDLE_ERROR(fork);
        return;
    }
    if (pid1 == 0) {
        // "in" side of the pipe
        //close the read side
        if (setpgrp() == FAIL) {
            HANDLE_ERROR(setpgrp);
            return;
            close(pipefd[0]);
            close(pipefd[1]);
            //handle fd's
        }
        close(pipefd[0]);
        //redirect output to pipe
        int output = pipeToErr ? STDERR_FILENO : STDOUT_FILENO;
        //redirect output to pipe
        if (dup2(pipefd[1], output) == FAIL) {
            HANDLE_ERROR(dup2);
            return;
        }
        close(pipefd[1]);   //do i want to close this now or later?
        in->execute();  //TODO: better error handling
        exit(0);
    }

    pid_t pid2 = fork();

    if (pid2 == FAIL) {
        HANDLE_ERROR(fork);
        return;
    }

    if (pid2 == 0) {
        // "out" side of the pipe
        //close the write side
        if (setpgrp() == FAIL) {
            HANDLE_ERROR(setpgrp);
            return;
            //handle fd's
        }
        close(pipefd[1]);
        //redirect input to pipe
        if (dup2(pipefd[0], STDIN_FILENO) == FAIL) {
            HANDLE_ERROR(dup2);
            return;
        }
        close(pipefd[0]);   //do i want to close this now or later?
        out->execute(); //TODO: better error handling
        exit(0);
    }
    else {
        //parent
        close(pipefd[0]);
        close(pipefd[1]);
        //wait for both children
        SmallShell& shell = SmallShell::getInstance();
        shell.currentProcess = pid1;
        shell.pipedProcess = pid2;
        if (waitpid(pid1, nullptr, 0) == FAIL) {
            HANDLE_ERROR(waitpid);
        }
        if (waitpid(pid2, nullptr, 0) == FAIL) {
            HANDLE_ERROR(waitpid);
        }
        shell.currentProcess = NO_PROCESS_RUNNING;
        shell.pipedProcess = NO_PROCESS_RUNNING;
    }
    //fork
    //connect pipe
    //excute each command
}



//redirection execute
void RedirectionCommand::execute() {
    int fd;
    mode_t old_mask = umask(0);
    if (overwrite) {
        fd = open(filePath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
    else {
        fd = open(filePath.c_str(), O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
    umask(old_mask);
    if (fd == FAIL) {
        HANDLE_ERROR(open);
        return;
    }
    pid_t pid = fork();
    if (pid == FAIL) {
        HANDLE_ERROR(fork);
        return;
    }
    if (pid == 0) {
        if (setpgrp() == FAIL) {
            HANDLE_ERROR(setpgrp);
            return;
        }
        if (dup2(fd, STDOUT_FILENO) == FAIL) {
            HANDLE_ERROR(dup2);
            return;
        }
        close(fd);
        in->execute();
        exit(0);
    }
    else {
        SmallShell& shell = SmallShell::getInstance();
        shell.currentProcess = pid;
        if (waitpid(pid, nullptr, 0) == FAIL) {
            HANDLE_ERROR(waitpid);
        }
        shell.currentProcess = NO_PROCESS_RUNNING;
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
struct DirEntry {
    string name;
    bool isDir;
};

bool DirEntCmp1(const DirEntry& a, const DirEntry& b) {
    if (a.isDir != b.isDir) {
        return !a.isDir; //trust the logic
    }
    return a.name < b.name;
}

void sortDirEntryVector(vector<DirEntry>& entries) {
    sort(entries.begin(), entries.end(), DirEntCmp1);
}

void printDirEntryVector(vector<DirEntry>& entries) {
    for (auto entry : entries) {
        string type = entry.isDir ? "directory: " : "file: ";
        cout << type << entry.name << endl;
    }
}
void ListDirCommand::execute() {
    if (argc > 2) {
        cerr << ("smash: listdir: too many arguments") << endl;
        return;
    }
    string folder_path;
    if (argc > 1) {
        folder_path = argv[1];
    }
    else {
        char buff[COMMAND_MAX_LENGTH];
        getCWD(buff);
        if (!buff) {
            HANDLE_ERROR("getcwd");
        };
        folder_path = buff;
    }
    char dir_buff[DIR_BUFF_SIZE];   //piaza said 4096 bytes
    linux_dirent* dirent;
    int file_descriptor = open(folder_path.c_str(), O_RDONLY | O_DIRECTORY);
    if (file_descriptor == -1) {
        perror("smash error: open failed");
        return;
    }
    ssize_t b_read;
    vector<DirEntry> dir_entries;
    while (1) {
        b_read = syscall(SYS_getdents, file_descriptor, dir_buff, DIR_BUFF_SIZE);
        if (b_read == 0) {
            break;
        }
        if (b_read == -1) {
            perror("smash error: getdents failed");
            return;
        }
        for (int bpos = 0; bpos < b_read;) {
            dirent = (linux_dirent*) (dir_buff + bpos);
            bpos += dirent->d_reclen;
            char dir_type = *(dir_buff + bpos + dirent->d_reclen - 1);
            if (dir_type == DT_REG || dir_type == DT_DIR) {
                if (dirent->d_name[0] == '.') { continue; }
                dir_entries.push_back({ string(dirent->d_name), dir_type == DT_DIR });
            }
        }
    }
    sortDirEntryVector(dir_entries);
    printDirEntryVector(dir_entries);

}

void GetUserCommand::execute() {
    if (argc > 2) {
        cerr << ("smash error: getuser: too many arguments") << endl;
        return;
    }
    pid_t p = argc > 1 ? atoi(argv[1]) : getpid();
    if (kill(p, 0) != 0) {
        cerr << "smash error: getuser: process " << to_string(p) << " does not exist" << endl;
        return;
    }
    string status_file = "/proc/" + to_string(p) + "/status";
    int file_d = open(status_file.c_str(), O_RDONLY);
    if (file_d == -1) {
        perror("smash error: open failed");
        return;
    }
    char buff[DIR_BUFF_SIZE];
    ssize_t b_read = read(file_d, buff, DIR_BUFF_SIZE - 1);
    if (b_read == -1) {
        close(file_d);
        perror("smash error: read failed");
        return;
    }
    buff[b_read] = '\0';
    close(file_d);
    string file_contents = buff;
    string user_id_str = file_contents.substr(file_contents.find("Uid:") + 5);
    user_id_str = user_id_str.substr(0, user_id_str.find_first_of('\n'));
    string group_id_str = file_contents.substr(file_contents.find("Gid:") + 5);
    group_id_str = group_id_str.substr(0, group_id_str.find_first_of('\n'));
    struct passwd* user = getpwuid(stoul(user_id_str.substr(user_id_str.find('\t'))));
    struct group* group = getgrgid(stoul(group_id_str.substr(group_id_str.find('\t'))));

    cout << "User: " << user->pw_name << endl << "Group: " << group->gr_name << endl;
}

void aliasCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if (argc == 1) {
        smash.aliases.printAliases();
        return;
    }
    if (!Aliases::isLegalAliasFormat(cmd_Copy)) {
        cerr << ("smash error: alias: invalid alias format") << endl;
        return;
    }
    string key, value;
    Aliases::parseAliasCommand(cmd_Copy, &key, &value);

    if (!smash.aliases.addAlias(cmd_Copy)) {
        cerr << "smash error: alias " << key << " already exists or is a reserved word" << endl;
    }
}

void unaliasCommand::execute() {
    if (argc == 1) {
        cerr << ("smash error: unalias: not enough arguments") << endl;
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    string key = _trim(argv[1]);
    if (!smash.aliases.removeAlias(key)) {
        cerr << "smash error: unalias: " << key << " alias does not exist" << endl;
    }
}


#define BASH_PATH "/bin/bash"
#define BASH_FLAG "-c"
void ExternalCommand::execute() {
    bool wildcard = false;
    char** newargv = argv;
    if (strchr(commandString, '*') || strchr(commandString, '?')) {
        //means it has a wildcard and we need bash
        newargv = new char* [4];
        string command;
        for (int i = 0; i < argc; i++) {
            command += argv[i];
            if (i != argc - 1) {
                command += " ";
            }
            else {
                command += ";exit";
            }
        }

        //char c_BASH_PATH[]="/bin/bash\0";
        //char c_BASH_FLAG[] ="-c\0";
        //the following two lines wont compile for me
        //(ISO C++ wont allow string to char* convertions)
        //idk why but leave this here  for me until resolved for easy fix thank you :)

        //i aint reading all that ^^^

        const char* path = BASH_PATH;
        const char* flag = BASH_FLAG;

        newargv[0] = const_cast<char*>(path); //im not going to change any of these, compiler should chill out fr fr
        newargv[1] = const_cast<char*>(flag);
        newargv[2] = const_cast<char*>(command.c_str());
        newargv[3] = nullptr;
        wildcard = true;
    }
    pid_t processPid = fork();

    if (processPid < 0) {   //fork failed
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
            shell.jobsList.removeFinishedJobs();    //is this needed? yes!
            shell.jobsList.addJob(this, processPid);
        }
        else {
            //document as currenly running and wait for it to finish
            shell.currentProcess = processPid;
            if (waitpid(processPid, nullptr, 0) == FAIL) {
                HANDLE_ERROR(waitpid);
            }

            if (wildcard) {
                delete[] newargv;
            }
            shell.currentProcess = NO_PROCESS_RUNNING;
        }
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
    int maxJobID = entries.empty() ? 0 : entries.rbegin()->first;
    JobEntry* newEntry = new JobEntry(maxJobID + 1, pid, cmd->commandString);
    entries[maxJobID + 1] = newEntry;
}
void JobsList::printJobsList() {
    for (auto it = entries.begin(); it != entries.end(); it++) {
        cout << "[" << it->first << "] " << it->second->cmd_str << endl;
    }
}

void JobsList::killAllJobs() {
    cout << "smash: sending SIGKILL signal to " << entries.size() << " jobs:" << endl;
    for (auto it = entries.begin(); it != entries.end();) {
        cout << it->second->pid << ": " << it->second->cmd_str << endl;
        if (kill(it->second->pid, SIGKILL) == FAIL) {
            HANDLE_ERROR(kill);
            //what to do if kill fails? do we still remove the job?
        }
        it = entries.erase(it);
    }
}

void JobsList::removeFinishedJobs() {
    for (auto it = entries.begin(); it != entries.end();) {
        if (it->second->isFinished()) {
            delete it->second;
            it = entries.erase(it);
        }
        else {
            it++;
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
    if (!entries.count(jobId)) {
        return;
    }
    delete entries[jobId];
    entries.erase(jobId);
}

void JobsList::bringJobToForeground(int jobId) {
    assert(entries.count(jobId) || jobId == -1); //we shouldnt get here if the job doesnt exist
    JobEntry* job;
    if (jobId == -1) {
        job = entries.rbegin()->second;
    }
    else {
        job = entries[jobId];
    }
    SmallShell& shell = SmallShell::getInstance();
    pid_t pid = job->pid;
    shell.jobsList.removeJobById(job->id);
    shell.currentProcess = pid;
    if (waitpid(pid, nullptr, 0) == FAIL) {
        HANDLE_ERROR(waitpid);
    }
    shell.currentProcess = NO_PROCESS_RUNNING;
}





/* ALIAS */
std::regex pattern("^alias [a-zA-Z0-9_]+='[^']*'$"); // Regex pattern for matching alias commands

Aliases::Aliases() {
    saved_words = { "chprompt", "showpid", "pwd", "cd", "jobs", "fg", "quit",
                    "kill", "alias", "unalias", "listdir", "getuser", "watch" };

};

bool Aliases::isLegalAliasFormat(const char* cmd_line) {
    return std::regex_match(cmd_line, pattern);
}

bool Aliases::isLegalAliasFormat(const string& cmd_line) {
    return regex_match(cmd_line, pattern);
}

void Aliases::deAlias(char cmd_line[COMMAND_MAX_LENGTH]) {
    string dealiased_str = cmd_line;

    size_t first_ws_pos = dealiased_str.find_first_of(WHITESPACE);
    string first_word = dealiased_str.substr(0, first_ws_pos);
    auto it = aliases_map.find(first_word);
    if (it != aliases_map.end()) {
        if (first_word == cmd_line) {
            dealiased_str = it->second;
        }
        else {
            dealiased_str = it->second + dealiased_str.substr(first_ws_pos);
        }
        memset(cmd_line, 0, COMMAND_MAX_LENGTH);
        strcpy(cmd_line, dealiased_str.c_str());
    }

}

bool Aliases::addAlias(const char* cmd_line) {
    assert(isLegalAliasFormat(cmd_line));
    string key, value;
    parseAliasCommand(cmd_line, &key, &value);
    if (isAliasOrReseved(key)) {
        return false;
    }
    aliases_map[key] = value;
    alias_list.push_back(key);
    return true;
}
bool Aliases::isAliasOrReseved(string& key) {
    return (aliases_map.find(key) != aliases_map.end() || saved_words.find(key) != saved_words.end());
}


bool Aliases::removeAlias(string& key) {
    auto it = aliases_map.find(key);
    if (it == aliases_map.end()) {
        return false;
    }
    aliases_map.erase(it);
    alias_list.remove(key);
    return true;
}

void Aliases::printAliases() {
    if (aliases_map.empty()) {
        return;
    }
    for (auto& key : alias_list) {
        std::cout << key << "='" << aliases_map.at(key) << "'" << endl;
    }

}

bool Aliases::parseAliasCommand(const char* cmd_line, string* key, string* value) {
    string trimmed = _trim(string(cmd_line));
    size_t key_begin_pos = trimmed.find_first_of(WHITESPACE);//should be after the word aliases
    key_begin_pos = trimmed.find_first_not_of(WHITESPACE, key_begin_pos);
    size_t key_end_pos = trimmed.find("=");
    size_t value_begin_pos = trimmed.find_first_of("'") + 1;
    size_t value_end_pos = trimmed.find_last_of("'");
    if (key_begin_pos == trimmed.npos || key_end_pos == trimmed.npos
        || value_begin_pos == trimmed.npos || value_end_pos == trimmed.npos) {
        return false;
    }
    *key = trimmed.substr(key_begin_pos, key_end_pos - key_begin_pos);
    *key = _trim(*key);
    *value = trimmed.substr(value_begin_pos, value_end_pos - value_begin_pos);
    return true;
}


