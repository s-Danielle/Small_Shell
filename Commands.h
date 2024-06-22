#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define DEFAULT_PROMPT_LINE ("smash> ")
class Command {
// TODO: Add your data members
public:
    char commandString[COMMAND_MAX_LENGTH];
    Command(const char *cmd_line){
        memset(this->commandString, 0, COMMAND_MAX_LENGTH);
        strcpy(this->commandString,cmd_line);
    };

    virtual ~Command()=default;

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmdLine) : Command(cmdLine) {};

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class WatchCommand : public Command {
    // TODO: Add your data members
public:
    WatchCommand(const char *cmd_line);

    virtual ~WatchCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
private:
     char* plast_cwd;
public:
// TODO: Add your data members public:
    ChangeDirCommand(const char *cmd_line,  char *pLast_cwd) : BuiltInCommand(cmd_line), plast_cwd(pLast_cwd){}

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {}

    void execute() override;
};


class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    };
    // TODO: Add your data members
public:
    JobsList();

    ~JobsList();

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class ListDirCommand : public BuiltInCommand {
public:
    ListDirCommand(const char *cmd_line);

    virtual ~ListDirCommand() {}

    void execute() override;
};

class GetUserCommand : public BuiltInCommand {
public:
    GetUserCommand(const char *cmd_line);

    virtual ~GetUserCommand() {}

    void execute() override;
};

class aliasCommand : public BuiltInCommand {
public:
    aliasCommand(const char *cmd_line);

    virtual ~aliasCommand() {}

    void execute() override;
};

class unaliasCommand : public BuiltInCommand {
public:
    unaliasCommand(const char *cmd_line);

    virtual ~unaliasCommand() {}

    void execute() override;
};

class changePrompt : public BuiltInCommand {
private:
     char* pPromptLine;
public:
    changePrompt(const char *cmd_line, char* promptLine): BuiltInCommand(cmd_line), pPromptLine(promptLine) {  }

    virtual ~changePrompt() {}

    void execute() override;
};

class SmallShell {
private:
    SmallShell();
public:
     char prompt_line[COMMAND_MAX_LENGTH];
     char last_path[COMMAND_MAX_LENGTH];
    char* aliases[COMMAND_MAX_LENGTH]{}; //not its final form

    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell()=default;

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed

};

#endif //SMASH_COMMAND_H_
