#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

#define ARG_BUFF 50
#define IN_BUFF 300

#define CMD_BUFF 10

char** history;
int hist_count = 0;
//The SIGINT interupt interupts input, so this checks if the NULL was because of
int eof;

//Because for some reason C doesn't have a standard minimum function
#define max(a, b) (((a) > (b)) ? (a) : (b))

//Prints out history when ctrl+c is pressed
void handle_SIGINT() {
	int i;
	printf("\n");
	for(i=max(0, hist_count-CMD_BUFF); i<hist_count; i++) {
		printf("%d: %s\n", i+1, history[i%10]);
	}
	eof = 0;
	//exit(0);
}

//setup takes a string 'in' and parses it, seperating the string into token and
//checking if the last character is '&'. The tokens are placed in 'args' and the
//background flag is set in 'background'
int setup(char* in, char** args, int* background) {
	int i,j;
	
	//The long and arduous process of finding if the user has used the history
	//'r' feature
	if(in[0] == 'r') {
		//If the user has entered another command, use the first letter to find
		//the last command used with that letter 
		if(in[1]==' ') {
			char first = in[2];
			//To check if the command exists.
			in = NULL;
			for(i=max(0, hist_count-CMD_BUFF); i<hist_count; i++) {
				if(history[i%10][0] == first) {
					in = malloc(sizeof(char) * strlen(history[i%10]));
					strcpy(in, history[i%10]);
				}
			}
			
			if(in == NULL) {
				fprintf(stderr, "ERROR: Command starting with \'%c\' could not be found\n", first);
				return 1;
			}
		//If it is just 'r', use the last command in the history
		} else if(in[1]=='\0') {
			//Checking if the count is 0 to throw an error if it was
			if(hist_count) {
				in = malloc(sizeof(char) * strlen(history[(hist_count-1)%10]));
				strcpy(in, history[(hist_count-1)%10]);
			} else {
				fprintf(stderr, "ERROR: no commands in history\n");
				return 1;
			}
		}
		
		printf("Running command \'%s\'\n\n", in);
	}
	
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
	
	return 0;
}

int main() {
	int background = 0;
	int len;
	pid_t pid;
	
	//global variables must be set in the main method because reasons
	history = malloc(sizeof(char*) * CMD_BUFF);
	eof = 1;
	
	//Setup SIGINT interupt
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	sigaction(SIGINT, &handler, NULL);
	
	//Setup all of the command buffers
	char** args = malloc(sizeof(char*) * ARG_BUFF);
	char* in = malloc(sizeof(char) * IN_BUFF);
	char* hist = malloc(sizeof(char) * IN_BUFF);
	
	while(1) {
		printf("COMMAND-> ");
		
		//This gets the full line from stdin and makes sure it doesn't overflow
		//the buffer.
		if(fgets(in, IN_BUFF, stdin)==NULL) {
			//Check for EOF (Ctrl-D) because Ctrl-C doesn't exit.
			if(eof) {
				printf("\nEOF reached, goodbye.\n");
				return 0;
			}
		}
		
		eof=1;
		len = strlen(in);
		//Remove the trailing newline character
		if(len > 1) {
			if(in[len-1] == '\n')
				in[len-1] = '\0';
			
			hist = malloc(sizeof(char) * strlen(in));
			strcpy(hist, in);
			
			//Parse the string
			if(setup(in, args, &background))
				continue;
			
			pid = fork();
			
			if(pid < 0) {
				fprintf(stderr, "ERROR: fork could not be created. Exiting");
				return 1;
			} else if (pid > 0) {
				if(!background)
					wait();
					
				//Put the command list into history
				if(!(hist[0]=='r' && (hist[1]=='\0' || hist[1]==' '))) {
					free(history[hist_count%10]);
					history[hist_count%10] = malloc(sizeof(char) * strlen(hist));
					strcpy(history[hist_count%10], hist);
					hist_count++;
					free(hist);
				}
			} else {
				if(execvp(*args, args) < 0) {
					fprintf(stderr, "ERROR: \'%s\' could not be found\n", *args);
					return 1;
				}
			}
		}
		//When the interupt is called, read in is skipped. This tricks 'strlen'
		//into thinking the string is an empty string
		in[0]='\0';
	}

	//Because the compiler will whine anyways. Will never be reached.
	return 0;
}