#include "../include/userInterface.h"
#include "../include/progressBarList.h"
#include <stdlib.h>
#include <stdio.h>

ProgressBar* createDefaultProgressBar(UserInterface *ui,char* label){

    ProgressBar* bar = malloc(sizeof(ProgressBar));
    ProgressBarNode* barNode = malloc(sizeof(ProgressBar));


    initProgressBar(bar, 100, 0, label, "=", "[", "]");


    barNode->progressBar = bar;

    appendProgressBar(&ui->barList,barNode);
    



    
}

//Must be passed an progressBarList struct
//TODO: Implement thread mutex, that's the lines are commented
void* print_bars(void* arg) {

    ProgressBarList *progressBarList = (ProgressBarList *)arg;
    ProgressBarNode *actualProgressBar =  progressBarList->head;

    while (1)
    {
        while(actualProgressBar != NULL) {
        //pthread_mutex_lock(&lock);
        printProgressBar(actualProgressBar->progressBar);

        actualProgressBar = actualProgressBar->next;
        //TODO: Modify this line so it works with n bars
        //pthread_mutex_unlock(&lock);
    }
    printf("\033[%dA",progressBarList->length);
    }
    
    
    
    return NULL;
}


