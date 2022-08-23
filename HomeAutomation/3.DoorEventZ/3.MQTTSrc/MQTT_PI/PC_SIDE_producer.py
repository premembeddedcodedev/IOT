# Python program to explain the
# use of wait() method for Condition object
import json
import paho.mqtt.client as mqtt
import threading
import time
import random

HOST = "192.168.29.65"
PORT = 1883

z = 0

class subclass:
  # Initialising the shared resources
  def __init__(self):
    self.x = []
  
  # Add an item for the producer
  def produce_item(self, x_item):
    print("Producer adding an item to the list")
    self.x.append(x_item)
    
  # Consume an item for the consumer
  def consume_item(self):
    print("Consuming from the list")
    consumed_item = self.x[0]
    print("Consumed item: ", consumed_item)
    self.x.remove(consumed_item)

def on_connect(client, userdata, flags, rc):
        client.subscribe('#', qos=1)

def pass_to_func_and_pub(data_to_pub):
    try:
        unpacked_json = json.loads(data_to_pub)
    except Exception as e:
        print("Couldn't parse raw data: %s" % data_to_pub, e)
    else:
      global z
      z = unpacked_json['Value']
      #print("JSON:", unpacked_json)
      print("Value:", z)
      if z < 4:
        print("Producing an item, time it will take(seconds): " + str(z))
        time.sleep(z)
      
        print("Producer acquiring the lock")
        condition_obj.acquire()
        try:
        # Produce an item
         subclass_obj.produce_item(z)
        # Notify that an item  has been produced
         condition_obj.notify()
        finally:
         # Releasing the lock after producing
         condition_obj.release()

def on_message(client, userdata, message):
        #print('Topic: %s | QOS: %s  | Message: %s' % (message.topic, message.qos, message.payload))
        pass_to_func_and_pub(message.payload)

#def producer(subclass_obj, condition_obj):
#    # Selecting a random number from the 1 to 3
#    r = random.randint(1,3)
#    print("Random number selected was:", r)
#    
#    # Creting r number of items by the producer
#    #for i in range(1, r):
#    if z < 4:
#      print("Producing an item, time it will take(seconds): " + str(z))
#      time.sleep(z)
#      
#      print("Producer acquiring the lock")
#      condition_obj.acquire()
#      try:
#        # Produce an item
#        subclass_obj.produce_item(z)
#        # Notify that an item  has been produced
#        condition_obj.notify()
#      finally:
#        # Releasing the lock after producing
#        condition_obj.release()
      
def consumer(subclass_obj, condition_obj):
    condition_obj.acquire()
    while True:
      try:
        # Consume the item 
        subclass_obj.consume_item()
      except:
        print(".")
        value = condition_obj.wait(10)
        if value:
          print("Item produced notified")
          continue
        else:
          print("No item to consume, list empty")
          print("Waiting timeout")
          #break
        
    # Releasig the lock after consuming
    condition_obj.release()
    
if __name__=='__main__':
  client = mqtt.Client()
  client.on_connect = on_connect
  client.on_message = on_message
  client.connect(HOST, PORT)
  client.loop_start()
    
  # Initialising a condition class object
  condition_obj = threading.Condition()
  # subclass object
  subclass_obj = subclass()
  
  # Producer thread
  #pro = threading.Thread(target=producer, args=(subclass_obj,condition_obj,))
  #pro.start()
  
  # consumer thread
  con = threading.Thread(target=consumer, args=(subclass_obj,condition_obj,))
  con.start()

  #pro.join()
  con.join()
  print("Producer Consumer code executed")

