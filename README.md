# Shell

This project involves creating a shell interface in C that accepts user commands and executes them in separate processes. The implementation includes forking child processes using fork(), executing commands with the execvp() family of system calls, and handling background execution with '&'. The project also incorporates features such as a history mechanism, input and output redirection using '>', '<', and communication between processes via pipes. The implementation involves using system calls like dup2() and pipe() to manage file descriptors and facilitate inter-process communication.

