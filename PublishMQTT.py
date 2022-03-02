import paho.mqtt.client as mqtt
import time
import ssl

hostname = "mqtt.thingstream.io"
clientid = "device:d4574f32-d632-4501-8d5b-b8c9e801b5e5"
username = "K7T841RL7HZ3P98IBS6J"
password = "Xrs5FIZ0MdHKaVHEmFw35kE8UTrR0qI3yk1ui6h3"

def on_connect(client, userdata, flags, rc):    
    print("Connected with result code "+str(rc))    
        
client = mqtt.Client(client_id=clientid)
client.username_pw_set(username,password)

#Insecure connection on port 1883 !!!
client.connect(host=hostname, port=1883, keepalive=60)
id = 0
while True:
    client.publish("TEST1","STOP",1)
    time.sleep(180)
    id += 1