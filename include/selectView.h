#ifndef SELECTVIEW_H
#define SELECTVIEW_H

typedef struct 
{
    char* requestPrompt;
    char** selections;
    char* answerPrompt;
    int numSelections;
} SelectView;


int enableSelectView(SelectView* selectView);

SelectView* createSelectView(char* requestPrompt, char **selections,int numSelections,char* answerPrompt);
#endif