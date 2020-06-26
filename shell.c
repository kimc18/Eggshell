#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "linenoise.h"

//functions
void varInitializer();
void outputShellVar(char line[]);
char **string_to_array(char *input);
char* Shell_Variable(char s[]);
int indexOfWord(char string[], char findWord[]);
char* substring(const char *source, int first_char, int last_char);
char *replaceWord(const char *s, const char *oldW, const char *newW);
void print_var(char line[], const char needle[], char shellVar[], int first_char);

#define MAX_ARGS 255

//was going to use this struct to give shell variables an index
struct vars 
{
    char variable[200];
};

int main(int argc, char** argv)
{

    char *line;
    char *args[MAX_ARGS];

    varInitializer();

    char **command, **command2, **command3;
    char *input;
    pid_t child_pid;
    int stat_loc;

    while ((line = linenoise(getenv("PROMPT"))) != NULL)
    {
        const char needle_cd[10] = "cd";
        const char needle_prompt[100] = "$PROMPT";
        const char needle_pipe[100] = "|";
        char *cd, *prompt, *pipe_cmd;

        cd = strstr(line, needle_cd);
        prompt = strstr(line, needle_prompt);
        pipe_cmd = strstr(line, needle_pipe);

        int p[2];
        int pid, r;

        if(strstr(line, "echo") != NULL)
        {
            //i used needle cos of the finding needle in haystack thing
            const char needle_home[100] = "$HOME";
            const char needle_cwd[100] = "$CWD";
            const char needle_shell[100] = "$SHELL";
            const char needle_ter[100] = "$TERMINAL";
            const char needle_echo[100] = "echo";
            const char needle_path[100] = "$PATH";
            const char needle_prompt[100] = "$PROMPT";
            const char needle_dollar[100] = "$";
            //i dont get why i still need to keep these here
            //i was trying to refactor but for some reason code won't work without these here
            char *echo;

            echo = strstr(line, needle_echo);

            if (echo != NULL)
            {
                outputShellVar(line);
            }
        }
        //used to set a variable, did not work as intended
        else if (strstr(line, "=") != NULL)
        {
            const char deli[] = "=";
            char *token3, *token4;

            token3 = strtok(line, deli);
            //printf("String before equal sign: %s\n", token3);
            token4 = strtok(NULL, " ");
            //printf("String after equal sign: %s\n", token4);

            setenv(token3, token4, 1);
        }
        //exits out of program
        else if (strcmp(line, "exit") == 0)
        {
            exit(0);
        }
        prints shell variables
        else if (strcmp(line, "showenv") == 0)
        {
            printf("PATH=%s\n", Shell_Variable("PATH"));
            printf("PROMPT=%s\n", Shell_Variable("PROMPT"));
            printf("SHELL=%s\n", Shell_Variable("SHELL"));
            printf("USER=%s\n", Shell_Variable("USER"));
            printf("HOME=%s\n", Shell_Variable("HOME"));
            printf("CWD=%s\n", Shell_Variable("CWD"));
            printf("TERMINAL=%s\n", Shell_Variable("TERMINAL"));
        }
        //changes directory
        else if (strcmp(line, "cd..") == 0)
        {
            chdir("..");
        }
        //checks if cd is present in string then takes substring after it and sets it as new directory
        else if (cd != NULL)
        {
            char* substr = substring(line, 3, strlen(line)); //should take what comes after cd
            if(chdir(substr) != 0)
                perror("Unable to chnage directory");
        }
        //sets the prompts line
        else if (prompt != NULL)
        {
            int beginningOfWord = indexOfWord(line, needle_prompt);
            char* substr_prompt_till_end = substring(line, beginningOfWord, strlen(line));
            char* substr_prompt_input = substring(substr_prompt_till_end, 7, strlen(substr_prompt_till_end));
            setenv("PROMPT", substr_prompt_input, 0);
        }
        //non-working pipe operator
        else if (pipe_cmd != NULL)
        {
            const char deli[] = "|";
            //char *token3, *token4;

            command2 = string_to_array(strtok(line, deli));
            //printf("String before equal sign: %s\n", token3);
            command3 = string_to_array(strtok(NULL, " "));
            //printf("String after equal sign: %s\n", token4);
            
            pipe(p);

            if(!fork())
            {
                close(1);
                dup(p[1]);
                close(p[0]);
                execvp(command2[0], command2);
            }
            else
            {
                close(0);
                dup(p[0]);
                close(p[1]);
                execvp(command3[0], command3);
            }
        }
        //as eg. ls -a would be a stand alone command then it was set as the else in this statement, external commands can be processed here
        else
        {
            command = string_to_array(line);
            child_pid = fork();

            if(child_pid == 0)
            {
                execvp(command[0], command);
                printf("The command entered is invalid.\n");
                return;
            }
            else
            {
                waitpid(child_pid, &stat_loc, WUNTRACED);
            }
        }

        free(command);
        linenoiseFree(line);
    }
   return(0);
}

//function to get shell variables
char* Shell_Variable(char s[])
{

    struct vars path;
    struct vars home;
    struct vars cwd;
    struct vars user;
    struct vars shell;
    struct vars terminal;
    struct vars prompt;

    strcpy(path.variable, "PATH");
    strcpy(home.variable, "HOME");
    strcpy(cwd.variable, "CWD");
    strcpy(user.variable, "USER");
    strcpy(shell.variable, "SHELL");
    strcpy(terminal.variable, "TERMINAL");
    strcpy(prompt.variable, "PROMPT");

    if (strcmp(s, path.variable)==0)
    {
        return getenv("PATH");
    }
    else if(strcmp(s, home.variable)==0)
    {
        return getenv("HOME");
    }
    else if(strcmp(s, cwd.variable)==0)
    {
        char cwd_[PATH_MAX];

        if(getcwd(cwd_, sizeof(cwd_)) != NULL)
        {
            return getcwd(cwd_, sizeof(cwd));
        }
       /* else
        {
            return perror("Error obtaining current working directory.");
        }*/
    }
    else if(strcmp(s, user.variable)==0)
    {
        return getenv("USER");
    }
    else if(strcmp(s, shell.variable)==0)
    {
       char* buf = (char*)malloc(sizeof(char)*(2025)); //since it was returning an address before using malloc i used malloc to allocate mem just for this, then i freed it ofc
       ssize_t len = readlink("/proc/self/exe", buf, 2024); //whilst debugging i found that line 204 would only allocate 7 bytes so i manually set it to 2024 just in case here
       buf[len] = '\0';
       free(buf);
       setenv("SHELL", buf, 1);
       return getenv("SHELL");
    }
    else if(strcmp(s, terminal.variable)==0)
    {
        char *tty;
        tty = ttyname(fileno(stdin));

        if(tty != NULL)
        {
            return tty;
        }
    }
    else if(strcmp(s, prompt.variable) == 0)
    {
        return getenv("PROMPT");
    }
    else if (strcmp(s, "exit") == 0)
    {
        exit(1);
    }
}

//mainly used to find a word in a given string
char* substring(const char *source, int first_char, int last_char)
{
    int length = last_char - first_char; // gets the length of the string, when using func i think use size of string to get last char or something, ended up using strlen

    char *dest = (char*)malloc(sizeof(char) * (length + 1)); // allocates enough mem for string and adding mem for null char

    strncpy(dest, (source + first_char), length);

    return dest;
}

//used to replace words
char *replaceWord(const char *s, const char *oldW, const char *newW)
{
    char *result;
    int i, count = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    //will count how many times the old word appears
    for (i = 0; s[i] != '\0'; i++)
    {
        if(strstr(&s[i], oldW) == &s[i])
        {
            count++;
            
            //will jump to index of old word
            i += oldWlen - 1;
        }
    }

    //make new string with enough lenght
    result = (char *)malloc(i + count * (newWlen - oldWlen) + 1);

    i = 0;

    while(*s)
    {
        //will compare the substring with the result
        if(strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else 
        {
            result[i++] = *s++;
        }
    }

    result[i] = '\0';
    return result;
}

//initializes shell variables using the function Shell_Variable
void varInitializer()
{
    setenv("PROMPT", "eggshell-1.0> ", 1);
    setenv("PATH", Shell_Variable("PATH"), 1);
    setenv("CWD", Shell_Variable("CWD"), 1);
    setenv("USER", Shell_Variable("USER"), 1);
    setenv("HOME", Shell_Variable("HOME"), 1);
    setenv("SHELL", Shell_Variable("SHELL"), 1);
    setenv("TERMINAL", Shell_Variable("TERMINAL"), 1);
}

//was used to find the position of the first character in a given word
int indexOfWord(char line[], char wordSearch[])
{
    int i, j, flag;

    for (i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == wordSearch[0])
        {
            flag = 1;
            for (j = 0; wordSearch[j] != '\0'; j++)
            {
                if (line[i + j] != wordSearch[j])
                {
                    flag = 0;
                    break;
                }
            }
        }
        if (flag == 1)
        {
            break;
        }
    }

    if(flag == 0)
    {
        printf("Error PROMPT cannot be changed.\n");
        return 0;
    }
    else
    {
        return i+1; 
    }
}

//used to convert the input from linenoise into array so that it can be used in execvp
char **string_to_array(char *line)
{
    char **command = malloc(8 * sizeof(char *));
    char *sep = " ";
    char *parsed;
    int i = 0;

    parsed = strtok(line, sep);
    while(parsed != NULL)
    {
        command[i] = parsed;
        i++;
        parsed = strtok(NULL, sep);
    }

    command[i] = NULL;

    return command;
}

//used to print shell variables
void outputShellVar(char line[])
{
    //the word needle was used as I kept the convention of finding the needle in the hay stack
    const char needle_user[100] = "$USER"; 
    const char needle_home[100] = "$HOME";
    const char needle_cwd[100] = "$CWD";
    const char needle_shell[100] = "$SHELL";
    const char needle_ter[100] = "$TERMINAL";
    const char needle_echo[100] = "echo";
    const char needle_path[100] = "$PATH";
    const char needle_prompt[100] = "$PROMPT";
    const char needle_dollar[100] = "$";
    char *result = NULL;
    char *user, *echo, *home, *cwd, *shell, *terminal, *path, *prompt, *dollar;

    user = strstr(line, needle_user);
    echo = strstr(line, needle_echo);
    home = strstr(line, needle_home);
    cwd = strstr(line, needle_cwd);
    shell = strstr(line, needle_shell);
    terminal = strstr(line, needle_ter);
    path = strstr(line, needle_path);
    prompt = strstr(line, needle_prompt);
    dollar = strstr(line, needle_dollar);

    if(user != NULL)
    {
        print_var(line, needle_user, "USER", 5);
    }
    else if (home != NULL)
    {
        print_var(line, needle_home, "HOME", 5);
    }
    else if (cwd != NULL)
    {
        print_var(line, needle_cwd, "CWD", 5);
    }
    else if (shell != NULL)
    {
        print_var(line, needle_shell, "SHELL", 5);
    }
    else if (terminal != NULL)
    {
        print_var(line, needle_ter, "TERMINAL", 5);
    }
    else if (path != NULL)
    {
        print_var(line, needle_path, "PATH", 5);
    }
    else if (dollar != NULL)
    {
        print_var(line, needle_home, "HOME", 5);
    }
    else if (prompt != NULL && dollar != NULL)
    {
        //tried to print prompt, however my logic seems to be incorrect here
        char* substr = substring(line, 5, strlen(line));
        printf("substr = %s\n", substr);
        result = replaceWord(substr, needle_prompt, Shell_Variable("PROMPT"));
        printf("result: %s\n", result); 
        printf("%s\n", getenv("PROMPT"));
    }
    else
    {
        //for just echoing out what comes after echo
        char* substr2 = substring(line, 5, strlen(line));
        printf("%s\n",substr2);
        //printf("Line does not contain echo.\n");
        //token = strtok(NULL, " ");
        //printf("%s", token);
    }

    //my program is not able to create its own shell variables
}

//refactored code so as to not copy and paste same thing multiple times
//this code is used to replace eg. $USER with the actual username
void print_var(char line[], const char needle[], char shellVar[], int first_char)
{
    char* result = NULL;
    char* substr = substring(line, first_char, strlen(line));
    result = replaceWord(substr, needle, Shell_Variable(shellVar));
    printf("%s\n", result);
}