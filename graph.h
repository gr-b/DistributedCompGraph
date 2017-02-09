// File: graph.h
// Author: Griffin Bishop
// Operating Systems A16 Project 3

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_LEN (128)
//#define PRINTINFO (1)
// End includes

sem_t semaphores[26][26];

extern int PRINTINFO;
extern time_t start_time;
pthread_t threads[26];
extern int numnodes;
extern struct threadnode* nodes;
extern struct threadnode* head;
extern int shared_value;

struct arraystack{
	int* head;
	int values[128];
	int size;
};

struct threadnode{
	struct threadnode* next;
	sem_t semaphore;
	char label;
	int computed_value;
	int compute_time;
	char dependencies[26];
	int numdependencies;
	struct arraystack* stack;
	struct arraystack* opstack;
};

// Function prototypes
int parsefile(char* filename);
int parseline(char* line);
void init_semaphores();
void *thread_start(void* thread_node);
void create_threads();
void printsem(sem_t* sem);
int elapsed_time();
int pop(struct arraystack* stack);
void push(struct arraystack* stack, int value);
struct arraystack* create_stack();
int compute_value(struct threadnode* node);
struct arraystack* reverse(struct arraystack* fromstack);
