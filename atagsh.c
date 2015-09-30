#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include  <sys/types.h>

#define ARG_BUFF 50
#define IN_BUFF 300

//setup takes a string 'in' and parses it, seperating the string into token and
//checking if the last character is '&'. The tokens are placed in 'args' and the
//background flag is set in 'background'
void setup(char* in, char** args, int* background) {
	int i,j;
	
	//strtok splits a string into a series of tokens. Initialized here with 'in'
	char* arg = strtok(in, " ");
	//This for loop populates the 'args' by retreving the next token.
	for(i=0; i<ARG_BUFF && arg != NULL; i++, arg = strtok(NULL , " ")) {
		args[i] = arg;
	}
	
	//Makes the array of arguments null-terminated
	if(i<ARG_BUFF)
		args[i] = NULL;
	
	//Finds the last character in the last argument by iterating through the
	//array pointer
	for(arg = args[i-1]; *arg != NULL; arg++){}
	
	//Look at the last character in the last string and set the background flag
	//accordingly. Also removes the last character if it's '&'
	arg--;
	if(*arg == '&') {
		*background = 1;
		*arg = '\0';
	} else {
		*background = 0;
	}
}

int main() {
	int background = 0;
	int len;
	pid_t pid;
	//printf("Checkpoint Alpha\n");
	
	char** args = malloc(sizeof(char*) * ARG_BUFF);
	char* in = malloc(sizeof(char) * IN_BUFF);
	
	//printf("Checkpoint Bravo\n");
	
	while(1) {
		printf("ATAGSH> ");
		
		//This gets the full line from stdin and makes sure it doesn't overflow
		//the buffer.
		fgets(in, IN_BUFF, stdin);
		len = strlen(in);
		//Remove the trailing newline character
		if(len > 1) {
			if(in[len-1] == '\n')
				in[len-1] = '\0';
		
			//Parse the string
			setup(in, args, &background);
			
			pid = fork();
			
			if(pid < 0) {
				fprintf(stderr, "ERROR: fork could not be created. Exiting");
				return 1;
			} else if (pid > 0) {
				if(!background)
					wait();
			} else {
				execvp(*args, args);
			}
		}
		
		/*
		printf("Testing string: \"%s\"\n", in);
		
		int i=0;
		for(i=0; args[i]!= NULL; i++)
			printf("args[%d]=\"%s\"\n", i, args[i]);
		
		printf("background=%d\n", background);
		*/
	}

	
	return 0;
}