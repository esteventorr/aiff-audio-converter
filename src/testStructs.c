#include "../include/userInterface.h"
#include "pthread.h"
#include "unistd.h"
#include "../include/selectView.h"
#include <string.h>

typedef struct 
{
    ProgressBarList* progressBarList;
    int stopFlag;
} ThreadArgs;

int main()
{
    pthread_t thread_id;

    ProgressBar* progressbars[7];

    UserInterface ui;
    memset(&ui,0,sizeof(ui));
    progressbars[0] = createDefaultProgressBar(&ui, "lean");
    progressbars[1] = createDefaultProgressBar(&ui, "lean1");
    progressbars[2] = createDefaultProgressBar(&ui, "lean2");
    progressbars[3] = createDefaultProgressBar(&ui, "lean3");
    progressbars[4] = createDefaultProgressBar(&ui, "lean4");
    progressbars[5] = createDefaultProgressBar(&ui, "lean5");
    progressbars[6] = createDefaultProgressBar(&ui, "lean6");


    ThreadArgs threadArgds;
    threadArgds.progressBarList = &ui.barList;
    threadArgds.stopFlag = 0;


    pthread_create(&thread_id, NULL, print_bars, &threadArgds);


    for (int i = 0; i < 7; i++)
    {
        sleep(1);
        updateProgressBar(progressbars[i],100);
    }

    sleep(1);

    threadArgds.stopFlag = 1;

    SelectView *sv;

    char *selection[] = {"lean", "dope", "kush"};

    sv = createSelectView("Lean right now", selection, 3, "respond xd");

    
    enableSelectView(sv);

    // initiate process handles files;char

    // code that updates pbs

    return 0;
}