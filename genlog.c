#include <stdio.h>
#include <pthread.h> // NOTE: this requires you to compile and link with pthread
#include <time.h>

void genlog(char *caller_name, char *callee_name){
    
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    printf("function: %s-called by: %s-thread:%ld-timestamp:%ld\n", callee_name, 
            caller_name, pthread_self(), ts.tv_nsec);
}