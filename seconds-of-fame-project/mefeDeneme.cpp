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

// DEFAULT INPUT PARAMETERS
float p = 0.8; // answering probability
int n = 5;     // number of commentators
int q = 3;     // number of questions
int t = 10;    // maximum possible speak time

// Helper parameters
int speaker = -1;           // -1 is the moderator, 0 to n is commentators, -9 is idle.
bool theEnd = false;        // when it is true it is end of the program
int currentQuestionNum = 0; // tracks which question we are discussing
std::queue<int> micQueue;   // queue for waiting commentators to answer
int commentatorID = 1;
pthread_mutex_t speakerLock; // lock for the speakers
bool commEnd = false;
pthread_mutex_t question_mutex; // lock for the next question

struct commentator
{
    int commentatorID;     // id of the commentator 1 to n
    char status;           // status of the commentator. it can waits on queue 'Q', speaks 'S' or just waits idle 'I'
    time_t requestTime;    // time when the commentator is joins the queue
    time_t execTime;       // time when the commentator leaves the queue
    time_t turnaroundTime; // execTime-requestTime
    pthread_mutex_t commentator_mutex = speakerLock;
};

struct moderator
{
    char status;      // status of the moderator. it can waits commentators answers 'W' or ask questions 'A'
    int question = 0; // current question
};

// Helper functions
// void *commentatorExec(void *i, int t, double p); // single commentator action
//void *commentatorExec(int id, double p, int t); // single commentator action
void *moderatorExec(void *ptr);                 // moderator action
float getSpeakTime(int t);                      // calculates random speak time between 1 and t
void *commentatorExec(void *ptr) ;

void *commentatorExec(void *ptr) //  single commentator action
{
    /*
    // randomize the commentators
    double sleepTime = ((double) rand() / (RAND_MAX));
    pthread_sleep(sleepTime * 2);
    */

    pthread_mutex_lock(&speakerLock);
    float prob = rand() % 10;
    
    if (p * 10 >= prob)
    {
        // commentator wants to answer
        printf("in COMMENTATOR %d \n", commentatorID + 1);
        // wait for new question
        printf("---Commentator %d speaks \n", commentatorID + 1);
        pthread_sleep(getSpeakTime(1));
        micQueue.push(commentatorID);
    }
    else{
        printf("---Commentator %d DON'T speaks \n", commentatorID + 1);

    }
        commentatorID++;
    /*
        printf("---In commentator lock released:\n");
        //pthread_mutex_lock(&iter_lock);
        printf("---In iter_lock in commentator:\n");
        */

    //pthread_mutex_unlock(&iter_lock);
    printf("end of the commentator %d \n", commentatorID);
    // exit(0);
    pthread_mutex_unlock(&speakerLock);
    if (commentatorID == n)
    {
        //pthread_cond_broadcast(&new_question);
        pthread_mutex_unlock(&question_mutex);
        printf("\n\n");
    }
    return NULL;
}

void *moderatorExec(void *ptr) //   moderator action
{
    pthread_mutex_lock(&question_mutex);
    pthread_t thread[n];
    if (currentQuestionNum < q)
    {
        commEnd = false;
        commentatorID = 0;
        //pthread_mutex_lock(&speakerLock);
        printf("ask question %d \n", currentQuestionNum + 1);
        currentQuestionNum++;
        //printf("+++In iter_lock in moderator:\n");
        //pthread_mutex_unlock(&speakerLock);

        //pthread_mutex_lock(&iter_lock);
        //printf("+++Time to iterate question. \n");
        // waits for new question signal from commentator
        //pthread_cond_wait(&new_question, &iter_lock);

        //pthread_mutex_unlock(&iter_lock);
        for (int i = 0; i < n; i++)
        {
            pthread_create(&thread[i], NULL, commentatorExec, NULL);
        }
    }

    return NULL;
}

// returns a random double value speaktime
float getSpeakTime(int t)
{
    srand(time(NULL));
    double speakTime = rand() % t + 1; // random speak time
    return speakTime;
}

// Helper function to take inputs
void inputCommands(int argc, char *argv[], double &p, int &n, int &q, int &t)
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
    return NULL;
}

int main(int argc, char *argv[])
{

    // takes input parameters
    //inputCommands(argc, argv, p, n, q, t);

    pthread_t modThread[q];

    printf("Total %d Commentators \n", n);
    printf("Total %d Questions \n", q);
    for (int i = 0; i < q; i++)
    {
        moderatorExec(NULL);
    }
    //pthread_mutex_unlock(&lock2);

    // wait for commentator threads in moderator
    if (currentQuestionNum == q)
    {
        pthread_mutex_lock(&question_mutex);
        printf("Program is finished!\n");
        return 1;
    }
}
