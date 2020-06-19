#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "linenoise.h"

int Shell_Variable(char **s);
char* substring(const char *source, int first_char, int last_char);

#define MAX_ARGS 255

struct vars 
{
    char variable[200];
    int var_index;
};

int main(int argc, char** argv)
{

   char *line, *token = NULL, *args[MAX_ARGS];
   int tokenIndex;
   //char *token2 = *args[MAX_ARGS];

    while ((line = linenoise("eggshell-1.0> ")) != NULL)
    {
        if(strstr(line, "echo") != NULL && strstr(line, "$") != NULL)
        {
            printf("Line contains echo.\n");
            printf("%s\n", line);
            printf("%zu\n", strlen(line));
            char* substr = substring(line, 6, strlen(line));

            token = strtok(line, " "); 
            //token2 = strtok(NULL, " ");
            
            printf("sub-string: %s\n", substr);
            //printf("%s\n", token);

            //printf("Token 1: %s\n", token); //checking that the term entered by user is being extracted
            Shell_Variable(&substr);
            token = strtok(NULL, " ");
            //printf("Token 2: %s\n", token);
        }
        else
        {
            //printf("Line does not contain echo.\n");
            token = strtok(NULL, " ");
            //printf("%s", token);
        }

        linenoiseFree(line);
    }
   return(0);
}

int Shell_Variable(char **s)
{

    struct vars path;
    struct vars home;
    struct vars cwd;
    struct vars user;
    struct vars shell;
    struct vars terminal;

    path.var_index = "1";
    strcpy(path.variable, "PATH");
    home.var_index = "2";
    strcpy(home.variable, "HOME");
    cwd.var_index = "3";
    strcpy(cwd.variable, "CWD");
    user.var_index = "4";
    strcpy(user.variable, "USER");
    shell.var_index = "5";
    strcpy(shell.variable, "SHELL");
    terminal.var_index = "6";
    strcpy(terminal.variable, "TERMINAL");

    if (strcmp(*s, path.variable)==0)
    {
        printf("PATH: %s\n", getenv("PATH"));
        return path.var_index;
    }
    else if(strcmp(*s, home.variable)==0)
    {
        printf("HOME: %s\n", getenv("HOME"));
        return home.var_index;
    }
    else if(strcmp(*s, cwd.variable)==0)
    {
        char cwd_[PATH_MAX];

        if(getcwd(cwd_, sizeof(cwd_)) != NULL)
        {
            printf("CWD: %s\n", cwd_);
            return cwd.var_index;
        }
        else
        {
             perror("Error obtaining current working directory.");
        }
    }
    else if(strcmp(*s, user.variable)==0)
    {
        printf("USERNAME: %s\n", getenv("USER"));
        return user.var_index;
    }
    else if(strcmp(*s, shell.variable)==0)
    {
       char buf[1024];
       ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
       buf[len] = '\0';

       printf("SHELL: %s", buf);
       return shell.var_index;
    }
    else if(strcmp(*s, terminal.variable)==0)
    {
        char *tty;
        tty = ttyname(fileno(stdin));

        if(tty != NULL)
        {
            printf("TERMINAL: %s\n", tty);
            return terminal.var_index;
        }
    }
    else if (strcmp(*s, "exit") == 0)
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