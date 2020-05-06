from ctypes import cdll, c_char_p

LOGFILE = './log.log'

dll = cdll.LoadLibrary('./logger_test.so')
dll.readLog.restype = c_char_p

print("BEFORE : ")
print(dll.readLog(c_char_p(LOGFILE.encode())).decode())

log = input("Log : ")
dll.writeLog(c_char_p(LOGFILE.encode()), c_char_p(log.encode()))

print("AFTER : ")
print(dll.readLog(c_char_p(LOGFILE.encode())).decode())
