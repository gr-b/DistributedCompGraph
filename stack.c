#include "graph.h"

/*struct arraystack{
	int* head;
	int values[128];
	int size;
};*/

struct arraystack* create_stack(){
	struct arraystack* stack = (struct arraystack*)malloc(sizeof(struct arraystack));
	stack->head = stack->values;
	stack->size = 0;
	return stack;
}

void push(struct arraystack* tostack, int value){
	if(tostack->head > tostack->values+128){
		 fprintf(stderr,"Stack overflow\n");
		 return;
	}

	*(tostack->head) = value;
	tostack->head++;
	tostack->size++; 
}

int pop(struct arraystack* fromstack){
	if(fromstack->head < fromstack->values){ 
		fprintf(stderr, "Stack underflow\n");
		return -1;
	}
	//int retval = *(fromstack->head);
	*(fromstack->head) = 0;
	fromstack->head--;
	fromstack->size--;
	return *(fromstack->head);
}

struct arraystack* reverse(struct arraystack* fromstack){
	struct arraystack* reverse = create_stack();

	while(fromstack->size > 0)
		push(reverse, pop(fromstack));

	free(fromstack);	
	return reverse;
}
