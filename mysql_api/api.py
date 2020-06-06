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
        
        '''
        char* GetMessage(int channel_id, int last_message);
        char* NewMessage(int user_id, int channel_id, char* message);
        char* CreateUser(char* user_id, char* user_pw, char* user_username);
        char* Login(char* user_id, char* user_pw);
        char* GetUserInfo(int user_uid);
        char* DeleteUser(int user_uid, char* user_pw);
        '''
        
        self.dll.GetMessage.restype = c_char_p
        self.dll.NewMessage.restype = c_char_p
        self.dll.CreateUser.restype = c_char_p
        self.dll.Login.restype = c_char_p
        self.dll.GetUserInfo.restype = c_char_p
        self.dll.DeleteUser.restype = c_char_p
        return
    
    def login(self, user_id, user_pw):
        '''
            입력
                user_id (string) : 사용자 id
                user_pw (string): 사용자 pw
            출력 (tuple)
                (
                    로그인 성공 여부(bool), 
                    로그인 성공시: uid(int), username(str), 
                                channels(list) [channel_id (int), channel_name (str), last_message_id(int)]
                    / 로그인 실패시: error code(int)
                )
        '''
        import json
        import hashlib
        from ctypes import c_char_p
        
        if type(user_id).__name__ != 'str' or type(user_pw).__name__ != 'str':
            raise TypeError
        
        user_id = c_char_p(user_id.encode('utf-8'))
        user_pw = c_char_p(hashlib.sha256(user_pw.encode('utf-8')).hexdigest().encode('utf-8'))
        
        result = json.loads(self.dll.Login(user_id, user_pw).decode('utf-8'))
        if result['result'] == 'success':
            user_infos = self.get_user_info(result['UID'])
            if user_infos[2] is not True:
                return (False, -2,)
            else:
                return (True, result['UID'], user_infos[1], user_infos[3],)
        else:
            '''
                -1 : Wrong ID or PASSWORD
                -2 : Deactivated account
            '''
            return (False, result['error_code'],)
        
    def create_user(self, user_id, user_pw, user_username):
        '''
            입력
                user_id (string) : 사용자 id
                user_pw (string): 사용자 pw
                user_username (string) : 사용자 닉네임
            출력 (tuple)
                (
                    계정 생성 성공 여부(bool), 
                    계정 생성 실패시 : error code(int)
                )
        '''
        import json
        import hashlib
        from ctypes import c_char_p
        
        if type(user_id).__name__ != 'str' or type(user_pw).__name__ != 'str' or type(user_username).__name__ != 'str':
            raise TypeError
        
        user_id = c_char_p(user_id.encode('utf-8'))
        user_pw = c_char_p(hashlib.sha256(user_pw.encode('utf-8')).hexdigest().encode('utf-8'))
        user_username = c_char_p(user_username.encode('utf-8'))
        
        result = json.loads(self.dll.CreateUser(user_id, user_pw, user_username).decode('utf-8'))
        if result['result'] == 'success':
            return (True,)
        else:
            '''
                -1 : LOGIN_ID 중복
                -2 : USERNAME 중복
            '''
            return (False, result['error_code'],)   
    
    def delete_user(self, user_uid, user_pw):
        '''
            입력
                user_uid (int) : 사용자 uid
                user_pw (string): 사용자 pw
            출력 (tuple)
                (
                    계정 삭제 성공 여부(bool), 
                    계정 삭제 실패시 : error code(int)
                )
        '''
        import json
        import hashlib
        from ctypes import c_char_p, c_int
        
        if type(user_uid).__name__ != 'int' or type(user_pw).__name__ != 'str':
            raise TypeError
        
        user_uid = c_int(user_uid)
        user_pw = c_char_p(hashlib.sha256(user_pw.encode('utf-8')).hexdigest().encode('utf-8'))
        
        result = json.loads(self.dll.DeleteUser(user_uid, user_pw).decode('utf-8'))
        if result['result'] == 'success':
            return (True,)
        else:
            return (False, result['error_code'],) 
    
    def get_message(self, channel_id, last_message_id):
        '''
            입력
                channel_id (int) : 채널 id
                last_message_id (int): 마지막으로 확인한 메시지 id
            출력 (tuple)
                (
                    성공 여부(bool), 
                    성공시: message count(int) : 메시지 개수,
                          message(list) [
                                            {
                                                'username': 메시지를 보낸 사용자 닉네임 (string),
                                                'id': 메시지 id (int),
                                                'text': 메시지 본문 (string)
                                            }, ...
                                        ]
                    / 실패시: error code(int)
                )
        '''
        import json
        from ctypes import c_int
        
        if type(channel_id).__name__ != 'int' or type(last_message_id).__name__ != 'int':
            raise TypeError
        
        channel_id = c_int(channel_id)
        last_message_id = c_int(last_message_id)
        
        result = json.loads(self.dll.GetMessage(channel_id, last_message_id).decode('utf-8'))
        if result['result'] == 'fail':
            return (False, result['error_code'],)
        elif 'message' not in result:
            return(True, 0, [],)
        else:
            return (True, len(result['message']), result['message'],)
        pass        
        
    def get_user_info(self, user_uid):
        '''
            입력
                user_uid (int) : 사용자 고유 ID
            출력 (tuple)
                (
                    성공 여부(bool), 
                    성공시: username (str) : 사용자 닉네임,
                          active (bool) : 사용자 활성화 여부,
                          channels(list) [
                                            [
                                                int channel_id,
                                                string channel_name : trashchat.channel_settings.CHANNELNAME
                                                int last_message_id  : 해당 채널에서 마지막으로 확인한 MESSAGE_ID 
                                            ], ...
                                        ]
                    / 실패시: error code(int)
                )
        '''
        import json
        from ctypes import c_int
        
        if type(user_uid).__name__ != 'int':
            raise TypeError
        
        user_uid = c_int(user_uid)
        
        result = json.loads(self.dll.GetUserInfo(user_uid).decode('utf-8'))
        if result['result'] == 'fail':
            return (False, result['error_code'],)
        else:
            return (True, result['username'], 
                    True if result['active'] != 0 else False, result['channels'],)    
                    
    def new_message(self, user_uid, channel_id, message):
        '''
            입력
                user_uid (int) : 사용자 uid
                channel_id (int) : 채널 id
                message (string): 보낸 메시지 본문
            출력 (tuple)
                (
                    성공 여부(bool), 
                    실패시: error code(int)
                )
        '''
        import json
        from ctypes import c_int, c_char_p
        
        if type(user_uid).__name__ != 'int' or type(channel_id).__name__ != 'int' or type(message).__name__ != 'str':
            raise TypeError
        
        user_uid = c_int(user_uid)
        channel_id = c_int(channel_id)
        message = c_char_p(message.encode('utf-8'))
        
        result = json.loads(self.dll.NewMessage(user_uid, channel_id, message).decode('utf-8'))
        if result['result'] == 'fail':
            return (False, result['error_code'],)
        else:
            return (True,)
