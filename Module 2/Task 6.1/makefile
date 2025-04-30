all:
	gcc -c linkedList.c -o linkedList.o
	ar rcs liblinkedList.a linkedList.o
	gcc main.c -L. -llinkedList -o main
	./main