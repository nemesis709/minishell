# minishell

this project was designed as a shell without using <stdio.h>

1. goal

minishell is a custom Linux shell.

minishell is a program that

- takes input from a user
- interprets it
- executes a command



2. compile

    gcc shell.c -o shell


3. basics

input & output redirection with dup2() to duplicate a file descriptor

shell runs with while loop and execute command with fork()

child process executes linux command with execvp()


4. details

minishell has 4 types of command to be processed.

- cmd_type1: Programs that need to be loaded (fork-exec *) and run
- cmd_type2: Program to be implemented (with arguments)
- cmd_type3: Program to be implemented (without arguments)
- cmd_type4: etc(./ , ../ , cd, pwd ...)

multiple commands splits with pipe split.

args split with space.