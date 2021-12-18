#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "buffer.h"

static ring_buffer_421_t buffer;
static sem_t mutex;
static sem_t fill_count;
static sem_t empty_count;


// test func
void* call_producer(void* arg);
void* call_consumer(void* arg);

long init_buffer_421(void)
{
	// Ensure we're not initializing a buffer that already exists.
	if (buffer.read || buffer.write) {
		printf("init_buffer_421(): Buffer already exists. Aborting.\n");
		return -1;
	}

	// Create the root node.
	node_421_t *node;
	node = (node_421_t *) malloc(sizeof(node_421_t));

	// Create the rest of the nodes, linking them all together.
	node_421_t *current;
	int i;
	current = node;

	// Note that we've already created one node, so i = 1.
	for (i = 1; i < SIZE_OF_BUFFER; i++)
	{
		current->next = (node_421_t *) malloc(sizeof(node_421_t));
		current = current->next;
	}
	// Complete the chain.
	current->next = node;

	buffer.read = node;
	buffer.write = node;
	buffer.length = 0;

	// Initializing the semaphores.

	if(sem_init(&mutex, 0, 1) != 0) // initialized to 1. This is a binary semaphore
    {
        printf("initialization failed: Aborting.\n");
        return -1;
    }

	if(sem_init(&fill_count, 0, 0) != 0) // initialized to 0 to indicate that not a single slot in the buff is filled yet
    {
        printf("initialization failed: Aborting.\n");
        return -1;
    }

	if(sem_init(&empty_count, 0, 20) != 0) // initialized to 20 to indicate that all 20 slots in the buffer are empty
    {
        printf("initialization failed: Aborting.\n");
        return -1;
    }

	printf("Buffer initialized successfully\n");

	return 0;
}

long enqueue_buffer_421(char* data)
{
	// NOTE: You have to modify this function to use semaphores.
	if (!buffer.write)
    {
		printf("write_buffer_421(): The buffer does not exist. Aborting.\n");
		return -1;
	}


    sem_wait(&empty_count);
    sem_wait(&mutex);

    printf("------ Enqueueing element into buffer ------\n");
    printf("%c%c%c%c%c%c%c.............\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);

	memcpy(buffer.write->data, data, DATA_LENGTH);

//  Advance the pointer.
	buffer.write = buffer.write->next;
	buffer.length++;

	printf("data enqueued successfully!!!\n");
    printf("%d item in buffer\n", buffer.length);

   	 sem_post(&fill_count);
	 sem_post(&mutex);

	return 0;
}

long dequeue_buffer_421(char * data) {

	if (!buffer.read)
    {
		printf("read_buffer_421(): The buffer does not exist. Aborting.\n");
		return -1;
	}


	sem_wait(&fill_count); // if fill count is > 0, we proceed
   	sem_wait(&mutex);  // acquiring the lock

    	printf("------ Dequeueing element from buffer ------\n");

	memcpy(data, buffer.read->data, DATA_LENGTH);
	data[DATA_LENGTH] = '\0'; // appending null

    	printf("%s\n", data);

	// advancing the pointer
	buffer.read = buffer.read->next;
	buffer.length--;

    printf("data dequeued successfully!!!\n");
	printf("%d item in buffer\n", buffer.length);

	 sem_post(&empty_count); // increasing empty count
     sem_post(&mutex); // releasing the lock


	return 0;
}

long delete_buffer_421(void)
{

	// Tip: Don't call this while any process is waiting to enqueue or dequeue.
	if (!buffer.read)
    {
		printf("delete_buffer_421(): The buffer does not exist. Aborting.\n");
		return -1;
	}

	sem_wait(&mutex);

	// Get rid of all existing nodes.
	node_421_t *temp;
	node_421_t *current = buffer.read->next;

	while (current != buffer.read)
    {
		temp = current->next;
		free(current);
		current = temp;
	}

	// Free the final node.
	free(current);
	current = NULL;

	// Reset the buffer.
	buffer.read = NULL;
	buffer.write = NULL;
	buffer.length = 0;

	printf("Buffer deleted successfully!\n");

	sem_post(&mutex);

	sem_destroy(&mutex);
	sem_destroy(&fill_count);
	sem_destroy(&empty_count);

	return 0;
}

void print_semaphores(void)
{
	// You can call this method to check the status of the semaphores.
	// Don't forget to initialize them first!
	// YOU DO NOT NEED TO IMPLEMENT THIS FOR KERNEL SPACE.
	int value;
	sem_getvalue(&mutex, &value);
	printf("sem_t mutex = %d\n", value);
	sem_getvalue(&fill_count, &value);
	printf("sem_t fill_count = %d\n", value);
	sem_getvalue(&empty_count, &value);
	printf("sem_t empty_count = %d\n", value);
	return;
}
