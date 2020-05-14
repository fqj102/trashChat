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
const char* DBNAME = "trashchat";
const char* DBADDR = "tcp://127.0.0.1:3306";

// name mangling을 방지한다. 모든 함수 프로토타입은 중괄호 안에서 실행한다.
extern "C"
{
    char* DBSampleQuery (char* id_input, char* pw_input);
    char* CreateUser(char* user_id, char* user_pw, char* user_username);
    char* Login(char* user_id, char* user_pw);
    char* DeleteUser(int user_uid, char* user_pw);
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
    con = driver->connect(DBADDR, USERNAME, PASSWORD);
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
    con = driver->connect(DBADDR, USERNAME, PASSWORD);
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


char* CreateUser(char* user_id, char* user_pw, char* user_username)
{
    /*
    CreateUser : 사용자를 생성
        char* user_id : 생성할 사용자의 로그인 ID
        char* user_pw : 생성할 사용자의 로그인 비밀번호
        char* user_usernmae : 생성할 사용자의 닉네임
    */
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
    sql::ResultSet  *res;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;   
    sql::PreparedStatement  *prep_stmt;
    
    // MySQL에 연결한다. 
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect(DBADDR, USERNAME, PASSWORD);
    con->setSchema(DBNAME);
    
    std::string return_string = "{"; 
    try
    {
        // 쿼리를 실행한다.
        prep_stmt = con -> prepareStatement("INSERT INTO `users` (`UID`, `LOGIN_ID`, `LOGIN_PASSWORD`, `USERNAME`, `CHANNELS`) VALUES (NULL, ?, ?, ?, NULL)");
        prep_stmt -> setString(1, user_id);
        prep_stmt -> setString(2, user_pw);
        prep_stmt -> setString(3, user_username);
        res = prep_stmt -> executeQuery();
        
        return_string = return_string + "\"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        res = NULL;
        return_string = return_string + "\"result\": \"fail\", ";
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

char* Login(char* user_id, char* user_pw)
{
    /*
    Login : 로그인
        char* user_id : 생성할 사용자의 로그인 ID
        char* user_pw : 생성할 사용자의 로그인 비밀번호
    */
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
    sql::ResultSet  *res;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;   
    sql::PreparedStatement  *prep_stmt;
    
    // MySQL에 연결한다. 
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect(DBADDR, USERNAME, PASSWORD);
    con->setSchema(DBNAME);
    
    std::string return_string = "{"; 
    try
    {
        // 쿼리를 실행한다.
        prep_stmt = con -> prepareStatement("SELECT `UID`, IFNULL(`CHANNELS`, "") AS 'CHANNELS' FROM `users` WHERE `LOGIN_ID` = ? AND `LOGIN_PASSWORD` = ?");
        prep_stmt -> setString(1, user_id);
        prep_stmt -> setString(2, user_pw);
        res = prep_stmt -> executeQuery();
        
        res -> next();
        return_string = return_string + "\"UID\": \"" + res->getString("UID") + "\", "; 
        return_string = return_string + "\"CHANNELS\": \"" + res->getString("CHANNELS") + "\", ";
        return_string = return_string + "\"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        res = NULL;
        return_string = return_string + "\"result\": \"fail\", ";
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

char* DeleteUser(int user_uid, char* user_pw)
{
    /*
    DeleteUser : 사용자 삭제
        char* user_uid : 생성할 사용자의 고유 ID
        char* user_pw : 생성할 사용자의 로그인 비밀번호
    */
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
    sql::ResultSet  *res;
    sql::Connection *con;
    sql::mysql::MySQL_Driver *driver;   
    sql::PreparedStatement  *prep_stmt;
    
    // MySQL에 연결한다. 
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect(DBADDR, USERNAME, PASSWORD);
    con->setSchema(DBNAME);
    
    std::string return_string = "{"; 
    try
    {
        // 쿼리를 실행한다.
        prep_stmt = con -> prepareStatement("DELETE FROM `users` WHERE `UID` = ? AND `LOGIN_PASSWORD` = ?");
        prep_stmt -> setInt(1, user_uid);
        prep_stmt -> setString(2, user_pw);
        res = prep_stmt -> executeQuery();
        
        return_string = return_string + "\"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        return_string = return_string + "\"result\": \"fail\", ";
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