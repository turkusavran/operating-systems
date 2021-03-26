#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main () {
    pid_t pid;
    int i = 0;
    int level = 0;

    // Base process
    printf("Base Process ID: %d, level: %d\n", getpid(), level++);
    for (i = 0; i < 4; i++) {
        // Fork process
        pid = fork();

        if (pid < 0) { // Error occured
            fprintf(stderr, "Fork Failed");
            return 1;
        } 
        else if (pid == 0) { // Child process
            printf("Process ID: %d, Parent ID: %d, level: %d\n", getpid() ,getppid(), level);
        } 
        else { // Parent process
            wait(NULL);
            level ++;
        }
    }

    return 0;
}