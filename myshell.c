#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "myshell.h"

//Main function
int main(int argc, char *argv[]){ 
    
    int len; //length of entered command
    char buffer[BUFSIZE];	// roomfor 1000 characters
    char* tokens[BUFSIZE]; //holds an array of tokenized strings of the original users input
    char *cmd; //pointer to entered command
    int tokenCounter = 0; //Counter for tokens
    int end = 0; //Used to keep loop going until user inputs exit command
    int ret_code; //Return code to check for errors
    int numberOfPipes = 0; //Number of pipes in the command, default is 0
    while(!end){      
         
        printDirectory();
        cmd = fgets(buffer, BUFSIZE, stdin); //Get user input;
        while(strcmp(cmd, "\n") == 0){ //If user input is empty(\n), keep getting input
            printDirectory();
            cmd = fgets(buffer, BUFSIZE, stdin);
        }
        
        if(cmd != NULL){ 
            //check for the newline character and overwrite with \0
            len = strlen(buffer);
            if(buffer[len-1] == '\n'){
                buffer[len-1] = '\0';
            }
        }

        tokenizeString(buffer, cmd, tokens);
        tokenCounter = countTokens(tokens); //Count the number of tokens
        numberOfPipes = pipeCounter(tokens); //Count the number of pipes
        //cd command implementation
        if(strcmp(tokens[0], "cd") == 0){
            ret_code = chdir(tokens[1]);
            if(ret_code == -1){
                perror("Error in cd");
            }
        }
        
        //exit commands implementation
        else if(strcmp("exit", tokens[0]) == 0){
            end = 1;
        }

        else if(numberOfPipes == 0){
            execute(tokens, tokenCounter);
        }
        else {
            executeWithPipes(tokens, tokenCounter, numberOfPipes);
        }
    }
    //End program
    printf("Program has successfully exited\n");
    return 0; 
}

//Function that prints current directory using getcwd function
void printDirectory() {
    char cwd[BUFSIZE]; //Contains path to current working directory
    static int percentage = 37; //Ascii value for percentage
    getcwd(cwd, sizeof(cwd)); //Get the current working directory
    printf("%s%c", cwd, percentage); //Print the current working directory
}

//Function that splits a string into an array of strings  wherever there was a delimiter
char** tokenizeString(char* buf, char* cmd, char** tokens) {
    char* p = cmd; //Pointer to the user input string
    int i; //Index variable for loop

    for(i = 0; i < BUFSIZE; i++){
        tokens[i] = strtok_r(p, " ", &p);
        if(tokens[i] == NULL){
            break;
        }
    }
    return tokens;
}

//Function that returns the number of tokens in an array of tokens
//Input is a pointer to an array of tokens
int countTokens(char** tokens){
    int i; //Index variable for loop
    int end = 0; //Variable to stop the loop
    int tokenCounter = -1; //Counter for tokens

    for(i = 0; end != 1; i++){
        if(tokens[i] == NULL){
            end = 1;
        }
        tokenCounter++;
    }
    return tokenCounter;
}

//Function that returns the number of tokens in an array of tokens
//Input is a pointer to an array of tokens
int pipeCounter(char** tokens){
    int i; //Index variable for loop
    int end = 0; //Variable to stop the loop
    int pipeCounter = 0; //Counter for tokens
    char* p = tokens[0];
    for(i = 0; end != 1; i++){
        if(tokens[i] == NULL){
            end = 1;
        }
        if(strcmp(p, "|") == 0){
            pipeCounter++;
        }
        p = tokens[i];
    }
    return pipeCounter;
}



//Method that executes a command with no piping required
//Parameter is the array of tokenized strings and the total number of tokenized strings
void execute(char** tokens, int numberOfTokens){
    char* command[COMMAND_LIMIT]; //Array of all the strings to execute a system command with execvp
    int status; // status of parent or child process
    int ret_code; // return code
    int i; // variable used for loop
    pid_t pid; //pid of the current child or parent process

    pid = fork(); //create child process
    if(pid == -1){ //fork failed, therefore print error message
        perror("error creating child process");
    }
    // Child process
    if(pid == 0){
        //Puts every string together in an array to execute command
        for(i = 0; i< numberOfTokens; i++){
            command[i] = tokens[i];
        }
        command[numberOfTokens] = NULL;//The array needs to terminate with NULL
        ret_code = execvp(tokens[0], command); // execute command
        if(ret_code == -1){ //execute failed, so print error message
            perror("error executing command");
            exit(1);
        }
        else{ //successful execution: return 0
            exit(0);
        }
    }
    // Parent process
    else{  
        wait(&status); //Parent is waiting on child process
    }
}

//Method that executes a command with no piping required
//Parameter is the array of tokenized strings and the total number of tokenized strings
void executeWithPipes(char** tokens, int numberOfTokens, int numberOfPipes){
    int fd[2]; //This int array contains file descriptors
    char* commandsArray[numberOfPipes+1][COMMAND_LIMIT]; //Array holding an array of all the strings to execute a system command with execvp
    pid_t pid; //pid of the child or the parent process
    int status; //status of the process running
    int lastPipeReadEnd; //Holds the read end of the last pipe
    int ret_code; //return code of the process
    int i; //Variable for loop
    int j; //Second Variable for loop
    
    

    tokens[numberOfTokens] = "|"; //Append a pipe character to the end of tokens
    int index; //Keeps track of the current loop index
    int totalIndex = 0; //Keeps track of the total index throughout all loops
    /**This loop puts all the input up until it gets to a pipe character
     * into the first array of commands held by commands[0] and then
     * sets the value 1 after the end of that array to NULL
     * it then continues to do this to the commands held by commands[i]
     * Until it reaches the end of the loop and every single command
     * array is contained in the commands variable
     */
    for(i = 0; i < numberOfPipes+1; i++){
        index = 0;
        for(j = totalIndex; (strcmp(tokens[j], "|") != 0); j++){
            commandsArray[i][index] = tokens[j];
            index++;
            totalIndex++;
        }
        commandsArray[i][index] = NULL;
        totalIndex++;
    }

    if (fork() == 0){  //Fork so that the parent can continue running after all the commands are executed
        int beginning = 1; //Variable to check if it is the beginning of the loop
        for (i = 0; i < numberOfPipes; i++) { 
            ret_code = pipe(fd); //Beginning piping
            if (ret_code == 1){ //Error protection against a pipe failure
                printf("error opening the pipe");
            }
            pid = fork(); // create the first child process of the loop
            if(pid == -1){ //Fork error checking
                perror("error creating the child process");
            }

            if (pid == 0) { //Child process of the loop     
                /**This first part of the program replaces the current pipes
                 * STDIN with the previous pipes read end 
                 * We check if its the first loop since the first pipe has no pipe before it
                 */
                if(beginning != 1){ //Check if it is not the first loop
                    ret_code = dup2(lastPipeReadEnd, STDIN);
                    close(lastPipeReadEnd); //Since the pipe is duplicated we should close the original pipe
                    if(ret_code == -1){ //error checking for a dup2 error
                        perror("error with duplicating pipe");
                    }
                }
                ret_code = dup2(fd[WRITE], STDOUT); //replace STDOUT with the new pipes write end
                close(fd[WRITE]); //Since the pipe is duplicated we should close the original pipe
                if(ret_code == -1){ //error checking for a dup2 error
                        perror("error with duplicating pipe");
                    }
                ret_code = execvp(commandsArray[i][0], commandsArray[i]); //execute the command
                if(ret_code == -1){ //Error checking the command
                    perror("error executing command");
                    exit(1);
                }
                close(fd[WRITE]);
                exit(0);
            }

            else{ //Parent process of the loop
                close(fd[WRITE]);//Close the write side of the pipe for the parent process 
                close(lastPipeReadEnd); //Close unused pipe
                wait(&status);
                if(status == -1){
                    perror("error with second child process");
                }
            }
            
            beginning = 0; // Set beginning to false
            lastPipeReadEnd = fd[READ]; //Save read end data of the last pipe so the next loop can access it

            if (i == numberOfPipes-1) { //If its the last loop we execute the final command
                ret_code = dup2(fd[READ], STDIN); //Replace STDIN with read end of the last pipe
                if(ret_code == -1){ //error checking for a dup2 error
                        perror("error with duplicating pipe");
                    }
                close((fd[WRITE])); //Close unused side of the pipe
                ret_code = execvp(commandsArray[numberOfPipes][0], commandsArray[numberOfPipes]); //execute final command and print result to STDOUT
                if(ret_code != 0){ //Error checking last command
                    perror("final command failed");
                }
            }
        } 
    }
    
    else{ //Parent waiting for all the children to finish
        wait(&status);
        if(status == -1) {
            perror("wait failed:");
        }
    }
}