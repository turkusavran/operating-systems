#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

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

struct commentator
{
    int commentatorID;     // id of the commentator 1 to n
    char status;           // status of the commentator. it can waits on queue 'Q', speaks 'S' or just waits idle 'I'
    time_t requestTime;    // time when the commentator is joins the queue
    time_t execTime;       // time when the commentator leaves the queue
    time_t turnaroundTime; // execTime-requestTime
    pthread_mutex_t commentator_mutex;
};

struct moderator
{
    char status;        // status of the moderator. it can waits commentators answers 'W' or ask questions 'A'
    int question = 0;   // current question
};

std::queue<commentator> micQueue; // queue for waiting commentators to answer

// INPUT PARAMETERS
int t = 10;  // maximum possible speak time
int p = 0.8; // answering probability
int n = 1;   // number of commentators
int q = 3;   // number of questions

struct commentator l[n];   // array for the commentators
pthread_cond_t threads[n]; // array for the commentators' threads

// Helper functions
void *commentatorExec(void *point); // single commentator action
void *moderatorExec(void *point);   // moderator action
float getSpeakTime();               // calculates random speak time between 1 and t

// Helper parameters
int speaker = -1;                   // -1 is the moderator, 0 to n is commentators, -9 is idle.
bool theEnd = false;                // when it is true it is end of the program
int currentQuestionNum = 0;         // tracks which question we are discussing

// creates pthreads
int pthread_create(
    pthread_t *,
    const pthread_attr_t *,
    void *(*start_routine)(void *),
    void *);

// TODO: BURASI ONEMLI ASIL IS BURADA
void *commentatorExec(void *point, int i) //  single commentator action
{
    float prob = rand() % 10;
    if (p * 10 >= prob)
    {
        // commentator wants to answer
        commentator c = l[i];
        c.status = 'W';
        while (speaker != -9)
        {
            // waits until microphone become available
            pthread_sleep(1);
        }
        // its time to speak
        speaker = i;
        c.status = 'S';
        pthread_sleep(getSpeakTime());
        speaker = -9;
        c.status = 'I';
    }
}

// TODO: BURASI ONEMLI ASIL IS BURADA
void *moderatorExec(void *point); //   moderator action
{
    // checks if all comentators have 'I' idle status to ask next question
    pthread_sleep(2);
    bool b = true;
    while (b)
    {
        pthread_sleep(1);
        bool readyToGo = true;
        for (size_t i = 0; i < n; i++)
        {
            commentator c = l[i];
            if (c.status = !'I')
            {
                readyToGo = false;
            }
        }
        if (readyToGo)
        {
            b = false;
        }   
    }

    // TODO: should ask the new question
    if (currentQuestionNum < q)
    {
        currentQuestionNum++;
    }
    else{
        theEnd = true;
    }
}

double getSpeakTime()
{
    srand(time(NULL));
    double speakTime = rand() % t + 1; // random speak time
    return speakTime;
}

int main()
{
    // TODO: Yukarıda oluşturduğumuz exec action functionları burada kullanmamız lazım

    pthread_t threads[n];
    int tn;
    for (tn = 0; tn < n; tn++)
    {
        commentator c;
        c.commentatorID = tn;
        c.status = 'I';
        l[tn] = c;
        pthread_create(&threads[tn], NULL, commentatorExec, (void *)tn, (int)tn);
    }

    for (tn = 0; tn < n; tn++)
    {
        pthread_join(threads[tn], NULL);
    }
    return 0;
}
