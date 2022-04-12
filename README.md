# minishell

this project was designed as a shell 
without using <stdio.h>

1. goal

minishell is a custom Linux shell.

minishell is a program that

- takes input from a user
- interprets it
- executes a command



2. compile

- gcc shell.c -o shell



3. code explanation

shell runs with while loop and execute command with fork().

child process executes linux command with execvp().