#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <queue>
#include <iostream>
#include <strings.h>
#include <getopt.h>
#include <fstream>
#include <unistd.h>

// Function declarations
int pthread_sleep(double seconds);
void *moderatorExec(void *ptr); // moderator action
float getSpeakTime(int t);      // calculates random speak time between 1 and t
void *commentatorExec(void *ptr);
void inputCommands(int argc, char *argv[], double p, int &n, int &q, int &t);

// Default input parameters
float p = 0.8; // answering probability
int n = 5;     // number of commentators
int q = 3;     // number of questions
int t = 10;    // maximum possible speak time

// Global variables
int speaker = -1;           // -1 is the moderator, 0 to n is commentators, -9 is idle.
bool theEnd = false;        // when it is true it is end of the program
int currentQuestionNum = 0; // tracks which question we are discussing  // queue for waiting commentators to answer
int commentatorID = 0;
pthread_mutex_t speakerLock; // lock for the speakers
pthread_mutex_t questionLock; // lock for the next question
pthread_cond_t questionCond;

// Queues
//std::queue<Commentator> answerQueue;

// Structures
struct Commentator
{
    int commentatorID;     // id of the commentator 1 to n
    char status;           // status of the commentator. it can waits on queue 'Q', speaks 'S' or just waits idle 'I'
    time_t requestTime;    // time when the commentator is joins the queue
    time_t execTime;       // time when the commentator leaves the queue
    time_t turnaroundTime; // execTime-requestTime
    pthread_mutex_t commentator_mutex = speakerLock;
};

struct Moderator
{
    char status;      // status of the moderator. it can wait commentators answers 'W' or ask questions 'A'
    int question = 0; // current question
};

// Main
int main(int argc, char *argv[])
{
    pthread_t moderatorThread;
    
    // takes input parameters
    inputCommands(argc, argv, p, n, q, t);
    pthread_t commentatorThread[n];
    printf("Total %d Commentators \n", n);
    printf("Total %d Questions \n \n", q);

    pthread_cond_init(&questionCond, NULL);
    pthread_mutex_init(&questionLock, NULL);
    pthread_mutex_init(&speakerLock, NULL);
    
    pthread_create(&moderatorThread, NULL, &moderatorExec, NULL);
    for (int i = 0; i < n; i++) 
    {
        printf("Commentator thread %d created \n", i + 1);
        pthread_create(&commentatorThread[i], NULL, commentatorExec, NULL);
    }

    return 0;
}

void *commentatorExec(void *ptr)
{
    for (int i = 0 ; i < q ; i++){
        pthread_mutex_lock(&speakerLock);
        pthread_cond_wait(&questionCond, &questionLock);
        
        float prob = rand() % 10;

        if (p * 10 >= prob)
        {
            // commentator wants to answer
            printf("in COMMENTATOR %d \n", commentatorID + 1);
            printf("---Commentator %d speaks \n", commentatorID + 1);
            pthread_sleep(getSpeakTime(t));

            //push to queue

            //wait for you can speak signal

            //speak

            //konusmam bitti signal yolla
        }
        else
        {
            printf("---Commentator %d doesn't speak \n", commentatorID + 1);
        }
        commentatorID++;
        
        printf("end of the commentator %d \n", commentatorID);
        if (commentatorID == n)
        {
            // signal yolla her sey pushlandi bu sru icin
            commentatorID = 0;
            printf("\n\n");
        }
        pthread_mutex_unlock(&speakerLock);
    }
    return NULL;
    //pthread_exit(0);
}

void *moderatorExec(void *ptr) // moderator action
{               
    for (int i = 0; i < q; i++)
    {   
        printf("ask question %d \n", currentQuestionNum + 1);
        currentQuestionNum++;
        pthread_cond_signal(&questionCond);

        // her seyin pushalnmasini bekle
        // queue yu sirayla popla while da
            // you can speak signal
            // konusmam bitti signal bekle

        //her sey bitti sinyalini bekle sonraki soruya gec

    }

    // signal yolla her sey bitti
    exit(0);
}

// returns a random double value speaktime
float getSpeakTime(int t)
{
    srand(time(NULL));
    double speakTime = rand() % t + 1; // random speak time
    return speakTime;
}

// Helper function to take inputs
void inputCommands(int argc, char *argv[], double p, int &n, int &q, int &t)
{
    int cmd;

    while ((cmd = getopt(argc, argv, "p:n:q:t:")) != -1)
    {
        switch (cmd)
        {
        case 'p':
            p = atof(optarg);
            std::cout << "Answer with probability:  " << optarg << std::endl;
            break;
        case 'n':
            n = atoi(optarg);
            std::cout << " Number of commentators:  " << optarg << std::endl;
            break;
        case 'q':
            q = atoi(optarg);
            std::cout << " Number of questions:  " << optarg << std::endl;
            break;
        case 't':
            t = atoi(optarg);
            std::cout << " Maximum speak time:  " << optarg << std::endl;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
    //return NULL;
}

/**
 * pthread_sleep takes an integer number of seconds to pause the current thread
 * original by Yingwu Zhu
 * updated by Muhammed Nufail Farooqi
 * updated by Fahrican Kosar
 */

int pthread_sleep(double seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    if (pthread_mutex_init(&mutex, NULL))
    {
        return -1;
    }
    if (pthread_cond_init(&conditionvar, NULL))
    {
        return -1;
    }

    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}
