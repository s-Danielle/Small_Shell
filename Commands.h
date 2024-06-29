#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <map>
#include <unordered_map>
#include <cstring>
#include <list>
#include <unordered_set>
#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (21)
#define DEFAULT_PROMPT_LINE ("smash> ")
#define NO_PROCESS_RUNNING (-1)
#define DIR_BUFF_SIZE (4096)

class Command {
public:
    int argc;
    char** argv;
    char commandString[COMMAND_MAX_LENGTH];
    Command(const char* cmd_line, int argc, char** argv) :argc(argc), argv(argv) {
        memset(this->commandString, 0, COMMAND_MAX_LENGTH);
        strcpy(this->commandString, cmd_line);
    };

    virtual ~Command() {
        for (int i = 0; i < argc; i++) {
            delete[] argv[i];
        }
    }; //we need to free memory here (argv)

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmdLine, int argc = 0, char** argv = nullptr) : Command(cmdLine, argc, argv) {};//TODO: remove default values

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
    bool isBg;
    // static char * bashPath = "/bin/bash";
    // static char * bashFlag = "-c";
public:
    ExternalCommand(const char* cmd_line, int argc, char** argv, bool isBg) : Command(cmd_line, argc, argv), isBg(isBg) {};

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    Command* in;
    Command* out;
    char inCmd[COMMAND_MAX_LENGTH];
    char outCmd[COMMAND_MAX_LENGTH];
    char* inargv[COMMAND_MAX_ARGS];
    char* outargv[COMMAND_MAX_ARGS];
    char* cmdCopy; //WILL be messed with
    bool pipeToErr;
public:
    PipeCommand(const char* cmd_line, char* cmdCopy, int argc, char** argv); 

    virtual ~PipeCommand() {
        delete in;
        delete out;
    }

    void execute() override;
};

class WatchCommand : public Command {
public:
    WatchCommand(const char* cmd_line);

    virtual ~WatchCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    Command* in;
    char* cmdCopy; //WILL be messed with
    // char* filePath;
    std::string filePath;  //ugly hack but whatever
    char* inargv[COMMAND_MAX_ARGS];
    bool overwrite;
public:
    explicit RedirectionCommand(const char* cmd_line, char* cmdCopy, int argc, char** argv);

    virtual ~RedirectionCommand() {
        delete in;
    }

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
private:
    char* plast_cwd;
public:
    ChangeDirCommand(const char* cmd_line, int argc, char** argv, char* pLast_cwd) :
    BuiltInCommand(cmd_line,argc,argv), plast_cwd(pLast_cwd) {}

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
    bool isKill = false;
public:
    QuitCommand(const char* cmd_line, int argc, char** argv) : BuiltInCommand(cmd_line, argc, argv) {
        if (argc > 1 && strcmp(argv[1], "kill") == 0) {
            isKill = true;
        }
    };

    virtual ~QuitCommand() {}

    void execute() override;
};

class JobsList {
public:
    class JobEntry {
    public:
        int id;
        pid_t pid;
        char cmd_str[COMMAND_MAX_LENGTH];
        JobEntry(int id, pid_t pid, const char* str) : id(id), pid(pid) { strcpy(cmd_str, str); }
        ~JobEntry() = default;
        bool isFinished();
    };
    std::map<int, JobEntry*> entries;

public:
    JobsList() = default;

    ~JobsList() = default;

    void addJob(Command* cmd, pid_t pid);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry* getJobById(int jobId);

    void removeJobById(int jobId);

    void bringJobToForeground(int jobId = -1);

    // JobEntry* getLastJob(int* lastJobId);

    // JobEntry* getLastStoppedJob(int* jobId);
};

class JobsCommand : public BuiltInCommand {
public:
    JobsCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~JobsCommand() = default;

    void execute() override;
};

class KillCommand : public BuiltInCommand {
public:
    KillCommand(const char* cmd_line, int argc, char** argv) : BuiltInCommand(cmd_line, argc, argv) {};

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
public:
    ForegroundCommand(const char* cmd_line, int argc, char** argv) : BuiltInCommand(cmd_line, argc, argv) {};

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class ListDirCommand : public BuiltInCommand {
public:
    ListDirCommand(const char* cmd_line, int argc, char** argv) : BuiltInCommand(cmd_line, argc, argv) {};

    virtual ~ListDirCommand() = default;

    void execute() override;
};

class GetUserCommand : public BuiltInCommand {
public:
    GetUserCommand(const char* cmd_line, int argc, char** argv) : BuiltInCommand(cmd_line, argc, argv){}

    virtual ~GetUserCommand() = default;

    void execute() override;
};

class aliasCommand : public BuiltInCommand {
public:
    aliasCommand(const char* cmd_line,int argc, char** argv):BuiltInCommand(cmd_line,argc,argv){}

    virtual ~aliasCommand() {}

    void execute() override;
};

class unaliasCommand : public BuiltInCommand {
public:
    unaliasCommand(const char* cmd_line,int argc, char** argv):BuiltInCommand(cmd_line,argc,argv){}

    virtual ~unaliasCommand() {}

    void execute() override;
};

class changePrompt : public BuiltInCommand {
public:
    changePrompt(const char* cmd_line,int argc, char** argv) : BuiltInCommand(cmd_line,argc,argv) {}

    virtual ~changePrompt() {}

    void execute() override;
};

/* ALIAS */

// using namespace std;
class Aliases {
private:
    // static const regex aliases_pattern;
    std::map<std::string,std::string> aliases_map;
    std::list <std::string> alias_list;
    std::unordered_set<std::string> saved_words;
public:
    Aliases();
    bool addAlias(const char* cmd_line); //returns false if exists or reserved
    bool removeAlias(std::string &key);
    bool isAliasOrReseved(std::string &key);
    static bool isLegalAliasFormat(const char* cmd_line);
    static bool isLegalAliasFormat(const std::string& cmd_line);
    void printAliases();
    static bool parseAliasCommand(const char* cmd_line, std::string* key, std::string* value);
    void deAlias(char *cmd_line);
};

class SmallShell {
private:
    SmallShell();
public:
    //why are these public? //because I can
    JobsList jobsList;
    Aliases aliases;
    char prompt_line[COMMAND_MAX_LENGTH];
    char last_path[COMMAND_MAX_LENGTH];
    pid_t currentProcess = NO_PROCESS_RUNNING;
    pid_t pipedProcess = NO_PROCESS_RUNNING; //will hold the pid of the process that is piped to


    Command* CreateCommand(const char* cmd_line, char* cmdCopy, int argc, char** argv, bool isBg);

    SmallShell(SmallShell const&) = delete; // disable copy ctor
    void operator=(SmallShell const&) = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell() = default;

    void executeCommand(const char* cmd_line);

};





#endif //SMASH_COMMAND_H_
