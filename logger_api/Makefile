all: 
	make logger.so


logger.so: logger.h logger.cpp
	g++ -c -Wall -Werror -fpic logger.cpp -o logger.o
	g++ -shared -o logger.so logger.o
clean:
	rm -f *.o
	rm -f *.so

