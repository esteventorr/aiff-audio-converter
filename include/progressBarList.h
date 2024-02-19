#ifndef PROGRESSBARLIST_H
#define PROGRESSBARLIST_H
#include "progressBarNode.h"

typedef struct 
{
    ProgressBarNode* head; //Beginning of LinkedList
    ProgressBarNode* tail; // End of LinkedList
    int length;
} ProgressBarList;

void appendProgressBar(ProgressBarList *barList,ProgressBarNode *progressBarNode);

#endif