// File: graph.c
// Author: Griffin Bishop
// Operating Systems A16

// Currently you can't do more than single digit time?
// Semaphores currently do not use 2D array,
// but it still works.
// Why do you need a semaphore for every edge when you can just have a semaphore for every
// node? A node signals its completion and then the rest have access.

#include "graph.h"

int PRINTINFO = 0;
time_t start_time;
int shared_value = 0;
int numnodes = 0;
struct threadnode* head = NULL;
struct threadnode* nodes = NULL;

sem_t shared_value_mutex;

int main(int argc, char** argv){
	printf("CS3013: Operating Systems Project 3\n");
	if(argc < 2){ 
		fprintf(stderr, "Not enough arguments.\n"); 
		return 1;}
	else if(argc > 2 && !strncmp(argv[2],"-v", 2))
		PRINTINFO = 1; // Enable verbose mode

	printf("Recieved (%d) arguments.\n", argc);
	printf("Loading file: (%s)\n", argv[1]);

	// Parse the lines of the file 
	if( parsefile(argv[1]) == -1){
		fprintf(stderr, "Incorrect graph file format\n");
		return 1;}

	//Get the current system time (epoch) before we start the threads
	start_time = time(NULL);	

	//init_semaphores();
	sem_init(&shared_value_mutex, 0, 1);			
	create_threads();
	
	int i;
	for(i = 0; i < numnodes; i++)
		pthread_join(threads[i], NULL);
	printf("Total computation resulted in a value of %d after %d seconds.\n", shared_value, elapsed_time());
	
	pthread_exit(NULL);//return 0;
}

/* elapsed_time()
 * Returns the elapsed time (int) since the threads started
 */
int elapsed_time(){
	return time(NULL) - start_time;
}

/* wait_for_deps(node)
 * Waits for all threads that are
 * depended on by the node given.
 */
void wait_for_deps(struct threadnode* node){
	int i;
	int numdeps = node->numdependencies;
	for(i = 0; i < numdeps; i++){ // For each dependency:
		/*int dep_id = node->dependencies[i] - 'A';
		int id = node->label - 'A';
		printf("Thread (%d) waiting on [%d][%d]\n",id, id, dep_id);
		
		sem_t waiton = semaphores[id][dep_id];
		sem_wait(&waiton);*/
		int dep_label = node->dependencies[i];
		// Find the semaphore
		struct threadnode* mynode = nodes;
		while(mynode != NULL){ // For every node, if the label matches, wait on its sem
			if(mynode->label == dep_label){
				sem_wait(&(mynode->semaphore));
				sem_post(&(mynode->semaphore));
				if(PRINTINFO)printsem(&(mynode->semaphore));
				if(PRINTINFO)printf("Current thread: %c\n", node->label);
			}

			mynode = mynode->next;
		}
	}
}

/* signal_deps(id) !!!!! UNUSED !!!!!!
 * signals the threads dependent on the given id
 * by posting the semaphores in the column of the semaphore 2D array of the id.
 */
void signal_deps(int id){
	int i;
	for(i = 0; i < 26; i++){
		int oldval;

		sem_t asem = semaphores[i][id];
		sem_getvalue(&asem, &oldval);	
	
		sem_post(&asem);
		int newval;
		sem_getvalue(&asem, &newval);
		if(PRINTINFO) printf("Signaling: [%d][%d] -> newval: %d. Oldval: %d\n", i, id, newval, oldval);
		
	}
}

void printsem(sem_t* sem){
	int val;
	sem_getvalue(sem, &val);
	printf("Sem val: %d.\n", val);
}

/* thread_start(node)
 * Called function when the threads start.
 * 1. Waits for dependent threads to complete
 * 2. Compute value (look up from node)
 * 3. sleep for node compute_time
 * 4. Accumulate computed value into global variable.
 * 5. Signal completion for all dependents (with the global semaphore list)
 */
void *thread_start(void *thread_node){
	struct threadnode* node = (struct threadnode*)thread_node;

	// Wait for completion of dependent nodes.
	wait_for_deps(node);	
	if(PRINTINFO) printf("Thread (%c) started!\n", node->label);
	
	// Lookup value in node, produce value
	int value = node->computed_value;

	// Wait for computation time
	sleep(node->compute_time);

	// Produce the value;
	//shared_value += value;
	sem_wait(&shared_value_mutex);
	if(node->stack != NULL){
		 node->opstack = reverse(node->opstack);
		 value = compute_value(node);
	}
	
	shared_value += value;
	sem_post(&shared_value_mutex);

	
	printf("Node (%c) computed a value of (%d) after (%d) seconds.\n", node->label, value, elapsed_time());
	
	// Signal dependent thread semaphores
	//signal_deps(node->label - 'A');
	sem_post(&(node->semaphore));
	if(PRINTINFO)printsem(&(node->semaphore));

	pthread_exit(NULL);
}

/* create_threads()
 * Creates one thread for every node in the global threadnode list.
 * Starts it the thread_start function, giving it it a pointer to the 
 * threadnode node.
 */
void create_threads(){
	struct threadnode* node = nodes;
	int i = 0;
	while(node != NULL){
		int thread_id = node->label - 'A';
		int code = pthread_create(&threads[i], NULL, thread_start, (void *)node);
		if(code){
			fprintf(stderr, "Thread creation error: %d.\n", code);
			exit(-1);
		}

		if(PRINTINFO)printf("Created thread (%d)\n", thread_id);	
		node = node->next; 
		i++;
	}
}

/* init_semaphores()
 * Creates a semaphore for every edge (from the global node list)
 * and puts them into the global semaphore list
 */
void init_semaphores(){
	// Goes through the list of threadnode nodes
	// Creates a semaphore in the 2d array for each dependency

	struct threadnode* node = nodes;
	while(node != NULL){ // For every node:
		char label = node->label;
		int id = label - 'A'; // Convert label to id number
		
		int numdeps = node->numdependencies;
		int i;
		for(i = 0; i < numdeps; i++){ // For every dependency in each node:
			char dep = node->dependencies[i];
			int dep_id = dep - 'A'; // Convert label to id number
				
			sem_t *asem = &(semaphores[id][dep_id]);
			// Initialize the semaphore to -1 so that the dependent thread blocks on wait.
			sem_init(asem, 0, 0);
			//int val;
			//sem_getvalue(asem, &val);
			//printf("Sem val: %d\n", val);
		}			
		node = node->next;
	}
}

int calc_op(int next_value, int prev_value, char op){
	int value = 0;
	if(op == '+'){
		value = next_value + prev_value;
	} else if(op == '-'){
		value = next_value - prev_value;
	} else if(op == '*'){
		value = next_value * prev_value;
	} else if(op == '/'){
		if(prev_value == 0){
			fprintf(stderr, "DIVIDE BY ZERO ERROR!");
			return 0;
		}
		value = next_value / prev_value;
	} else if(op == '%'){
		if(prev_value == 0){
			fprintf(stderr, "DIVIDE BY ZERO ERROR!");
			return 0;
		}
		value = next_value % prev_value;
	}
	return value;
}

void check_for_v(struct threadnode* node){
	struct arraystack* stack = node->stack;
	int* ptr = stack->head;

	while(ptr > stack->values){
		ptr--;

		// black magic
		
		if(*ptr == 0xfeeddead){
			//printf("FOUND SIGNAL\n");
			*ptr = shared_value;
		}
		
	}
}

int compute_value(struct threadnode* node){
	struct arraystack* stack = node->stack;
	struct arraystack* opstack = node->opstack;

	check_for_v(node);

	if(stack->size == 0) return 0;
	
	if(opstack->size == 0){
		 return pop(stack);
	}else{
		char op = pop(opstack);
		int first = pop(stack);
		int second = pop(stack);
		int retval = calc_op(second, first, op);
		push(stack, retval);
		printf("Calc: %d %c %d = %d\n", second, op, first, retval);

		return compute_value(node);	
	}
	
}













