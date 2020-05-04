from flask import Flask, make_response, render_template
from flask import request as flask_request
from flask_sockets import Sockets
import os
import json

app = Flask(__name__)
app.secret_key = os.urandom(16)
sockets = Sockets(app)
 

ws_channels = ['1', '2']
ws_joined = {'1': [], '2': []}
ws_clients = {}

@app.route('/client')
def client():
    return render_template('websocket_client.html')

@app.route('/channels/list')
def list_channels():
    resp = make_response(json.dumps({'result': 'success', 'channels': ws_channels}))
    if flask_request.cookies.get('token') is None:
        resp.set_cookie('token', os.urandom(16))
    return resp
        
@app.route('/channels/leave/<channel_id>', methods=['GET'])
def leave_channel(channel_id):
    try:
        ws_joined[channel_id].remove(flask_request.cookies.get('token'))
        return json.dumps({'result': 'success'})
    except Exception as e:
        print(e)
        return json.dumps({'result': 'fail'})
        
@app.route('/channels/join/<channel_id>', methods=['GET'])
def join_channel(channel_id):
    try:
        ws_joined[channel_id].append(flask_request.cookies.get('token'))
        return json.dumps({'result': 'success'})
    except Exception as e:
        print(e)
        return json.dumps({'result': 'fail'})

@sockets.route('/chat')
def broadcast_chat(ws):
    token = flask_request.cookies.get('token')
    ws_clients[token] = ws
    
    while not ws.closed:   
        request = ws.receive()
        if ws.closed:
            print("Client left.")
            break
            
        request_json = json.loads(request)
        message = request_json['message']
        channel = request_json['channel']
    
        payload = f"{ws.environ['REMOTE_ADDR']} on channel {channel} said: {message}"

        for token in ws_joined[str(channel)]:
            try:
                ws_clients[token].send(payload) 
            except:
                ws_joined[channel].remove(token)
    
    try:
        ws.close()
        ws_joined[channel].remove(token)
        ws_clients.remove(ws)
    except Exception as e:
        print(e)
