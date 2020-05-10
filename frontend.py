'''
frontend.py : 프론트엔드
'''

from flask import Flask
from flask_sockets import Sockets
from logger_api.api import Logger
from mysql_api.api import Database
from chat.chat import Chat
import os
import json

'''
    전역변수를 선언한다.
'''
LOGFILE = './log.log'
WEB_CLIENT = '/web/client.html'

app = Flask(__name__)
app.secret_key = os.urandom(16)
sockets = Sockets(app)

logger = Logger(LOGFILE)
database = Database()
chat = Chat()

'''
개발용 변수 설정
'''

message_id = 0
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
    return render_template(WEB_CLIENT)

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

