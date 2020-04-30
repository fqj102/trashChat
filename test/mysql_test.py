from ctypes import cdll, c_int
id = int(input('User id? '))

dll = cdll.LoadLibrary('./mysql_test.so')
dll.DBSampleQuery(c_int(id))
