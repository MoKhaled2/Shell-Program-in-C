# Shell Program in C

This C program simulates a simple shell that supports various functionalities, including executing commands, handling piping (`|`), redirection (`>` and `>>`), compound commands (semicolon `;`), and logical AND (`&&`) commands. It is designed to execute commands as if it's a basic terminal shell.

## Key Components of the Code:

### 1. **Header Files**
   - `stdio.h`: Standard I/O functions like `printf`, `fgets`, and `scanf`.
   - `stdlib.h`: Memory management and process control functions like `malloc`, `free`, `exit`.
   - `unistd.h`: Provides access to the system’s API (e.g., `fork()`, `execvp()`, `getcwd()`).
   - `string.h`: String manipulation functions like `strtok`, `strcpy`, `strcspn`.
   - `sys/types.h`, `sys/wait.h`: For process-related functions, particularly `fork()` and `wait()`.
   - `fcntl.h`: File control options used in redirection.

### 2. **Function: `execute_command`**
   This function is used to execute a single command entered by the user.
   
   - The input command is broken into arguments using `strtok()`.
   - Handles built-in commands such as `cd` (change directory).
   - Uses `fork()` to create a child process, where `execvp()` is used to execute the command in the child process.
   - If the command is `cd`, it directly changes the directory using `chdir()`.
   - If the command is a `grep` command, it removes any surrounding quotes in the arguments.

### 3. **Function: `handle_piping`**
   This function handles commands that include pipes (`|`). It separates the command into multiple parts using `strtok()` and sets up pipes between processes.

   - For each command, a pipe is created using `pipe()`.
   - `fork()` creates a child process that either reads from the previous command’s output or writes to the next command’s input.
   - After execution, the pipe is closed and the program waits for the child processes to finish.

### 4. **Function: `handle_redirection`**
   This function handles output redirection using `>` (overwrite) or `>>` (append).

   - The command is checked for the presence of `>` or `>>`, and if either is found, the file for output redirection is determined.
   - The file is opened in write mode (with append support for `>>`).
   - If redirection is detected, the standard output is redirected to the file using `dup2()` and the command is executed.
   
### 5. **Function: `handle_compound_commands`**
   This function handles compound commands separated by a semicolon (`;`), where multiple commands are executed sequentially.

   - The input command is split into subcommands using `strtok()`.
   - Each subcommand is executed one by one in sequence using `execute_command()`.

### 6. **Function: `handle_and_commands`**
   This function handles logical AND (`&&`), where the next command is executed only if the previous command was successful (i.e., returned 0).

   - The input command is split using `strtok()` based on `&&`.
   - It checks the exit status of the previous command (`wait()`) and only proceeds if the previous command was successful (status 0).

### 7. **Main Function: Shell Loop**
   The main loop of the shell interacts with the user:
   
   - The shell continuously prints a prompt with the username, hostname, and current working directory.
   - It accepts user input using `fgets()` and processes the command entered.
   - Based on the command type (e.g., `|`, `>`, `;`, `&&`), the corresponding handler functions are called.
   - If the command is `exit`, the loop terminates.

### 8. **Built-in Commands**
   - `cd`: Changes the current directory. If no argument is provided, it outputs an error message.
   - `exit`: Exits the shell.

### Example of Commands and Functionality:

- **Single Command Execution:**
   ```bash
   ls -l
