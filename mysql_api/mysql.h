#ifndef MYSQL_TEST_H
#define MYSQL_TEST_H

#include <string>
#include <cstring>

#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>

// DB 접속에 필요한 정보를 정의한다. 
const char* USERNAME = "test";
const char* PASSWORD = "testpassword";
const char* DBNAME = "test";

// name mangling을 방지한다. 모든 함수 프로토타입은 중괄호 안에서 실행한다.
extern "C"
{
    char* DBSampleQuery (char* id_input, char* pw_input);
    int GetRowCount(std::string userid);
}

int GetRowCount(std::string userid)
{
    // connector에 연결할 때 쓰는 객체를 선언한다. 
    sql::PreparedStatement *prep_stmt;
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
        prep_stmt = con -> prepareStatement("SELECT COUNT(*) FROM `test` WHERE `ID` = ?");
        prep_stmt -> setString(1, userid);
        res = prep_stmt -> executeQuery();
        
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
    delete prep_stmt;
    delete con;
    
    // 행 갯수를 반환한다.
    return return_code;
}

char* DBSampleQuery(char* id_input, char* pw_input)
{
    int rowCount = GetRowCount(id_input); // 아이디가 존재하는지 확인한다.
    if (rowCount < 0) // 에러 발생
    {
        return (char *) "{\"error_code\": 0}";
    }
    else if (rowCount == 0) // 만족하는 결과 없음
    {
        return (char *) "{\"error_code\": 1}";
    }
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
    sql::ResultSet  *res;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;   
    sql::PreparedStatement  *prep_stmt;
    
    // MySQL에 연결한다. 
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", USERNAME, PASSWORD);
    con->setSchema(DBNAME);
    
    std::string return_string = "{"; 
    try
    {
        // 쿼리를 실행한다.
        prep_stmt = con -> prepareStatement("SELECT * FROM test WHERE `ID` = ? AND `PASSWORD` = ? ORDER BY `UID` ASC");
        prep_stmt -> setString(1, id_input);
        prep_stmt -> setString(2, pw_input);
        res = prep_stmt -> executeQuery();
        
        // 결과를 해석한다.
        while (res -> next()) 
        {
            return_string = return_string + "\"" + res->getString("uid") + "\": \"" + res->getString("username") + "\", "; 
        }        
        return_string = return_string + "\"data_count\": " + std::to_string(rowCount) + "}";
    }
    catch (sql::SQLException &e) 
    {
        res = NULL;
        return_string = return_string + "\"error code\": " + std::to_string(e.getErrorCode()) + "}";
    }
    
    // 사용한 Connector를 삭제한다. 
    delete res;
    delete prep_stmt;
    delete con;
    
    // 반환할 값을 준비한다. (string을 char* 형식으로 전환)   
    char *cstr = new char[return_string.length() + 1];
    strcpy(cstr, return_string.c_str());
    
    return (char*) cstr;
}

#endif