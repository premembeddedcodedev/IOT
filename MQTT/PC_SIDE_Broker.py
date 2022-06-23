#https://book.hacktricks.xyz/network-services-pentesting/1883-pentesting-mqtt-mosquitto
import paho.mqtt.client as mqtt
import time
import os

HOST = "127.0.0.1" #this is the PC address = 127.0.0.1 loopback address
PORT = 1883

def on_connect(client, userdata, flags, rc):
        print("on connect")
        client.subscribe('#', qos=1)
        client.subscribe('$SYS/#')

def on_message(client, userdata, message):
        print("on message")
        print('Topic: %s | QOS: %s  | Message: %s' % (message.topic, message.qos, message.payload))

def main():
        client = mqtt.Client()
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(HOST, PORT)
        client.loop_start()
        #time.sleep(10)
        #client.loop_stop()

if __name__ == "__main__":
        main()
        n = 1
        while n==1:
            n=1

