#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "buffer.h"

void* call_producer(void* arg);
void* call_consumer(void* arg);


int main()
{

    int rv = 1000;

    { // calling enqueue without initializing
        printf("Test: calling enqueue without initializing the buffer\n");

        rv = enqueue_buffer_421("1");

        if (rv < 0)
        {
            printf("failed to enqueue the data!\n");
        }
        else
        {
            printf("Successfully enqueued!\n");
        }

        printf("\n");
    }


    { // calling dequeue without initializing
        printf("Test: calling dequeue without initializing the buffer\n");

        char str[DATA_LENGTH];
        rv = dequeue_buffer_421(str);

        if (rv < 0)
        {
            printf("failed to dequeue!\n");
        }
        else
        {
            printf("Successfully dequeued!\n");
        }

        printf("\n");
    }

    { // initializing the buffer
        printf("Test: Initializing the buffer\n");

        rv = init_buffer_421();

        if (rv < 0)
        {
            printf("initialization failed!\n");
        }
        else
        {
            printf("Successfully initialized!\n");
        }

        printf("\n");
    }

    { // re-initializing the buffer

        printf("Test: reinitializing the buffer without deleting the existing one\n");

        rv = init_buffer_421();

        if (rv < 0)
        {
            printf("initialization failed!\n");
        }
        else
        {
            printf("Successfully initialized!\n");
        }

        printf("\n");
    }


    { // deleting the buffer
        printf("Test: deleting the buffer\n");

        rv = delete_buffer_421();

        if (rv < 0)
        {
            printf("delete failed!\n");
        }
        else
        {
            printf("delete function ran successfully!\n");
        }

        printf("\n");
    }


    { // deleting the non existing buffer
        printf("Test: deleting the non-existing buffer\n");

        rv = delete_buffer_421();

        if (rv < 0)
        {
            printf("failed to delete the buffer!\n");
        }
        else
        {
            printf("delete function ran successfully!\n");
        }

        printf("\n");
    }

    printf("----------------------------------");
    printf(" Starting the multi-threading test ");
    printf("----------------------------------\n");


    pthread_t producer, consumer; // two threads
    init_buffer_421();

    pthread_create(&producer, NULL, &call_producer, NULL);
    pthread_create(&consumer, NULL, &call_consumer, NULL);

    if (pthread_join(producer, NULL) != 0)
    {
        perror("Failed to join thread");
    }
    if (pthread_join(consumer, NULL) != 0)
    {
        perror("Failed to join thread");
    }

    delete_buffer_421();

    return 0;

}

void* call_producer(void* arg)
{
    printf("producer!\n\n");

    char data[DATA_LENGTH];

    int value = 0;

    for (int i = 0; i < 5000; i++)
    {
        for (int j = 0; j < DATA_LENGTH; j++)
        {
            data[j] = (value % 10) + '0';
        } // creating the data to feed the enqueue func

        usleep((rand() % 11) * 1000);

        enqueue_buffer_421(data);
        value++;

        printf("\n");
    }

    return NULL;
}

void* call_consumer(void* arg)
{
    printf("consumer!\n\n");

    char str[DATA_LENGTH+1];

    for (int i = 0; i < 5000; i++)
    {
        usleep((rand() % 11) * 1000);

        dequeue_buffer_421(str);

        printf("\n");
    }

    return NULL;
}
