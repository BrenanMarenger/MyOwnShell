// Brenan Marenger

// finding current dir: https://stackoverflow.com/questions/4807629/how-do-i-find-the-current-directory
// readline: https://en.wikipedia.org/wiki/GNU_Readline
// https://web.mit.edu/gnu/doc/html/rlman_2.html
// readline history: https://stackoverflow.com/questions/38792542/readline-h-history-usage-in-c
// wordexp: https://man7.org/linux/man-pages/man3/wordexp.3.html
// pipe: https://linux.die.net/man/2/pipe
// waitpid: https://linux.die.net/man/2/waitpid

/*
Working: 25
1 Can run an executable.
1 Control-L clears the screen
1 Tab Completion
1 Arrow History
1 Knows how to change directory
2 Automatically runs a file called .myshell when it starts
1 Catch Keyboard interrupt
1 Change Prompt
3 Can do file output redirection ">"
3 Can do file input redirection "<"
2 Can run commands from a file (.filename)
1 Queue commands
1 Can have lots of semicolons
5 Can do command piping "|"
2 Can run commands in the background.
1 change what at least 2 keys do
--Ctr+p -> possible completions ex. $cd new(^p) -> newDir/  newFile
--ESC -> removes the first character of the input

1 Bang # command **works but bugs with 0, negative numbers**
*/

#include <iostream>
#include <wordexp.h>
#include <string>
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
#include <readline/history.h>
#include <array>
#include <fstream>
#include <signal.h>
#include <assert.h>
using namespace std;

void bangThis(string arg);
string GetCurrentWorkingDir();
void changeDir(const char *path);
int runShell(char *line[], int flag);
char **parse(char *input);
void outputRedir(string temp);
void inputRedir(string temp);
void pipeThis(string temp);
int escHandler(int count, int key);

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

bool is_number(string &s) // checks if input is a number
{
    return (strspn(s.c_str(), "1234567890") == s.size());
}

void bangThis(string arg) // bang command "!"
{
    arg.erase(arg.begin()); // removes '!'
    if (arg.find("-") != string::npos)
    {
        cout << "No negatives!" << endl;
        return;
    }
    int toNum;
    if (is_number(arg)) // if is a number
    {
        if ((toNum = stoi(arg)) > 0) // if given number is > 0
        {
            HISTORY_STATE *myhist = history_get_history_state();
            HIST_ENTRY **mylist = history_list();
            int count = (myhist->length) - 1;
            if (toNum > count)
            {
                cout << "Error, no history for " << toNum << endl;
                free(myhist);
                free(mylist);
                return;
            }
            else
            {
                char *temp = {mylist[(count - toNum)]->line};
                cout << mylist[(count - toNum)]->line << endl;
                free(myhist);
                free(mylist);
                runShell((parse(temp)), 0);
            }
        }
    }
    else
    {
        cout << "Not a valid number" << endl;
    }
}

int runShell(char **w, int flag)
{
    string temp = w[0];
    if (!(strcmp(w[0], "cd"))) // change directory
    {
        changeDir(w[1]);
        return 1;
    }
    else if (((temp.rfind(".", 0)) == 0) && !(temp.find("/") != string::npos))
    {
        temp.erase(temp.begin());
        cout << "Opening " << temp << "..." << endl;
        ifstream is(temp);
        if (!(is.is_open()))
        {
            cout << "File doesn't exist" << endl;
            return 1;
        }
        string line;
        char buffer[50];
        while (getline(is, line))
        {
            strcpy(buffer, line.c_str());
            runShell(parse(buffer), 0);
        }
        is.close();
        return 1;
    }
    else if ((temp.rfind("!", 0)) == 0) // bang
    {
        bangThis(w[0]);
        return 1;
    }
    else if (!(strcmp(w[0], "exit")))
    {
        cout << "Dying a slow and painful death..." << endl;
        return 0;
    }
    else
    {
        pid_t cpid;
        cpid = fork();
        if (cpid == 0)
        {
            execvp(w[0], w);
            cout << "Error: Command does not exist" << endl;
        }
        else
        {
            int ret;
            int status;
            if (flag == 1)
            {
                cout << "Running in backgroud" << endl; //&
                do
                {
                    ret = waitpid(cpid, &status, WNOHANG);
                } while (ret > 0);
            }
            else
            {
                waitpid(cpid, &status, WUNTRACED | WCONTINUED);
            }
        }
    }

    return 1;
}

void handle_sigint(int sig) // Ctr + C Catching
{
    cout << endl
         << "You don't do that, try typing exit" << endl;
}

void outputRedir(string temp) // Output Redirection ">"
{
    string cmdBuf;
    string file;

    for (int i = 0; i < temp.length(); i++)
    {
        if (temp[i] == '>')
        {
            break;
        }
        cmdBuf += temp[i];
    }
    for (int i = cmdBuf.length() + 1; i < temp.length(); i++)
    {
        if (temp[i] != ' ')
        {
            file += temp[i];
        }
    }
    // converting to correct params
    int n = cmdBuf.length() + 1;
    char command[n];
    strcpy(command, cmdBuf.c_str());
    const char *c = file.c_str();
    // open file, change stdout
    auto fd = open(c, O_CREAT | O_WRONLY | O_TRUNC, 0664);
    if (fd == -1)
    {
        cout << "error" << endl;
    }
    int stdoutCopy = dup(1);
    if (dup2(fd, 1) == -1)
    {
        cout << "error" << endl;
    }
    runShell(parse(command), 0);
    dup2(stdoutCopy, 1);
}

void inputRedir(string temp) // Input Redirection "<"
{
    string cmdBuf;
    string file;
    for (int i = 0; i < temp.length(); i++)
    {
        if (temp[i] == '<')
        {
            break;
        }
        cmdBuf += temp[i];
    }
    for (int i = cmdBuf.length() + 1; i < temp.length(); i++)
    {
        if (temp[i] != ' ')
        {
            file += temp[i];
        }
    }
    // converting to correct params
    int n = cmdBuf.length() + 1;
    char command[n];
    strcpy(command, cmdBuf.c_str());
    const char *c = file.c_str();
    // open file, change stdin
    auto fd = open(c, O_CREAT | O_RDONLY, 0664);

    if (fd == -1)
    {
        cout << "error" << endl;
    }
    int stdCopy = dup(1);
    if (dup2(fd, 0) == -1)
    {
        cout << "error" << endl;
    }
    runShell(parse(command), 0);
    dup2(stdCopy, 0);
}

void pipeThis(string temp) // Piping "|"
{
    string cmdBuf;
    string cmdBuf2;
    for (int i = 0; i < temp.length(); i++)
    {
        if (temp[i] == '|')
        {
            break;
        }
        cmdBuf += temp[i];
    }
    for (int i = cmdBuf.length() + 1; i < temp.length(); i++)
    {
        if (temp[i] != ' ')
        {
            cmdBuf2 += temp[i];
        }
    }
    cout << "Piping " << cmdBuf << " to " << cmdBuf2 << endl;
    int n = cmdBuf.length() + 1;
    char command[n];
    strcpy(command, cmdBuf.c_str());
    int x = cmdBuf2.length() + 1;
    char command2[x];
    strcpy(command2, cmdBuf2.c_str());
    // PIPE STUFF
    int stdoutCopy = dup(1);
    int stdinCopy = dup(0);

    if (fork() == 0)
    {
        int pipefd[2];
        pid_t cpid;
        int status;
        if (pipe(pipefd) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        cpid = fork(); // second fork
        if (cpid == 0)
        {
            // sub child
            close(pipefd[0]); // closed write end
            dup2(pipefd[1], 1);
            char **w = parse(command);
            execvp(w[0], w);
        }
        else
        {
            wait(&status);
            close(pipefd[1]); // closed write end
            dup2(pipefd[0], 0);
            char **w = parse(command2);
            execvp(w[0], w);
        }
    }
    else
    {
        int status;
        wait(&status);
        dup2(stdoutCopy, 1);
        dup2(stdinCopy, 0);
    }
}

char **parse(char *input) // Formatting input using wordexp
{
    string temp = input;

    char *args[7];
    int num = sscanf(input, "%ms %ms %ms %ms %ms %ms %ms", &args[0], &args[1], &args[2], &args[3], &args[4], &args[5], &args[6]);
    wordexp_t words;
    wordexp(args[0], &words, 0);
    for (int i = 1; i < num; i++)
    {
        wordexp(args[i], &words, WRDE_APPEND);
    }
    return words.we_wordv;
}

int escHandler(int count, int key)
{ // keybind handler for ESC
    cout << endl
         << GetCurrentWorkingDir()
         << endl;
    rl_delete_text(0, 1);
    return 0;
}

int main()
{
    ifstream is(".myshell");
    string line;
    char buffer[50];
    while (getline(is, line))
    {
        // line to char *
        strcpy(buffer, line.c_str());
        runShell(parse(buffer), 0);
    }
    is.close();

    char *input;
    char *prompt;
    rl_bind_key(27, escHandler);                      /* ascii code for ESC */
    rl_bind_keyseq("\\C-p", rl_possible_completions); // ctr-p
    while (1)
    {

        signal(SIGINT, handle_sigint);
        cout << GetCurrentWorkingDir();

        if (getenv("PS1") == NULL)
        {
            input = readline("$");
        }
        else
        {
            input = readline(getenv("PS1"));
        }

        add_history(input);
        string temp = input;
        string cmdBuf;

        if ((temp.find(';') != string::npos)) // Run Queue
        {
            for (int i = 0; i <= temp.length(); i++)
            {
                if (temp[i] == ';' || temp.length() == i)
                {
                    int n = cmdBuf.length() + 1;
                    char command[n];
                    strcpy(command, cmdBuf.c_str());
                    runShell(parse(command), 0);
                    cmdBuf.clear();
                    memset(command, 0, n);
                }
                else
                {
                    cmdBuf += temp[i];
                }
            }
        }
        else if ((temp.find("PS1") != string::npos)) // Prompt Changing
        {
            prompt = (char *)malloc(sizeof(char) * (temp.length() + 1));
            strcpy(prompt, temp.c_str());
            putenv(prompt);
        }
        else if ((temp.find("|") != string::npos)) // Piping
        {
            pipeThis(temp);
        }
        else if ((temp.find(">") != string::npos)) // Output redirection
        {
            outputRedir(temp);
        }
        else if ((temp.find("<") != string::npos)) // Input Redirection
        {
            inputRedir(temp);
        }
        else if ((temp.back() == '&'))
        {
            runShell(parse(input), 1);
        }
        else
        {
            if (strlen(input) != 0)
            {
                if (runShell((parse(input)), 0) == 0)
                {
                    return 0;
                }
            }
        }
        free(input);
    }
}