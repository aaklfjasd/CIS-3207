#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <dirent.h>

char *readLine();
void decodeInput(char *, char **, int *, int *);
int builtinCommand(char **);
char **initPath();
char *getFullPath(char *, char **);

int main (int argc, char **argv) {
    // Load in PATH variable
    char **PATH = initPath();

    // Get current working directory
    char cwd[100];
    getcwd(cwd, 100);
    strcat(cwd, "/myshell");
    // Set shell envvar to contain cwd
    setenv("shell", cwd, 1);

    // If arguments have been passed, read input from the file
    if (argc > 1) {
        int file = open(argv[1], O_RDONLY);
        dup2(file, 0);
        close(file);
    }

    // Start the shell loop
    printf("Welcome!\n");
    while (1) {
        // Prompt for input
        printf("\n%s >> ", cwd);
        char *input = readLine();

        // Decode input
        char **paramList = (char **)malloc(sizeof(char*) * 4);
        int inputFile = NULL;
        int outputFile = NULL;
        decodeInput(input, paramList, &inputFile, &outputFile);

        // Create child process to handle command
        int pid = fork();
        if (pid == 0) {
            // Child will execute this code
            // If no command was passed, do nothing
            if (paramList[0] == NULL) continue;

            // Set up file redirection
            if (inputFile != NULL) {
                dup2(inputFile, 0);
                close(inputFile);
            }
            if (outputFile != NULL) {
                dup2(outputFile, 1);
                close(outputFile);
            }

            // Check if a buitin command must be run, and run it
            int builtin = builtinCommand(paramList);
            if (builtin == 2) {
                return 1;
            } else if (builtin == 1) {
                return 0;
            }

            // Search for the executable
            paramList[0] = getFullPath(paramList[0], PATH);
            if(paramList[0] == NULL) {
                printf("File not found or could not be executed\n");
                return 0;
            }

            // Set parent variable in child process
            getcwd(cwd, 100);
            strcat(cwd, "/Shell");
            setenv("parent", cwd, 1);

            // Exec the child process
            execvp(paramList[0], paramList);
        } else {
            // Parent runs this code
            // Close any file descriptors
            if (inputFile != NULL) {
                close(inputFile);
                inputFile = 0;
            }
            if (outputFile != NULL) {
                close(outputFile);
                outputFile = 0;
            }
            //Check the return code from the child, if it is 1 then we know we have to quit.
            int returnCode = 0;
            waitpid(pid, &returnCode, 0);
            if(WEXITSTATUS(returnCode) == 1) {
                break;
            }
        }
    }
    return 0;
}

// Decode the input string
void decodeInput(char *input, char **paramList, int *inputFile, int *outputFile) {
    // Tokenize the input string, splitting on spaces and the trailing newline
    char *token = strtok(input, " \n");
    // Assume 4 parameters
    int paramLen = 4;

    int inputFound = 0, outputFound = 0;

    // Token loop
    int i = 0;
    while (token != NULL) {
        // Detect redirect symbols
        if (strcmp(token, "<") == 0) {
            inputFound = 1;
            continue;
        }
        if (strcmp(token, ">") == 0) {
            outputFound = 1;
            continue;
        }
        if (strcmp(token, ">>") == 0) {
            outputFound = 2;
            continue;
        }

        // If redirect symbols are found, set up the file descriptor for redirection
        if (inputFound == 1) {
            *inputFile = open(token, O_RDONLY);
            inputFound = 0;
            continue;
        }
        if (outputFound == 1) {
            *outputFile = open(token, O_WRONLY|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
            outputFound = 0;
            continue;
        }
        if (outputFound == 2) {
            *outputFile = open(token, O_WRONLY|O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);
            outputFound = 0;
            continue;
        }

        // Set current token as the ith parameter
        paramList[i] = token;
        // Get next token
        token = strtok(NULL, " \n");
        i++;
        // If the end of the list has been reached, increase its size by two
        if (i == paramLen) {
            paramLen += 2;
            paramList = (char **)realloc(paramList, paramLen * sizeof(char *));
        }
    }
    // We no longer need the token
    free(token);
    // Set the last parameter to NULL
    paramList[i] = NULL;
}

// Get full path to an executable
char *getFullPath(char *file, char **PATH) {
    // Check if file exists in this directory
    if(access(file, X_OK) != -1) {
        return file;
    }

    // Search each directory in the PATH variable for where the file is located, and if found, return it
    int fileLen = (int) strlen(file);
    char *fullpath = NULL;
    for (int i = 0; PATH[i] != NULL; i++) {
        fullpath = (char *)malloc((strlen(PATH[i]) + fileLen + 1) * sizeof(char));
        strcpy(fullpath, PATH[i]);
        strcat(fullpath, "/");
        strcat(fullpath, file);
        if(access(fullpath, X_OK) != -1) {
            return fullpath;
        }
        free(fullpath);
    }
    // File was never found, so return NULL
    return NULL;
}

// Handle builtin commands. If a command is run, it prints 1, otherwise 0
int builtinCommand(char **paramList) {
    if (strcmp(paramList[0], "quit") == 0) {
        // Quit should return 2, so that the shell knows to quit
        printf("Goodbye!\n");
        return 2;
    } else if (strcmp(paramList[0], "cd") == 0) {
        // Change directory, and print a response if it can't
        if (chdir(paramList[1]) == -1) {
            printf("Directory not found\n");
        }
        return 1;
    } else if (strcmp(paramList[0], "dir") == 0) {
        // Print contents of given directory
        char *loc = paramList[1];
        if (loc == NULL) {
            // If no directory was given, use the current one
            loc = ".";
        }
        printf("%s\n", loc);
        DIR *d;
        struct dirent *dir;
        d = opendir(loc);
        if (d) {
            // Loop through the directory and print the name of everything it finds
            while ((dir = readdir(d)) != NULL) {
                printf("%s\n", dir->d_name);
            }
            closedir(d);
        }
        return 1;
    } else if (strcmp(paramList[0], "clr") == 0) {
        // Print an ANSI escape code to clear the screen
        printf("\e[2J");
        return 1;
    } else if (strcmp(paramList[0], "environ") == 0) {
        // Loop through the environment variables and print them
        for (int i = 0; environ[i] != NULL; i++) {
            printf("%s\n", environ[i]);
        }
        return 1;
    } else if (strcmp(paramList[0], "echo") == 0) {
        // Print nothing if no params passed
        if (paramList[1] == NULL) {
            printf("\n");
            return 1;
        }
        // Replace the null character between parameters with spaces
        for (int i = 1; paramList[i] != NULL; i++) {
            paramList[i][strlen(paramList[i])] = ' ';
        }
        // print the params
        printf("%s\n", paramList[1]);
        return 1;
    }
    else if (strcmp(paramList[0], "pause") == 0) {
        printf("Press enter to continue\n");
        // Loop until enter is pressed
        char c = (char) getchar();
        while (c != '\n') {
            c = (char) getchar();
        }
        return 1;
    }
    else if (strcmp(paramList[0], "help") == 0) {
        // print contents of help file
        int c;
        FILE *file;
        file = fopen("readme", "r");
        if (file) {
            while ((c = getc(file)) != EOF)
                putchar(c);
            fclose(file);
        }
        return 1;
    }
    return 0;
}

// Reads a line of input
char *readLine() {
    // Get a line from stdin and return it
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

// Sets up the PATH variable
char **initPath() {
    // Get the PATH variable
    char *rawPath = getenv("PATH");
    int pathlen = 4;
    // Set up an array to read it into
    char **PATH = (char **)malloc(pathlen * sizeof(char *));
    // Tokenize it
    char *token = strtok(rawPath, ":");

    int i = 0;
    while (token != NULL) {
        // Store current token in array
        PATH[i] = token;
        token = strtok(NULL, ":");
        i++;
        // Increaze size if necessary
        if (i == pathlen) {
            pathlen += 2;
            PATH = realloc(PATH, pathlen * sizeof(char *));
        }
    }
    // Token no longer needed
    free(token);
    // Set last item in array to NULL
    PATH[i] = NULL;
    // Return the array
    return PATH;
}