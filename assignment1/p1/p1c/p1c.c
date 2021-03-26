#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid;

    // Fork process
    pid = fork();

    if (pid < 0) { // Error occured
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { // Child process
        exit(1);
    }
    else { // Parent process
        sleep(5);
    }
    return 0;
}