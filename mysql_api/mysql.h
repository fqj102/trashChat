#ifndef MYSQL_TEST_H
#define MYSQL_TEST_H

#include <string>
#include <cstring>

#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>

/* DB 접속에 필요한 정보를 정의 */
const char* USERNAME = "test";
const char* PASSWORD = "testpassword";
const char* DBNAME = "trashchat";
const char* DBADDR = "tcp://127.0.0.1:3306";

extern "C" // name mangling을 방지한다. 
{
    /* 메시지 관련 함수 */
    char* GetMessage(int channel_id, int last_message);
    char* NewMessage(int user_id, int channel_id, char* message);
    
    /* 사용자 관련 함수 */
    char* CreateUser(char* user_id, char* user_pw, char* user_username);
    char* Login(char* user_id, char* user_pw);
    char* DeleteUser(int user_uid, char* user_pw);
    
}

char* CreateUser(char* user_id, char* user_pw, char* user_username)
{
    /*
    CreateUser : 사용자를 생성
        char* user_id : 생성할 사용자의 로그인 ID
        char* user_pw : 생성할 사용자의 로그인 비밀번호
        char* user_usernmae : 생성할 사용자의 닉네임
        
    반환값 : JSON
        result : 질의 결과를 나타낸다. (필수)
        - success / fail 
        error_code : 에러가 발생한 경우 포함된다. 
        - int : MySQL 에러 코드를 나타낸다.
    */
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
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
        prep_stmt = con -> prepareStatement("INSERT INTO `users` (`LOGIN_ID`, `LOGIN_PASSWORD`, `USERNAME`) VALUES (?, ?, ?)");
        prep_stmt -> setString(1, user_id);
        prep_stmt -> setString(2, user_pw);
        prep_stmt -> setString(3, user_username);
        prep_stmt -> executeUpdate();
        
        return_string = return_string + "\"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        return_string = return_string + "\"result\": \"fail\", ";
        return_string = return_string + "\"error code\": " + std::to_string(e.getErrorCode()) + "}";
    }
    
    // 사용한 Connector를 삭제한다. 
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
    
    반환값 : JSON
        result : 질의 결과를 나타낸다. (필수)
        - success / fail 
        error_code : 에러가 발생한 경우 포함된다. 
        - int : MySQL 에러 코드를 나타낸다.
    */
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
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
        /* 1. 사용자에서 제거 (* 로그인이만 안되게 처리. Username은 남아있어야 하기 때문 ) */
        prep_stmt = con -> prepareStatement("UPDATE `users` SET `LOGIN_PASSWORD` = \'##DELETED##\', "
        "ACTIVE = 0 WHERE `UID` = ? AND `LOGIN_PASSWORD` = ?");
        prep_stmt -> setInt(1, user_uid);
        prep_stmt -> setString(2, user_pw);
        prep_stmt -> executeUpdate();        
        
        /* 2. 채널 목록에서 제거 */
        prep_stmt = con -> prepareStatement("DELETE FROM `users` WHERE `UID` = ? AND `LOGIN_PASSWORD` = ?");
        prep_stmt -> setInt(1, user_uid);
        prep_stmt -> setString(2, user_pw);
        prep_stmt -> executeUpdate();
        
        return_string = return_string + "\"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        return_string = return_string + "\"result\": \"fail\", ";
        return_string = return_string + "\"error code\": " + std::to_string(e.getErrorCode()) + "}";
    }
    
    // 사용한 객체를 삭제한다. 
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
        
    반환값 : JSON
        result : 질의 결과를 나타낸다. (필수)
        - success / fail 
        error_code : 에러가 발생한 경우 포함된다. 
        - int : MySQL 에러 코드를 나타낸다.
        UID : 로그인에 성공한 경우 식별번호를 반환한다.
        - int : 사용자를 식별하는 데 쓰는 번호
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
        prep_stmt = con -> prepareStatement("SELECT `UID` FROM `users` WHERE `LOGIN_ID` = ? AND `LOGIN_PASSWORD` = ?");
        prep_stmt -> setString(1, user_id);
        prep_stmt -> setString(2, user_pw);
        res = prep_stmt -> executeQuery();
        
        res -> next();
        return_string = return_string + "\"UID\": \"" + res->getString("UID") + "\", "; 
        return_string = return_string + "\"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        res = NULL;
        return_string = return_string + "\"result\": \"fail\", ";
        return_string = return_string + "\"error code\": " + std::to_string(e.getErrorCode()) + "}";
    }
    
    // 사용한 객체를 삭제한다. 
    delete res;
    delete prep_stmt;
    delete con;
    
    // 반환할 값을 준비한다. (string을 char* 형식으로 전환)   
    char *cstr = new char[return_string.length() + 1];
    strcpy(cstr, return_string.c_str());
    
    return (char*) cstr;
}

char* GetMessage(int channel_id, int last_message)
{
    /*
    GetMessage : 메시지를 요청
        int channel_id : CHANNEL_ID
        int last_message : MESSAGE_ID
        
    반환값 : JSON
        result : 질의 결과를 나타낸다. (필수)
        - success / fail 
        error_code : 에러가 발생한 경우 포함된다. 
        - int : MySQL 에러 코드를 나타낸다.
        message : 입력된 MESSAGE_ID 이후에 온 메시지가 있으면 포함된다.
        - {
            username : 메시지를 전송한 사용자
            - string : USERNAME
            id : 메시지 식별번호를
            - int : MESSAGE_ID
            text : 메시지 본문
            - string : MESSAGE
        } : 각 메시지를 딕셔너리로 남은 리스트이다.
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
        std::string query_string = "SELECT U.USERNAME, M.MESSAGE_ID, M.MESSAGE FROM `messages` AS M "; 
        query_string += "JOIN `users` AS U ON M.UID = U.UID AND M.MESSAGE_ID > ? AND M.CHANNEL_ID = ? ";
        query_string += "ORDER BY M.MESSAGE_ID ASC";
        
        prep_stmt = con -> prepareStatement(query_string);
        prep_stmt -> setInt(1, last_message);
        prep_stmt -> setInt(2, channel_id);
        res = prep_stmt -> executeQuery();
        
        return_string += "\"message\" : [";
        while (res -> next())
        {
            return_string += "{";
            return_string += "\"username\": \"" + res->getString("USERNAME") + "\", "; 
            return_string +=  "\"text\": \"" + res->getString("MESSAGE") + "\", ";
            return_string +=  "\"id\": \"" + res->getString("MESSAGE_ID") + "\"},";
        }
        return_string = return_string.substr(0, return_string.length()-1);
        return_string += "], \"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        res = NULL;  // delete 할 때 버그가 있어서 추가
        return_string = return_string + "\"result\": \"fail\", ";
        return_string = return_string + "\"error code\": " + std::to_string(e.getErrorCode()) + "}";
    }
    
    // 사용한 Connector를 삭제한다. 
    delete res;
    delete prep_stmt;
    delete con;
    
    // 반환할 값을 준비한다. (string을 char* 형식으로 전환. Ctypes는 std::string 자료형을 지원하지 않는다.)   
    char *cstr = new char[return_string.length() + 1];
    strcpy(cstr, return_string.c_str());
    
    return (char*) cstr;
}

char* NewMessage(int user_id, int channel_id, char* message)
{
    /*
    NewMessage : 메시지 등록
        int user_id : UID
        int channel_id : CHANNEL_ID
        char* message : MESSAGE 
        
    반환값 : JSON
        result : 질의 결과를 나타낸다. (필수)
        - success / fail 
        error_code : 에러가 발생한 경우 포함된다. 
        - int : MySQL 에러 코드를 나타낸다.
    */
    
    // connector에 연결할 때 쓰는 객체를 선언한다. 
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
        // 
        con -> setAutoCommit(false);        
        
        std::string query_string = "INSERT INTO `messages` (`UID`, `CHANNEL_ID`, `MESSAGE_ID`, `MESSAGE`) ";
        query_string += "VALUES (?, ?, NULL, ?)";
        
        prep_stmt = con -> prepareStatement(query_string);
        prep_stmt -> setInt(1, user_id);
        prep_stmt -> setInt(2, channel_id);
        prep_stmt -> setString(3, message);
        prep_stmt -> executeUpdate();
        
        query_string = "UPDATE `channel_settings` SET `LAST_MESSAGE_ID` = (SELECT LAST_INSERT_ID()) ";
        query_string += "WHERE `CHANNEL_ID` = ?";
        prep_stmt = con -> prepareStatement(query_string);
        prep_stmt -> setInt(1, channel_id);
        prep_stmt -> executeUpdate();
        
        // 준비되었으면 commit
        con -> commit();
        
        return_string += "\"result\": \"success\"}";
    }
    catch (sql::SQLException &e) 
    {
        return_string = return_string + "\"result\": \"fail\", ";
        return_string = return_string + "\"error code\": " + std::to_string(e.getErrorCode()) + "}";
    }
    
    // 사용한 객체를 삭제한다. 
    delete prep_stmt;
    delete con;
    
    // 반환할 값을 준비한다. (string을 char* 형식으로 전환. Ctypes는 std::string 자료형을 지원하지 않는다.)   
    char *cstr = new char[return_string.length() + 1];
    strcpy(cstr, return_string.c_str());
    
    return (char*) cstr;
}

#endif