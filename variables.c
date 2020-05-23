#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "linenoise.h"

int Shell_Variable(char **s);

#define MAX_ARGS 255

//used to create shell variable name and assign an index for it
struct vars 
{
    char variable[200];
    int var_index;
};

int main(void)
{

   char *line, *token = NULL, *args[MAX_ARGS];
   int tokenIndex;

    while ((line = linenoise("$ ")) != NULL)
    {
      token = strtok(line, " "); 

      while(token != NULL)
      {
        //printf("%s\n", token); //checking that the term entered by user is being extracted
      
        Shell_Variable(&token);
         
        token = strtok(NULL, " ");
      }

      linenoiseFree(line);
    }
   return(0);
}

//possible shell variables that can be called
int Shell_Variable(char **s)
{

    struct vars path;
    struct vars home;
    struct vars cwd;
    struct vars user;
    struct vars shell;
    struct vars terminal;

    //index is assigned and name is set using strcpy
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

    //code for functionality of variables
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
        char resolved_path[PATH_MAX];
        realpath("../../", resolved_path);

        char filename[] = "variables.c";
        char* path = realpath(filename, NULL);

        if(path == NULL)
        {
            printf("cannot find file with name[%s]\n", filename);
        } 
        else
        {
            printf("SHELL: %s\n", path);
            return shell.var_index;
            free(path);
        }   
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
    //used to exit program
    else if (strcmp(*s, "exit")==0)
    {
        exit(1);
    }
}
