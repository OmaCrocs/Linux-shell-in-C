/*
 * file: shell.c
 * author: Ungerank Mirko
 * date: 05.05.21
 * description: Simple Shell in C
 */

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*Built in shell commands*/
int fun_cd(char **args);
int fun_help(char **args);
int fun_exit(char **args);
int fun_ls(char **args);
int fun_echo(char **args);
int fun_rm(char **args);

//builtin commands, followed by their functions
char *builtin_str[] = {
        "cd",
        "help",
        "exit",
        "ls",
        "echo",
        "rm"
};

int (*builtin_func[])(char **) = {
        &fun_cd,
        &fun_help,
        &fun_exit,
        &fun_ls,
        &fun_echo,
        &fun_rm
};

int fun_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int fun_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected argument for \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("error");
        }
    }
    return 1;
}

int fun_ls(char **args) {
    if (args[1] == NULL) {
        system("ls\n");
    }
    return 1;
}

int fun_echo(char **args) {
    printf("%s\n", args[1]);
    return 1;
}

int fun_rm(char **args) {
    int del = remove(args[1]);
    if (!del)
        printf("The file has been successfully deleted.\n");
    else
        printf("There was an error deleting the file.\n");
    return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int fun_help(char **args) {
    int i;
    printf("Type in the commands, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < fun_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the 'man' command for commands not implemented into this project.\n");
    return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int fun_exit(char **args) {
    return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int fun_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("fun");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("fun");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int fun_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < fun_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return fun_launch(args);
}

#define fun_RL_BUFSIZE 1024

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *fun_read_line(void) {
    int bufsize = fun_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "fun: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += fun_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "fun: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define fun_TOK_BUFSIZE 64
#define fun_TOK_DELIM " \t\r\n\a"

/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **fun_split_line(char *line) {
    int bufsize = fun_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "fun: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, fun_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += fun_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "fun: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, fun_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void fun_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("Shell:)>");
        line = fun_read_line();
        args = fun_split_line(line);
        status = fun_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv) {
    fun_loop();

    return EXIT_SUCCESS;
}
