# -*- coding: utf-8 -*-
"""
Created on Tue Jul 19 15:52:09 2022

@author: tehoe
"""

import paho.mqtt.client as mqtt  # Import the client class
import time
from TestCSVConversion import traitement

nbr_message = 0
broker_address = "mqtt.thingstream.io"
clientid = "device:b9154e9d-d7dd-4c09-bc50-a7c4639acfb7"  # Change pour chaque thing.
username = "K7T841RL7HZ3P98IBS6J"
password = "vzi1K2cPJvEve35/vfsHmSY3srCtWHkN5HPGU92h"  # Change pour chaque thing.
b= "hello"

a = "{'batteries' : [ { 'id': 1, 'soc': 42, 'soh': 'good', 'voltage': 1.3,'current':1.56, 'temperature': 12.98, 'date' : '15-07-2022' }, {  'id': 2,'soc': 20,'soh': 'low', 'voltage': 0.5,'current': 2.1 },{'id': 3,'soc': 40,'soh': 'medium','voltage': 1.5,'current': 2.1},{'id': 5,'soc': 20,'soh': 'good','voltage': 2.0,'current': 3}, {'id': 'total', 'soc': 60, 'soh': 'medium','voltage': 1.7,'current': 3 } ],'solarPannels':[ {'date' : '14-07-22','current' : 8.23,'voltage': 6.4,'power': 48.9 }]}"
def on_connect(client, userdata, flag, rc):  # Response to a connection event
    print("Connected with result code " + str(rc) + "  (0: Connection successful / 1: Connection refused - incorrect protocol version / 2: Connection refused - invalid client identifier / 3: Connection refused - server unavailable / 4: Connection refused - bad username or password / 5: Connection refused)")  # rc is the connection result
    if rc == 0:
        client.subscribe("TEST1",1)  # subscribe to a topic with a qos


def on_message(client, userdata, msg):  # Response to a message received event
    global nbr_message
    global b 
    time.sleep(1)  # wait a little to give the script time to process the callback
    message_received = str(msg.payload.decode("utf-8"))
    
    print("received message number " + str(nbr_message) + "=", message_received)

    # Sauvegarde dans un fichier
    data_f = "data" + str(nbr_message) + ".txt"
    fichier = open(data_f, "a")
    fichier.write(message_received)
    b= message_received
    fichier.close()
    print(message_received)
    nbr_message += 1
    traitement(b)
    
    
       
       

# Create client object with a persistent connection (!clean session), in this mode the broker will store subscription information, and undelivered messages for the client.
client = mqtt.Client(client_id=clientid, clean_session= False)

# Callbacks are functions that are called in response to an event, here we attach function to callback
client.on_connect = on_connect  # Response to a connection event
client.on_message = on_message  # Response to a message received event

client.username_pw_set(username, password)

# Connection to the brooker
# port : the network port of the server host to connect to. Defaults to 1883.
# keepalive : maximum period in seconds allowed between communications with the broker. If no other messages are being exchanged, this controls the rate at which the client will send ping messages to the broker
client.connect(host=broker_address, port=1883, keepalive=600)

client.loop_forever()

