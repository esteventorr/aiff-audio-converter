#include "../include/progressBarList.h"


void appendProgressBar(ProgressBarList *barList,ProgressBarNode *progressBarNode){

    if(barList->head == 0){
        barList->head = progressBarNode;
        barList->length = 1;
    }else{;
        barList->tail->next = progressBarNode;
        barList->tail++;
        barList->length++;
    }
    barList->tail = progressBarNode;
    
}