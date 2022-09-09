
#define BLYNK_TEMPLATE_ID "TMPLZoeBRvSy"
#define BLYNK_DEVICE_NAME "SampleHomeAutomation"
#define BLYNK_AUTH_TOKEN "Fe1EVDqsOJwj20Li3v2RFsmmZjoqdjic"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include "namedMesh.h"
#include <RH_NRF24.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoOTA.h>

#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04

BlynkTimer timer; 

char auth[] = BLYNK_AUTH_TOKEN;
RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather
char ssid[] = "TP-Link_F524";  // Enter your Wifi Username
char pass[] = "prem@123";  // Enter your Wifi password

int ledpin = D4;
int v0_value = 0;
int v1_value = 0;
int v2_value = 0;
int v3_value = 0;
int v4_value = 0;
int v5_value = 0;
int v6_value = 0;

void Send_Data_to_Display_Client(uint8_t value)
{
        nrf24.send((uint8_t *)&value, 1);
        nrf24.waitPacketSent();
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
#if 1
BLYNK_WRITE(V0)
{
	v0_value = param.asInt();
	Serial.print("V0 value is : ");
	Serial.println(v0_value);
	if(v0_value == 1)
		v0_value = 8;
	else
		v0_value = 0;
	Serial.print("After Conversion V0 value is : ");
	Serial.println(v0_value);

	Send_Data_to_Display_Client(v0_value);
}

BLYNK_WRITE(V1)
{
	v1_value = param.asInt();
	Serial.print("V1 value is : ");
	Serial.println(v1_value);
	if(v1_value == 1)
		v1_value = 16;
	else
		v1_value = 0;
	Serial.print("After Conversion V1 value is : ");
	Serial.println(v1_value);
	Send_Data_to_Display_Client(v1_value);
}

BLYNK_WRITE(V2)
{
	v2_value = param.asInt();
	Serial.print("V2 value is : ");
	Serial.println(v2_value);
	Serial.print("After Conversion V2 value is : ");
	if(v2_value == 1)
		v2_value = 32;
	else
		v2_value = 0;
	Send_Data_to_Display_Client(v2_value);
}

BLYNK_WRITE(V3)
{
	v3_value = param.asInt();
	Serial.print("V3 value is : ");
	Serial.println(v3_value);
	Serial.print("After Conversion V3 value is : ");
	if(v3_value == 1)
		v3_value = 64;
	else
		v3_value = 0;
	Send_Data_to_Display_Client(v3_value);
}

BLYNK_WRITE(V4)
{
	v4_value = param.asInt();
	Serial.print("V4 value is : ");
	Serial.println(v4_value);
	Serial.print("After Conversion V4 value is : ");
	if(v4_value == 1)
		v4_value = 128;
	else
		v4_value = 0;
	Send_Data_to_Display_Client(v4_value);
}
BLYNK_WRITE(V5)
{
	v5_value = param.asInt();
	Serial.print("V5 value is : ");
	Serial.println(v5_value);
	Serial.print("After Conversion V5 value is : ");
	if(v5_value == 1)
		v5_value = 3;
	else
		v5_value = 0;
	Send_Data_to_Display_Client(v5_value);
}
BLYNK_WRITE(V6)
{
	v6_value = param.asInt();
	Serial.print("V6 value is : ");
	Serial.println(v6_value);
	Serial.print("After Conversion V6 value is : ");
	if(v6_value == 1)
		v6_value = 5;
	else
		v6_value = 0;
	Send_Data_to_Display_Client(v6_value);
}
#endif
void notifyOnFire()
{
	digitalWrite(D4, HIGH);
	Blynk.logEvent("first-e","Temp above 30 degree");
	//Blynk.notify("Alert : Fire in the House");
	delay(1000);
	digitalWrite(D4, LOW);
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

void setup()
{     
	Serial.begin(9600);
	Serial.println("\nSetup init....!\n");
	nrf_config();
	Serial.println("\nNRF Setup....: Done!\n");
	Blynk.begin(auth, ssid, pass);    
	Serial.println("\nSetup Completed....!\n");
	ota_config();
	//timer.setInterval(2000L,notifyOnFire);
}

#define DEBUG 1
void receive_data_from_mesh()
{
	uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
	uint8_t len = 30;//sizeof(struct message);
	uint8_t i = 0;
	if (nrf24.waitAvailableTimeout(1000)){  
		if (nrf24.recv(buf, &len)){

			Serial.print("got message: ");
			Serial.println(len);
			//memcpy((uint8_t *)&ClientData, buf, sizeof(struct message));
			//debug_prints();
			//Send_Data_to_Display_Client();
#ifdef DEBUG
			i = 0;
			while(i<len) {
				Serial.print(buf[i]);
				Serial.print(" ");
				if (buf[i] & DOOR_EVENT_1_MAIN)
				{
					Blynk.logEvent("first-e","FE: Door is opened");
				}
				if (buf[i] & DOOR_EVENT_1_NORTH)
				{
					Blynk.logEvent("first-n","FN:Door is opened");
				}
				if (buf[i] & DOOR_EVENT_2_SECOND)
				{
					Blynk.logEvent("second-n","SN: Door is opened");
				}
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
		//Serial.println("No message, is nrf24_server running?");
	}

}
void loop()
{
	ArduinoOTA.handle();
	Blynk.run(); 
	receive_data_from_mesh();
}
