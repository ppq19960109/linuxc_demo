#include "commom.h"
   pthread_t thread;


   void *thread_routine(void *arg)
   {
       while(1)
       {


       }
       pthread_exit(NULL);
   }
int pthread_init() {


    if (pthread_create(&thread, NULL, thread_routine, NULL)!=0) {
        fprintf(stderr, "create thread fail.\n");
        exit(-1);
    }


    return 0;

fail:

    return -1;
}

void pthread_destory() {
   pthread_cancel(thread);
}
