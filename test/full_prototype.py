'''
    내부 기능들을 정의한다.
'''

class Logger():
    def __init__(self, logfile):
        '''
            입력
                logfile (string) : 로그 파일의 경로
        '''
        from ctypes import cdll, c_char_p
        self.dll = cdll.LoadLibrary('./logger_test.so')
        self.dll.readLog.restype = c_char_p
        self.logfile = logfile
        return
    
    def write_log(self, log):
        from ctypes import c_char_p
        self.dll.writeLog(c_char_p(LOGFILE.encode()), c_char_p(log.encode()))
        return
    
    def read_full_log(self):
        from ctypes import c_char_p
        full_log = self.dll.readLog(c_char_p(LOGFILE.encode())).decode()
        return full_log

class Database():
    def __init__(self):
        from ctypes import cdll, c_char_p
        self.dll = cdll.LoadLibrary('./mysql_test.so')
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

class Chat():
    def __init__(self):
        self.channels = []  # 채널 리스트
        self.joined = {}  # key : 채널(string) / value : 사용자들의 토큰(string)의 리스트
        self.clients = {}  # key : 사용자 토큰 (string) / value : (사용자 웹소켓 (geventwebsocket.websocket), 채널 id) (tuple)
        self.client_data = {}  
        # key : 사용자 토큰 (string) / value : {'id': 유저명 (string), 'login': 로그인 일시(datetime.datetime), 'uid': 고유 id(string)} (dict)
        return
   
    def channel_list(self):
        import json
        return json.dumps({'result': 'success', 'channels': self.channels})
    
    def join_channel(self, user):
        '''
            입력
                user (dict) : {'key' : 사용자 토큰, 'channel_id' : 채널}
        '''
        self.joined[user['channel_id']].append(user['key'])
        
        try:
            self.clients[user['key']] = (self.clients[user['key']][0], user['channel_id'])
        except KeyError:
            self.clients[user['key']] = (None, user['channel_id'])        
        
        return "{\"result\": \"success\"}"
    
    def leave_channel(self, user):
        '''
            입력
                user (dict) : {'key' : 사용자 토큰, 'channel_id' : 채널}
        '''        
        self.joined[user['channel_id']].remove(user['key'])
        return "{\"result\": \"success\"}"
        
    def send_message(self, user, message):
        '''
            입력
                user (string) : 사용자 토큰
                message (string) : 전송할 메시지
        '''        
        from datetime import datetime
        payload = f"{self.client_data[user]['id']} said: {message} <br /> ({datetime.strftime(datetime.now(), '%Y-%m-%d %H:%M:%S')})"
        channel = self.clients[user][1]

        for token in self.joined[str(channel)]:
            try:
                self.clients[token][0].send(payload) 
            except:
                self.leave_channel({'key': user, 'channel_id': channel})
                self.clients.pop(user)
                self.client_data.pop(user)
        return "{\"result\": \"success\"}"
        
'''
    flask에 필요한 라이브러리들을 불러온다.
'''

from flask import Flask
from flask_sockets import Sockets
import os
import json

'''
    전역변수를 선언한다.
'''
LOGFILE = './log.log'
message_id = 0

app = Flask(__name__)
app.secret_key = os.urandom(16)
sockets = Sockets(app)

logger = Logger(LOGFILE)
database = Database()
chat = Chat()

chat.channels.append("1")
chat.joined['1'] = []
chat.channels.append("2")
chat.joined['2'] = []

'''
    endpoint를 설정한다.
    /login
    /logout
    /chat
    /channels/list
    /channels/join/<channel_id>
    /channels/leave/<channel_id>
    /client
'''

@app.route('/login', methods=['post'])
def login():
    from flask import make_response, abort, request
    from datetime import datetime
    import json
    import os
    
    u_id = request.get_json(force=True)['id']
    u_pw = request.get_json(force=True)['pw']
    
    login_result = database.login(u_id, u_pw)
    if login_result[0] == False:
        abort(403)
    else:
        resp = make_response(json.dumps({'result': 'success', 'message': login_result[1]}))
        
        while True:
            token = os.urandom(16).hex()
            if list(chat.client_data.keys()).count(token) != 0:  # 키값 중복
                continue
            else:    
                chat.client_data[token] = {'id': login_result[1]['USERNAME'],
                'uid': login_result[1]['UID'], 'login':datetime.strftime(datetime.now(), '%Y-%m-%d %H:%M:%S')}
                break
            
        resp.set_cookie('token', token)
        return resp

@app.route('/logout')
def logout():
    from flask import abort, request
    token = request.cookies.get('token')
    if request.cookies.get('token') is None:  # 로그인하지 않았는데 로그아웃 시도
        abort(403)
        
    chat.client_data.remove(token)
    if list(chat.clients.keys()).count(token) != 0:
        channel_id = chart.clients[token][1]
        chat.clients.remove(token)
        chat.joined[channel_id].remove(token)
    
    return "{\"result\": \"success\"}"
    

@app.route('/client')
def client():
    from flask import render_template
    return render_template('websocket_client_full.html')

@app.route('/channels/list')
def list_channels():
    return chat.channel_list()
        
@app.route('/channels/leave/<channel_id>')
def leave_channel(channel_id):
    from flask import request
    if request.cookies.get('token') is None:
        abort(403)
    return chat.leave_channel({'key': request.cookies.get('token'), 'channel_id': channel_id})
        
@app.route('/channels/join/<channel_id>')
def join_channel(channel_id):
    from flask import request
    if request.cookies.get('token') is None:
        abort(403)
    return chat.join_channel({'key': request.cookies.get('token'), 'channel_id': channel_id})

@sockets.route('/chat')
def broadcast_chat(ws):
    from flask import request, abort
    from datetime import datetime
    import json
    
    if request.cookies.get('token') is None:
        abort(403)
        
    global message_id
    token = request.cookies.get('token')
    chat.clients[token] = (ws, chat.clients[token][1])
  
    while not ws.closed:   
        request = ws.receive()
        if ws.closed:
            print("Client left.")
            break
            
        request_json = json.loads(request)  
        message = request_json['message']
        channel = request_json['channel']
        
        if channel == chat.clients[token][1]:
            chat.send_message(token, message)
            message_id += 1
            payload = f"{message_id} | {channel} | {chat.client_data[token]['uid']} | {message} | {datetime.strftime(datetime.now(), '%Y-%m-%d %H:%M:%S')}"
            logger.write_log(payload)
        else:  # 클라이언트 측 값이 변조됨
            abort(403)
    try:
        ws.close()
        chat.leave_channel({'key': token, 'channel_id': chat.clients[token][1]})
        chat.clients.pop(token)
        chat.client_data.pop(token)
    except Exception as e:
        print(e)

