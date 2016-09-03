
#include "types.h"
#include "stat.h"
#include "user.h"



int
main(int argc, char **argv) 
    {



    

    printf(1, "---------random test---------\n");
    int pid;
    int status;

    int i;
    for (i = 1; i < 6; i++) {
        static uint z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
        uint b;
        b = ((z1 << 6) ^ z1) >> 13;
        z1 = ((z1 & 4294967294U) << 18) ^ b;
        b = ((z2 << 2) ^ z2) >> 27;
        z2 = ((z2 & 4294967288U) << 2) ^ b;
        b = ((z3 << 13) ^ z3) >> 21;
        z3 = ((z3 & 4294967280U) << 7) ^ b;
        b = ((z4 << 3) ^ z4) >> 12;
        z4 = ((z4 & 4294967168U) << 13) ^ b;
        printf(1, "number: %d\n", (z1 ^ z2 ^ z3 ^ z4) % 50);
    }


    printf(1, "---------fork test---------\n");

    pid = fork();
    if (pid < 0)
        exit(-10);
    if (pid == 0)
        exit(-13);
    else {

        int n = wait(&status);
        int pidParent = getpid();
        printf(1, "pid: %d\n", pid);
        printf(1, "pidParent: %d\n", pidParent);

        printf(1, "status: %d\n", status);
        printf(1, "return val from wait: %d\n", n);
        //exit(-12);
    }

    printf(1, "---------status test---------\n");

    int pid2;
    int status2;

    if (!(pid2 = fork())) {
        exit(0x7f);
    } else {
        wait(&status2);
    }
    if (status2 == 0x7f) {
        printf(1, "OK\n");
    } else {
        printf(1, "FAILED\n");
    }

    printf(1, "---------exit test---------\n");
    //exit(12);
    return 7;
}

