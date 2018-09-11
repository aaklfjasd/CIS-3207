#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <pthread.h>
#include "queue.h"

#define NUM_WORKERS 10
#define PORT 827
#define DELIMS " \r\n.?!,"

char **generateDict(char *);
void *workerFunc(void *);
void *logFunc(void *);
int checkSpelling(char *);
void toLowerCase(char *);

char **dict;
int dictLen = 0;

queue *workQueue;
queue *logQueue;

pthread_mutex_t workMutex, logMutex;
pthread_cond_t workAvailable, logAvailable;

int main(int argc, char *argv[]) {
    // Initialize queues
    workQueue = initQueue();
    logQueue = initQueue();

    // Delcare thread pool for workers and thread for logging
    pthread_t workers[NUM_WORKERS];
    pthread_t logger;

    // Initialize mutexes and condition variables
    pthread_mutex_init(&workMutex, NULL);
    pthread_cond_init(&workAvailable, NULL);

    pthread_mutex_init(&logMutex, NULL);
    pthread_cond_init(&logAvailable, NULL);


    // Check for dictionary argument
    if (argc >= 2) {
        printf("Loading dict from %s\n", argv[1]);
        generateDict(argv[1]);
    } else {
        generateDict("/usr/share/dict/words");
    }

    // Create thread pool
    for (int t=0; t < NUM_WORKERS; t++){
        pthread_create(&(workers[t]), NULL, workerFunc, NULL);
    }

    // Create logger thread
    pthread_create(&logger, NULL, logFunc, NULL);

    // Declare necessary socket variables
    int sockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Ensure socket was created
    if (sockfd < 0) {
        printf("ERROR opening socket\n");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    // Set up socket info
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Attempt to bind socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) {
        printf("ERROR on binding\n");
        exit(1);
    }

    // Listen on socket
    listen(sockfd,5);

    clilen = sizeof(cli_addr);

    while(1) {
        // Accept an incoming connection
        int *newsockfd = (int *)malloc(sizeof(int));
        *newsockfd = accept(sockfd,
                           (struct sockaddr *) &cli_addr,
                           &clilen);
        if (newsockfd < 0) {
            printf("ERROR on accept\n");
        }

        // Get exclusive access to the connections queue
        pthread_mutex_lock(&workMutex);
        // Add the new connection to the queue
        addQueue(workQueue, newsockfd);
        // Signal the workers that there is a connection available
        pthread_cond_signal(&workAvailable);
        // Release mutual exclusion
        pthread_mutex_unlock(&workMutex);
        // Repeat forever
    }

    return 0;
}

// Generate the dictionary array for spellchecking
char **generateDict(char *file) {
    // Open the file, and ensure it was opened correctly
    FILE *dictFile = fopen(file, "r");
    if (dictFile == NULL) {
        printf("Dictionary file couldn't be opened.\n");
        exit(1);
    }

    // Assume the dictionary is fairly large
    int dictSize = 10000;
    dict = (char **) malloc(dictSize * sizeof(char *));

    // Buffer for words, no word should be longer than 50 letters
    char word[50];
    // Loop through all words in the file
    while (fscanf(dictFile, "%s", word) > 0) {
        // If the array is full, make it bigger
        if (dictLen == dictSize) {
            dictSize += 1000;
            dict = realloc(dict, dictSize * sizeof(char *));
        }
        // Copy the word to the dictionary
        dict[dictLen] = malloc(sizeof(word));
        strcpy(dict[dictLen], word);
        dictLen++;
    }
    // Close the file
    fclose(dictFile);
}

// Function to be run by worker threads
void *workerFunc(void *arg) {
    while (1) {
        // Get exclusive access to the connection queue
        pthread_mutex_lock(&workMutex);
        // Wait for a connection to appear in the queue
        while (empty(workQueue))
            pthread_cond_wait(&workAvailable, &workMutex);
        // Remove the element from the queue
        int *fd = (int *) removeQueue(workQueue);
        // Release the queue
        pthread_mutex_unlock(&workMutex);

        // Buffer for reading from the socket
        char buf[1024];
        memset(&buf, 0, sizeof(buf));

        // Read from the socket
        read(*fd, buf, 1024);

        // Tokenize the received string
        char *word = strtok(buf, DELIMS);
        while (word != NULL) {
            char *message = malloc(128);
            // Check the word spelling, and give an appropriate response
            if (checkSpelling(word)) {
                sprintf(message, "\"%s\" was spelled correctly.\n", word);
            } else {
                sprintf(message, "\"%s\" was spelled incorrectly.\n", word);
            }
            // Print the response
            dprintf(*fd, "%s", message);
            // Add the response to the logging queue
            pthread_mutex_lock(&logMutex);
            addQueue(logQueue, message);
            pthread_mutex_unlock(&logMutex);
            // Tell the logger that data is available
            pthread_cond_signal(&logAvailable);
            // Get the next word
            word = strtok(NULL, DELIMS);
        }
        // Acknowledge the end of the input
        dprintf(*fd, "All words checked\n");
        // Close and free the socket descriptor
        close(*fd);
        free(fd);
    }
}

// Function to be run by the logger thread
void *logFunc(void *arg) {
    // Open the logging file in append mode
    FILE *log = fopen("logfile.txt", "a");
    while (1) {
        // Get exclusive access to the log queue
        pthread_mutex_lock(&logMutex);
        // Wait for an item to appear in the queue
        while (empty(logQueue))
            pthread_cond_wait(&logAvailable, &logMutex);
        // Remove the message from the queue
        char *message = removeQueue(logQueue);
        // Release the queue
        pthread_mutex_unlock(&logMutex);
        // Print the removed to the log file
        fprintf(log, "%s", message);
        fflush(log);
        free(message);
    }
}

// Function to check the spelling of a word
int checkSpelling(char *buff) {
    // Convert the word to lowercase
    toLowerCase(buff);
    // Check every word in the dictionary to see if it is the same
    for (int i = 0; i < dictLen; i++) {
        if (strcmp(buff, dict[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to put a word in all lowercase
void toLowerCase(char *buff) {
    for (int i = 0; buff[i] != '\0'; i++) {
        if(buff[i] >= 'A' && buff[i] <= 'Z') {
            buff[i] = 'a' + (buff[i] - 'A');
        }
    }
}