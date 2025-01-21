#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

// Function to execute a single command
void execute_command(char *command) {
    char *args[100];
    int arg_count = 0;
    char *token = strtok(command, " ");

    while (token != NULL) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    if (arg_count == 0) return;

    // Handle built-in commands like "cd"
    if (strcmp(args[0], "cd") == 0) {
        if (arg_count < 2) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (chdir(args[1]) != 0) {
            perror("cd");
        }
        return;
    }

    // Handle grep command with double quotes
    if (strcmp(args[0], "grep") == 0) {
        for (int i = 1; i < arg_count; i++) {
            if (args[i][0] == '"') {
                // Remove the starting quote
                memmove(args[i], args[i] + 1, strlen(args[i]));
                // Remove the ending quote
                args[i][strlen(args[i]) - 1] = '\0';
            }
        }
    }

    // Create a child process to execute the command
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}

// Function to handle piping
void handle_piping(char *command) {
    char *commands[10];
    int command_count = 0;
    char *token = strtok(command, "|");

    while (token != NULL && command_count < 10) {
        commands[command_count++] = token;
        token = strtok(NULL, "|");
    }

    int pipes[2], in_fd = 0;

    for (int i = 0; i < command_count; ++i) {
        pipe(pipes);
        pid_t pid = fork();

        if (pid == 0) {
            dup2(in_fd, STDIN_FILENO);
            if (i < command_count - 1) dup2(pipes[1], STDOUT_FILENO);
            close(pipes[0]);
            execute_command(commands[i]);
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
            close(pipes[1]);
            in_fd = pipes[0];
        }
    }
}

// Function to handle redirection
void handle_redirection(char *command) {
    char *output_file = NULL;
    int append = 0;

    // Check for ">>" or ">"
    char *redir = strstr(command, ">>");
    if (redir) {
        append = 1;
        *redir = '\0';
        output_file = strtok(redir + 2, " ");
    } else {
        redir = strstr(command, ">");
        if (redir) {
            *redir = '\0';
            output_file = strtok(redir + 1, " ");
        }
    }

    if (output_file) {
        int fd = open(output_file, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
        if (fd < 0) {
            perror("open");
            return;
        }

        pid_t pid = fork();
        if (pid == 0) {
            dup2(fd, STDOUT_FILENO);
            close(fd);
            execute_command(command);
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
            close(fd);
        }
    } else {
        execute_command(command);
    }
}

// Function to handle compound commands (semicolon and &&)
void handle_compound_commands(char *command) {
    char *subcommands[10];
    int command_count = 0;
    char *token = strtok(command, ";");

    while (token != NULL && command_count < 10) {
        subcommands[command_count++] = token;
        token = strtok(NULL, ";");
    }

    for (int i = 0; i < command_count; ++i) {
        execute_command(subcommands[i]);
    }
}

// Function to handle commands with &&
void handle_and_commands(char *command) {
    char *subcommands[10];
    int command_count = 0;
    char *token = strtok(command, "&&");

    while (token != NULL && command_count < 10) {
        subcommands[command_count++] = token;
        token = strtok(NULL, "&&");
    }

    int status = 0;
    for (int i = 0; i < command_count; ++i) {
        execute_command(subcommands[i]);
        // Check the status of the previous command
        if (status != 0) {
            break; // If the previous command failed, stop executing
        }
        wait(&status);
    }
}

int main() {
    char username[32], hostname[32], cwd[256], command[1024];

    // Get username and hostname
    getlogin_r(username, sizeof(username));
    gethostname(hostname, sizeof(hostname));

    while (1) {
        // Get current working directory
        getcwd(cwd, sizeof(cwd));

        // Display the prompt
        printf("[%s@%s:%s]$ ", username, hostname, cwd);
        fflush(stdout);

        // Read user input
        if (!fgets(command, sizeof(command), stdin)) break;
        command[strcspn(command, "\n")] = '\0'; // Remove newline

        // Exit on "exit"
        if (strcmp(command, "exit") == 0) break;

        // Handle piping
        if (strstr(command, "|")) {
            handle_piping(command);
        }
        // Handle redirection
        else if (strstr(command, ">")) {
            handle_redirection(command);
        }
        // Handle compound commands
        else if (strstr(command, ";")) {
            handle_compound_commands(command);
        }
        // Handle commands with &&
        else if (strstr(command, "&&")) {
            handle_and_commands(command);
        }
        // Execute normal commands
        else {
            execute_command(command);
        }
    }

    return 0;
}