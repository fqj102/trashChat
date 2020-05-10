all: 
	make mysql.so

mysql.so: mysql.h mysql.cpp
	g++ -c -Wall -Werror -fpic mysql.cpp -l mysqlcppconn -o mysql.o
	g++ -shared -o mysql.so mysql.o -l mysqlcppconn

clean:
	rm -f *.o
	rm -f *.so
