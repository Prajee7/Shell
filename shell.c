/**
* ---------------------------------------
* This project involves creating a shell interface in C that accepts user
* commands and executes them in separate processes. The implementation includes
* forking child processes using fork(), executing commands with the execvp() family
* of system calls, and handling background execution with '&'. The project also
* incorporates features such as a history mechanism, input and output redirection 
* using '>', '<', and communication between processes via pipes. The implementation 
* involves using system calls like dup2() and pipe() to manage file descriptors and 
* facilitate inter-process communication.
*/
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAX_LINE 80 /* The maximum length command */
char hist[MAX_LINE + 2];
int lsORgr = 0; // For redirecting input and output
                // 0 = nothing, 1 = '<', 2 = '>'
void printAsciiArt(void);

int main(int argc, char **argv)
{
    if (argc == 2 && equal(argv[1], "--interactive"))
    {
        return interactiveShell();
    }
    else
    {
        return runTests();
    }
}

// interactive shell to process commands
int interactiveShell()
{
    bool should_run = true;
    char *line = NULL; // Just initialize to NULL, getline will allocate
    size_t len = 0;    // Use a size_t for len, which is the type getline expects

    while (should_run)
    {
        printf("osh> ");
        fflush(stdout);

        ssize_t n = getline(&line, &len, stdin);

        // Handle EOF (Ctrl+D)
        if (n == -1)
        {
            printf("Exiting shell\n");
            should_run = false;
            continue;
        }

        // Strip newline character read by getline
        if (n > 0 && line[n - 1] == '\n')
        {
            line[n - 1] = '\0';
        }

        // Check for history command
        if (equal(line, "!!"))
        {
            if (strlen(hist) == 0)
            {
                printf("No commands in history.\n");
                continue;
            }
            printf("Executing last command: %s\n", hist);
            processLine(hist);
            continue;
        }

        // Store the command in history
        strncpy(hist, line, MAX_LINE);

        // Exit on command
        if (equal(line, "exit"))
        {
            should_run = false;
            continue;
        }

        // Skip empty lines
        if (equal(line, ""))
        {
            continue;
        }

        processLine(line);
    }

    free(line); // Free memory allocated by getline
    return 0;
}

void processLine(char *line)
{
    char *command = strtok(line, ";"); // Split commands based on ;

    while (command != NULL)
    {
        int should_wait = 1; // Initialize to true
        char *input_file = NULL;
        char *output_file = NULL;

        // Check if command should run in background
        if (command[strlen(command) - 1] == '&')
        {
            command[strlen(command) - 1] = '\0'; // Remove &
            should_wait = 0;
        }

        char *args[MAX_LINE / 2 + 1];
        int i = 0;

        args[i] = strtok(command, " ");
        while (args[i] != NULL)
        {
            if (strcmp(args[i], ">") == 0)
            {
                args[i] = strtok(NULL, " ");
                if (args[i] != NULL)
                {
                    output_file = args[i];
                    args[i] = strtok(NULL, " ");
                }
                continue;
            }
            else if (strcmp(args[i], "<") == 0)
            {
                args[i] = strtok(NULL, " ");
                if (args[i] != NULL)
                {
                    input_file = args[i];
                    args[i] = strtok(NULL, " ");
                }
                continue;
            }
            else if (strcmp(args[i], "&") == 0)
            {
                should_wait = 0;
                args[i] = NULL;
                break;
            }
            else if (strcmp(args[i], "|") == 0)
            {
                // Basic pipe implementation
                args[i] = NULL;
                int fd[2];
                pipe(fd);
                if (fork() == 0)
                {
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    execvp(args[0], args);
                    perror(args[0]);
                    exit(EXIT_FAILURE);
                }
                if (fork() == 0)
                {
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    char *cmd2 = strtok(NULL, " ");
                    char *args2[MAX_LINE / 2 + 1];
                    int j = 0;
                    while (cmd2 != NULL)
                    {
                        args2[j++] = cmd2;
                        cmd2 = strtok(NULL, " ");
                    }
                    args2[j] = NULL;
                    execvp(args2[0], args2);
                    perror(args2[0]);
                    exit(EXIT_FAILURE);
                }
                close(fd[0]);
                close(fd[1]);
                wait(NULL);
                wait(NULL);
                return;
            }
            i++;
            args[i] = strtok(NULL, " ");
        }

        if (strcmp(args[0], "ascii") == 0)
        {
            printAsciiArt();
            command = strtok(NULL, ";");
            continue;
        }

        pid_t pid = fork();

        if (pid == 0)
        {
            // Child process
            if (output_file != NULL)
            {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                if (fd < 0)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (input_file != NULL)
            {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (execvp(args[0], args) == -1)
            {
                perror(args[0]);
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0)
        {
            // Parent process
            if (should_wait)
            {
                int status;
                waitpid(pid, &status, 0);
            }
        }
        else
        {
            perror("fork");
        }

        command = strtok(NULL, ";"); // Get the next command
    }
}

void printAsciiArt()
{
    printf("  |\\_/|        ****************************     (\\_/)\n");
    printf(" / @ @ \\       *  \"Purrrfectly pleasant\"  *    (='.'=)\n");
    printf("( > º < )      *                              *    (\")_(\")\n");
    printf(" `>>x<<´      *                               *\n");
    printf(" /  O  \\     *********************************\n\n");
}

int runTests()
{
    printf("*** Running basic tests ***\n");
    char lines[7][MAXLINE] = {
        "ls", "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt",
        "ls | wc", "ascii"};
    for (int i = 0; i < 7; i++) // not testing ascii now. will implement later.
    {
        printf("* %d. Testing %s *\n", i + 1, lines[i]);
        processLine(lines[i]);
    }

    return 0;
}

bool equal(const char *a, const char *b)
{
    if (!a || !b)
    {
        fprintf(stderr, "equal: NULL pointer passed!\n");
        return false;
    }
    return (strcmp(a, b) == 0);
}

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line)
{
    size_t len = 0;
    ssize_t n = getline(line, &len, stdin);
    if (n > 0)
    {
        (*line)[n - 1] = '\0'; // Removing the newline character
    }
    else if (n == -1)
    {
        if (feof(stdin))
        {
            // End of file (CTRL+D)
            fprintf(stderr, "fetchline: End of file detected.\n");
        }
        else if (ferror(stdin))
        {
            // Error reading from stdin
            perror("fetchline");
        }
    }
    return n;
}
