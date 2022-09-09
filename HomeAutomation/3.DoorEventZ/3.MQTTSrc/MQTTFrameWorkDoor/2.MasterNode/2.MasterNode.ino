#include <ESP8266WiFi.h>
#include "uMQTTBroker.h"
#include <ArduinoOTA.h>
/*
 * Your WiFi config here
 */
char ssid[] = "SHSIAAP2";     // your network SSID (name)
char pass[] = "prem@123"; // your network password
bool WiFiAP = false;      // Do yo want the ESP as AP?

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

		virtual void onDisconnect(IPAddress addr, String client_id) {
			Serial.println(addr.toString()+" ("+client_id+") disconnected");
		}

		virtual bool onAuth(String username, String password, String client_id) {
			Serial.println("Username/Password/ClientId: "+username+"/"+password+"/"+client_id);
			return true;
		}

		virtual void onData(String topic, const char *data, uint32_t length) {
			char data_str[length+1];
			os_memcpy(data_str, data, length);
			data_str[length] = '\0';

			Serial.println("received topic '"+topic+"' with data '"+(String)data_str+"'");
			Serial.println((uint8_t)data_str[0]);
			if((uint8_t)data_str[0] == 1)
				ESP.restart();
			
			//printClients();
		}

		// Sample for the usage of the client info methods

		virtual void printClients() {
			for (int i = 0; i < getClientCount(); i++) {
				IPAddress addr;
				String client_id;

				getClientAddr(i, addr);
				getClientId(i, client_id);
				Serial.println("Client "+client_id+" on addr: "+addr.toString());
			}
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
	Serial.println("Starting MQTT broker");
	myBroker.init();

        ota_config();

	/*
	 * Subscribe to anything
	 */
	myBroker.subscribe("BrokerReset");
}

int counter = 0;

void loop()
{
	ArduinoOTA.handle();

	/*
	 * Publish the counter value as String
	 */
	//myBroker.publish("broker/counter", (String)counter++);

	// wait a second
	delay(1000);
}
