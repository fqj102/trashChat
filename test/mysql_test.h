#include <iostream>
#include <string>

// MySQL Connector API 관련 헤더파일들
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>

// 전역변수로 실행에 필요한 자료형을 미리 정의한다. 
sql::Statement *stmt;
sql::ResultSet  *res;
sql::Connection *con;
sql::mysql::MySQL_Driver *driver;


extern "C" // Name mangling이 일어나는 것을 방지한다. 
{
    void DBSampleQuery(int userid);
}

void DBSampleQuery(int userid)
{
    // DB 접속에 필요한 정보를 정의한다. 
    const char* USERNAME = "test";
    const char* PASSWORD = "testpassword";
    const char* DBNAME = "test";
    
    // MySQL에 연결한다. 
    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", USERNAME, PASSWORD);
    con->setSchema(DBNAME);
    
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

    // 사용한 Connector를 삭제한다. 
    delete res;
    delete stmt;
    delete con;
    
    return;
}
