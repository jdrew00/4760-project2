/*
Jacob Drew
4760 Project 2
Master.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include<ctype.h>
#include<string.h>
#include<errno.h>
#include<signal.h>


#define BUF_SIZE 1024
#define SHM_KEY 0x5050

struct shmseg {
   int cnt;
   int complete;
   char buf[BUF_SIZE];
};

int main(int argc, char *argv[])
{
    // declare variables
    int opt;
    int ss;
    int n;

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
            if (n > 20)
            {
                n = 20;
                printf("Number of processes set to 20 for safety\n");
                printf("N: %d\n", n);
            }
            else
            {
                printf("N: %d\n", n);
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

    //setup shared memory:
    //from geeks for geeks:
  
    int shmid;
    struct shmseg *shmp;
    char *bufptr;

    // shmget returns an identifier in shmid
    //1024 as size 
    //0777 so anyone can read write execute this memory, also 777 is an abnormal permision so it makes it easy to see that it is detatched when we run 'ipcs'
    shmid = shmget(SHM_KEY,1024,0777|IPC_CREAT);
    if (shmid == -1) {
      perror("Master: Shared memory");
      return 1;
   }

   // Attach to the segment to get a pointer to it.
   shmp = shmat(shmid, NULL, 0);
   if (shmp == (void *) -1) {
      perror("Shared memory attach");
      return 1;
   }
    
    

    // destroy the shared memory
    shmctl(shmid,IPC_RMID,NULL);

    printf("shared memory: %s \n", str);

    return 0;
}