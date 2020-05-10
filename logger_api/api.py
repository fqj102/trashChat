'''
logger_api/api.py : C++로 작성된 로깅 관련 API를 사용하는 파이썬 래퍼
'''


class Logger():
    def __init__(self, logfile):
        '''
            입력
                logfile (string) : 로그 파일의 경로
        '''
        from ctypes import cdll, c_char_p
        self.dll = cdll.LoadLibrary('./logger_api/logger.so')
        self.dll.readLog.restype = c_char_p
        self.logfile = logfile
        return
    
    def write_log(self, log):
        from ctypes import c_char_p
        self.dll.writeLog(c_char_p(self.logfile.encode()), c_char_p(log.encode()))
        return
    
    def read_full_log(self):
        from ctypes import c_char_p
        full_log = self.dll.readLog(c_char_p(self.logfile.encode())).decode()
        return full_log