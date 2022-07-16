/*
 * uMQTTBroker demo for Arduino (C++-style)
 * 
 * The program defines a custom broker class with callbacks, 
 * starts it, subscribes locally to anything, and publishs a topic every second.
 * Try to connect from a remote client and publish something - the console will show this as well.
 */

#include <Arduino_JSON.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "uMQTTBroker.h"
#include <RH_NRF24.h>
#include "painlessMesh.h"
#include "namedMesh.h"

#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04
#define NUMCLIENTS 3
#define DOOR_EVENT_2 0x03

/* SensorBits Enablement */
#define TEMP_ENABLE (1<<1)
#define LIGHT_ENABLE (1<<2)
#define SPEAKER_ENABLE (1<<3)
#define MOVEMENT_ENABLE (1<<4)
#define PHOTOS_ENABLE (1<<5)
#define NOISE_ENABLE (1<<6)
#define GAS_ENABLE (1<<7)
#define AIR_ENABLE (1<<8)
#define NODE_1	(1<<9)
#define NODE_2	(1<<10)
#define NODE_3	(1<<11)

String readings;
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
struct message {
	/* Data get it from DoorClient */
	int SensorBits;
	int DoorClient;
	int DoorStatus;
	int SpeakerStatus;
	int Temparature;
	double Pressure;
	double Altitude;
	int Humidity;
	int LightStatus;
	int MovmentStatus;
	int CapturePhotos;
	int NoiseStatus;
	int GasStatus;
	int AirQualityStatus;

	/* Data get it from Email Node */
	uint8_t NodeNumber;
	uint8_t SpeakerEnable;
	uint8_t TempSensorEnable;
	uint8_t LightSensorEnable;
	uint8_t MovementSensorEnable;
	uint8_t PhotosEnable;
	uint8_t NoiseSensorEnable;
	uint8_t GasSensorEnable;
	uint8_t AirQualitySensorEnable;
	float h;
	float t;
	float f;
	float hif;
	float hic;
};

struct message ClientData;
struct message Clients[NUMCLIENTS];
namedMesh mesh;

uint8_t len = sizeof(struct message);
void KeepAlive() ; 
String to1 = "1stFloorMCU"; //nodename in the server
String to2 = "2ndFloorMCU"; //nodename in the server
String to1_N = "1stFloorMCU_North"; //nodename in the server

/*
 * Your WiFi config here
 */
char ssid[] = "XXXXXX";     // your network SSID (name)
char pass[] = "XXXXXX"; // your network password
bool WiFiAP = false;      // Do yo want the ESP as AP?
RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather
void Ciritical_Door_event()
{
	Serial.println("Sending to gateway");
	nrf24.send((uint8_t *)&ClientData.DoorStatus, 4);
	nrf24.waitPacketSent();
}
String EnableConfigDataForDoor(int NodeNumber)
{
	JSONVar jsonReadings;
	jsonReadings["Node"] = NodeNumber;
	jsonReadings["SensorBits"] = ClientData.SensorBits;
	readings = JSON.stringify(jsonReadings);
	return readings;
}

int GetNodeNumber(int Data)
{
	if ((Data >> 9) & 0x1)
		ClientData.NodeNumber = DOOR_EVENT_1_MAIN;	
	if ((Data >> 10) & 0x1)
		ClientData.NodeNumber = DOOR_EVENT_1_NORTH;	
	if ((Data >> 11) & 0x1)
		ClientData.NodeNumber = DOOR_EVENT_2_SECOND;	

	return ClientData.NodeNumber;
}

/*
 * Custom broker class with overwritten callback functions
 */
class myMQTTBroker: public uMQTTBroker
{
	public:
		virtual bool onConnect(IPAddress addr, uint16_t client_count) {
			Serial.println(addr.toString()+" connected");
			return true;
		}

		virtual bool onAuth(String username, String password) {
			Serial.println("Username/Password: "+username+"/"+password);
			return true;
		}

		virtual void onData(String topic, const char *data, uint32_t length) {
#if 0
			char data_str[length+1];
			os_memcpy(data_str, data, length);
			data_str[length] = '\0';

			Serial.println("received topic '"+topic+"' with data '"+(String)data_str+"'");
#else
			char str[length+1];
			int i;
			Serial.print("Message arrived [");
			Serial.print(topic);
			Serial.print("] ");
			for ( i = 0; i < length; i++) {
				str[i]=(char)data[i];
				Serial.print((char)data[i]);
			}

			str[i] = 0;
			Serial.println();
			StaticJsonDocument <256> doc;
			deserializeJson(doc,str);
			const char* sensor = doc["sensor"];
			long time = doc["time"];
			ClientData.DoorStatus = doc["Value"];
			ClientData.h = doc["Humidity"];
			ClientData.t = doc["Temp"];
			ClientData.f = doc["F"];
			ClientData.hif = doc["HIF"];
			ClientData.hic = doc["HIC"];
			Serial.print("Door Status Received as: ");
			Serial.println(ClientData.DoorStatus);
			Serial.print(F(" Humidity: "));
			Serial.print(ClientData.h);
			Serial.print(F("%  Temperature: "));
			Serial.print(ClientData.t);
			Serial.print(F("C "));
			Serial.print(ClientData.f);
			Serial.print(F("F  Heat index: "));
			Serial.print(ClientData.hif);
			Serial.print(F("C "));
			Serial.print(ClientData.hic);
			Serial.println(F("F"));
#endif
		}
};

myMQTTBroker myBroker;

/*
 * WiFi init stuff
 */
void startWiFiClient()
{
	Serial.println("Connecting to "+(String)ssid);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pass);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");

	Serial.println("WiFi connected");
	Serial.println("IP address: " + WiFi.localIP().toString());
}

void startWiFiAP()
{
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid, pass);
	Serial.println("AP started");
	Serial.println("IP address: " + WiFi.softAPIP().toString());
}

void ota_config()
{
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}

	ArduinoOTA.onStart([]() {
			Serial.println("Start");
			});
	ArduinoOTA.onEnd([]() {
			Serial.println("\nEnd");
			});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
			});
	ArduinoOTA.onError([](ota_error_t error) {
			Serial.printf("Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
			else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
			else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
			else if (error == OTA_END_ERROR) Serial.println("End Failed");
			});
	ArduinoOTA.begin();
}

void nrf_config()
{
	if (!nrf24.init()) 
		Serial.println("init failed");

	if (!nrf24.setChannel(3)) 
		Serial.println("setChannel failed");

	if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
		Serial.println("setRF failed");
}

void setup()
{
	Serial.begin(9600);
	Serial.println();
	Serial.println();

	// Start WiFi
	if (WiFiAP)
		startWiFiAP();
	else
		startWiFiClient();

	// Start the broker
	ota_config();
	Serial.println("Starting PVs MQTT broker - 1");
	myBroker.init();
	nrf_config();
	Serial.println("NRF config Completed");

	/*
	 * Subscribe to anything
	 */
	myBroker.subscribe("DoorEvents");
}

int counter = 0;

void ExtractEmailNodeData()
{
	if(buf[0] & TEMP_ENABLE)
		ClientData.SensorBits |= ClientData.TempSensorEnable;
	if(buf[0] & LIGHT_ENABLE)
		ClientData.SensorBits |= ClientData.LightSensorEnable;
	if(buf[0] & SPEAKER_ENABLE)
		ClientData.SensorBits |= ClientData.SpeakerEnable;
	if(buf[0] & MOVEMENT_ENABLE)
		ClientData.SensorBits |= ClientData.MovementSensorEnable;
	if(buf[0] & PHOTOS_ENABLE)
		ClientData.SensorBits |= ClientData.PhotosEnable;
	if(buf[0] & NOISE_ENABLE)
		ClientData.SensorBits |= ClientData.NoiseSensorEnable;
	if(buf[0] & GAS_ENABLE)
		ClientData.SensorBits |= ClientData.GasSensorEnable;
	if(buf[0] & AIR_ENABLE)
		ClientData.SensorBits |= ClientData.AirQualitySensorEnable;
}

void receive_data_from_mesh()
{
	uint8_t i = 0;
	if (nrf24.waitAvailableTimeout(1000)){  
		if (nrf24.recv(buf, &len)){
#ifdef DEBUG_LEVEL_1
			Serial.print("got message: ");
			Serial.println(len);
#endif
			ExtractEmailNodeData();
			memcpy((uint8_t *)&ClientData, buf, sizeof(struct message));
#ifdef DEBUG_LEVEL_1
			debug_prints();
			send_to_master();
#endif
#ifdef DEBUG_LEVEL_1
			i = 0;
			while(i<len) {
				Serial.print(buf[i]);
				Serial.print(" ");
				i++;
			}
#endif
			Serial.println();
		}
		else
		{
			Serial.println("recv failed");
		}

	}
	else
	{
#ifdef DEBUG_LEVEL_2
		Serial.println("No message, is nrf24_server running?");
#endif
	}

}

void loop()
{
	/*
	 * Publish the counter value as String
	 */
	ArduinoOTA.handle();

	if (ClientData.DoorStatus & DOOR_EVENT_1_MAIN)
	{
		Ciritical_Door_event();
		ClientData.DoorStatus &=~DOOR_EVENT_1_MAIN;
	}
	if (ClientData.DoorStatus & DOOR_EVENT_1_NORTH)
	{
		Ciritical_Door_event();
		ClientData.DoorStatus &=~DOOR_EVENT_1_NORTH;
	}
	if (ClientData.DoorStatus & DOOR_EVENT_2_SECOND)
	{
		Ciritical_Door_event();
		ClientData.DoorStatus &=~DOOR_EVENT_2_SECOND;
	}
	receive_data_from_mesh();
	//myBroker.publish("broker/counter", (String)counter++);

	delay(1000);
}

