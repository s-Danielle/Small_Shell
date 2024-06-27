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
class Command {
    // TODO: Add your data members
public:
    int argc;
    char** argv;
    char commandString[COMMAND_MAX_LENGTH];
    Command(const char* cmd_line, int argc, char** argv) :argc(argc), argv(argv) {
        memset(this->commandString, 0, COMMAND_MAX_LENGTH);
        strcpy(this->commandString, cmd_line);
    };

    virtual ~Command(){
        for(int i = 0; i < argc; i++){
            delete[] argv[i];
        }
    }; //we need to free memory here (argv)

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmdLine, int argc = 0, char** argv = nullptr) : Command(cmdLine, argc, argv) {};//TODO: remove default values

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
    bool isBg;
public:
    ExternalCommand(const char* cmd_line, int argc, char** argv, bool isBg) : Command(cmd_line, argc, argv), isBg(isBg) {};

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
    Command* in;
    Command* out;
public:
    PipeCommand(const char** cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class WatchCommand : public Command {
    // TODO: Add your data members
public:
    WatchCommand(const char* cmd_line);

    virtual ~WatchCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
private:
    char* plast_cwd;
public:
    // TODO: Add your data members public:
    ChangeDirCommand(const char* cmd_line, char* pLast_cwd) : BuiltInCommand(cmd_line), plast_cwd(pLast_cwd) {}

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
    // TODO: Add your data members public:
    bool isKill = false;
    public:
    QuitCommand(const char* cmd_line, int argc, char** argv): BuiltInCommand(cmd_line, argc, argv) {
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
    // TODO: Add extra methods or modify existing ones as needed
};

class JobsCommand : public BuiltInCommand {
public:
    JobsCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~JobsCommand() = default;

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char* cmd_line, JobsList* jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class ListDirCommand : public BuiltInCommand {
public:
    ListDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ListDirCommand() = default;

    void execute() override;
};

class GetUserCommand : public BuiltInCommand {
public:
    GetUserCommand(const char* cmd_line);

    virtual ~GetUserCommand() {}

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
private:
    char* pPromptLine;
public:
    changePrompt(const char* cmd_line, char* promptLine) : BuiltInCommand(cmd_line), pPromptLine(promptLine) {}

    virtual ~changePrompt() {}

    void execute() override;
};

/* ALIAS */

using namespace std;
class Aliases {
private:
    // static const regex aliases_pattern;
    map<string,string> aliases_map;
    list <string> alias_list;
    unordered_set<string> saved_words;
public:
    Aliases();
    bool addAlias(const char* cmd_line); //returns false if exists or reserved
    bool removeAlias(string &key);
    bool isAliasOrReseved(string &key);
    static bool isLegalAliasFormat(const char* cmd_line);
    static bool isLegalAliasFormat(const string& cmd_line);
    void printAliases();
    static bool parseAliasCommand(const char* cmd_line, string* key, string* value);
    void deAlias(char *cmd_line);
};

class SmallShell {
private:
    //TODO: figure a way to print them in order (Q?)
    SmallShell();
public:
    //why are these public?
    JobsList jobsList;
    Aliases aliases;
    char prompt_line[COMMAND_MAX_LENGTH];
    char last_path[COMMAND_MAX_LENGTH];
    pid_t currentProcess = -1;


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
    // TODO: add extra methods as needed

};









#endif //SMASH_COMMAND_H_
