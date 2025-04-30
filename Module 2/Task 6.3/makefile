all:
	gcc -fPIC -c addition.c -o addition.o
	gcc -fPIC -c substraction.c -o substraction.o
	gcc -fPIC -c multiplicate.c -o multiplicate.o
	gcc -fPIC -c division.c -o division.o
	
	gcc -shared addition.o -o libaddition.so
	gcc -shared substraction.o -o libsubstraction.so
	gcc -shared multiplicate.o -o libmultiplicate.so
	gcc -shared division.o -o libdivision.so

	gcc -o main main.c -L. -ldl