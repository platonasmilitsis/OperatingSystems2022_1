all: main child

main: main.o helperfunctions.o
	gcc -g3  main.o helperfunctions.o -o main -lpthread -lrt

child: child.o helperfunctions.o
	gcc -g3  child.o helperfunctions.o -o child -lpthread -lrt

main.o : main.c
	gcc -c -g3  main.c -lpthread -lrt

child.o : child.c
	gcc -c -g3 child.c -lpthread -lrt

helperfunctions.o : helperfunctions.c
	gcc -c -g3 helperfunctions.c
	
clean: 
	rm main.o helperfunctions.o child.o main child