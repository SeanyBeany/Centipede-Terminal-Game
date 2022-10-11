#ifndef MYSHELL_H
#define MYSHELL_H
#define BUFSIZE 1000 //BUFSIZE is 1000 which should account for command limit
#define EXIT_SUCCESS 0 //Return 0 is successful so this define just makes things simpler
#define COMMAND_LIMIT 100 //Limit of 100 arguments in the command
#define STDOUT 1 //1 represents the stdout for dup2
#define STDIN 0 //1 represents the stdin for dup2
#define READ 0 //Adds clarity to close() method
#define WRITE 1 //Adds clarity to close() method


//Function that prints current directory using getcwd function
void printDirectory();

//Function that splits a string into an array of strings  wherever there was a delimiter
char** tokenizeString(char* buf, char* cmd, char** tokens);

//Function that returns the number of tokens in an array of tokens
//Input is a pointer to an array of tokens
int countTokens(char** tokens);

//Function that returns the number of pipes ("|") in an array of tokens
//Input is a pointer to an array of tokens and the buf and cmd parameters are so that it can use strcmp without segfaulting
int pipeCounter(char** tokens);

//Function that executes a command without piping
//Parameter is the array of tokenized strings and the total number of tokenized strings
void execute(char** tokens, int numberOfTokens);

//Function that executes commands with a pipe
void executeWithPipes(char** tokens, int numberOfTokens, int numberOfPipes);

#endif
