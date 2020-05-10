class Connection
{
    constructor() 
    {
        this.websocket = null;
        this.joinedChannel = null;
        this.joined = false;
        this.srv_adddress = 'http://192.168.219.50:1234';
        this.getChannels();
    }
    
    connect(url)
    {
        this.websocket = new WebSocket(url);
        this.websocket.onmessage = function(message) 
        {
            var node = document.createElement("P");
            var textnode = document.createTextNode(message.data);
            node.appendChild(textnode);
            document.getElementById("messages").appendChild(node);
        }
    }
    
    disconnect() 
    {
        this.websocket.close()
    }
    
    send(message)
    {
        console.log('{"channel": "'+ this.joinedChannel + '", "message": "' + message + '"}');
        this.websocket.send('{"channel": "'+ this.joinedChannel + '", "message": "' + message + '"}');
    }
    
    getChannels()
    {
        var xhr = new XMLHttpRequest();
        var responseObj = ''; 
        
        xhr.withCredentials = true;
        xhr.open('GET', this.srv_adddress + '/channels/list');
        xhr.responseType = 'json';
        xhr.send();
        xhr.onload = () => {
           responseObj = xhr.response;
           console.log(responseObj);
            var channels = '';
            for (var i = 0; i < responseObj.channels.length; i++)
            {
                channels += '<br /><li><a onclick="joinserver(' + responseObj.channels[i] + ');">' + responseObj.channels[i] + '</a></li>';
            }
            document.getElementById("servers").innerHTML = channels;
        }

    }
    
    joinChannel(channelId)
    {
        console.log(channelId);
        if (this.joined)
        {
            var xhr = new XMLHttpRequest();
            var responseObj = '';
            
            xhr.withCredentials = true;
            xhr.open('GET', this.srv_adddress + '/channels/leave/' + this.joinedChannel);
            xhr.responseType = 'json';
            xhr.send();
            xhr.onload = () => {
               responseObj = xhr.response;
               console.log(responseObj);
            }
            document.getElementById("status").innerHTML = "None";
        }
        
        var xhr = new XMLHttpRequest();
        
        xhr.withCredentials = true;
        xhr.open('GET', this.srv_adddress + '/channels/join/' + channelId);
        xhr.responseType = 'json';
        xhr.send();
        xhr.onload = () => {
            var responseObj = xhr.response;
            console.log(responseObj);
            if (responseObj.result == 'success')
            {
                document.getElementById("status").innerHTML = channelId;
                this.joined = true;
                this.joinedChannel = channelId;
            }
            else 
            {
                alert("Error");
            }
        }        
    }
}
