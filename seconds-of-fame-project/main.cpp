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
#include <math.h>

// Function declarations
int pthread_sleep(double seconds);
void *moderatorExec(void *ptr); // moderator action
double getSpeakTime(int t);     // calculates random speak time between 1 and t
void *commentatorExec(void *ptr);
void inputCommands(int argc, char *argv[], double &p, int &n, int &q, int &t);
void timelog();

// Default input parameters
double p = 0.8; // answering probability
int n = 5;      // number of commentators
int q = 3;      // number of questions
int t = 10;     // maximum possible speak time
int commentatorID = 0;
int commentator = 0;
time_t start_time;
struct timespec tstart = {0, 0}, tend = {0, 0};

// Mutexes and conditions
pthread_mutex_t speakerLock;  // lock for the speakers
pthread_mutex_t questionLock; // lock for the next question
pthread_cond_t questionCond;
pthread_mutex_t queueLock; // lock for push answers to queue
pthread_cond_t queueCond;
pthread_mutex_t speakLock; // lock for speaking in order
pthread_cond_t speakCond;
pthread_mutex_t endSpeakLock; // lock for end of speaking
pthread_cond_t endSpeakCond;
pthread_mutex_t endProgram; // lock for end of program
pthread_cond_t endProgramCond;

// Queues
std::queue<int> answerQueue;

// Main
int main(int argc, char *argv[])
{
    time(&start_time);
    pthread_t moderatorThread;

    clock_gettime(CLOCK_MONOTONIC, &tstart);

    // takes input parameters
    inputCommands(argc, argv, p, n, q, t);
    pthread_t commentatorThread[n];
    printf("Total %d Commentators \n", n);
    printf("Total %d Questions \n\n", q);

    for (int i = 0; i < n; i++)
    {
        pthread_create(&commentatorThread[i], NULL, commentatorExec, NULL);
    }
    pthread_sleep(1);
    pthread_create(&moderatorThread, NULL, &moderatorExec, NULL);

    for (int i = 0; i < n; i++)
    {
        pthread_join(commentatorThread[i], NULL);
    }

    pthread_join(moderatorThread, NULL);
    return 0;
}

void *commentatorExec(void *ptr)
{
    int myID = pthread_self();
    bool willSpeak = false;

    for (int i = 0; i < q; i++)
    {
        //printf("COMMENTATOR %d\n",myID);
        pthread_cond_wait(&questionCond, &questionLock);
        pthread_mutex_unlock(&questionLock);

        float prob = rand() % 10;
        if (p * 10 >= prob)
        {
            willSpeak = true;
            // commentator wants to answer

            timelog();
            printf("--Commentator %d will speak \n", myID);

            // push to queue
            pthread_mutex_lock(&queueLock);
            answerQueue.push(myID);
            pthread_mutex_unlock(&queueLock);
        }
        else
        {
            willSpeak = false;
        }
        commentator++;

        // Signal to moderator that queue pushing is ended
        if (commentator == n)
        {
            pthread_sleep(0.2);
            pthread_cond_signal(&queueCond);
        }

        if (willSpeak)
        {
            // Wait for speak signal
            pthread_cond_wait(&speakCond, &speakLock);
            //if(myID == commentatorID){

            // Speak
            timelog();
            printf(">>>Commentator %d speaks \n", myID);
            pthread_sleep(getSpeakTime(t));

            // Send end of speaking signal
            pthread_cond_signal(&endSpeakCond);
            // }
            pthread_mutex_unlock(&speakLock);
        }

        timelog();

        printf("End of the commentator %d \n", myID);
    }

    pthread_exit(0);
}

void *moderatorExec(void *ptr) // moderator action
{
    for (int i = 0; i < q; i++)
    {
        timelog();

        printf("Moderator asks question %d \n", i + 1);
        pthread_sleep(0.2);
        pthread_cond_broadcast(&questionCond);

        // Wait every question to be pushed
        pthread_cond_wait(&queueCond, &queueLock);
        pthread_mutex_unlock(&queueLock);
        int queueSize = answerQueue.size();

        // Pop the queue in order
        for (int i = 0; i < queueSize; i++)
        {
            commentatorID = answerQueue.front();
            pthread_mutex_lock(&queueLock);
            answerQueue.pop();
            pthread_mutex_unlock(&queueLock);

            // Send speak signal
            pthread_cond_signal(&speakCond);

            // Wait for end of speaking signal
            pthread_cond_wait(&endSpeakCond, &endSpeakLock);
            pthread_mutex_unlock(&endSpeakLock);
        }
        printf("\n\n");
        commentator = 0;
    }

    pthread_exit(0);
}

// Return a random speaktime value
double getSpeakTime(int t)
{
    srand(time(NULL));
    double speakTime = rand() % t + 1; // random speak time
    return speakTime;
}

// Take and parse inputs
void inputCommands(int argc, char *argv[], double &p, int &n, int &q, int &t)
{
    int cmd;

    while ((cmd = getopt(argc, argv, "p:n:q:t:")) != -1)
    {
        switch (cmd)
        {
        case 'p':
            p = atof(optarg);
            printf("Answer with probability: %s\n", optarg);
            break;
        case 'n':
            n = atoi(optarg);
            printf("Number of commentators: %s\n", optarg);
            break;
        case 'q':
            q = atoi(optarg);
            printf("Number of questions: %s\n", optarg);
            break;
        case 't':
            t = atoi(optarg);
            printf("Maximum speak time: %s\n", optarg);
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }
}

// prints the timestamps as milliseconds
void timelog()
{
    clock_gettime(CLOCK_MONOTONIC, &tend);
    printf("%0.3f ms    ", ((double)tend.tv_sec + 1.0e-9 * tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9 * tstart.tv_nsec));

    /*
    time_t current_time;
    time(&current_time);
    double diff = difftime(current_time, start_time);
    printf("%f \n", diff*1000);
    */
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
