#include "../include/userInterface.h"
#include "pthread.h"
int main(){
    pthread_t thread_id;

    
    UserInterface ui;
    createDefaultProgressBar(&ui,"lean");
    createDefaultProgressBar(&ui,"lean1");
    createDefaultProgressBar(&ui,"lean2");
    createDefaultProgressBar(&ui,"lean3");

    pthread_create(&thread_id,NULL,print_bars,&ui.barList);

    
    pthread_join(thread_id, NULL);

    return 0;

}