'''
mysql_api/api.py : C++로 작성된 DB API를 사용하는 파이썬 래퍼

전역변수
    - DB_LOCATION : 접속 주소
    - DB_USERNAME : 접속 계정명
    - DB_PASSWORD : 접속 비밀번호
    - DB_PORT : 접속 포트
'''

class Database():
    def __init__(self):
        from ctypes import cdll, c_char_p
        self.dll = cdll.LoadLibrary('./mysql_api/mysql.so')
        self.dll.DBSampleQuery.restype = c_char_p
        return
    
    def login(self, u_id, u_pw):
        '''
            입력
                u_id (string) : 사용자 id
                u_pw (string): 사용자 pw
            출력 (tuple)
                (로그인 성공 여부(bool), 자세한 메시지(string))
        '''
        from hashlib import md5
        from ctypes import c_char_p
        import json
        
        u_pw = md5(u_pw.encode()).hexdigest()

        res = json.loads(self.dll.DBSampleQuery(c_char_p(u_id.encode()), 
                        c_char_p(u_pw.encode())).decode())

        if list(res.keys()).count('error_code') != 0:
            if res['error_code'] == 1:  # ID가 틀렸다.
                return (False, "Wrong ID")
            else:  # 알 수 없는 오류
                print(res['error_code'])
                return (False, "Unexpected Error")
        elif len(list(res.keys())) == 1: # ID는 맞지만 PW가 틀렸다.
            return (False, "Wrong Password")
        else:
            return (True, {'UID' : list(res.keys())[0], 'USERNAME' : res[list(res.keys())[0]] })    