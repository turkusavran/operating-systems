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
double getSpeakTime(int t);      // calculates random speak time between 1 and t
void *commentatorExec(void *ptr);
void inputCommands(int argc, char *argv[], double p, int &n, int &q, int &t);

// Default input parameters
float p = 0.8; // answering probability
int n = 5;     // number of commentators
int q = 3;     // number of questions
int t = 10;    // maximum possible speak time
int speaker = 0;
int commentatorID = 0;

// Global variables      
pthread_mutex_t speakerLock;    // lock for the speakers
pthread_mutex_t questionLock;   // lock for the next question
pthread_cond_t questionCond;    // cond for next question
pthread_mutex_t queueLock;      // lock for push answers to queue
pthread_cond_t queueCond;
pthread_mutex_t speakLock;      // lock for speaking in order
pthread_cond_t speakCond;
pthread_mutex_t endSpeakLock;
pthread_cond_t endSpeakCond;
pthread_mutex_t endOfQuestionLock;   // lock for the next question
pthread_cond_t endOfQuestionCond;

// Queues
std::queue<int> answerQueue;

// Main
int main(int argc, char *argv[])
{
    pthread_t moderatorThread;
    
    // takes input parameters
    inputCommands(argc, argv, p, n, q, t);
    pthread_t commentatorThread[n];
    printf("Total %d Commentators \n", n);
    printf("Total %d Questions \n\n", q);
    
    for (int i = 0; i < n; i++) 
    {
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
    int myID = pthread_self();
    bool willSpeak = false;
   
    for (int i = 0 ; i < q ; i++){
        pthread_cond_wait(&questionCond, &questionLock);
        pthread_mutex_unlock(&questionLock);
        speaker = 0;
        float prob = rand() % 10;
        //printf("random %f, our prob %f\n", prob, p);
        //if(p * 10 >= prob)
        if(false)
        {
            speaker++;
            willSpeak = true;
            // commentator wants to answer
            printf("Commentator %d will speak \n", myID);

            //push to queue
            pthread_mutex_lock(&queueLock);
            answerQueue.push(myID);
            pthread_mutex_unlock(&queueLock);
            printf("%d\n",answerQueue.back());
        }
        else
        {
            printf("---Commentator %d will not speak \n", myID);
        }

        //Signal everything is pushed
        pthread_cond_signal(&queueCond);

        if(willSpeak){
            //wait for you can speak signal
            pthread_cond_wait(&speakCond, &speakLock);
            if(myID == commentatorID){
                //speak
                printf("---Commentator %d speaks \n", myID);
                pthread_sleep(getSpeakTime(t));

                //konusmam bitti signal yolla
                pthread_cond_signal(&endSpeakCond);
            }
            pthread_mutex_unlock(&speakLock);
        } else {
            //konusmam bitti signal yolla
            pthread_cond_signal(&endSpeakCond);
        }

        printf("end of the commentator %d \n", myID);
        if (speaker==-1)
        {
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
        printf("Ask question %d \n", i + 1);
        pthread_cond_broadcast(&questionCond);

        // her seyin pushalnmasini bekle
        pthread_cond_wait(&queueCond, &queueLock);
        pthread_mutex_unlock(&queueLock);
       
        // Pop the queue
        for(int i = 0 ; i < speaker ; i++){
            //queuedon popla id yi esitle
            commentatorID = answerQueue.front();
            answerQueue.pop();
            printf("popped %d\n",answerQueue.front());
            // you can speak signal
            pthread_cond_signal(&speakCond);

            // konusmam bitti signal bekle
            pthread_cond_wait(&endSpeakCond, &endSpeakLock);
            pthread_mutex_unlock(&endSpeakLock);
        }
        speaker = -1;
        //commentator = 0;
        //her sey bitti sinyalini bekle sonraki soruya gec
       //pthread_cond_wait(&questionCond, &questionLock);
        //pthread_mutex_unlock(&questionLock);

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
void inputCommands(int argc, char *argv[], double p, int &n, int &q, int &t)
{
    int cmd;

    while ((cmd = getopt(argc, argv, "p:n:q:t:")) != -1)
    {
        switch (cmd) {
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
