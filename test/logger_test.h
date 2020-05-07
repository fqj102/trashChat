#ifndef LOGGER_TEST_H
#define LOGGER_TEST_H

#include <string>
#include <cstring>
#include <fstream>

extern "C"
{
    char* readLog(char* filename);  // 로그파일 전체를 읽어서 반환한다.
    void writeLog(char* filename, char* message);  // 로그파일 맨 앞에 로그를 추가한다.
    char* readLogById(char* filename, int logId);
}
std::string* parseLog(std::string log);

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

char* readLogById(char* filename, int logId)
{
    std::ifstream logfile;
    logfile.open(filename);
    
    std::string file_data;
    std::string line;
    while (!logfile.eof())
    {
        std::getline(logfile,line);
        std::string *parsed = parseLog(line);
        file_data += line + "\n";
        if (std::stoi(parsed[0]) == logId)
        {
            break;
        }
    }
    logfile.close();
    
    file_data = file_data.substr(0, file_data.length() - 1);
    char *cstr = new char[file_data.length() + 1];
    strcpy(cstr, file_data.c_str());
    
    return (char *)cstr;
}

std::string* parseLog(std::string log)
{
	/*
		로그 형식 : message id (int) | channel id (int) | user id (int) | "message" (string) | date (string)
	*/
	std::string* return_data = new std::string[5];

	for(int i = 0; i < 5; i++)  // 로그는 5개의 데이터를 가지고 있다. 
	{
		bool isString = false;
		for(unsigned int pos = 0; pos < log.length(); pos++)
		{
			if(!isString && log[pos] == '|')
			{
				return_data[i] = log.substr(0, pos - 1);
				log = log.substr(pos+2, log.length() - 1);
				break;
			}
			else if(log[pos] == '\"' && !isString)
			{
				isString = true;
			}
			else if(log[pos] == '\"' && isString && log[pos-1] != '\\')
			{
				isString = false;
			}
		}
	}
	return_data[4] = log;
	
	return return_data;
}
#endif

