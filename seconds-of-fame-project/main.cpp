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
int speaker = 0;
int commentator = 0;

// Global variables      
int commentatorID = 0;
pthread_mutex_t speakerLock;    // lock for the speakers
pthread_mutex_t questionLock;   // lock for the next question
pthread_cond_t questionCond;    // cond for next question
pthread_mutex_t queueLock;      // lock for push answers to queue
pthread_cond_t queueCond;
pthread_mutex_t speakLock;      // lock for speaking in order
pthread_cond_t speakCond;
pthread_mutex_t endSpeakLock;
pthread_cond_t endSpeakCond;

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
    
    for (int i = 0; i < n; i++) 
    {
        printf("Commentator thread %d created \n", i + 1);
        pthread_create(&commentatorThread[i], NULL, commentatorExec, NULL);
    }
    pthread_sleep(2);
    pthread_create(&moderatorThread, NULL, &moderatorExec, NULL);

    for(int i = 0 ; i < n ; i++){
        pthread_join(commentatorThread[i], NULL);
    }

    pthread_join(moderatorThread, NULL);
    return 0;
}

void *commentatorExec(void *ptr)
{
    for (int i = 0 ; i < q ; i++){
        //pthread_mutex_lock(&questionLock);
        pthread_cond_wait(&questionCond, &questionLock);
        pthread_mutex_unlock(&questionLock);

        float prob = rand() % 10;
        commentator++;
        //if (p * 10 >= prob)
        if(true)
        {
            speaker++;
            // commentator wants to answer
            printf("In COMMENTATOR %d \n", commentator);

            //push to queue
            if(commentator == n) {
                //her sey pushlandi
                pthread_cond_signal(&queueCond);
            }
            //wait for you can speak signal
            pthread_cond_wait(&speakCond, &speakLock);
            pthread_mutex_unlock(&speakLock);

            //speak
            printf("---Commentator %d speaks \n", commentator);
            pthread_sleep(getSpeakTime(t));

            //konusmam bitti signal yolla
            pthread_cond_signal(&endSpeakCond);
        }
        else
        {
            printf("---Commentator %d doesn't speak \n", commentator);
        }
        
        printf("end of the commentator %d \n", commentator);
        if (commentator == n)
        {
            commentator = 0;
            printf("\n\n");
           // pthread_cond_signal(&questionCond);

        }

    }
    pthread_exit(0);
}

void *moderatorExec(void *ptr) // moderator action
{               
    for (int i = 0; i < q; i++)
    {   
        printf("ask question %d \n", i + 1);
        pthread_cond_broadcast(&questionCond);

        // her seyin pushalnmasini bekle
        pthread_cond_wait(&queueCond, &queueLock);
        pthread_mutex_unlock(&queueLock);
        // queue yu sirayla popla while da
        for(int i = 0 ; i < speaker ; i++){
            // you can speak signal
            pthread_cond_signal(&speakCond);
            // konusmam bitti signal bekle
            pthread_cond_wait(&endSpeakCond, &endSpeakLock);
            pthread_mutex_unlock(&endSpeakLock);
        }
        speaker = 0;
        //commentator = 0;
        //her sey bitti sinyalini bekle sonraki soruya gec
       //pthread_cond_wait(&questionCond, &questionLock);
        //pthread_mutex_unlock(&questionLock);

    }

    pthread_exit(0); 
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
