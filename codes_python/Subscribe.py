import paho.mqtt.client as mqtt  # Import the client class
import time

nbr_message = 0
broker_address = "mqtt.thingstream.io"
clientid = "device:94941562-fb03-4e61-b017-2926a1dab1ea"  # Change pour chaque thing.
username = "K7T841RL7HZ3P98IBS6J"
password = "+Ettw+umgGa/JGo38eR+ZHXUHCpIupUaMTe1EyEt"  # Change pour chaque thing.


def on_connect(client, userdata, flag, rc):  # Response to a connection event
    print("Connected with result code " + str(rc) + "  (0: Connection successful / 1: Connection refused - incorrect protocol version / 2: Connection refused - invalid client identifier / 3: Connection refused - server unavailable / 4: Connection refused - bad username or password / 5: Connection refused)")  # rc is the connection result
    if rc == 0:
        client.subscribe(topic="TEST1", qos=1)  # subscribe to a topic with a qos


def on_message(client, userdata, msg):  # Response to a message received event
    global nbr_message
    time.sleep(1)  # wait a little to give the script time to process the callback
    message_received = str(msg.payload.decode("utf-8"))
    print("received message number " + str(nbr_message) + "=", message_received)

    # Sauvegarde dans un fichier
    data_f = "data" + str(nbr_message) + ".txt"
    fichier = open(data_f, "a")
    fichier.write(message_received)
    fichier.close()

    nbr_message += 1


# Create client object with a persistent connection (!clean session), in this mode the broker will store subscription information, and undelivered messages for the client.
client = mqtt.Client(client_id=clientid, clean_session=False)

# Callbacks are functions that are called in response to an event, here we attach function to callback
client.on_connect = on_connect  # Response to a connection event
client.on_message = on_message  # Response to a message received event

client.username_pw_set(username, password)

# Connection to the brooker
# port : the network port of the server host to connect to. Defaults to 1883.
# keepalive : maximum period in seconds allowed between communications with the broker. If no other messages are being exchanged, this controls the rate at which the client will send ping messages to the broker
client.connect(host=broker_address, port=1883, keepalive=600)
client.loop_forever()
