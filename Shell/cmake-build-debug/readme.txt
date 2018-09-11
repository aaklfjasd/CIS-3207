HOW TO USE THIS SHELL

Enter a command, and press enter.

This shell includes the following builtin commands:
cd - Changes the current directory to whatever is specified. The shell can only see files in its current directory.
clr - Clears the console output.
dir - Prints the contents of the current directory, or the directory specified.
environ - prints out all environment variables. These are variables that the shell knows and may be used by programs.
echo - prints out whatever you type
help - display this manual
pause - wait for enter to be pressed
quit - quit the shell

This shell also supports input and output redirection. If you type < filename or > filename, the stdin and stdout
of the shell is changed for the executed process. This allows input or output to come from or go to a file.

Executing this shell with a file as a command line argument will allow you to run a batch file. The file should contain a list
of commands to be executed.