import paho.mqtt.client as mqtt
import time
import ssl
import json

hostname = "mqtt.thingstream.io"
clientid = "device:d4574f32-d632-4501-8d5b-b8c9e801b5e5"
username = "K7T841RL7HZ3P98IBS6J"
password = "Xrs5FIZ0MdHKaVHEmFw35kE8UTrR0qI3yk1ui6h3"

def readJson(jsonMessage) :

    # remplace les single quotes par des double quotes
    jsonMessage = jsonMessage.replace('\'', '\"')
    # parse le message JSON
    return json.loads(jsonMessage)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    if rc==0:
        #subscribe to all topics
        client.subscribe("TEST1")
        
def on_message(client, userdata, msg):
   print(msg.topic+' '+msg.payload.decode())
   message = readJson(str(msg.payload.decode("utf-8")))
   print(message)

client = mqtt.Client(client_id=clientid)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set(username,password)

#Insecure connection on port 1883 !!!
client.connect(host=hostname, port=1883, keepalive=600)
client.loop_forever()
