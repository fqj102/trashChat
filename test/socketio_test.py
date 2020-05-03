from flask import Flask
from flask_sockets import Sockets
import os

app = Flask(__name__)
app.secret_key = os.urandom(16)
sockets = Sockets(app)
 
@sockets.route('/echo')
def echo_socket(ws):    
    while not ws.closed:        
        message = ws.receive()
        ws.send(message)    
    ws.close()

ws_clients = []
@sockets.route('/socket')
def broadcast_chat(ws):    
    ws_clients.append(ws)
    while not ws.closed:
        message = ws.receive()
        if ws.closed:
            print("Client left.")
            break
    
        payload = f"{ws.environ['REMOTE_ADDR']} said: {message}"
        for client in ws_clients:
            if not client.closed:
                client.send(payload)       
            else:
                print("Client left without warning.")
    ws.close()
    ws_clients.remove(ws)
