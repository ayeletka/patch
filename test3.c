
#include "types.h"
#include "stat.h"
#include "user.h"

void f0(int num)
{
	printf(1, " performing f0:   proc %d received signal number:%d\n", getpid(),num);
}

void f1(int signal){
	printf(1, " performing f1:   proc %d received signal number:%d\n", getpid(),signal);
}

void f2(int signal){
	printf(1, " performing f2:   proc %d received signal number:%d\n", getpid(),signal);
}


void f3(int signal){
	printf(1, " performing f3:   proc %d received signal number:%d\n", getpid(),signal);
}


int
main(int argc, char **argv) 
{
	//defualt handlers for father
	int j;
	for (j = 0; j < 32; ++j){
		signal(j, f0);
	} 

	//sending 32 signals to father
	for (j = 0 ; j < 32; ++j){
		sigsend(getpid(), j);
	}

	int firstDadPid= getpid();
	printf(1,"father pid:%d child pid will be:%d\n", firstDadPid, firstDadPid+1);	
	
	//updating handlers for father
	signal(1,f1);
	signal(2,f2);
	signal(3,f3);
	signal(4,f0);
	signal(5,f2);
	signal(6,f3);
	signal(7,f0);
	signal(8,f1);
	signal(9,f2);
	signal(10,f0);

	int sonPid = fork();

	if(!sonPid) /* son proccess*/{
		sigsend(firstDadPid, 1);
		sigsend(firstDadPid, 2);
		sigsend(firstDadPid, 3);
		sigsend(firstDadPid, 4);
		sigsend(firstDadPid, 20);
		exit(0);
	}
	else   /* dad proccess*/{
		sigsend(sonPid,5);
		sigsend(sonPid,6);
		sigsend(sonPid,7);
		sigsend(sonPid,23);
		wait(&sonPid);
		exit(0);
	}
	
	return 0;
} 
