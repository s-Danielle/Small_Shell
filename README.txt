## Overview
**Smash** is a small shell program that emulates the behavior of a Unix-like terminal. It supports both built-in and external commands, providing a modular and extensible design for command execution. The shell includes basic job management, I/O redirection, signal handling, and more. This implementation follows a structured design using **Singleton** and **Factory Method** patterns for modularity and clarity.

## Features
- **Built-in Commands**:
  - `chprompt`: Change the shell prompt.
  - `pwd`: Print the working directory.
  - `showpid`: Display the process ID of the shell.
  - `cd`: Change the current directory.
  - `fg`: Bring a background job to the foreground.
  - `jobs`: Display the list of current jobs.
  - `kill`: Terminate a job by its ID.
- **External Commands**:
  - Execute any valid executable in the system, including commands like `ls`, `cat`, or scripts.
- **Background Execution**:
  - Support for running external commands in the background using `&`.
- **I/O Redirection**:
  - Redirect input and output using `>`, `<`, and `>>`.
- **Signal Handling**:
  - `SIGINT`: Handle Ctrl+C to terminate foreground jobs.
  - `SIGALRM` (Bonus): Manage time-limited alarms for processes.

---

## File Structure

### Core Files
- **`Commands.h`/`Commands.cpp`**:
  - Defines classes for all commands. Each command is represented by a class inheriting from `BuiltInCommand` or `ExternalCommand`.
  - Each command implements the `execute()` virtual method for execution logic.

- **`signals.h`/`signals.cpp`**:
  - Handles signal management, including custom handlers for `SIGINT` and `SIGALRM`.

- **`smash.cpp`**:
  - Contains the main function, which runs an infinite loop to read and process commands using the `SmallShell::executeCommand` method.

### Support Files
- **`Makefile`**:
  - Builds the project and runs basic tests using provided input/output files.
  - Example targets:
    - `make`: Build the project.
    - `make test`: Run basic tests using `test_input1.txt`.
    - `make zip`: Prepare a submission zip file.

- **`test_input1.txt` / `test_expected_output1.txt`**:
  - Basic test cases to validate functionality.

---

## Design Patterns
- **Singleton Pattern**:
  - Used for `SmallShell` to ensure there is only one instance managing the shell state and commands.

- **Factory Method Pattern**:
  - Used in `SmallShell::CreateCommand` to instantiate the appropriate command class based on user input.

---

## How to Build
1. Clone the repository:
   ```bash
   git clone <repository_url>
   cd smash
   ```
2. Build the project using the Makefile:
   ```bash
   make
   ```

---

## How to Run
1. Run the shell program:
   ```bash
   ./smash
   ```
2. Use built-in commands or external commands as needed.

---

## How to Test
Run the provided test cases to validate functionality:
```bash
make test
```
This will use `test_input1.txt` and compare the output with `test_expected_output1.txt`.

---

## Example Commands
### Built-In Commands
```bash
smash> chprompt new_prompt
new_prompt> pwd
/home/user
new_prompt> showpid
12345
new_prompt> cd /tmp
```

### External Commands
```bash
smash> ls -l
smash> gcc main.cpp -o main &
```

### Job Management
```bash
smash> jobs
[1] sleep 1000 &
smash> fg 1
```

---

## Bonus Features (Optional)
- **SIGALRM Handling**:
  - Allow users to set time limits for command execution using alarms.

---

## Good Practices
- Modular design ensures each command is independent and easy to extend.
- Signal handling ensures smooth operation and proper cleanup of child processes.

---

## Conclusion
This shell is a lightweight and extensible implementation of a Unix-like terminal. Its modular design and use of design patterns make it easy to maintain and extend. Use this as a foundation to explore advanced shell features!
