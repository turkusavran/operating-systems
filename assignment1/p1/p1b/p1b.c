#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    
    // Fork process
    pid = fork();

    if (pid < 0) { // Error occured
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { // Child process
        execlp("/bin/ps", "ps f", NULL);
    }
    else { // Parent process
        wait(NULL);
        printf("Child finished execution.\n");
    }

    return 0;
}