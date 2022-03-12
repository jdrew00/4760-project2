/*
Jacob Drew
4760 Project 2
slave.c

External resources:
https://www.geeksforgeeks.org/difference-fork-exec/
https://stackoverflow.com/questions/42295035/implementation-of-bakery-algorithm-in-c-for-forked-processes
https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_shared_memory.htm
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 1024
#define SHM_KEY 0x5050

int *choosing;
int *turnNum;
int childProc;
int shmid;
struct shmseg *shmp;
int shmid_choosing, shmid_turnNum;

struct shmseg
{
    int cnt;
    int complete;
    char buf[BUF_SIZE];
};

pid_t *children;

void handle_sigterm(int signum, siginfo_t *info, void *ptr)
{
    // detaching and deleting shared memory
    shmdt(shmp);
    shmdt(choosing);
    shmdt(turnNum);
    shmctl(shmid, IPC_RMID, NULL);
    shmctl(shmid_choosing, IPC_RMID, NULL);
    shmctl(shmid_turnNum, IPC_RMID, NULL);

    fprintf(stderr, "Process #%i was terminated by master\n", childProc);
    exit(0);
}

void catch_sigterm()
{
    static struct sigaction _sigact;
    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = handle_sigterm;
    _sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &_sigact, NULL);
}

int main(int argc, char *argv[])
{
    char *mypid = malloc(6); // ex. 34567
    sprintf(mypid, "%s", argv[1]);

    char logName[80];
    strcpy(logName, "logfile.");
    strcat(logName, mypid);

    char nn[6];
    strcat(nn, argv[2]);

    FILE *outputFile;
    outputFile = freopen(logName, "w+", stdout);

    FILE *cTestStream;
    cTestStream = fopen("ctest", "a");

    // define memory segment and pointer to message struct

    shmid = shmget(SHM_KEY, 1024, 0777 | IPC_CREAT);
    if (shmid == -1) // if creating shared memory is unsuccessful throw error
    {
        perror("Shared memory");
        return 1;
    }

    pid_t parent;
    pid_t child;
    parent = getppid();
    child = getpid();
    childProc = (int)(child - parent);
    key_t shmkey;
    shmkey = ftok("./slave", 118372); // arbitrary key #2
    shmid_choosing = shmget(shmkey, sizeof(choosing), 0600 | IPC_CREAT);
    choosing = (int *)shmat(shmid_choosing, NULL, 0);
    shmkey = ftok("./slave", 118373); // arbitrary key #3
    shmid_turnNum = shmget(shmkey, sizeof(turnNum), 0600 | IPC_CREAT);
    turnNum = (int *)shmat(shmid_turnNum, NULL, 0);

    catch_sigterm();
    signal(SIGINT, SIG_IGN);

    // Attach to the segment to get a pointer to it.
    shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *)-1)
    {
        perror("Shared memory attach");
        return 1;
    }

    int max = 0;

    int i;
    // child process loops five times
    for (i = 0; i < 5; i++)
    {

        // execute code to enter critical section
        choosing[(childProc - 1)] = 1;
        int maxCounter;
        for (maxCounter = 0; maxCounter < 18; maxCounter++) // only up to 18 processes
        {
            if ((turnNum[maxCounter]) > max)
                max = (turnNum[maxCounter]);
        }
        turnNum[(childProc - 1)] = 1 + max;
        // printf("turnNum for process #%i = %i\n", childProc, turnNum[(childProc - 1)]);
        choosing[(childProc - 1)] = 0;
        int j;
        for (j = 0; j < 18; j++)
        {
            while (choosing[j] == 1)
            {
            }
            while ((turnNum[j] != 0) && (turnNum[j] < turnNum[(childProc - 1)]))
            {
            }
        }

        // write to "c test"

        // get the time:
        char cTestString[100];
        time_t temp;
        struct tm *timeptr;
        temp = time(NULL);
        timeptr = localtime(&temp);

        // sleep for random number between 1-5
        int randomnumber;
        int pNum = atoi(argv[1]);
        srand(time(NULL) + (pNum * 10));
        randomnumber = (rand() % 5) + 1;

        char randomIntChar[2];
        sprintf(randomIntChar, "%d", randomnumber);

        sleep(randomnumber);

        // building the string
        strftime(cTestString, sizeof(cTestString), "%T", timeptr); // formats string in HH:MM:SS
        strcat(cTestString, " Queue ");
        strcat(cTestString, randomIntChar);
        strcat(cTestString, " File modified by process number ");
        strcat(cTestString, mypid);

        fprintf(outputFile, "%s\n", cTestString);
        fprintf(cTestStream, "%s\n", cTestString);

        randomnumber = (rand() % 5) + 1;
        sleep(randomnumber);
        // exit from critical section
        turnNum[(childProc - 1)] = 0;
    }

    // detatch shared memory
    if (shmdt(shmp) == -1)
    {
        perror("Master: shmdt");
        return 1;
    }

    shmdt(choosing);
    shmdt(turnNum);
    shmdt(shmp);
    shmctl(shmid, IPC_RMID, NULL);
    shmctl(shmid_choosing, IPC_RMID, NULL);
    shmctl(shmid_turnNum, IPC_RMID, NULL);

    fclose(outputFile);
    return 0;
}