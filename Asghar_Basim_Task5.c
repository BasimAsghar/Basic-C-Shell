//Asghar_Basim_task5
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* Simple example of using gnu readline to get lines of input from a user. 
Needs to be linked with -lreadline -lcurses
add_history tells the readline library to add the line to it's internal history, so that using up-arrow (or ^p) will allows the user to see/edit previous lines.
*/
int counter;

char **get_line(char *line)
{
	char **args = malloc(64 * sizeof(char*));
	int pos = 0;
	char *word = strtok(line, " ");
	while(word != NULL){
		//printf("Does this work %s\n", word); 
		args[pos] = word;
		pos++;
		word = strtok(NULL, " ");
	}
	args[pos] = NULL;
	
	counter = pos;
	return args;
}

//Added two new functions getline1 and getline2. These split up the line into the commands seperated by the pipe

char **get_line2(char **args1, int p)
{
	char **temp_1 = malloc(64 * sizeof(char*));
	for(int i = 0; i<p; i++){
		temp_1[i] = args1[i];
		printf("temp_1[%i] = %s \n", i,temp_1[i]);
	}
	temp_1[p] = '\0';
	return temp_1;
}

char **get_line3(char **args1, int p)
{
	char **temp_2 = malloc(64 * sizeof(char*));
	for(int i = 0; i< (counter - p - 1); i++){
		temp_2[i] = args1[p+i+1];
		printf("temp_2[%i] = %s \n", i,temp_2[i]);
	}
	temp_2[counter-1] = '\0';
	return temp_2;
}

/* Old code that was initially used to make pipes that didn't work out

int spawn_proc(int in, int out, char **arg){
	pid_t pid;
	
	if((pid = fork()) == 0)
	{
		if(in != 0)
		{
			dup2 (in, 0);
			close (in);
		}
		if(out != 1)
		{
			dup2 (out, 1);
			close (out);
		}
		
		return runcmd(arg);
			
	}
	return pid;
}

int fork_pipes(char **arg, int p, int count)
{
	int i;
	pid_t pid;
	int in, fd [2];
	
	in = 0;
	
	for(int i = 0; i < p-1; i++){
		pipe (fd);
		spawn_proc (in, fd[1], args)
	}
	return args;
}
*/
int main(int argc, char **argv) {

	//Variables Used in Program
	int pipe_c = 0;
	char *s;
	char **args;
	int pfd[2];
	int pid, status;
	while (s=readline("prompt> ")) {
	args = get_line(s);
	add_history(s);

	for(int x = 0; x < counter; x++){
		if(strcmp(args[x], "|") == 0)
		{
			pipe_c = x;
		}
		
	}
	//Added an if that checks if there is pipes present. If so it runs a special command runpipe
	if(pipe_c > 0){
		if((pid = fork()) == -1){
			perror("Fork");
			exit(1);
		}
		else if(pid == 0){
			runpipe(args, pipe_c, pfd);
		}
		else{
			while((pid = wait(&status)) != -1)
				fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
		}
	}
	else
		runcmd(args, s);
	
	free(s);
	}
return(0);
}

//Runpipe modifies the stdin and stdout so that the pipe can run the external commands.
//Recieves the position of where the pipe char is and sends the line and position to getline2/3.
//Getline sends back the modified lines that are each command.
//Only supports a maximum of 1 pipe char. Instructions did not ask for multiple. 
void runpipe(char **args, int pipe_c, int pfd[]){
	char **temp1, **temp2;
	temp1 = get_line2(args, pipe_c);
	temp2 = get_line3(args, pipe_c);
	pid_t pid;
	if(pipe(pfd) == -1){
		perror("Error with pipe");
	}
	if((pid = fork()) == -1)
		perror("Failed to fork");
		exit(1);
	if(pid == 0)
	{ //Child
		dup2(pfd[0],0);
		close(pfd[1]);
		
		execvp(temp2[0], temp2);
		perror("Temp1Pipe");
	}
	else
	{
	//Parent
		dup2(pfd[1],1);
		
		close(pfd[0]);
		execvp(temp1[0],temp1);
	}


}

void runcmd(char **args, char *s) {
	char **args_temp = malloc(64 * sizeof(char*));
	int j = 0;
	
	while(args[j] != NULL){
	if( (strcmp(args[j],"<") == 0) || (strcmp(args[j],">") == 0) ){
		break;
	}
	args_temp[j] = args[j];
	j++;
	}
	
	//Variables Used in Program
	char *wordtemp, *value;
	char cwd[1024];
	//int pos = 0;
	int in1 = 0, out1 = 0;
	int in1_loc = -1, out1_loc = -1;
	/* adds the line to the readline history buffer */
	
	//This while loop sets all commands into **args
	
	if (strcmp(args[0], "set") == 0){
		if((wordtemp = args[1]) == NULL){
			perror("Need varname\n");
		}
		
		if((value = args[3])== NULL){
				perror("SetVar failed");
			}
			if(setenv(wordtemp, value,1)==-1){
				perror("Failure to set");
			}
			if(putenv(value)!=0){
				perror("Failed to put");
			}
		
	}
	
	else if (strcmp(args[0], "delete") == 0){
		if(args[1] == NULL){
			perror("Need Env name\n");

		}
		if(unsetenv(args[1]) == -1){
			perror("Failed to Delete");
		}
	
	}
	else if (strcmp(args[0], "print") == 0){
		if((wordtemp=args[1])== NULL){
			perror("Need Environment Variable Name\n");
		}
		printf("Current Env value is %s\n", getenv(wordtemp));	
	}
	
	
	else if (strcmp(args[0], "pwd") == 0){
		if(getcwd(cwd, sizeof(cwd)) != NULL)
			printf("Current working directory: %s\n", cwd);
		else
			perror("Failed to print working directory");
	
	}
	else if (strcmp(args[0], "cd") == 0){
		if(chdir(args[1]) == -1){
			perror("CHDIR failed");
			}
				printf("Changed CWD %s\n", cwd);
		}

	else if (strcmp(args[0], "exit") == 0){
		exit(1);
		
	
	}
	
	else if( ((args[0][0]) ==  "/" ) || ((args[0][0]) == ".") )
	{
	//Path specified will be passed to execute file
		pid_t pid;
		if((pid = fork()) == -1){
			perror("Fork Failed");
			return 1;
		}
		//Child Process
		if(pid ==  0){
			if(execl("/bin/sh", "sh", "-c", s, NULL) == -1){
				perror("EXECL failed");
			}
		}
		//Parent Process
		if(pid > 0){
			if(wait(NULL) == -1){
				perror("Wait failed");
				exit(1);
			}
		}
	}
	
	else{
		for(int i = 0; i < counter; i++)
		{
			if(in1 = (strcmp(args[i], "<") == 0))
			{
				in1_loc = i;
				//printf("Input");

			}
			else if(out1 = (strcmp(args[i], ">") == 0))
			{
				out1_loc = i;
			}
		}
		if(in1_loc > 0 && out1_loc < 0){
			int fd1;
			pid_t pid;
			if((pid = fork()) < 0){
				perror("Error forking");
			}
			else if(pid == 0){	
				printf("Opening");
				fd1 = open(args[in1_loc+1], O_RDONLY);
				dup2(fd1, STDIN_FILENO);
				close(fd1);
				if(execvp(args_temp[0], args_temp)== -1)
					perror("Execvp failed");
		
				}
			else
			{
				if(wait(NULL) == -1){
					perror("ERROR WAITING");
					exit(1);
				}
			}
		}
		else if(out1_loc > 0 && in1_loc < 0){
			int fd2;
			pid_t pid;
			if((pid = fork()) < 0){
				perror("Error forking");
			}
			else if(pid == 0){
				fd2 = creat(args[out1_loc+1], 0644);
				dup2(fd2, STDOUT_FILENO);
				close(fd2);
				if(execvp(args_temp[0], args_temp)== -1)
					perror("Execvp failed");
			}
			else
				if(wait(NULL) == -1){
						perror("ERROR WAITING");
						exit(1);
					}
		}

		else if(in1_loc > 0 && out1_loc >0){
			int fd1, fd2;
			pid_t pid;
			printf("Input/Output detected \n");
			if((pid = fork()) < 0){
				perror("Error forking");
			}
			else if(pid == 0){
				fd1 = open(args[in1_loc+1], O_RDONLY);
				dup2(fd1, STDIN_FILENO);
				fd2 = creat(args[out1_loc+1], 0644);
				dup2(fd2, STDOUT_FILENO);
				close(fd1);
				close(fd2);
				if(execvp(args_temp[0], args_temp)== -1)
					perror("Execvp failed");
			}
			else{
				if(wait(NULL) == -1){
						perror("ERROR WAITING");
						exit(1);
					}
				}
		}

		else{
			pid_t pid;
			if((pid = fork()) == -1){
				perror("Fork Failed");
			}
			//Child Process
			else if(pid ==  0){
				if(execvp(args[0], args)== -1)
					perror("Execvp failed");
			}
			//Parent Process
			else if(pid > 0){
				if(wait(NULL) == -1){
					perror("Wait failed");
					exit(1);
				}
			}
		}
	

	/* clean up! */
	}

}



