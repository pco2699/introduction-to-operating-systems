#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_READERS 5
#define NUM_WRITERS 5
#define NUM_WRITES 20
#define NUM_READS 20

unsigned int gSharedValue = 0;
pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;  	/* mutex lock for buffer */
pthread_cond_t gReadPhase = PTHREAD_COND_INITIALIZER; 
pthread_cond_t gWritePhase = PTHREAD_COND_INITIALIZER;
int gReaders = 0; int gWaitingReaders = 0;


void *writer (void *param);
void *reader (void *param);

int main(int argc, char *argv[]) {

    
    int readerNum[NUM_READERS];
    int writerNum[NUM_WRITERS];

	pthread_t readerThreadIDs[NUM_READERS];
    pthread_t writerThreadIDs[NUM_WRITERS];
	int i;

    for (i = 0; i < NUM_WRITERS; i++) {
        writerNum[i] = i;
        if(pthread_create(&writerThreadIDs[i], NULL, writer, &writerNum[i]) != 0) {
            fprintf(stderr, "Unable to create writer thread\n");
            exit(1);
        }
    }

    for (i = 0; i < NUM_READERS; i++) {
        readerNum[i] = i;
        if(pthread_create(&readerThreadIDs[i], NULL, reader, &readerNum[i]) != 0) {
            fprintf(stderr, "Unable to create reader thread\n");
            exit(1);
        }
    }

    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writerThreadIDs[i], NULL);
    }

    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(readerThreadIDs[i], NULL);
    }

	return 0;
}

/* Produce value(s) */
void *writer(void *param) {

    int id = *((int*) param);
	int i, numReaders = 0;
	for (i = 0; i < NUM_WRITES; i++) {
		usleep(1000 * random() % NUM_READERS + NUM_WRITERS);
		pthread_mutex_lock (&gMutex);	
			while (gReaders != 0)
                pthread_cond_wait(&gWritePhase, &gMutex);
            gReaders = -1;
            numReaders = gReaders;
		pthread_mutex_unlock(&gMutex);

        fprintf(stdout, "[w%d] writing %u* [readers: %d]\n", id, ++gSharedValue, numReaders);

        pthread_mutex_lock(&gMutex);
            gReaders = 0;
            if (gWaitingReaders > 0) {
                pthread_cond_broadcast(&gReadPhase);
            } else {
                pthread_cond_signal(&gWritePhase);
            }
        pthread_mutex_unlock(&gMutex);
	}

    pthread_exit(0);
}

void *reader(void *param) {
    
    int id = *((int*) param);

    int i; int numReaders = 0;
    for (i = 0; i < NUM_READS; i++) {
        usleep(1000 * random() % NUM_READERS + NUM_WRITERS);
        pthread_mutex_lock(&gMutex);
            gWaitingReaders++;
            while(gReaders == -1)
                pthread_cond_wait(&gReadPhase, &gMutex);
            gWaitingReaders--;
            numReaders = ++gReaders;
        pthread_mutex_unlock(&gMutex);

        fprintf(stdout, "[r%d] reading %u  [readers: %d]\n", id, gSharedValue, numReaders);

        pthread_mutex_lock(&gMutex);
            gReaders--;
            if (gReaders == 0)
                pthread_cond_signal(&gWritePhase);
        pthread_mutex_unlock(&gMutex);
    }

    pthread_exit(0);
}