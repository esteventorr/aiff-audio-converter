#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#define BAR_LENGTH 100

// Struct for the progress bar
typedef struct {
    int total;      
    int current;    
    char* label;    
    char* progressString;   
    char* delimiterL;
    char* delimiterR;
    char bar[BAR_LENGTH+1]; 
} ProgressBar;  

// Function declarations
void initProgressBar(ProgressBar* bar, int total, int current, char* label, char* progressString, char* delimiterL, char* delimiterR);
void updateProgressBar(ProgressBar* bar, int progress);
void printProgressBar(const ProgressBar* bar);

#endif