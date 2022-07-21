import time

import paho.mqtt.client as mqtt
import json
from CSVconversion import traitement

hostname = "mqtt.thingstream.io"
clientid = "device:77d5e918-673f-4639-afa6-c08082db6802"
username = "K7T841RL7HZ3P98IBS6J"
password = "G5l/8382odCJ+u0DyE/agXzLzuhD14Hlcz3kEVQG"

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
        client.subscribe("Cameskin", 1)
        
def on_message(client, userdata, msg):
   print(msg.topic+' '+str(msg.payload.decode("utf-8")))
   ms = str(msg.payload.decode("utf-8"))
   traitement(ms)
   #message = readJson(str(msg.payload.decode("utf-8")))
   #print(message)

client = mqtt.Client(client_id=clientid, clean_session = False)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set(username,password)

#Insecure connection on port 1883 !!!
client.connect(host=hostname, port=1883, keepalive=600)
client.loop_forever()

