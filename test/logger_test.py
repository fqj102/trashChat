from ctypes import cdll, c_char_p, c_int

LOGFILE = './log.log'

dll = cdll.LoadLibrary('./logger_test.so')
dll.readLogById.restype = c_char_p

id = int(input("Read log until: "))
print(dll.readLogById(c_char_p(LOGFILE.encode()), c_int(id)).decode())
