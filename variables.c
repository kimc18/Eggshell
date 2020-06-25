#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "linenoise.h"

void varInitializer();
void outputShellVar(char line[]);
char **string_to_array(char *input);
char* Shell_Variable(char s[]);
int indexOfWord(char string[], char findWord[]);
char* substring(const char *source, int first_char, int last_char);
char *replaceWord(const char *s, const char *oldW, const char *newW);

#define MAX_ARGS 255

struct vars 
{
    char variable[200];
};

int main(int argc, char** argv)
{

    char *line, *token = NULL, *args[MAX_ARGS];
    int tokenIndex;

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
        else if (strcmp(line, "exit") == 0)
        {
            exit(0);
        }
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
        else if (strcmp(line, "cd..") == 0)
        {
            chdir("..");
        }
        else if (cd != NULL)
        {
            char* substr = substring(line, 3, strlen(line)); //should take what comes after cd
            if(chdir(substr) != 0)
                perror("Unable to chnage directory");
        }
        else if (prompt != NULL)
        {
            int beginningOfWord = indexOfWord(line, needle_prompt);
            char* substr_prompt_till_end = substring(line, beginningOfWord, strlen(line));
            char* substr_prompt_input = substring(substr_prompt_till_end, 7, strlen(substr_prompt_till_end));
            setenv("PROMPT", substr_prompt_input, 0);
        }
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
       //ssize_t len = readlink("/proc/self/exe", &buf, sizeof(buf)-1);
       ssize_t len = readlink("/proc/self/exe", buf, 2024); //whilst debugging i found that line 204 would only allocate 7 bytes so i manually set it to 2024 just in case here
       buf[len] = '\0';
       free(buf);
       //printf("SHELL: %s", buf);
       setenv("SHELL", buf, 1);
       return getenv("SHELL");
    }
    else if(strcmp(s, terminal.variable)==0)
    {
        char *tty;
        tty = ttyname(fileno(stdin));

        if(tty != NULL)
        {
            //printf("TERMINAL: %s\n", tty);
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

char* substring(const char *source, int first_char, int last_char)
{
    int length = last_char - first_char; // gets the length of the string, when using func i think use size of string to get last char or something, ended up using strlen

    char *dest = (char*)malloc(sizeof(char) * (length + 1)); // allocates enough mem for string and adding mem for null char

    strncpy(dest, (source + first_char), length);

    return dest;
}

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

void outputShellVar(char line[])
{
    const char needle_user[100] = "$USER"; //i used needle cos of the finding needle in haystack thing
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
        char* substr = substring(line, 5, strlen(line));
        result = replaceWord(substr, needle_user, Shell_Variable("USER"));   
        printf("%s\n", result);
    }
    else if (home != NULL)
    {
        char* substr = substring(line, 5, strlen(line));
        result = replaceWord(substr, needle_home, Shell_Variable("HOME"));
        printf("%s\n", result);
    }
    else if (cwd != NULL)
    {
        char* substr = substring(line, 5, strlen(line));
        result = replaceWord(substr, needle_cwd, Shell_Variable("CWD"));
        printf("%s\n", result);
    }
    else if (shell != NULL)
    {   
        char* substr = substring(line, 5, strlen(line));
        result = replaceWord(substr, needle_shell, Shell_Variable("SHELL"));
        printf("%s\n", result);
    }
    else if (terminal != NULL)
    {
        char* substr = substring(line, 5, strlen(line));
        result = replaceWord(substr, needle_ter, Shell_Variable("TERMINAL"));
        printf("%s\n", result);
    }
    else if (path != NULL)
    {
        char* substr = substring(line, 5, strlen(line));
        result = replaceWord(substr, needle_path, Shell_Variable("PATH"));
        printf("%s\n", result);
    }
    else if (dollar != NULL)
    {
        char* substr = substring(line, 6, strlen(line));
        printf("%s\n", substr);
        result = replaceWord(substr, needle_dollar, getenv(substr));
        printf("%s\n", result);
    }
    else if (prompt != NULL && dollar != NULL)
    {
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



