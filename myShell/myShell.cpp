// Brenan Marenger
// finding current dir: https://stackoverflow.com/questions/4807629/how-do-i-find-the-current-directory
// readline: https://en.wikipedia.org/wiki/GNU_Readline

/*
Working:
1 Knows how to change directory
2 Tab Completion and Arrow History
1 Can run an executable

*/
#include <iostream>
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
            execvp(args[0], args);
            cout << "Error: File could not run." << endl;
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