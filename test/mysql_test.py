from ctypes import cdll, c_int, c_char_p
id = int(input('User id? '))

dll = cdll.LoadLibrary('./mysql_test.so')
dll.DBSampleQuery.restype = c_char_p
print(dll.DBSampleQuery(c_int(id)))
    