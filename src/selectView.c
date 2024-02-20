#include "../include/selectView.h"
#include <stdlib.h>
#include <stdio.h>
int enableSelectView(SelectView *selectView)
{



    int selectedOption = -1;
    int validInput = 0;

    printf("\n");
    while (!validInput)
    {

        
        printf("%s\n\n", selectView->requestPrompt);

        for (int i = 0; i < selectView->numSelections; i++)
        {

            printf("%d) %s\n",(i+1),selectView->selections[i]);
        }

        printf("\n\33[2K%s:",selectView->answerPrompt);

        scanf("%d", &selectedOption);

        validInput = selectedOption > 0 && selectedOption <= selectView->numSelections;

        if(!validInput){
            printf("\033[%dA",(4+selectView->numSelections));
        }
}
return selectedOption;
}

SelectView *createSelectView(char *requestPrompt, char **selections, int numSelections, char *answerPrompt)
{

    SelectView *newSelectView = malloc(sizeof(SelectView));

    newSelectView->selections = selections;
    newSelectView->requestPrompt = requestPrompt;
    newSelectView->answerPrompt = answerPrompt;
    newSelectView->numSelections = numSelections;

    return newSelectView;
}
