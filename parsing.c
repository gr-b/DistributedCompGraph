// File: parsing.c
// Author: Griffin Bishop
// Project 3

#include "graph.h"

/* read_line(file, buffer, length)
 * Reads the next line of the file into the given buffer of (len) size.
 * Returns -1 if there is a problem, or the number of characters buffered otherwise.
 */
int read_line(FILE* file, char* buffer, size_t len){
	char c;
	int i;
	for(i = 0; i < len; i++){
		c = fgetc(file);
		if(!feof(file)){
			if(c == '\n'){
				buffer[i] = 0;
				return i+1;
			}else{
				buffer[i] = c;
			}	
		} else return -1;
	}
	return -1; // Should not get here.
}

/* parsefile(file)
 * Consumes a file name and for each line in the file, 
 * parses the node label, the computed value, the computation time,
 * and a list of nodes which the current node depends on.
 * Puts this information in the global threadinfo array.
 * Returns 0 if the graph file format was correct, or -1 if not.
*/
int parsefile(char* filename){
	FILE* graphfile = fopen(filename,"r");
	if(graphfile == NULL){ 
		fprintf(stderr, "Error opening file: %s.\n", filename); 
		return 1;
	}else printf("File %s opened successfully!\n", filename);

	// File is open, parse it
	// For each line:
	char* line;
	size_t len = MAX_LEN; // Max length
	int numchars;
	while((	numchars = read_line(graphfile, line, len)) != -1){
		if(PRINTINFO)printf("Read line: %s\n", line);
		if( parseline(line) == -1)
			return -1;
	}

	return 0;
}

/* check_valid_int(token)
 * Checks the parsed token given to see if it is a valid integer
 * Is a macro which returns -1 if the int is not valid.i
 * If atoi returns zero, check if the token itself was just 0.
 */
#define check_valid_int(token) \
	if(atoi(token) == 0) \
		if(token[0] != '0' && strnlen(token, MAX_LEN)) \
			return -1; \
	if( token == NULL ) return -1; \

/* check_valid_letter(token) 
 * Checks the parsed token given to see if it is a valid, single, uppercase letter.
 * Is a macro which returns -1 if the token is not a valid char.i
 * The token is not valid if:
 * 	-It is null
 * 	-Its length is more than 1
 * 	-it is not an uppercase char (between 'A' and 'Z')
 */
#define check_valid_label(token) \
	if( token == NULL || strnlen(token, MAX_LEN) > 1) \
		return -1; \
	if( !( token[0] >= 'A' && token[0] <= 'Z') ) \
		return -1; \

	
/* parseline(line)
 * Takes a line of the file(a string), and parses out the node label,
 * the computed value, the computation time, and a list of nodes which
 * it depends on. Puts this information in the global threadinfo array.
 * Returns -1 if the graph file format was incorrect, 0 otherwise.
 */
int parseline(char* line){
	struct threadnode* node = (struct threadnode*)malloc(sizeof(struct threadnode));
	sem_init(&(node->semaphore), 0, -1); // Initialize the semaphore to -1 because it's not done.

	if( nodes == NULL ){
		nodes = node;
		head = node;
	} else {
		head->next = node;
		head = node;
	}

	// Use strtok() to break line into tokens, atoi() to get an int for the fields.
	char* token;
	// Get the first 3 (All entried must have three):
	// Thread label
	token = strtok(line, " "); 
	check_valid_label(token);

	node->label = token[0];
	if(PRINTINFO) printf("Thread label: %c.\n", node->label);
	//////////////////	

	// Computed Value
	token = strtok(NULL, " ");
	check_valid_int(token); 
	
	node->computed_value = atoi(token);
	if(PRINTINFO) printf("Computed Value: %d.\n", node->computed_value);
	/////////////////

	// Computation time
	token = strtok(NULL, " ");
	check_valid_int(token);
	
	node->compute_time = atoi(token);
	if(PRINTINFO) printf("Compute Time: %d.\n", node->compute_time);
	////////////////


	// Dependencies
	int i = 0;
	int hasSeenEquals = 0; // are we looking at Polish Stack Notation numbers or operators?
	while((token =  strtok(NULL, " ")) != NULL){
		if(token != NULL && strnlen(token, MAX_LEN) > MAX_LEN){ // Check for invalid format
			return -1;
		}else {
			if(token[0] == '='){
				 hasSeenEquals = 1; 
				 continue;
			}

			if(!hasSeenEquals){
				check_valid_label(token);		
				node->dependencies[i] = token[0];
				if(PRINTINFO) printf("Dependency found: %c.\n", node->dependencies[i]); 
				i++;
				node->numdependencies++;
			} else {
				// Set up stack:
				if(node->stack == NULL){ // If we don't have a stack node yet, create the first one.
					node->stack = create_stack();
					node->opstack = create_stack();
					//node->stack = (struct stack*) malloc(sizeof(struct stack));
				} else {
					// The stack is linked backwards. Create a new node and link it to the current one.
					//struct stack* temp = (struct stack*) malloc(sizeof(struct stack));
					//temp->prev = node->stack;
					//node->stack = temp;
				}

				
				// We are after the = 
				// Valid tokens are I, V, +, -, /, *, %, and any number.
				int valid = 0;
				if(strnlen(token, 4) < 2){
					if(token[0] == 'I' || token[0] == 'V' || token[0] == '+' || token[0] == '-' 
						|| token[0] == '/' || token[0] == '*' || token[0] == '%'){
						valid = 1;
						if(token[0] == 'I')
							push(node->stack, node->label-'A');
						else if(token[0] == 'V')
							push(node->stack, 0xfeeddead);
						else
							push(node->opstack, token[0]);
						//node->stack->type = OPERATOR;
						//node->stack->operator_char = token[0];	
					}
				}
				// Check if it is a number
				if(strnlen(token, MAX_LEN) > 0 && atoi(token) > 0){ 
					valid = 1;
					push(node->stack, atoi(token));
					//node->stack->type = INTEGER;
					//node->stack->value = atoi(token);
				} else if(strnlen(token, MAX_LEN) == 0 && token[0] == '0'){
					 valid = 1;
					 push(node->stack, atoi(token));
					 //node->stack->type = INTEGER;
					 //node->stack->value = 0;
				}

				if(!valid) return -1;

			}
		}
	}
	if(PRINTINFO) printf("%d dependencies found.\n", i);
	numnodes++;

	return 0;
}


