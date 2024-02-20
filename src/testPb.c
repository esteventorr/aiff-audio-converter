#include "../include/progressBar.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define NUM_BARS 20

ProgressBar bars[NUM_BARS];


void* update_bars(void* arg) {
    while(1) {

        for(int i = 0; i < NUM_BARS; i++) {
            int progress = rand() % (bars[i].total + 1);  // Random progress
            updateProgressBar(&bars[i], progress);
        }
        sleep(1);  // Sleep for a while before updating again
    }
    return NULL;
}

void* print_bars(void* arg) {
    while(1) {

        for(int i = 0; i < NUM_BARS; i++) {
            printProgressBar(&bars[i]);
        }
        //TODO: Modify this line so it works with n bars
        printf("\033[20A");
        sleep(1);  // Sleep for a while before printing again
    }
    return NULL;
}

int main() {
    pthread_t update_thread, print_thread;

    // Initialize the progress bars
    for(int i = 0; i < NUM_BARS; i++) {

        char* processName = malloc(50);   

        snprintf(processName,50,"Loading process %d",i);


        initProgressBar(&bars[i], 100, 0, processName, "=", "[", "]");
    }


    // Create the threads
    pthread_create(&update_thread, NULL, update_bars, NULL);
    pthread_create(&print_thread, NULL, print_bars, NULL);

    // Wait for the threads to finish (they won't, in this example)
    pthread_join(update_thread, NULL);
    pthread_join(print_thread, NULL);

    return 0;
}