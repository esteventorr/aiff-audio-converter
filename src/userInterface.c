#include "../include/userInterface.h"
#include "../include/progressBarList.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef struct 
{
    ProgressBarList* progressBarList;
    int stopFlag;
} ThreadArgs;


ProgressBar* createDefaultProgressBar(UserInterface *ui,char* label){

    ProgressBar* bar = malloc(sizeof(ProgressBar));
    ProgressBarNode* barNode = malloc(sizeof(ProgressBarNode));


    initProgressBar(bar, 100, 0, label, "=", "[", "]");


    barNode->progressBar = bar;

    appendProgressBar(&ui->barList,barNode);

    return bar;    


    
}

//Must be passed an progressBarList struct
//TODO: Implement thread mutex, that's the lines are commented
void* print_bars(void* arg) {

    ThreadArgs *threadArgs = arg;

    ProgressBarList* progressBarList = threadArgs->progressBarList;

    ProgressBarNode *actualProgressBar;
    while (1)
    {
        actualProgressBar =  progressBarList->head;
        
        while(actualProgressBar != NULL) {
        printProgressBar(actualProgressBar->progressBar);

        actualProgressBar = actualProgressBar->next;

    }
    if(threadArgs->stopFlag){break;}

    printf("\033[%dA",progressBarList->length);

    }
    
    
    
    return NULL;
}


