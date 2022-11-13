// Brenan Marenger
// finding current dir: https://stackoverflow.com/questions/4807629/how-do-i-find-the-current-directory
// readline: https://en.wikipedia.org/wiki/GNU_Readline
// glob: https://man7.org/linux/man-pages/man3/glob.3.html
//wordexp: https://man7.org/linux/man-pages/man3/wordexp.3.html

/*
Working:
1 Knows how to change directory
2 Tab Completion and Arrow History
1 Can run an executable
2 Automatically runs a file called .myshell when it starts

NOTES:
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

    command; anotherCommand

    while forever
        for each semicolon
        doCommand(args)


    startup file (bash)

        // if >
        //  fd = open(w[0], O_CREAT | O_WRONGLY | O_TRUNC, 0664); O_ <- exclusive, file already exists
        // if error -> perror die
        // dup2(1, fd)
        // edit arg list
        //  ls -l > foo
        //  if <
        // fd = open(w[0], O_CREAT | O_RDONGLY, 0664) //append mode
        ////dup2(0, fd)
*/
#include <iostream>
#include <wordexp.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h> //TODO: correction
#include <glob.h>
#include <array>
#include <fstream>
#include <assert.h>
using namespace std;

string listOfCommands = "Run an executable(./), bang(!), change directory(cd)";

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

void runProgram(char **args) // runs an executable
{
    if (fork() == 0)
    {
        execvp(args[0], args);
        cout << "Error: File could not run." << endl;
    }
    else
    {
        int status;
        wait(&status);
        return;
    }
}

void globThis(const char *arg) // list command with -l
{
    if (fork() == 0)
    {
        glob_t globbuf;
        globbuf.gl_offs = 2;
        glob("*.c", GLOB_DOOFFS, NULL, &globbuf);
        //glob("../*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
        globbuf.gl_pathv[0] = "ls";

        if (!(strcmp(arg, "-l"))) // fix
        {
            cout << "Went here" << endl;
            globbuf.gl_pathv[1] = "-l";
            execvp("ls", &globbuf.gl_pathv[0]);
        }

        cout << "Error: list command could not run" << endl;
    }
    else
    {
        int status;
        wait(&status);
        return;
    }
}

bool is_number(string &s) //checks if input is a number
{
    return (strspn(s.c_str(), "123456789") == s.size());
}

void bangThis(string arg)// bang command (!)
{
    HISTORY_STATE *myhist = history_get_history_state();
    HIST_ENTRY **mylist = history_list();

    arg.erase(arg.begin()); // removes '!'

    if (is_number(arg))
    {
        if (fork() == 0)
        {
            int count = (myhist->length) - 1; // skipping the current !call
            int toNum = stoi(arg);
            char *temp = mylist[(count - toNum)]->line;
            cout << "History in spot " << arg << " is " << mylist[(count - toNum)]->line << endl;
            execvp(temp, &temp); // not working
            cout << "Error: Bang(!) could not run that command" << endl;
        }
        else
        {
            int status;
            wait(&status);
            free(myhist);
            free(mylist);
            return;
        }
    }
}

void runShell()
{
    while (1)
    {
        rl_bind_key('\t', rl_complete); // autofill with TAB
        cout << GetCurrentWorkingDir();
        char *input = readline("$");
        add_history(input);

        wordexp_t p;
        char **w;
        wordexp(input, &p, 0);
        w = p.we_wordv;
        string temp = w[0];
        
        //for each loop here (example; nextCommand; ..)
        if (!(strcmp(w[0], "cd"))) // change directory
        {
            changeDir(w[1]);
        }
        else if ((temp.rfind("./", 0)) == 0) // execute files
        {
            runProgram(w);
        }
        else if (!(strcmp(w[0], "ls"))) // list command
        {
            globThis(w[1]);
        }
        else if (!(strcmp(w[0], "exit")))
        {
            break;
        }
        else if ((temp.rfind("!", 0)) == 0)
        {
            bangThis(w[0]);
        }
        else if (!(strcmp(w[0], "help")))
        {
            cout << listOfCommands << endl;
        }
        else
        {
            cout << "Error: Command does not exist" << endl;
        }
        free(input);
        wordfree(&p);
    }
}

int main()
{
    ifstream is(".myshell");
    string line;
    while (getline(is, line)) {
        cout << line << endl;
    }

    runShell();
    return 0;
}