#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>

#define BUFFER_SIZE 50
#define READ_END 0
#define WRITE_END 1

int main() {
    char *write_msg = malloc(BUFFER_SIZE);
	char read_msg[BUFFER_SIZE];
	pid_t pid;

	// Date sturcture
	struct timeval tv;

	//pipes
	int fd1[2];
	int fd2[2];
	int fd3[2];
	int fd4[2];
	int i;

	// Creating the pipes
	if (pipe(fd1) == -1) {
		fprintf(stderr,"Pipe failed");
		return 1;
	}
	if (pipe(fd2) == -1) {
		fprintf(stderr,"Pipe failed");
		return 1;
	}
	if (pipe(fd3) == -1) {
		fprintf(stderr,"Pipe failed");
		return 1;
	}
	if (pipe(fd4) == -1) {
		fprintf(stderr,"Pipe failed");
		return 1;
	} 

	// Fork a process
	pid = fork();
	if (pid < 0) { // Error occured
		fprintf(stderr, "Fork failed");
		return 1;
	}

	if (pid > 0) {  // ---------------- Parent process (A) 
		// Close the unused end of the pipe
		close(fd1[READ_END]);

		// Send the date to child process (B)
		gettimeofday(&tv, NULL);
		strcpy(write_msg,asctime(localtime(&tv.tv_sec)));
		write(fd1[WRITE_END], write_msg, strlen(write_msg)+1); 

		// Close the write end of the pipe 
		close(fd1[WRITE_END]);
		wait(NULL);

		// Read the date from (B)
		// Close the unused end of the first pipe 
		close(fd4[WRITE_END]);

		// Read the date from (A)
		read(fd4[READ_END], read_msg, BUFFER_SIZE);
		printf("Parent (A) read %s", read_msg);
		
		// Close the read end of the first pipe 
		close(fd4[READ_END]);

		// Terminate child process B
		kill(pid, SIGKILL);

		// Terminate A
		kill(pid, SIGKILL);
	
	}
	else { // ------------ Child process (B)
		// Close the unused end of the first pipe 
		close(fd1[WRITE_END]);

		// Read the date from (A)
		read(fd1[READ_END], read_msg, BUFFER_SIZE);
		printf("Child (B) read %s", read_msg);
		
		// Close the read end of the first pipe 
		close(fd1[READ_END]);

		// Sleep for 3 seconds
		sleep(3);

	    // ------------------ Creating C and sending date-----------------
		// Fork C
		pid = fork();	
		if (pid < 0) { // Error occured
			fprintf(stderr, "Fork failed");
			return 1;
		}

		if (pid > 0) {  // ------------- Parent process (B) 
			// Close the unused end of the second pipe
			close(fd2[READ_END]);

			// Send the date to child process (C)
			gettimeofday(&tv, NULL);
			strcpy(write_msg, asctime(localtime(&tv.tv_sec)));
			write(fd2[WRITE_END], write_msg, strlen(write_msg)+1); 

			// Close the write end of the second pipe 
			close(fd2[WRITE_END]);

			// Close the unused end of the third pipe 
			close(fd3[WRITE_END]);

			// Read the date from (C)
			read(fd3[READ_END], read_msg, BUFFER_SIZE);
			printf("Parent (B) read %s", read_msg);

			// Close the read end of the third pipe 
			close(fd3[READ_END]);

			// Sleep for 3 seconds
			sleep(3);

			// Close the read end of the fourth pipe
			close(fd4[READ_END]);

			// Send the date to child process (C)
			gettimeofday(&tv, NULL);
			strcpy(write_msg, asctime(localtime(&tv.tv_sec)));
			write(fd4[WRITE_END], write_msg, strlen(write_msg)+1); 

			// Close the write end of the fourth pipe 
			close(fd4[WRITE_END]);

			// Terminate child process (C)
			kill(pid, SIGKILL);

			// Terminate (A)
			exit(1);
		}
		else { // ----------------------- Child process (C)
			// Close the unused end of the pipe 
			close(fd2[WRITE_END]);

			// Read the date from the pipe 
			read(fd2[READ_END], read_msg, BUFFER_SIZE);
			printf("Child (C) read %s", read_msg);

			// Close the read end of the second pipe 
			close(fd2[READ_END]);

			// Sleep for 3 seconds
			sleep(3);

			// Send date to the parent process (B)
			gettimeofday(&tv, NULL);
			strcpy(write_msg, asctime(localtime(&tv.tv_sec)));
			write(fd3[WRITE_END], write_msg, strlen(write_msg)+1);

			close(fd3[WRITE_END]);
		}
	}

	free(write_msg);

	return 0;
}