/*
Jacob Drew
4760 Project 2
slave.c
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

int main()
{

    int shmid;
    //struct shmseg *shmp;
    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("Shared memory");
        return 1;
    }

    printf("hello");
    return 0;
}