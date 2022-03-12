/*
Jacob Drew
4760 Project 2
Master.c

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

#define BUF_SIZE 1024
#define SHM_KEY 0x5050

struct shmseg
{
    int cnt;
    int complete;
    char buf[BUF_SIZE];
};

pid_t *children;
int n;
int shmid;
struct shmseg *shmp;

void handle_sigalrm(int signum, siginfo_t *info, void *ptr)
{
    // prevents multiple interrupts
    signal(SIGINT, SIG_IGN);

    fprintf(stderr, "Master ran out of time\n");

    // detaching and deleting shared memory
    shmdt(shmp);
    shmctl(shmid, IPC_RMID, NULL);

    // creating tmp_children to replace children
    // this way children can be freed before SIGTERM
    pid_t tmp_children[n];
    int i;
    for (i = 0; i < n; i++)
    {
        tmp_children[i] = children[i];
    }

    // freeing allocated memory
    free(children);

    // terminate child processes
    for (i = 0; i < n; i++)
    {
        kill(tmp_children[i], SIGTERM);
    }
}

void handle_sigint(int signum, siginfo_t *info, void *ptr)
{
    // prevents multiple interrupts
    signal(SIGINT, SIG_IGN);
    signal(SIGALRM, SIG_IGN);

    fprintf(stderr, " interrupt was caught by master\n");

    // detaching and deleting shared memory
    shmdt(shmp);
    shmctl(shmid, IPC_RMID, NULL);

    // creating tmp_children to replace children
    // this way children can be freed before SIGTERM
    pid_t tmp_children[n];
    int i;
    for (i = 0; i < n; i++)
    {
        tmp_children[i] = children[i];
    }

    // freeing allocated memory
    free(children);

    // terminate child processes
    for (i = 0; i < n; i++)
    {
        kill(tmp_children[i], SIGTERM);
    }
}

void catch_sigalrm()
{
    static struct sigaction _sigact;
    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = handle_sigalrm;
    _sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGALRM, &_sigact, NULL);
}

void catch_sigint()
{
    static struct sigaction _sigact;
    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = handle_sigint;
    _sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &_sigact, NULL);
}


int main(int argc, char *argv[])
{
    // declare variables
    int opt;
    int ss;

    ss = 100;

    // getopt
    // process command line args
    while ((opt = getopt(argc, argv, ":t:h")) != -1)
    {
        switch (opt)
        {
        case 'h':
            printf("Help:\n");
            printf("How to run:\n");
            printf("master [-t ss n] [-h]\n");
            printf("ss the maximum time in seconds (default 100 seconds) after which the process should terminate itself if not completed\n");
            printf("n number of slave processes to execute\n");
            printf("If n is over 20 it will be set to 20 for safety!\n");
            // if -h is the only arg exit program
            if (argc == 2)
            {
                exit(0);
            }
            break;
        case 't':
            ss = atoi(optarg);
            n = atoi(argv[3]);
            printf("SS: %d\n", ss);
            if (n > 18)
            {
                n = 18;
                printf("Number of processes set to 20 for safety\n");
                // printf("N: %d\n", n);
            }
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    // setup shared memory:
    // from geeks for geeks:
    // shmget returns an identifier in shmid
    // 1024 as size
    // 0777 so anyone can read write execute this memory, also 777 is an abnormal permision so it makes it easy to see that it is detatched when we run 'ipcs'
    shmid = shmget(SHM_KEY, 1024, 0777 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("Master: Shared memory");
        return 1;
    }

    // Attach to the segment to get a pointer to it.
    shmp = shmat(shmid, (void *)0, 0);
    if (shmp == (void *)-1)
    {
        perror("Shared memory attach");
        return 1;
    }

    // catch sigs
    catch_sigint();
    catch_sigalrm();
    alarm(ss); // ss from command args

    // initializing pids
    if ((children = (pid_t *)(malloc(n * sizeof(pid_t)))) == NULL)
    {
        errno = ENOMEM;
        perror("children malloc");
        exit(1);
    }
    pid_t pid;
    int i;
    for (i = 0; i < n; i++)
    {
        // fork and exec one process
        pid = fork();
        if (pid == -1)
        {
            // pid == -1 means error occured
            printf("can't fork, error occured\n");
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {

            // have the child attatch to shared memory
            shmp = shmat(shmid, (void *)0, 0);
            if (shmp == (void *)-1)
            {
                perror("Shared memory attach");
                return 1;
            }

            char *childNum = malloc(6);
            sprintf(childNum, "%d", i);

            children[i] = pid;
            char *args[] = {"./slave", (char *)childNum, "8", (char *)0};
            execvp("./slave", args);
            perror("execvp");
            exit(0);
        }

    }

    // waiting for all child processes to finish
    for (i = 0; i < n; i++)
    {
        int status;
        waitpid(children[i], &status, 0);
    }

    free(children);

    // detatch shared memory
    if (shmdt(shmp) == -1)
    {
        perror("Master: shmdt");
        return 1;
    }

    // destroy the shared memory
    // if (shmctl(shmid, IPC_RMID, NULL) == -1)
    // {
    //     perror("shmctl");
    //     return 1;
    // }

    return 0;
}