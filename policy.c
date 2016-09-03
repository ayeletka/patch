//
// Created by ayelet on 8/24/16.
//


#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
    if(argc != 2){
        printf(2, "usage: policy id...\n");
        exit(0);
    }
    printf(1, "Policy selected: %d", atoi(argv[1]));
    schedp(atoi(argv[1]));
    exit(0);
}