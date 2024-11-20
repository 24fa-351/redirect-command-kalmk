#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(
            stderr,
            "Usage: %s <inputFile> <outputFile> <command> <arg1> <arg2> ...\n",
            argv[0]);
        return 1;
    }

    int input_fd;
    if (strcmp(argv[1], "-") == 0)
    {
        input_fd = STDIN_FILENO;
    }
    else
    {
        input_fd = open(argv[1], O_RDONLY);
        if (input_fd < 0)
        {
            perror("Failed to open input file");
            return 1;
        }
    }

    int output_fd;
    if (strcmp(argv[2], "-") == 0)
    {
        output_fd = STDOUT_FILENO;
    }
    else
    {
        output_fd =
            open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (output_fd < 0)
        {
            perror("Failed to open output file");
            if (input_fd != STDIN_FILENO)
                close(input_fd);
            return 1;
        }
    }

    char **newargv = (char **)malloc(sizeof(char *) * (argc - 2));
    for (int ix = 3; ix < argc; ix++)
    {
        newargv[ix - 3] = argv[ix];
    }
    newargv[argc - 3] = NULL;

    int child_pid = fork();
    if (child_pid == 0)
    {
        if (input_fd != STDIN_FILENO)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(newargv[0], newargv);
        perror("execvp failed");
        exit(1);
    }

    if (input_fd != STDIN_FILENO)
        close(input_fd);
    if (output_fd != STDOUT_FILENO)
        close(output_fd);
    wait(NULL);
    printf("%s pid is %d. forked %d. Parent exiting\n", argv[0], getpid(),
           child_pid);

    free(newargv);
    return 0;
}