all: 
	make clean
	make mysql.o
	make mysql_test.so

mysql.o: mysql.cpp mysql_test.h
	g++ mysql.cpp -o mysql.o -l mysqlcppconn

mysql_test.so: mysql_test.h mysql.cpp
	g++ -c -Wall -Werror -fpic mysql.cpp -l mysqlcppconn -o mysql_test.o
	g++ -shared -o mysql_test.so mysql_test.o -l mysqlcppconn

clean:
	rm -f *.o
	rm -f *.so
