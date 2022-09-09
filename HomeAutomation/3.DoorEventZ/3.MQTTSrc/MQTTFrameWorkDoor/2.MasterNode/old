/*
 * uMQTTBroker demo for Arduino (C++-style)
 * 
 * The program defines a custom broker class with callbacks, 
 * starts it, subscribes locally to anything, and publishs a topic every second.
 * Try to connect from a remote client and publish something - the console will show this as well.
 */

#include <ESP8266WebServer.h>
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
#include <PubSubClient.h>
#include <ESP8266WiFiMulti.h>

#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04
#define NUMCLIENTS 3
#define DOOR_EVENT_2 0x03

/* payload_0 Enablement */
#define DOOR_ENABLE (1<<0)
#define TEMP_ENABLE (1<<1)
#define LIGHT_ENABLE (1<<2)
#define SPEAKER_ENABLE (1<<3)
#define MOVEMENT_ENABLE (1<<4)
#define PHOTOS_ENABLE (1<<5)
#define NOISE_ENABLE (1<<6)
#define GAS_ENABLE (1<<7)

/* payload_1 */
#define ARDUINO_BROKER_RESET_ENABLE	(1<<2)
#define ARDUINO_BROKER_CLIDATA_ENABLE	(1<<3)
#define ARDUINO_BROKER_INIT_RESET_ENABLE 2

/* Door Nodes:
 *	publish : BrokerEvents, DoorSensorEvents
 * 	subscribe: SensorConfigs, EnableNodeConfigs
 */
WiFiClient espClient;
PubSubClient client(espClient);
String clientId;
char *ipaddr[32];
int retry2 = 5;
int clientcount = 0;
const uint32_t connectTimeoutMs = 5000;
uint8_t broker_reset_check = 0;
const char* passwd = "prem@123";
const char* brokername = "ArduinoBroker";
ESP8266WiFiMulti wifiMulti;
String readings;
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
struct message {
	/* Data get it from DoorClient */
	uint8_t payload_0;
	uint8_t payload_1;
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

/*
 * Your WiFi config here
 */
char ssid[] = "TP-Link_F524";     // your network SSID (name)
char pass[] = "prem@123"; // your network password
bool WiFiAP = false;      // Do yo want the ESP as AP?
RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather
int value = 0;
const char* host = "esp8266-broker";
/* Web Server config -- start*/
ESP8266WebServer server(80);
//const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

/* Style */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex =
"<form name=loginForm>"
"<h1>ESP32 Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{window.open('/serverIndex')}"
"else"
"{alert('Error Password or Username')}"
"}"
"</script>" + style;
/* Server Index Page */
String serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;
/* Web Server config -- end*/

void browser_config()
{
	MDNS.begin(host);
	server.on("/", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", serverIndex);
			});
	server.on("/update", HTTP_POST, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
			ESP.restart();
			}, []() {
			HTTPUpload& upload = server.upload();
			if (upload.status == UPLOAD_FILE_START) {
			Serial.setDebugOutput(true);
			WiFiUDP::stopAll();
			Serial.printf("Update: %s\n", upload.filename.c_str());
			uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
			if (!Update.begin(maxSketchSpace)) { //start with max available size
			Update.printError(Serial);
			}
			} else if (upload.status == UPLOAD_FILE_WRITE) {
			if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Update.printError(Serial);
			}
			} else if (upload.status == UPLOAD_FILE_END) {
			if (Update.end(true)) { //true to set the size to the current progress
			Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
			} else {
				Update.printError(Serial);
			}
			Serial.setDebugOutput(false);
			}
			yield();
			});
	server.begin();
	MDNS.addService("http", "tcp", 80);
	Serial.printf("Ready! Open http://%s.local in your browser\n", host);
}

char* timeval[10];

#if 0
void clients_connected(char* str)
{
	for (int i = 0; i < getClientCount(); i++) {
		IPAddress addr;
		String client_id;

		getClientAddr(i, addr);
		getClientId(i, client_id);
		//Serial.println("Client "+client_id+" on addr: "+addr.toString());
		sprintf(timeval[i], "%s, %s", client_id, addr.toString());
		ipaddr[i] = timeval[i];
	}
	clientcount = getClientCount();
}
#endif
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
			char str[length+1];
			os_memcpy(str, data, length);
			str[length] = '\0';

			Serial.println("received topic '"+topic+"' with data '"+(String)str+"'");
			printClients();
			StaticJsonDocument <256> doc;
			deserializeJson(doc,str);

			const char* sensor = doc["sensor"];
			long time = doc["time"];
			uint8_t temp = doc["Value"];
			ClientData.payload_0 = doc["AllConfigs"];
			ClientData.payload_1 = doc["serverConfigs"];

			if(ClientData.payload_1 & ARDUINO_BROKER_RESET_ENABLE)
				broker_reset_check = broker_reset_check | temp;

			if(ClientData.payload_1 & ARDUINO_BROKER_CLIDATA_ENABLE) {
				Serial.println("Enabled client Data....");
			}
		}

		virtual void printClients() {
			for (int i = 0; i < getClientCount(); i++) {
				IPAddress addr;
				String client_id;
				char copy[32];
				getClientAddr(i, addr);
				getClientId(i, client_id);
				strcpy(timeval[i], addr.toString().c_str());
				//client_id.toCharArray(copy, client_id.length());
				//strcat(timeval[i], copy);
				Serial.println("Client "+client_id+" on addr: "+addr.toString());
				//sprintf(timeval[i], "%s, %s", client_id, addr.toString());
				//ipaddr[i] = timeval[i];
				clientcount = getClientCount();
			}
		}
};
#if 0
class myMQTTBroker: public uMQTTBroker
{
	public:
		virtual bool onConnect(IPAddress addr, uint16_t client_count) {
			Serial.println(addr.toString()+" connected");
			clients_connected(addr.toString());
			return true;
		}

		virtual bool onAuth(String username, String password) {
			Serial.println("Username/Password: "+username+"/"+password);
			return true;
		}

		virtual void printClients() {
			for (int i = 0; i < getClientCount(); i++) {
				IPAddress addr;
				String client_id;

				getClientAddr(i, addr);
				getClientId(i, client_id);
				Serial.println("Client "+client_id+" on addr: "+addr.toString());
				char timeval[64] = {};
				sprintf(timeval, "%s, %s", client_id, addr.toString());
				//clients_connected(timeval);
			}
		}

		virtual void onData(String topic, const char *data, uint32_t length) {
			/*myBroker.subscribe("BrokerEvents");*/
			char str[length+1];
			os_memcpy(str, data, length);
			str[length] = '\0';
			Serial.println("received topic '"+topic+"' with data '"+(String)str+"'");
			StaticJsonDocument <256> doc;
			deserializeJson(doc,str);

			const char* sensor = doc["sensor"];
			long time = doc["time"];
			ClientData.payload_1 = doc["serverConfigs"];

			if(ClientData.payload_1 & ARDUINO_BROKER_RESET_ENABLE) {
				uint8_t temp = doc["Value"];
				broker_reset_check = broker_reset_check | temp;
			}

			if(ClientData.payload_1 & ARDUINO_BROKER_CLIDATA_ENABLE) {
				Serial.println("Enabled client Data....");
			}

		}
};
#endif
myMQTTBroker myBroker;

void Ciritical_Door_event()
{
	Serial.println("Sending to gateway");
	nrf24.send((uint8_t *)&ClientData.DoorStatus, 1);
	nrf24.waitPacketSent();
}
String EnableConfigDataForDoor(int NodeNumber)
{
	JSONVar jsonReadings;
	jsonReadings["Node"] = NodeNumber;
	jsonReadings["SensorBits"] = ClientData.payload_0;
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
char out[128];
StaticJsonDocument<256> doc;

void mqtt_publish(char *pubstr)
{
	int b = serializeJson(doc, out);
	myBroker.publish(pubstr, out);
}

void mqtt_broker_initiated_nodereset(uint8_t event)
{
	doc["sensor"] = "BrokerData";
	doc["serverConfigs"] = event;
	doc["NodeIPAddress"] = WiFi.SSID();
	doc["BrokerName"] = brokername;
	Serial.println(brokername);

	int b = serializeJson(doc, out);
	myBroker.publish("ClientsData", out);
	Serial.println("Data Sent to broker.....");
}

void mqtt_broker_clidata(uint8_t event)
{
	//digitalWrite(BUILTIN_LED, HIGH);     // Initialize the BUILTIN_LED pin as an output
	doc["sensor"] = "BrokerData";
	doc["serverConfigs"] = event;
	doc["NodeIPAddress"] = WiFi.SSID();
	doc["BrokerName"] = brokername;
	Serial.println(brokername);
	doc["Cnumbers"] = clientcount;
	Serial.println(clientcount);
#if 0
	doc["client0"] = timeval[0];
	Serial.println(timeval[0]);
	doc["client1"] = timeval[1];
	Serial.println(timeval[1]);
	doc["client2"] = timeval[2];
	Serial.println(timeval[2]);
#endif
	int b = serializeJson(doc, out);
	myBroker.publish("ClientsData", out);
	Serial.println("Data Sent to broker.....");
}

void enable_config()
{
	doc["AllConfigs"] = DOOR_ENABLE;//config.SensorBits;
	//mqtt_publish();
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

void scan()
{
	String ssid;
	int32_t rssi;
	uint8_t encryptionType;
	uint8_t* bssid;
	int32_t channel;
	bool hidden;
	int scanResult;

	Serial.println(F("Starting WiFi scan..."));

	scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

	if (scanResult == 0) {
		Serial.println(F("No networks found"));
	} else if (scanResult > 0) {
		Serial.printf(PSTR("%d networks found:\n"), scanResult);

		// Print unsorted scan results
		for (int8_t i = 0; i < scanResult; i++) {
			WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);

			Serial.printf(PSTR("  %02d: [CH %02d] [%02X:%02X:%02X:%02X:%02X:%02X] %ddBm %c %c %s \n"),
					i,
					channel,
					bssid[0], bssid[1], bssid[2],
					bssid[3], bssid[4], bssid[5],
					rssi,
					(encryptionType == ENC_TYPE_NONE) ? ' ' : '*',
					hidden ? 'H' : 'V',
					ssid.c_str());
			delay(0);
		}
	} else {
		Serial.printf(PSTR("WiFi scan error %d"), scanResult);
	}
}

void wifi_scan_config()
{
	WiFi.disconnect();
	scan();
	WiFi.persistent(false);
	wifiMulti.addAP("SHSIAAP2", passwd);
	wifiMulti.addAP("JioFiber5G", passwd);
	wifiMulti.addAP("JioFiber4g", passwd);
	wifiMulti.addAP("TP-Link_F524", passwd);
	wifiMulti.addAP("TP-Link_F524_5G", passwd);

	if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
		Serial.print("WiFi connected: ");
		Serial.print(WiFi.SSID());
		Serial.print(" ");
		Serial.println(WiFi.localIP());
	} else {
		Serial.println("WiFi not connected!");
		ESP.restart();
	}
}

void setup()
{
	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	Serial.begin(9600);

	wifi_scan_config();

	ota_config();
	Serial.println("Starting PVs MQTT broker - 1");
	myBroker.init();

	myBroker.subscribe("BrokerEvents");
	browser_config();
	mqtt_broker_initiated_nodereset(ARDUINO_BROKER_INIT_RESET_ENABLE);
}

int counter = 0;

void validate_broker_reset()
{
	ClientData.DoorClient |= broker_reset_check;

	if(ClientData.DoorClient == 0x7)
		ESP.restart();
}

void loop()
{
	/*
	 * Publish the counter value as String
	 */
	ArduinoOTA.handle();

	if (ClientData.payload_1 & ARDUINO_BROKER_RESET_ENABLE)
	{
		validate_broker_reset();
		ClientData.payload_1 &=~ARDUINO_BROKER_RESET_ENABLE;
	}

	if (ClientData.payload_1 & ARDUINO_BROKER_CLIDATA_ENABLE)
	{
		mqtt_broker_clidata(ARDUINO_BROKER_CLIDATA_ENABLE);
		ClientData.payload_1 &=~ARDUINO_BROKER_CLIDATA_ENABLE;
	}

}

