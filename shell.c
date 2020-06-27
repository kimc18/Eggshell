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

    char **cmd1, **cmd2, **cmd3;
    pid_t child_pid;
    int stat_loc;

    while ((line = linenoise(getenv("PROMPT"))) != NULL)
    {
        const char needle_cd[10] = "cd";
        const char needle_prompt[100] = "$PROMPT";
        const char needle_pipe[100] = "|";
        const char needle_unset[100] = "unset";
        char *cd, *prompt, *pipe_cmd, *unset;

        cd = strstr(line, needle_cd);
        prompt = strstr(line, needle_prompt);
        pipe_cmd = strstr(line, needle_pipe);
        unset = strstr(line, needle_unset);

        //error checking
        int index_equal = indexOfWord(line, "=");
        int index_pipe = indexOfWord(line, "|");

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
        
        //unset
        else if (unset != NULL)
        {
            char *token, *token_unset;

            token = strtok(line, " "); //holds unset
            token_unset = strtok(NULL, " "); //holds the var to unset

            //unsetenv(token_unset);

            if(unsetenv(token_unset) < 0)
            {
                printf("Unable to unset the variable.\n");
            }
        }

        //used to set a variable, did not work as intended
        else if (strstr(line, "=") != NULL)
        {
            const char deli[] = "=";
            char *token3, *token4;

            //checking for location of equal sign, it shouldnt be the first char in the line inputted by user
            if (index_equal > 1)
            {
                token3 = strtok(line, deli);
                token4 = strtok(NULL, " ");

                char* substr = substring(token3, 0, strlen(token3));

                setenv(substr, token4, 1);
            }
            else
            {
                printf("Incorrect command, to set variable follow the example: VARIABLE=var.\n");
            }
        }

        //exits out of program
        else if (strcmp(line, "exit") == 0)
        {
            exit(0);
        }

        //prints shell variables
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
                perror("Unable to change directory");
        }
        //sets the prompts line
        else if (prompt != NULL)
        {
            int beginningOfWord = indexOfWord(line, needle_prompt);
            char* substr_prompt_till_end = substring(line, beginningOfWord, strlen(line));
            char* substr_prompt_input = substring(substr_prompt_till_end, 7, strlen(substr_prompt_till_end));
            setenv("PROMPT", substr_prompt_input, 0);
        }
        //this is only for a two command pipe
        //checks
        else if (pipe_cmd != NULL)
        {
            const char deli[] = "|";
            char *token3, *token4;

            token3 = strtok(line, deli);
            token4 = strtok(NULL, "");

            cmd2 = string_to_array(token3);
            cmd3 = string_to_array(token4);
            
            int fd[2];

            //checking for location of pipe operator sign, it shouldnt be the first char in the line inputted by user
            if (index_pipe > 1)
            {
                if(pipe(fd) == -1)
	            {
		            perror("Pipe failed.");
		            exit(1);
	            }

	            if(fork() == 0)
	            {
		            close(STDOUT_FILENO);
		            dup(fd[1]);
		            close(fd[0]);
		            close(fd[1]);

		            execvp(cmd2[0], cmd2);
		            perror("First command failed.");
		            exit(1);
	            }
	            if(fork() == 0)
	            {
		            close(STDIN_FILENO);
		            dup(fd[0]);
		            close(fd[1]);
		            close(fd[0]);

		            execvp(cmd3[0], cmd3);
		            perror("Second command failed");
		            exit(1);
	            }
            }
            else
            {
                printf("Incorrect command, to pipe commands follow the example: command|command.\n");
            }

	        close(fd[0]);
	        close(fd[1]);
	        wait(0);
	        wait(0);

        }
        //as eg. ls -a would be a stand alone command then it was set as the else in this statement, external commands can be processed here
        else
        {
            cmd1 = string_to_array(line);
            child_pid = fork();

            if(child_pid == 0)
            {
                execvp(cmd1[0], cmd1);
                printf("The command entered is invalid.\n");
                return;
            }
            else
            {
                waitpid(child_pid, &stat_loc, WUNTRACED);
            }
        }

        free(cmd3);
        free(cmd2);
        free(cmd1);
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
        else
        {
            return "Error getting current directory.\n";
        }
    }
    else if(strcmp(s, user.variable)==0)
    {
        return getenv("USER");
    }
    else if(strcmp(s, shell.variable)==0)
    {
       char* buf = (char*)malloc(sizeof(char)*(2025)); //since it was returning an address before using malloc i used malloc to allocate mem just for this, then i freed it ofc
       ssize_t len = readlink("/proc/self/exe", buf, 2024); //whilst debugging i found that line above would only allocate 7 bytes so i manually set it to 2024 just in case here
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
char *replaceWord(const char *line, const char *old, const char *new)
{
    char *result;
    int i, count = 0;
    int newLen = strlen(new);
    int oldLen = strlen(old);

    //will count how many times the old word appears
    for (i = 0; line[i] != '\0'; i++)
    {
        if(strstr(&line[i], old) == &line[i])
        {
            count++;
            
            i += oldLen - 1;
        }
    }

    //make new string with enough length
    result = (char *)malloc(i + count * (newLen - oldLen) + 1);

    i = 0;

    while(*line)
    {
        //will compare the substring with the result
        if(strstr(line, old) == line)
        {
            strcpy(&result[i], new);
            i += newLen;
            line += oldLen;
        }
        else 
        {
            result[i++] = *line++;
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
    const char needle_path[100] = "$PATH";
    const char needle_prompt[100] = "$PROMPT";
    const char needle_dollar[100] = "$";
    char *result = NULL;
    char *user, *home, *cwd, *shell, *terminal, *path, *prompt, *dollar;

    user = strstr(line, needle_user);
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
    else if (prompt != NULL)
    {
        print_var(line, needle_prompt, "PROMPT",5);
    }
    else if (dollar != NULL)
    {
        char *token, *token2;
        char* result = NULL;
	    //to find index of $
	    int dollar = indexOfWord(line, "$");
        
	    char* substr = substring(line, dollar, strlen(line));//gets MYVAR
        char* substr2 = substring(line, dollar - 1, strlen(line)); //gets $MYVAR

	    token = strtok(substr, " ");//holds MYVAR
        token2 = strtok(substr2, " ");//holds $MYVAR
        
        char* substr3 = substring(line, 5, strlen(line));
        if(getenv(token) != NULL)
        {
            result = replaceWord(substr3, token2, getenv(token));
            printf("%s\n", result); //segmentation fault if print_var() is used so I had to use this
        }
        else
        {
            printf("Variable does not exist.\n");
        }
    }
    else
    {
        //for just echoing out what comes after echo
        char* substr2 = substring(line, 5, strlen(line));
        printf("%s\n",substr2);
    }
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