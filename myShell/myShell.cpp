// Brenan Marenger
// finding current dir: https://stackoverflow.com/questions/4807629/how-do-i-find-the-current-directory
// readline: https://en.wikipedia.org/wiki/GNU_Readline

/*
Working:
1 Knows how to change directory
2 Tab Completion and Arrow History
1 Can run an executable

TODO:
file history

ls > foo.txt write ls to foo.txt creates foo.txt
ls > foo.txt -l
|           | ^only arg
dup2(what to change, where to go)

fd = open(w[0], O_CREAT | O_WRONGLY, 0664);
if bad
    perror
    exit

int ret = dup2(0, fd) standard out
-or-
int ret = dup2(1, fd) standard in
if error
    perror exit


*/
#include <iostream>
#include<sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h> //add correction
#include <glob.h>

using namespace std;

string GetCurrentWorkingDir() // returns current working directory
{
    std::string cwd("\0", FILENAME_MAX + 1);
    return getcwd(&cwd[0], cwd.capacity());
}

void changeDir(const char *path) // changes directory to given path
{
    if ((chdir(path)) == -1)
    {
        cout << "Error: Directory Doesn't Exist" << endl;
    }
}

void runProgram(char** args) //exec
{
    if (fork() == 0){
        execvp(args[0], args);
        cout << "Error: File could not run." << endl;
    } else{
        int status;
        wait(&status);
        return;
    }
}

void globThis(string path)
{
}

int main()
{
    char *args[10];
    rl_bind_key('\t', rl_complete); // autofill with TAB

    while (1)
    {
        cout << GetCurrentWorkingDir();
        char *input = readline("$");
        add_history(input); // adds input history
        int num = sscanf(input, "%ms %ms %ms", &args[0], &args[1], &args[2]);
        string program = args[0];

        if (!(strcmp(args[0], "cd")))
        {
            changeDir(args[1]);
        }
        else if ((program.rfind("./", 0)) == 0) // execute files
        {
            runProgram(args);
        }
        else if (!(strcmp(args[0], "ls")))
        {
            cout << "Displaying files in directory" << endl;
            globThis(GetCurrentWorkingDir());
        }
        else if (!(strcmp(args[0], "quit")))
        {
            break;
        }
        else
        {
            cout << "Error: Command does not exist" << endl;
        }
        free(input);
    }
    return 0;
}
