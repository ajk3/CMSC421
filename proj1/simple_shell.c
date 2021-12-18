/*
this program simulates the basic linux shell.
commands enterned by users are parsed and executed
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <string.h>
#include "utils.h"

//func prototype
char* readUserCommand();
char** parseUserCommand(char*);
void executeCommand(char**, char*);
void invokeShell();
void deallocMem(char**, char*);
void exitShell(char**, char*);
int parseStrToInt(char*);
void executeProc(char**, char*);

int main(int argc, char** argv)
{
	if(argc > 1)
	{
		fprintf(stderr,"Error: Program doesn't take any argument!\n");
		exit(1);
		return 1;
	}
	else
	{
	 	invokeShell();
	}

	return 0;
}

/*
	this function will take input from the user.
	input can be of any length
*/
char* readUserCommand()
{
   	int buffer = 256;

	char* str = calloc(buffer, sizeof(char));

	int idx  = 0;
	char in = 'a';

	do
	{
		printf("proj1 simple_shell$ ");
    		in = getchar();
	}while(in  == '\n');

	// getting users' input
	while(in != '\n')
    	{
       		str[idx] = in;
      		idx++;

      		if(idx >= buffer) // reallocating if the inital buffer is not sufficient
      		{
			buffer += 256;

			str = realloc(str, buffer);
      		}

     		 in = getchar();
    	}

	return str;
}

/*
	we'll use this function to tokenize the users' input
	this function uses provied utlity functions
*/

char** parseUserCommand(char* input)
{
	int start= 0;
	int idx = 0;

	int len =  (int)strlen(input);

	char* dupInput = calloc((len+1), sizeof(char));
	strncpy(dupInput, input, len);

	len = (int)strlen(dupInput);

	// we'll use this pointer to trim the input
	char* movingPtr = dupInput;

	// allocating the mem for the tokens
	char** tokens = calloc(sizeof(char*), len);

	// tokenizing the input
	while(start < len)
	{
		int firstUqs = first_unquoted_space(movingPtr);

		// in case the input only has one arg
		if(firstUqs == -1)
		{
			tokens[idx] = (char*) calloc(sizeof(char),(len+1));
			strncpy(tokens[idx], movingPtr, len);
			start = len;
		}
		else
		{
			tokens[idx] =  (char*) calloc(sizeof(char),(firstUqs+1));
			strncpy(tokens[idx], movingPtr, firstUqs);

			// triming the input. pointer points to the first char after space
			movingPtr +=  firstUqs+1;
			start = 0;

		}
			len = strlen(dupInput);
		        idx++;
	}

	// appending NULL, as execvp needs a null terminated array
	tokens[idx] = NULL;

	// unescaping the tokens
	int j = 0;

	while(tokens[j] != NULL)
	{
		char* tempStr = tokens[j];

		tempStr = unescape(tempStr, stderr);

		// if unescape func can't unescape the token
		if(tempStr == NULL)
		{
			printf("invalid command!!\n");
			free(dupInput);
			deallocMem(tokens, input);
			invokeShell();
		}

		int tempLen = (int) strlen(tempStr);
	
		// reallocating as we might lose some char during unescaping
		tokens[j] = (char*) realloc(tokens[j],(sizeof(char)*(tempLen+1)));
		strncpy(tokens[j], tempStr, tempLen+1);
		free(tempStr);
		j++;
	}

	free(dupInput);
	return tokens;
}

/*
	this fuction runs the command
*/

void executeCommand(char** tokens, char* input)
{
	pid_t pid = fork();
	int status;

        if(pid < 0)
        {
            printf("Error!!! Unable to fork the program!");
			deallocMem(tokens, input);
			exit(1);

        }
	else  if(pid  == 0)
	{
		// child process
		execvp(tokens[0], tokens);
	
		if(errno) // if execvp failed
		{
			deallocMem(tokens, input);
			exit(126);
		}
	}
	else
	{
		waitpid(pid, &status, 0);

		// if chid process has some issues
		if (!(WIFEXITED(status) && (WEXITSTATUS(status) == 0)))
		{

       		 	fprintf(stderr, "Error!! terminating all child process\n");
			deallocMem(tokens, input);
			invokeShell();
		}
	}
}

/*
	this function calls the appropirate function based on the input, and will continue unitl user enters exit
*/
void invokeShell()
{
       char* input =  readUserCommand();
       char** tokens = parseUserCommand(input);

	while(strcmp(tokens[0], "exit") != 0)
	{
		if((strcmp(tokens[0], "proc") == 0))
		{
			executeProc(tokens, input);
			deallocMem(tokens, input);
		}
		else
		{
			executeCommand(tokens,input);
			deallocMem(tokens, input);
		}

		input = readUserCommand();
		tokens = parseUserCommand(input);
	}

	if((strcmp(tokens[0], "exit") == 0))
	{
		exitShell(tokens, input);
	}

}

void deallocMem(char** arr, char* input)
{
	free(input);

	int i = 0;

	while(arr[i] != NULL)
	{
		free(arr[i]);
		i++;
	}

	free(arr[i]);
	free(arr);
}

void exitShell(char** tokens, char* input)
{
	int numArg = 0
;
	while(tokens[numArg] != NULL)
	{
		numArg++;
	}

	numArg--; // excluding exit from the count

	if(numArg < 2)
	{
		if(numArg == 0)
		{
			deallocMem(tokens, input);
			printf("Exiting with exit code: %d\n", 0);
			exit(0);
		}	
		else
		{
			int num = parseStrToInt(tokens[1]);

			if(num != -1) 
			{
				deallocMem(tokens, input);

				if(num > 255) // error code range from 0-255
				{
					num = num % 255;
				}

                printf("Exiting with exit code: %d\n", num);
                exit(num);
			}
			else
			{
				printf("failed to exit....\n");
				deallocMem(tokens, input);
				invokeShell();
			}
		}
	}
	else
	{
		printf("Can't take more than one arg\nexiting shell\n");
		deallocMem(tokens, input);
		exit(126);
	}

}

int parseStrToInt(char* str)
{
	int i = 0;

	while(str[i] != '\0')
	{
		if(!isdigit(str[i]))
		{
			return -1;
		}
		i++;
	}

	return atoi(str);

}

void executeProc(char** tokens, char* input)
{
	int numArg = 0;
	while(tokens[numArg] != NULL)
	{
		numArg++;
	}

	int newSize = (numArg*2);

	char* path = (char*)calloc(newSize, sizeof(char));

	int readIdx = 0;

	// creating the file path
	while(tokens[readIdx] != NULL)
	{
		strcat(path, "/");
		strcat(path, tokens[readIdx]);
		readIdx++;
	}

	FILE* inFile = NULL;
	inFile = fopen(path, "r");

	if(inFile == NULL)
	{
		printf("File \"%s\" not found\n", path);
		deallocMem(tokens, input);
		invokeShell();
	}

	char ch;

	ch = fgetc(inFile);

        while (ch != EOF)
	{
          //  putchar(ch);
	printf("%c", ch);
	    ch = fgetc(inFile);
	}

	printf("\n");

        fclose(inFile);  // closing the file

	free(path);
}
