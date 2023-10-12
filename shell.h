#ifndef SHELL_H
#define SHELL_H

#include <assert.h>  
#include <fcntl.h>   
#include <stdbool.h> 
#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  

#define MAXLINE 80
#define PROMPT "osh> "

#define RD 0
#define WR 1

// Returns true if C-strings a and b are equal, false otherwise.
bool equal(const char *a, const char *b);

// Fetches a line from stdin, and returns the length of the line read or -1 if failed to read.
// Also removes the newline character from the line read.
int fetchline(char **line);

// An interactive shell to process and execute input commands. Returns 0 upon successful execution.
int interactiveShell();

// Runs a set of predefined tests on the shell functionalities. Returns 0 upon successful execution.
int runTests();

// Processes the input line and performs the associated command operation.
void processLine(char *line);

#endif
