#include "namedMesh.h"
#include <RH_NRF24.h>

#define   MESH_SSID       "MCUServerNWAP"
#define   MESH_PASSWORD   "mcupasswd"
#define   MESH_PORT       5555
#define FIRST_MAIN_DOOR_STRING "1ST-MAIN-DOOREVENT"
#define FIRST_NORTH_DOOR_STRING "1ST-NORTH-DOOREVENT"
#define SECOND_MAIN_DOOR_STRING "2ND-MAIN-DOOREVENT"

#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04

RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather

Scheduler userScheduler; // to control your personal task
namedMesh mesh;
String nodeName = "MCUServerNWDev"; // Name needs to be unique

// Should be a message for us now
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
uint8_t data[2];
uint8_t len = sizeof(buf);

Task taskSendMessage( TASK_SECOND*10, TASK_FOREVER, []() {
		//String msg = String("MSG from: ") + nodeName ;
		//String to = "MCUServerNWDevice";
		//mesh.sendSingle(to, msg);
		//Serial.println("Server is Running ....\n");
		}); // start with a one second interval

void setup() {
	Serial.begin(9600);
	//SPI.begin();
	Serial.println("\nHello from Server....!\n");
	if (!nrf24.init())
	{
		Serial.println("init failed");

	}
	// Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
	if (!nrf24.setChannel(3))
	{
		Serial.println("setChannel failed");
	}
	if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
		Serial.println("setRF failed");
	}
	//mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
#if 1
	mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
	mesh.setName(nodeName); // This needs to be an unique name!
#endif
	//  mesh.onReceive([](uint32_t from, String &msg) {
	//    Serial.printf("\nRcvd MSGid from: %u, %s\n", from, msg.c_str());
	//  });
	//
	//  mesh.onReceive([](String &from, String &msg) {
	//    Serial.printf("\nRcvd MSG by name from: %s, %s\n", from.c_str(), msg.c_str());
	//  });t
#if 1
	mesh.onReceive([](String &from, String &msg) {
			//Serial.println("%s\n", msg.c_str());
			if(strcmp(msg.c_str(), FIRST_MAIN_DOOR_STRING) == 0) {
			Serial.println("FIRST_MAIN_DOOR_STRING Triggered ");
			data[0] = DOOR_EVENT_1_MAIN;
			}
			if (strcmp(msg.c_str(), FIRST_NORTH_DOOR_STRING) == 0) {
			Serial.println("FIRST_NORTH_DOOR_STRING Triggered ");
			data[0] |= DOOR_EVENT_1_NORTH;
			}
			if (strcmp(msg.c_str(), SECOND_MAIN_DOOR_STRING) == 0) {
			Serial.println("SECOND_MAIN_DOOR_STRING Triggered ");
			data[0] |= DOOR_EVENT_2_SECOND;
			}
			});

	mesh.onChangedConnections([]() {
			Serial.printf("\nChanged connection\n");
			});

	userScheduler.addTask(taskSendMessage);
	taskSendMessage.enable();
#endif
}

void event_send() {
	Serial.println("Sending to gateway");
	nrf24.send(data, 2);
	nrf24.waitPacketSent();
	data[0] = 0;
	if (nrf24.waitAvailableTimeout(1000))
	{
		if (nrf24.recv(buf, &len))
		{
			Serial.print("got reply: ");
			Serial.println((char*)buf);
		}
		else
		{
			Serial.println("recv failed");
		}
	}
	else
	{
		Serial.println("No reply.");
	}
}

void loop() {
#if 1
	mesh.update();
#endif
	if (data[0] & DOOR_EVENT_1_MAIN)
	{
		event_send();
		data[0] &=~DOOR_EVENT_1_MAIN;
	}
	if (data[0] & DOOR_EVENT_1_NORTH)
	{
		event_send();
		data[0] &=~DOOR_EVENT_1_NORTH;
	}
	if (data[0] & DOOR_EVENT_2_SECOND)
	{
		event_send();
		data[0] &=~DOOR_EVENT_2_SECOND;
	}
}
