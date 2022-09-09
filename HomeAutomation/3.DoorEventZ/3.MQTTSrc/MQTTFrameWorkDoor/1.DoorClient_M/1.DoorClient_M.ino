#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <ESP8266WiFiMulti.h>
#include "DHT.h"
#define D5 14
#define DHTPIN 2
#define DHTTYPE DHT11

/*Command References from mosquitto_pub : 
 * Door sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue1" --> Ex: "11"
 * Light sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue2"
 * Speaker sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue4"
 * Movement sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue8"
 * Photos sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "NodevalueF" TODO: Need to see how to send more than 15 value
 * Noise sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue10"
 * Gas sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue12"
 */

/* DoorConfigs - 1st byte payload*/
#define FIRSTEASTDOOR_ENABLE		(1<<0)
#define FIRSTNORTHDOOR_ENABLE		(1<<1)
#define FIRSTWESTDOOR_ENABLE		(1<<2)
#define SECONDNORTHDOOR_ENABLE		(1<<3)
#define SECONDGLASSEASTDOOR_ENABLE 	(1<<4)
#define SECONDBEDNORTHDOOR_ENABLE 	(1<<5)
#define WINDOW1DOOR_ENABLE 		(1<<6)
#define WINDOWTOPDOOR_ENABLE 		(1<<7)

/* SensorConfigs - 2nd byte payload*/
#define DOOR_ENABLE	(1<<0)
#define TEMP_ENABLE	(1<<1)
#define LIGHT_ENABLE	(1<<2)
#define SPEAKER_ENABLE	(1<<3)
#define MOVEMENT_ENABLE (1<<4)
#define PHOTOS_ENABLE	(1<<5)
#define NOISE_ENABLE	(1<<6)
#define GAS_ENABLE	(1<<7)

/* payload_1 */
#define ARDUINO_BROKER_RETRY_ENABLE 	(1<<0) /* value - 1 for pub*/
#define ARDUINO_NODE_RESET_ENABLE	(1<<1) /* value - 2 for pub*/
#define ARDUINO_BROKER_RESET_ENABLE	(1<<2) /* value - 3 for pub*/
#define ARDUINO_BROKER_CLIDATA_ENABLE	(1<<3) /* value - 4 for pub*/

/* Door Clients values*/
#define DOOR_EVENT_1_MAIN	0x01
#define DOOR_EVENT_1_NORTH	0x02
#define DOOR_EVENT_2_SECOND	0x04

WiFiUDP ntpUDP;
char out[256];
char timeval[32] = {};
char *Nodename = "First East Door";
ESP8266WiFiMulti wifiMulti;
StaticJsonDocument<512> doc;
uint8_t NodeValue = DOOR_EVENT_1_MAIN;
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* passwd = "prem@123";
uint8_t retry = 5;
uint8_t retry2 = 5;
String clientId;
String clientId2;
uint8_t mqtt_broker_status_1;
uint8_t mqtt_broker_status_2;

DHT dht(DHTPIN, DHTTYPE);
NTPClient timeClient(ntpUDP, "in.pool.ntp.org", utcOffsetInSeconds);

/* publish : BrokerEvents, DoorSensorEvents
 * subscribe: SensorConfigs, EnableNodeConfigs
 */

struct message {
	uint8_t payload_0;
	uint8_t DoorEnabled;
	uint8_t doorvalue;
	uint8_t payload_1;
	uint8_t payload_2;
	uint8_t payload_3;

	uint8_t SpeakerEnable;
	uint8_t TempSensorEnable;
	uint8_t LightSensorEnable;
	uint8_t MovementSensorEnable;
	uint8_t PhotosEnable;
	uint8_t NoiseSensorEnable;
	uint8_t GasSensorEnable;
	uint8_t AirQualitySensorEnable;

	/*	float h;
		float t;
		float f;
		float hif;
		float hic;*/
};

struct message config;

const uint32_t connectTimeoutMs = 5000;
int HH;
int MM;
int SEC;

float h;
float t;
float f;
float hif;
float hic;

// Update these with values suitable for your network.

const char* Arduino_mqtt_server = "192.168.29.119";
const char* PC_mqtt_server = "192.168.29.21";

char destination[17];

WiFiClient espClient;
WiFiClient espClient2;
PubSubClient client(espClient);
PubSubClient client2(espClient2);
//PubSubClient client(server, 1883, callback, ethClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
const char* host = "esp8266-webupdate";
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

void dht_sensor_data()
{
	h = dht.readHumidity();
	// Read temperature as Celsius (the default)
	t = dht.readTemperature();
	// Read temperature as Fahrenheit (isFahrenheit = true)
	f = dht.readTemperature(true);

	// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println(F("Failed to read from DHT sensor!"));
		return;
	}

	// Compute heat index in Fahrenheit (the default)
	hif = dht.computeHeatIndex(f, h);
	// Compute heat index in Celsius (isFahreheit = false)
	hic = dht.computeHeatIndex(t, h, false);
#if 1
	Serial.print(F(" Humidity: "));
	Serial.print(h);
	Serial.print(F("%  Temperature: "));
	Serial.print(t);
	Serial.print(F("C "));
	Serial.print(f);
	Serial.print(F("F  Heat index: "));
	Serial.print(hic);
	Serial.print(F("C "));
	Serial.print(hif);
	Serial.println(F("F"));
#endif
}

void callback(char* topic, byte* payload, unsigned int length) {
	//Serial.println("received topic '"+topic+"' with data '"+(String)payload+"'");
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	Serial.print(length);
	for (int i = 0; i < length; i++) {
		switch(i) {
			case 0: // Sensor configs
				config.payload_0 = (uint8_t)(payload[0] - '0');
				Serial.println(config.payload_0);
				break;
			case 1: // Node configs
				config.payload_1 = (uint8_t)(payload[1] - '0');
				Serial.println(config.payload_1);
				break;
			case 2: // Door value
				config.payload_2 = (uint8_t)(payload[2] - '0');
				config.doorvalue = config.payload_2 & 0x7; 
				Serial.println(config.payload_2);
				break;
			case 3: // Reset configs
				config.payload_3 = (uint8_t)(payload[3] - '0');
				Serial.println(config.payload_3);
				break;
			default:
				break;
		}
	}
	Serial.println();
}

void broker_1_subscribtion()
{
	client.subscribe("RxFromBroker");
}

void broker_2_subscribtion()
{
	client2.subscribe("RxFromBroker");
}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected() && (retry2 > 0)) {
		Serial.print("Attempting 1-MQTT connection...");
		// Create a random client ID
		clientId = "ESP8266Client-";
		clientId += String(random(0xffff), HEX);
		Serial.print(clientId);
		// Attempt to connect
		if (client.connect(clientId.c_str())) {
			Serial.println("connected");
			mqtt_broker_status_1 = 1;
			retry2 = 5;
			client.publish("outTopic", "hello world");
			broker_1_subscribtion();
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
		if(retry2 == 1)	{
			Serial.println("1- MQTT Retry count is exceeded");
			retry2--;
			mqtt_broker_status_1 = 0;
			break;
		}
		retry2--;
	}

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
#if 1
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
#else
	// WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0) {
		Serial.println("no networks found");
	}
	else {
		Serial.print(n);
		Serial.println(" networks found");
		for (int i = 0; i < n; ++i) {
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println();
			delay(10);
		}
	}
#endif
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

void client_connect() {
	// Loop until we're reconnected
	while (!client2.connected() && retry > 0) {
		clientId2 = "ESP8266Client-";
		clientId2 += String(random(0xffff), HEX);
		Serial.print("Attempting 2-MQTT connection for ...");
		Serial.print(clientId2);
		if (client2.connect(clientId2.c_str())) {
			Serial.println("connected");
			retry = 5;
			mqtt_broker_status_2 = 1;
			client2.publish("outTopic", "hello world");
			broker_2_subscribtion();
		} else {
			Serial.print("failed, rc=");
			Serial.print(client2.state());
			Serial.println(" try again in 5 seconds");
			delay(5000);
		}
		if(retry == 1)	{
			Serial.println("2-MQTT Retry count is exceeded");
			retry--;
			mqtt_broker_status_2 = 0;
			break;
		}
		retry--;
	}


}

void broker_1_config()
{
	client.setServer(Arduino_mqtt_server, 1883);
	client.setCallback(callback);
	broker_1_subscribtion();
}

void broker_2_config()
{
	client2.setServer(PC_mqtt_server, 1884);
	client2.setCallback(callback);
	broker_2_subscribtion();
}

void setup() {

	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	pinMode(D5, INPUT_PULLUP);
	pinMode(12, OUTPUT);

	Serial.begin(9600);
	Serial.println("Publisher: Door Events Node");

	wifi_scan_config();

	ota_config();

	dht.begin();
	dht_sensor_data();

	config.payload_0 = 0;
	config.payload_1 = 0;
	config.payload_2 = 0;
	config.payload_3 = 0;

	timeClient.begin();

	broker_1_config();
	broker_2_config();	
	browser_config();
}

void mqtt_publish(char *pubstr)
{
	int b = serializeJson(doc, out);

	if(mqtt_broker_status_1 == 1)
		boolean rc = client.publish(pubstr, out); 

	if(mqtt_broker_status_2 == 1)
		boolean rc2 = client2.publish(pubstr, out);
}

void mqtt_broker_clidata(uint8_t event)
{
	doc["sensor"] = "misc";
	doc["AllConfigs"] = 0;
	doc["serverConfigs"] = event;
	doc["time"] = timeval;
	doc["MQTTBroker1Status"] = mqtt_broker_status_1;
	doc["MQTTBroker2Status"] = mqtt_broker_status_2;
	doc["NodeIPAddress"] = WiFi.SSID();
	doc["NodeName"] = Nodename;

	doc["Value"] = NodeValue;
	mqtt_publish("BrokerEvents");
}

void mqtt_broker_reset(uint8_t event)
{
	doc["sensor"] = "Door";
	doc["AllConfigs"] = 0;
	doc["serverConfigs"] = event;
	doc["time"] = timeval;
	doc["MQTTBroker1Status"] = mqtt_broker_status_1;
	doc["MQTTBroker2Status"] = mqtt_broker_status_2;
	doc["NodeIPAddress"] = WiFi.SSID();
	doc["NodeName"] = Nodename;

	doc["Value"] = NodeValue;
	mqtt_publish("BrokerEvents");
}

void dooropen_msgsend(uint8_t event)
{
	doc["sensor"] = "Door";
	doc["AllConfigs"] = DOOR_ENABLE;
	doc["serverConfigs"] = 0;
	doc["time"] = timeval;
	doc["MQTTBroker1Status"] = mqtt_broker_status_1;
	doc["MQTTBroker2Status"] = mqtt_broker_status_2;
	doc["NodeIPAddress"] = WiFi.SSID();
	doc["NodeName"] = Nodename;

	doc["Value"] = event;
	mqtt_publish("DoorSensorEvents");
}

void temp_sensor_config()
{
	doc["sensor"] = "Temp";
	doc["AllConfigs"] = TEMP_ENABLE;
	doc["serverConfigs"] = 0;
	doc["time"] = timeval;
	doc["MQTTBroker1Status"] = mqtt_broker_status_1;
	doc["MQTTBroker2Status"] = mqtt_broker_status_2;
	doc["NodeIPAddress"] = WiFi.SSID();
	doc["NodeName"] = Nodename;

	doc["Value"] = 0;
	doc["Humidity"] = h;
	doc["Temp"] = t;
	doc["F"] = f;
	doc["HIF"] = hif;
	doc["HIC"] = hic;
	mqtt_publish("DoorSensorEvents");
}

void config_time()
{
	timeClient.update();
	//Serial.println(timeClient.getFormattedTime());
	strcpy(destination, daysOfTheWeek[timeClient.getDay()]);
	//Serial.println(destination);

	HH = timeClient.getHours();
	if(HH <= 19) 
		HH = timeClient.getHours() + 4;
	else
		HH = timeClient.getHours() - 20; //till 12 we are taking care in if

	MM = timeClient.getMinutes();
	if(MM <= 29)
		MM = timeClient.getMinutes() + 30;
	else {
		MM = timeClient.getMinutes() - 30;
		HH += 1;
	}

	SEC = timeClient.getSeconds();

	//Serial.print(", ");
	//Serial.print(HH);
	//Serial.print(":");
	//Serial.print(MM);
	//Serial.print(":");
	//Serial.println(SEC);

	sprintf(timeval, "%s, %u : %u : %u", destination, HH, MM, SEC);

	//Serial.println(timeval);
}

void broker_status_check()
{
	if (!client.connected()) {
		reconnect();
	}
	if (!client2.connected()) {
		client_connect();
	}
}

void dev_events_check()
{
	if (digitalRead(D5) == 1) {
		Serial.println("Door is open");
		dooropen_msgsend(NodeValue);
		config.DoorEnabled = 1;
	}

	if (config.payload_1 & ARDUINO_BROKER_RETRY_ENABLE) {
		retry = 5;
		retry2 = 5;
		config.payload_1 &= ~ARDUINO_BROKER_RETRY_ENABLE;
	}
#if 0
	/* Sending to Broker */
	if (mqtt_broker_status_1 == 0 && retry == 0) {
		mqtt_broker_reset(ARDUINO_BROKER_RESET_ENABLE);
		retry = 5;
		retry2 = 5;
	}
#endif
	/* Door Sensor fullnode restart */
	if ((config.payload_3 & ARDUINO_NODE_RESET_ENABLE) || (config.payload_1 & ARDUINO_NODE_RESET_ENABLE)) {
		if(config.doorvalue & NodeValue) {
			ESP.restart();
			config.payload_1 &= ~ARDUINO_NODE_RESET_ENABLE;
			config.payload_3 &= ~ARDUINO_NODE_RESET_ENABLE;
		}
	}

	if (config.payload_1 & ARDUINO_BROKER_CLIDATA_ENABLE) {
		mqtt_broker_clidata(ARDUINO_BROKER_CLIDATA_ENABLE);
		config.payload_1 &= ~ARDUINO_BROKER_CLIDATA_ENABLE;
	}
	
	if(mqtt_broker_status_1 == 0 && mqtt_broker_status_1 == 0) {
			retry = 5;
			//retry2 = 5;
	}
}

void config_events_check()
{
/*	if (config.payload_0 & TEMP_ENABLE) {
		if(config.doorvalue & NodeValue) {
			temp_sensor_config();
			config.payload_0 &= ~TEMP_ENABLE;
		}
	}*/
}

void loop()
{
	if ( WiFi.status() != WL_CONNECTED ) {
		ESP.restart();
	}

	server.handleClient();
	MDNS.update();

	broker_status_check();

	ArduinoOTA.handle();

	config_time();

	dev_events_check();

	config_events_check();

	client.loop();
	client2.loop();

	delay(5000);
}
