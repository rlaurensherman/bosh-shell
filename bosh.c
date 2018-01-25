/* Lauren Sherman */
#include <stdio.h> 
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

/* Set the max characters per line and max number of arguments */
#define MAX_LINE 128
#define MAX_ARGS 10

void readCommand();
void sigchldHandler(int sig); 
void foreground_proc(char *commandArgs[], int args);
void background_proc(char *commandArgs[], int args);

/* Start of main */
int main()
{
	readCommand();

	return 0;
}

/* SIGCHLD handler */
void sigchldHandler(int sig)
{
	pid_t bpid;
	int status;

	while((bpid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		if(WIFEXITED(status))
		{
			printf("command (%d) complete \n", bpid);
		}
		else 
		{
			printf("command (%d) had a problem \n", bpid);
		}
	}
}

/* Function for reading a command from the user */
void readCommand()
{
	char line[MAX_LINE];
	char *commandArgs[MAX_ARGS];
	char *sPtr; 
	int i;
	int a;
	int args = 0; /* Variable to store number of arguments */
	
	/* Initialize line and commandArgs */
	memset(line, '\0', MAX_LINE);
	for(i = 0; i < MAX_ARGS; i++)
	{
		commandArgs[i] = NULL;
	}

	/* Prompt user for command */
	printf("bosh (%d)> ", getpid());
	fgets(line, MAX_LINE, stdin);

	/* Replace newlines with null characters */
	for (a = 0; a < MAX_LINE; a++)
	{
		if (line[a] == '\n')
		{
			line[a] = '\0';
		}
	}

	/* While the user doesn't enter "quit" */
	while(strcmp(line, "quit") != 0)
	{
		if(strlen(line) > 1)
		{ 
			sPtr = strtok(line, " ");
			i = 0;
			while(sPtr != NULL)
			{
				commandArgs[i] = malloc(strlen(sPtr) + 1);
				strcpy(commandArgs[i], sPtr);
				args++;
				sPtr = strtok(NULL, " ");
				i++;
			}

			/* Background processes */
			if(strcmp(commandArgs[args-1], "&") == 0)
			{
				background_proc(commandArgs, args);
			}
			/* Foreground processes */
			else
			{
				foreground_proc(commandArgs, args);
			}

			/* Free and initialize commandArgs */
			for(i = 0; i < MAX_ARGS; i++)
			{
				free(commandArgs[i]);
				commandArgs[i] = NULL;
			}
		}  

		/* Re-initialize variables */
		memset(line, '\0', MAX_LINE);
		args = 0;

		/* Prompt user for next command */
		printf("bosh (%d)> ", getpid());
		fgets(line, MAX_LINE, stdin);

		/* Replace newlines with null characters */
		for (a = 0; a < MAX_LINE; a++)
		{
			if (line[a] == '\n')
			{
				line[a] = '\0';
			}
		}
	}
	
	printf("program (%d) done \n", getpid());
}

/* Function to fork foreground processes */
void foreground_proc(char* commandArgs[], int args)
{
	pid_t pid;
	pid_t waitPID;
	int status;
	int i;

	/* Fork */
	pid = fork();

	/* Linux error handling for fork() */
	if (pid < 0)
	{
		fprintf(stderr, "fork error: %s \n", strerror(errno));
		exit(-1);
	}
	else if(pid == 0)
	{
		for(i = 0; i < args; i++)
		{
			execvp(commandArgs[i], commandArgs);
			perror("execvp failed");
			exit(2);
		}		
	}
	waitPID = waitpid(pid, &status, 0);
	if(WIFEXITED(status))
	{
		printf("command (%d) complete \n", waitPID);
	}
	else 
	{
		printf("command (%d) had a problem \n", waitPID);
	}
}

/* Function to fork background processes */
void background_proc(char* commandArgs[], int args)
{
	pid_t pid;
	int i;

	struct sigaction action;

	memset(&action, '\0', sizeof(struct sigaction));
	action.sa_handler = sigchldHandler;
	action.sa_flags = 0;
	sigaction(SIGCHLD, &action, NULL); 

	free(commandArgs[args-1]);
	commandArgs[args-1] = NULL;

	/* Fork */
	pid = fork();

	/* Linux error handling for fork() */
	if (pid < 0)
	{
		fprintf(stderr, "fork error: %s \n", strerror(errno));
		exit(-1);
	}
	else if(pid == 0)
	{
		for(i = 0; i < (args-1); i++)
		{
			execvp(commandArgs[i], commandArgs);
			perror("execvp failed");
			exit(2);
		}
	}
}

