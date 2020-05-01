#ifndef MYSQL_TEST_H
#define MYSQL_TEST_H

#include <iostream>
#include <string>
#include <cstring>
#include "mysql_test.h"

#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>

// DB 접속에 필요한 정보를 정의한다. 
const char* USERNAME = "test";
const char* PASSWORD = "testpassword";
const char* DBNAME = "test";

// name mangling을 방지한다. 모든 함수 프로토타입은 중괄호 안에서 실행한다.
extern "C"
{
    char* DBSampleQuery(int userid);
    int GetRowCount(int userid);
}

char* DBSampleQuery(int userid)
{
    // 해당 입력값을 만족하는 행의 갯수를 파악한다.
    int rowCount = GetRowCount(userid);
    if (rowCount < 0) // 에러 발생
    {
        return (char *) "Error occured. (Unknown error)";
    }
    else if (rowCount == 0)
    {
        return (char *) "Error occured. (No data match)";
    }
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
    sql::Statement *stmt;
    sql::ResultSet  *res;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;   
    
    // MySQL에 연결한다. 
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", USERNAME, PASSWORD);
    con->setSchema(DBNAME);
    
    std::string return_string; 
    try
    {
        // 쿼리를 실행한다.
        stmt = con -> createStatement();    
        res = stmt->executeQuery("SELECT * FROM test WHERE ID = " + std::to_string(userid) + " ORDER BY id ASC");
        
        // 열(Column) 수를 받아온다. 
        sql::ResultSetMetaData *res_meta = res -> getMetaData();
        int colCount = res_meta -> getColumnCount();
        std::cout << "COLLUMNS : " << colCount << std::endl;
        
        // 각 행(Row)을 출력한다. 
        while (res->next()) {
            for (int i = 1; i <= colCount; i++)
            {
                std::cout << res->getString(i) << "  |  "; 
            }
            std::cout << std::endl;
        }        
        return_string = "Query was successfully executed.";
    }
    catch (sql::SQLException &e) 
    {
        res = NULL;
        return_string = "Error occured. (error code: " + std::to_string(e.getErrorCode()) + ")";
        std::cout << return_string << "\n( SQLState: " << e.getSQLState() << " )" << std::endl;
    }
    
    // 사용한 Connector를 삭제한다. 
    delete res;
    delete stmt;
    delete con;
    
    // 반환할 값을 준비한다. (string을 char* 형식으로 전환)   
    std::cout << return_string.length() << std::endl;
    char *cstr = new char[return_string.length() + 1];
    strcpy(cstr, return_string.c_str());
    
    return (char*) cstr;
}

int GetRowCount(int userid)
{
    // connector에 연결할 때 쓰는 객체를 선언한다. 
    sql::Statement *stmt;
    sql::ResultSet  *res;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;   
    
    // MySQL에 연결한다. 
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", USERNAME, PASSWORD);
    con->setSchema(DBNAME);
    
    int return_code = -1;
    try
    {
        // 쿼리를 실행한다.
        stmt = con -> createStatement();    
        res = stmt->executeQuery("SELECT COUNT(*) FROM test WHERE ID = " + std::to_string(userid));
        
        res -> next();
        return_code = res->getInt(1);
    }
    catch (sql::SQLException &e) 
    {
        res = NULL;
        return_code = -1;
    }
    
    // 사용한 Connector를 삭제한다. 
    delete res;
    delete stmt;
    delete con;
    
    // 행 갯수를 반환한다.
    return return_code;
}
#endif