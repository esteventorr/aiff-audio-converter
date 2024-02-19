#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "progressBarList.h"

typedef struct 
{
    ProgressBarList barList;

} UserInterface;



ProgressBar* createDefaultProgressBar(UserInterface *ui,char* label);
void* print_bars(void* arg);

#endif