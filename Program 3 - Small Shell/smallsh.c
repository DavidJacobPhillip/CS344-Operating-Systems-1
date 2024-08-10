/* Unable to run this shell completely on personal computer */
/* Run this on OS1 server for full functionality */

/*
Author:         Santosh Ramesh
Email:          rameshsa@oregonstate.edu
Date:           2-3-21
Description:    This program creates a shell called "small shell" with similar functionality to bash
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h> 
#include <sys/wait.h> 
#include <fcntl.h>
#include <signal.h>

/* GLOBAL VARIABLES */
int enable_background = 0;                      // variable that keeps track of whether background processes should or shouldn't be enabled

/* STRUCT */
// Description: holds all of the command input's information
struct input{
    char *command;
    char **args;             
    int num_args;                               // number of arguments
    int in_exist;                               // used to identify if an input redirection exists (0 if false, 1 if true)
    int out_exist;                              // used to identify if an output redirection exists (0 if false, 1 if true)
    char *input;
    char *output;
    int back;                                   // identifies whether process is background or foreground (0 if false, 1 if true)
    int ignore;                                 // used if the input is a comment or a space
}; 

/* STRUCT */
// Description: holds the various background processes that are existing
struct back_process{
    int pid;
    struct back_process *next;                  // holds pointer to the next background process
}; 

/* FUNCTION */
// Description: this function is a handler for "SIGTSTP"
// Note: uses information from exploration "Signal handling API"
void handle_SIGTSTP(int signo){
    char* enable_msg = "Entering foreground-only mode (& is now ignored)\n: ";
    char* disable_msg = "Exiting foreground-only mode\n: ";
    // if SIGSTP is called and background processes are currently enabled (this will disable them)
    if (enable_background == 0){
        enable_background = 1;
        write(STDOUT_FILENO, enable_msg, 51);
        fflush(stdout);                         // Flushing stdout
    } 
    
    // if SIGSTP is called and background processes are currently disabled (this will enable them)
    else{
        enable_background = 0;
        write(STDOUT_FILENO, disable_msg, 31);
        fflush(stdout);                         // Flushing stdout
    }
}

/* FUNCTION */
// Description: sets an intialized value for several of the arguments in the input struct
void initializer(char* recieve, struct input* ready){
    // initializing string used for inputs
    strcpy(recieve, "");

    // initializing struct used for inputs
    ready->command = NULL;
    ready->args = NULL;
    ready->num_args = 0;
    ready->in_exist = 0;
    ready->out_exist = 0;
    ready->input = NULL;
    ready->output = NULL;
    ready->back = 0;
    ready->ignore = 0;
}

/* FUNCTION */
// Description: frees the input struct's dynamically allocated memory
void input_freer(struct input* ready){
    free(ready->command);
    free(ready->input);
    free(ready->output);
    for(int i = 0; i < ready->num_args; i++){
        free(ready->args[i]);
    }
    free(ready->args);
}

/* FUNCTION */
// Description: frees the background struct's dynamically allocated memory
void background_freer(struct back_process* back){
    struct back_process* current = back;
    struct back_process* previous = current;
    while(current != NULL){
        previous = current;
        current = current->next;
        free(previous);
    }
}


/* FUNCTION */
// Description: expands all "$$" characters in a string to be the PID
void expander(char* recieve){
    // temporary strings used for modifying input
    char temporary[2048] = "";
    char adder[2];
    adder[1] = '\0';

    // obtaining PID and storing it as a string
    int pid_int = getpid();
    char pid_str[11];
    sprintf(pid_str, "%i", pid_int);

    // looping through the input to find the "$$"
    for(int i = 0; i < strlen(recieve); i++){
        if(i + 1 < strlen(recieve)){

          // copying over a character to temporary array if not "$$"
          if(recieve[i] != '$' || recieve[i+1] != '$'){
            adder[0] = recieve[i];
            strcat(temporary, adder);
          } else{

            // copying the pid if "$$"
            strcat(temporary, pid_str);
            i = i + 1;
          }
        } else{

          // edge case for last character
          adder[0] = recieve[i];
          strcat(temporary, adder);
        }
    }
    
    // copying temporary array back into "recieve" input
    strcpy(recieve, temporary);
}

/* FUNCTION */
// Description: goes through the input recieved by the terminal and parses it into a struct
void parser(char* recieve, struct input* ready){
    char parsing[2048];                         // stores the pre-parsed string
    strcpy(parsing, recieve);
    int counter = 1;
    char *token;

    /* ignoring parsing function if there is no input */
    if(strlen(recieve) == 0){
        ready->ignore = 1;
        return;
    }

    /* ignoring parsing function if first character is a hashtag */
    if(recieve[0] == '#'){
        ready->ignore = 1;
        return;
    }

    /* removing all extraneous spaces at the end of the input */
    int space = strlen(recieve) - 1;
    while(space){
        if (recieve[space] == ' '){
            memset(parsing,0,strlen(parsing));  // setting parsing to all zeros
            parsing[space] == 'a';
            strncpy(parsing, recieve, space);
            space--;
        } else{
            space = 0;
        }
    }

    strcpy(recieve, parsing);                   // resets recieve based on removed spaces
    expander(recieve);                          // passes in "recieve" to expand out all "$$" variables

    /* determining the number of arguments present */
    // incrementing when a space is found
    for (int i = 0; i < strlen(recieve); i++) {
        if (recieve[i] == ' '){
            counter++;
        }
    }
    ready->num_args = counter;
    
    /* placing first token into struct input's command by splitting string */
    token = strtok(recieve, " ");
    ready->command = (char*)malloc((strlen(token) + 1) * sizeof(char));
    strcpy(ready->command, token);
    
    /* placing all arguments into args */
    // mallocing enough space for the double pointer
    ready->args = malloc(counter * sizeof(char*));

    // placing elements into args
    for(int i = 0; i < counter; i++){
        ready->args[i] = malloc((strlen(token) + 1) * sizeof(char));
        strcpy(ready->args[i], token);
        token = strtok(NULL, " ");
    }

    /* looking through arguments to see if there is input/output redirection */
    for(int i = 0; i < counter; i++){
        // if input is found
        if(strcmp(ready->args[i], "<") == 0){
            ready->in_exist = 1;
            ready->input = malloc((strlen(ready->args[i + 1]) + 1) * sizeof(char));
            strcpy(ready->input, ready->args[i + 1]);
        }

        // if output is found
        if(strcmp(ready->args[i], ">") == 0){
            ready->out_exist = 1;
            ready->output = malloc((strlen(ready->args[i + 1]) + 1) * sizeof(char));
            strcpy(ready->output, ready->args[i + 1]);
        }
    }

    /* determining if it is a background process */
    if(strcmp(ready->args[ready->num_args - 1], "&") == 0){
        // directing the input/output to be "/dev/null" if unspecified
        char back_direct[2048] = "/dev/null";
        ready->back = 1;
        if(ready->in_exist == 0){
            ready->in_exist = 1;
            ready->input = malloc(strlen(back_direct) * sizeof(char));
            strcpy(ready->input, back_direct);
        }
        if(ready->out_exist == 0){
            ready->out_exist = 1;
            ready->output = malloc(strlen(back_direct) * sizeof(char));
            strcpy(ready->output, back_direct);
        }
    }

    strcpy(recieve, parsing);                   // makes "recieve" back to what it was prior to parsing
}

/* FUNCTION */
// Description: prints everything in the input struct (used for testing purposes)
void printer(char* recieve, struct input* ready){
    printf("\n \n");
    printf("---------- \n");

    // printing pid
    printf("\n");
    printf("pid:  %i\n", getpid());
    printf("\n");

    // printing input struct information
    printf("recieve: %s \n", recieve);
    printf("command: '%s' \n", ready->command);
    printf("args: \n");
    if(ready->num_args != 0){
        for(int i = 1; i < (ready->num_args + 1); i++){
            printf("\t arg #%i: '%s' \n", i, ready->args[i - 1]);
        }
    }
    printf("num_args: '%i' \n", ready->num_args);
    printf("in_exist: '%i' \n", ready->in_exist);
    printf("out_exist: '%i' \n", ready->out_exist);
    printf("input: '%s' \n", ready->input);
    printf("output: '%s' \n", ready->output);
    printf("back: '%i' \n", ready->back);
    printf("ignore: '%i' \n", ready->ignore);

    printf("---------- \n \n");
}

/* FUNCTION */
// Description: redirects the input/output within child process
// Note: uses information from exploration "Processes and I/O"
void redirector(struct input* ready, int* status){
    int result;
    if(ready->in_exist){
        // Open target file
        int sourceFD = open(ready->input, O_RDONLY, 0666);
        if (sourceFD == -1) { 
            printf("cannot open %s for input \n", ready->input);
            exit(1); 
        }

        // Redirect stdin to source file
        result = dup2(sourceFD, 0);
        if (result == -1) { 
            perror("source dup2()"); 
            exit(1); 
        }
    }

    if(ready->out_exist){
        // Open target file
        int targetFD = open(ready->output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (targetFD == -1) { 
            printf("cannot open %s for output \n", ready->output);
            exit(1); 
        }

        // Redirect stdout to target file
        result = dup2(targetFD, 1);
        if (result == -1) { 
            perror("target dup2()"); 
            exit(1);  
        }
    }
}

/* FUNCTION */
// Description: uses bash's built-in commands for foreground processes
// Note: utilizes code from exploration "Process API: Executing a New Program" & "Signal Handling API"
void fore_basher(struct input* ready, int* status, int* signal){
    int key = ready->num_args;                  // holds the number of commands that are actually to be used (everything but input/output redirection or background processes)

    // creating the input used for the "exec" function which will execute a new program using bash commands
    // concatenating just the first input
    char bash_command[2048] = "/bin/";  
    strcat(bash_command, (ready->args[0]));

    // reducing "keys" based on the existence of input/output redirection or background process
    if((ready->in_exist) == 1){
        if(!(strcmp(ready->input, "/dev/null")) == 0){
            // only reduces the number of arguments if the input wasn't auto assigned
            key = key - 2;
        }
    }
    if((ready->out_exist) == 1){
        if(!(strcmp(ready->output, "/dev/null")) == 0){
            // only reduces the number of arguments if the output wasn't auto assigned
            key = key - 2;
        }
    }
    if((ready->back) == 1){
        key = key - 1;
    }

    // creating the array passed into exec
    char *newargv[(key + 1)];
    for(int i = 0; i < key; i++){
        newargv[i] = (ready->args[i]);
    }
    newargv[key] = NULL;

    int childStatus;

    // Fork a new process
    pid_t spawnPid = fork();

    switch(spawnPid){
        case -1:
            perror("fork()\n");
            break;
        case 0:;
            // enabling the signal "SIGINT" in the front child only
            struct sigaction default_action = {0};
            default_action.sa_handler = SIG_DFL;
            sigaction(SIGINT, &default_action, NULL);

            // Redirecting stdin/out if needed
            redirector(ready, status);
            
            // executes the program
            execv(bash_command, newargv);

            // exec only returns if there is an error
            *status = 1;
            perror(ready->args[0]);
            break;
        default:
            // Wait for child's termination in parent processs
            spawnPid = waitpid(spawnPid, &childStatus, 0);

            // child exit by termination
            if(WIFEXITED(childStatus)){
                *status = childStatus;
                *signal = 0;
            } 
            
            // child exit by signal
            else if(WIFSIGNALED(childStatus)){
                printf("terminated by signal %i \n", WTERMSIG(childStatus));
                *signal = WTERMSIG(childStatus);
            }

            // child exited abnormally
            else{
                *status = 1;
                *signal = 0;
            }
            break;
    }
}

/* FUNCTION */
// Description: goes through the list of background processes and sees if any have finished 
void back_checker(struct back_process** head_pid_list, int *status, int *signal){
    struct back_process* current = (*head_pid_list);
    struct back_process* previous = current;
    int exit_status;
    int current_status;

    while (current != NULL){
        current_status = waitpid(current->pid, &exit_status, WNOHANG);

        // if process has completed
        if (current_status != 0){
            /* printing based on termination type */
            // if terminated normally
            if (WIFEXITED(exit_status)){
                printf("background pid %i is done: exit value %i \n", current->pid, WEXITSTATUS(exit_status));
                *status = exit_status;
                *signal = 0;
            } 

            // if terminated by signal
            else if (WIFSIGNALED(exit_status)){
                printf("background pid %i is done: terminated by signal %i \n", current->pid, WTERMSIG(exit_status));
                *signal = WEXITSTATUS(exit_status);
            }

            /* removing this process from the link */
            // checking if this link is the head
            if (current == (*head_pid_list)){
                (*head_pid_list) = current->next;
                current = current->next;
                free(previous);
                previous = current;
            } 

            // if link is between other links
            else{
                previous->next = current->next;
                free(current);
                current = previous->next;
            }

        } else{
            previous = current;
            current = current->next;
        }
    }
}

/* FUNCTION */
// Description: goes through the list of background processes and adds a new process
void back_add(struct back_process** head_pid_list, struct back_process** back_pid_list, int *child_pid){
    struct back_process* temp = malloc(sizeof(struct back_process));

    // setting the head if there isn't anything for it
    if ((*head_pid_list) == NULL){
        (*head_pid_list) = temp;
        (*back_pid_list) = (*head_pid_list);
    } else{
        (*back_pid_list)->next = temp;
    }

    // updating the rest of the list with values
    (*back_pid_list) = temp;
    (*back_pid_list)->pid = *child_pid;
    (*back_pid_list)->next = NULL;
}

/* FUNCTION */
// Description: uses bash's built-in commands for background processes
// Note: utilizes code from explorations "Process API: Executing a New Program" and "Process API: Monitoring Child"
void back_basher(struct input* ready, int* status, int* signal, int *child_pid){
    int key = ready->num_args;                  // holds the number of commands that are actually to be used (everything but input/output redirection or background processes)

    // creating the input used for the "exec" function which will execute a new program using bash commands
    // concatenating just the first input
    char bash_command[2048] = "/bin/";  
    strcat(bash_command, (ready->args[0]));

    // reducing "keys" (holds number of arguments) based on the existence of input/output redirection or background process
    if((ready->in_exist) == 1){
        if(!(strcmp(ready->input, "/dev/null")) == 0){
            // only reduces the number of arguments if the input wasn't auto assigned
            key = key - 2;
        }
    }
    if((ready->out_exist) == 1){
        if(!(strcmp(ready->output, "/dev/null")) == 0){
            // only reduces the number of arguments if the output wasn't auto assigned
            key = key - 2;
        }
    }
    if((ready->back) == 1){
        // since this is a background function, this is guaranteed to happen; copied directly from foreground process for code simplicilty of understanding
        key = key - 1;
    }

    // creating the array passed into exec
    char *newargv[(key + 1)];
    for(int i = 0; i < key; i++){
        newargv[i] = (ready->args[i]);
    }
    newargv[key] = NULL;

    int childStatus;

    // Fork a new process
    pid_t spawnPid = fork();

    switch(spawnPid){
        case -1:
            perror("fork()\n");
            break;
        case 0:;
            // Redirecting stdin/out if needed
            redirector(ready, status);
            
            // executes the program
            execv(bash_command, newargv);

            // exec only returns if there is an error
            *status = 1;
            perror(ready->args[0]);
            break;
        default:;
            // In the parent process
            printf("background pid is %d \n", spawnPid);

            // modifying child pid to be used in a linked list
            *child_pid = spawnPid;
            break;
    }
}

/* FUNCTION */
// Description: functionality for built-in commands of exit, cd, and status in foreground
// Note: utilizes code from exploration "Process API: Executing a New Program"
void commander(struct input* ready, struct back_process** head_pid_list, struct back_process** back_pid_list, int* status, int* signal, int* cont){
    int child_pid = 0;

    /* Handling "exit" command */
    char path[2048];
    if(strcmp(ready->command, "exit") == 0){
        *cont = 0;
        return;
    }

    /* Handling "cd" command */
    else if(strcmp(ready->command, "cd") == 0){
        char tilda[5] = "~";

        // going to home directory
        if (ready->num_args == 1){
            chdir(getenv("HOME"));
        }

        // going to absolute/relative path
        else{
            // using the "~ symbol"
            if (ready->args[1][0] == '~'){
                chdir(getenv("HOME"));
                strcpy(path, ready->args[1]);
                strcpy(path, &path[2]);
                chdir(path);
            } 
            
            // for all other relative/absolute paths
            else{
                chdir(ready->args[1]);
            }
        }
    }

    /* Handling "status" command */
    else if(strcmp(ready->command, "status") == 0){
        // if it was a signal termination
        if(*signal != 0){
            printf("terminated by signal %i \n", *signal);
        } 
        
        // if it was any other type of termination
        else{
            // converting all signals that aren't 0 into 1
            if ((*status) != 0){
                *status = 1;
            }
            printf("exit value %i \n", *status);
            
        }
    }
    
    /* all other commands, utilizing bash's built-in functions */
    else{
        // for foreground processes
        if(ready->back == 0 || enable_background == 1){
            fore_basher(ready, status, signal);
        }

        // for background processes
        else{
            back_basher(ready, status, signal, &child_pid);
            back_add(head_pid_list, back_pid_list, &child_pid);
        }
    }
}

int main(int argc, char *argv[]){
    int cont = 1;
    int status = 0;                             // the status of the last known foreground process' exit value
    int signal = 0;                             // whether or not the process was terminated using signal
    char recieve[2048];
    
    // ignoring the signal "SIGINT" in the main program
    struct sigaction ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_action, NULL);

	// // Register handle_SIGSTP as the signal handler
    struct sigaction SIGTSTP_action = {0};
	SIGTSTP_action.sa_handler = handle_SIGTSTP;
	SIGTSTP_action.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);  // No flags set

    // initializing structs
    struct input* ready = malloc(sizeof(struct input));
    struct back_process* head_pid_list = NULL;  // holds the head of the background process linked list (this will be changed throughout program)
    struct back_process* back_pid_list = NULL;  // holds the head of the background process linked list (this will remain static)

    while(cont){
        // initializing the program
        initializer(recieve, ready);

        // checks all processes that are running in the back
        back_checker(&head_pid_list, &status, &signal);

        // scans in the input and prints it
        printf(": ");
        scanf("%[^\n]s", recieve);              // specific format specifier derived from online resources
        while ((getchar()) != '\n');
        fflush(stdout);
        
        // parsing the command input
        parser(recieve, ready);
      
        // ignores all commands with empty inputs or hashtags
        if(ready->ignore == 0){
            // printer(recieve, ready);            // used for error handling, just prints the input recieved in the struct "ready"
            commander(ready, &head_pid_list, &back_pid_list, &status, &signal, &cont);
        }

        // freeing the struct
        input_freer(ready);
    }

    // frees all dynamic memory
    free(ready);
    background_freer(head_pid_list);
        
    return 0;
}

