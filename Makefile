all: trace

trace: LC4.o loader.o trace.c

	clang -g LC4.o loader.o trace.c -o trace

LC4.o: LC4.c

	clang -g LC4.c -c

loader.o: loader.c

	clang -g loader.c -c 

clean:
	rm -rf *.o

clobber: clean
	rm -rf trace
