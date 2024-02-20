#include "../include/progressBar.h"  
#include <stdio.h>
#include <string.h>
void initProgressBar(ProgressBar* bar, int total, int current, char* label, char* progressString, char* delimiterL, char* delimiterR) {
    bar->total = total;
    bar->current = current;
    bar->label = label;
    bar->progressString = progressString;
    bar->delimiterL = delimiterL;
    bar->delimiterR = delimiterR;

    // Initialize the progress bar string with spaces
    memset(bar->bar, ' ', BAR_LENGTH*sizeof(char));
    bar->bar[BAR_LENGTH] = '\0';  // Null-terminate the string
}

// Function to update the progress bar
void updateProgressBar(ProgressBar* bar, int progress) {
    if (progress < 0 || progress > bar->total) {
        printf("Error: Invalid progress value. %d %d\n",bar->total,progress);
        return;
    }
    bar->current = progress;

    // Update the progress bar string
    int progressPercentage = (BAR_LENGTH * bar->current) / bar->total;
    memset(bar->bar, bar->progressString[0], progressPercentage*sizeof(char));
    memset(bar->bar + progressPercentage, ' ', BAR_LENGTH - progressPercentage);
}

void printProgressBar(const ProgressBar* bar) {
    printf("%s%s%s%s %d%%\n", bar->label, bar->delimiterL, bar->bar,bar->delimiterR, (100 * bar->current) / bar->total);
}

