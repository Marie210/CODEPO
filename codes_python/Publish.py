import paho.mqtt.client as mqtt  # Import the client class

broker_address = "mqtt.thingstream.io"
clientid = "device:94941562-fb03-4e61-b017-2926a1dab1ea"  # Change pour chaque thing.
username = "K7T841RL7HZ3P98IBS6J"
password = "+Ettw+umgGa/JGo38eR+ZHXUHCpIupUaMTe1EyEt"  # Change pour chaque thing.

# Create client object with a persistent connection (!clean session), in this mode the broker will store subscription information, and undelivered messages for the client.
client = mqtt.Client(client_id=clientid, clean_session=False)
client.username_pw_set(username, password)

# Connection to the brooker
# port : the network port of the server host to connect to. Defaults to 1883.
# keepalive : maximum period in seconds allowed between communications with the broker. If no other messages are being exchanged, this controls the rate at which the client will send ping messages to the broker
client.connect(host=broker_address, port=1883, keepalive=600)

message = "*{ message test }*"
# publish to a topic, where the payload is the message send, with a qos
client.publish(topic="TEST1", payload=message, qos=1)
