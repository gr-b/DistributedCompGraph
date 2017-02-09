all: graph

graph: graph.c parsing.c graph.h stack.c
	gcc -c stack.c -g
	gcc -c parsing.c -lpthread -g
	gcc -c graph.c -lpthread -g
	gcc -g graph.o parsing.o stack.o -o graph -lpthread


clean:
	rm graph graph.o parsing.o stack.o
