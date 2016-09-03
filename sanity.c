//
// Created by ayelet on 8/24/16.
//


#include "types.h"
#include "user.h"
#include "statForSanity.h"
#include "stat.h"


int
main(int argc, char *argv[])
{
    printf(1, "this is sanity program\n");
    int i, pid;
    int totalWaiting1 = 0;
    int totalRunning1 = 0;
    int totalTA1 = 0;
    int totalWaiting2 = 0;
    int totalRunning2 = 0;
    int totalTA2 = 0;
    int totalWaiting3 = 0;
    int totalRunning3 = 0;
    int totalTA3 = 0;


        for (i = 0; i < 30; i++)
        {
            pid = fork();
            if (pid == 0)
            {
                //child process:

                int childPID = getpid();
                //Type cpu only: Calculate long calculation
                if(childPID % 3 == 0){

                   // int j;
                    int time = uptime();
                    while(uptime() - time < 30) {}
                  //  cont2:
                 /*   for(j = 0; j< 100000; j++){
                        int a = 5;
                        int b = 8;
                        a = a * b;
                        b = a * b;
                        a = a * b;
                    }
                    if ((uptime()) < (30 + time)) {
                        goto cont2;
                    }*/

                }
                    //Type 'Blocking only':  30 sequential calls to the sleep
                else if(childPID % 3 == 1){
                    int j;
                    for(j = 0; j < 10 ; j++){
                        sleep(1);
                    }
                }
                else if (childPID % 3 == 2){
                    int i;
                    for( i = 0 ; i<5 ; i++){
                    int time2 = uptime();
                    while(uptime() - time2 < 5) {}
                    sleep(1);
                    }


                }
                exit(0);

            }

        }

        for (i = 0; i < 30; i++) {
            struct perf *pPerf = 0;
            int status = 0;
            int pidChild2;
            pidChild2 = wait_stat(&status, pPerf);
            if (pidChild2 % 3 == 0) {
                totalRunning1 += pPerf->rutime;
                totalWaiting1 += pPerf->retime;
                totalTA1 += pPerf->ttime - pPerf->ctime;
                printf(1, "waiting time: %d, running time %d, sleeping time : %d turnaround time: %d \n",
                       pPerf->retime, pPerf->rutime, pPerf->stime, pPerf->ttime - pPerf->ctime);
            }
            if (pidChild2 % 3 == 1) {

                totalRunning2 += pPerf->rutime;
                totalWaiting2 += pPerf->retime;
                totalTA2 += pPerf->ttime - pPerf->ctime;
                printf(1, "waiting time: %d, running time %d, sleeping time : %d turnaround time: %d \n",
                       pPerf->retime, pPerf->rutime, pPerf->stime, pPerf->ttime - pPerf->ctime);
            }
            if (pidChild2 % 3 == 2) {
                totalRunning3 += pPerf->rutime;
                totalWaiting3 += pPerf->retime;
                totalTA3 += pPerf->ttime - pPerf->ctime;
                printf(1, "waiting time: %d, running time %d, sleeping time : %d turnaround time: %d \n",
                       pPerf->retime, pPerf->rutime, pPerf->stime, pPerf->ttime - pPerf->ctime);
            }

        }
printf(1,"\n\n\n");
        printf(1,"----Type 1: averages----\n");
        printf(1,"total TA: %d, total Running: %d, total waitng: %d\n", totalTA1/10,totalRunning1/10,totalWaiting1/10);
        printf(1,"----Type 2: averages----\n");
        printf(1,"total TA: %d, total Running: %d, total waitng: %d\n", totalTA2/10,totalRunning2/10,totalWaiting2/10);
        printf(1,"----Type 3: averages----\n");
        printf(1,"total TA: %d, total Running: %d, total waitng: %d\n", totalTA3/10,totalRunning3/10,totalWaiting3/10);


        printf(1,"****All: averages****\n");
        printf(1,"total TA: %d, total Running: %d, total waitng: %d\n", (totalTA3+totalTA2+totalTA1)/30,(totalRunning3+totalRunning2+totalRunning1)/30,(totalWaiting3+totalWaiting2+totalWaiting1)/30);


        exit(0);




}





