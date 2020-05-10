'''
chat.py : 채널 사용자에게 메시지를 전달
'''

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