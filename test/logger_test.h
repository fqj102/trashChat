#ifndef LOGGER_TEST_H
#define LOGGER_TEST_H

#include <string>
#include <cstring>
#include <fstream>

extern "C"
{
    char* readLog(char* filename);  // 로그파일 전체를 읽어서 반환한다.
    void writeLog(char* filename, char* message);  // 로그파일 맨 앞에 로그를 추가한다.
}

char* readLog(char* filename)
{
    std::ifstream logfile;
    logfile.open(filename);
    
    std::string file_data;
    std::string line;
    while (!logfile.eof())
    {
        std::getline(logfile,line);
        file_data += line + "\n";
    }
    logfile.close();
    
    file_data = file_data.substr(0, file_data.length() - 1);
    char *cstr = new char[file_data.length() + 1];
    strcpy(cstr, file_data.c_str());
    
    return (char *)cstr;
}

void writeLog(char* filename, char* message)
{
    std::string file_origin = readLog(filename);
    
    std::ofstream _logfile;
    _logfile.open(filename);
    _logfile << message + (std::string) "\n" + file_origin;
    _logfile.close();

    return;
}

#endif

