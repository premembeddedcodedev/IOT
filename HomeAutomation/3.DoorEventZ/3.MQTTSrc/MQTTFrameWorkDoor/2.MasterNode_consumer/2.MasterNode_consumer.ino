#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <RH_NRF24.h>
#include <BlynkSimpleEsp8266.h>
#define D5 14

/*Command References from mosquitto_pub : 
 * Door sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue1" --> Ex: "11"
 * Light sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue2"
 * Speaker sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue4"
 * Movement sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue8"
 * Photos sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "NodevalueF" TODO: Need to see how to send more than 15 value
 * Noise sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue10"
 * Gas sensor enable : byte0 = NadeValue, byte1 = 1, ==> string : "Nodevalue12"
 */

#define BLYNK_TEMPLATE_ID "TMPLZoeBRvSy"
#define BLYNK_DEVICE_NAME "MyHomeAutomation"
#define BLYNK_AUTH_TOKEN "Pec1K_-BT_WrW4Hkyi_1AINac3XyaHlo"

/* DoorConfigs - 1st byte payload*/
#define FIRSTEASTDOOR_ENABLE		(1<<0)
#define FIRSTNORTHDOOR_ENABLE		(1<<1)
#define FIRSTWESTDOOR_ENABLE		(1<<2)
#define SECONDNORTHDOOR_ENABLE		(1<<3)
#define SECONDGLASSEASTDOOR_ENABLE 	(1<<4)
#define SECONDBEDNORTHDOOR_ENABLE 	(1<<5)
#define WINDOW1DOOR_ENABLE 		(1<<6)
#define WINDOWTOPDOOR_ENABLE 		(1<<7)
char auth[] = BLYNK_AUTH_TOKEN;
/* SensorConfigs - 2nd byte payload*/
#define DOOR_ENABLE	(1<<0)
#define TEMP_ENABLE	(1<<1)
#define LIGHT_ENABLE	(1<<2)
#define SPEAKER_ENABLE	(1<<3)
#define MOVEMENT_ENABLE (1<<4)
#define PHOTOS_ENABLE (1<<5)
#define NOISE_ENABLE (1<<6)
#define GAS_ENABLE (1<<7)

/* NodeConfigs */
#define ARDUINO_BROKER_RETRY_ENABLE (1<<0)
#define ARDUINO_NODE_RESET_ENABLE (1<<1)
#define ARDUINO_BROKER_RESET_ENABLE (1<<2)
#define ARDUINO_BROKER_CLIDATA_ENABLE (1<<3)
#define ARDUINO_BROKER_ASSISTANT_RESET_ENABLE (1<<4)

/* Door Clients values*/
#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04
#define DOOR_EVENT_MASTER 0x08

int ledpin = D4;
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather
WiFiUDP ntpUDP;
uint8_t broker_reset_check = 0;
char out[256];
char timeval[32] = {};
char *Nodename = "Second North Door";
ESP8266WiFiMulti wifiMulti;
StaticJsonDocument<512> doc;
uint8_t NodeValue = DOOR_EVENT_2_SECOND;
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* passwd = "";
uint8_t retry = 5;
uint8_t retry2 = 5;
String clientId;
String clientId2;
uint8_t mqtt_broker_status_1;
uint8_t mqtt_broker_status_2;

NTPClient timeClient(ntpUDP, "in.pool.ntp.org", utcOffsetInSeconds);

/* publish : BrokerEvents, DoorSensorEvents
 * subscribe: SensorConfigs, EnableNodeConfigs
 */

struct message {
	/* Data get it from DoorClient */
	int SensorBits;
	uint8_t NodeConfigs;
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
uint8_t len = sizeof(struct message);

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
/*
 * Login page
 */

const char* loginIndex =
"<form name='loginForm'>"
"<table width='20%' bgcolor='A09F9F' align='center'>"
"<tr>"
"<td colspan=2>"
"<center><font size=4><b>ESP32 Login Page</b></font></center>"
"<br>"
"</td>"
"<br>"
"<br>"
"</tr>"
"<tr>"
"<td>Username:</td>"
"<td><input type='text' size=25 name='userid'><br></td>"
"</tr>"
"<br>"
"<br>"
"<tr>"
"<td>Password:</td>"
"<td><input type='Password' size=25 name='pwd'><br></td>"
"<br>"
"<br>"
"</tr>"
"<tr>"
"<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
"</tr>"
"</table>"
"</form>"
"<script>"
"function check(form)"
"{"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{"
"window.open('/index_html')"
"}"
"else"
"{"
" alert('Error Password or Username')/*displays error message*/"
"}"
"}"
"</script>";

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

uint8_t LED1pin = D7;
bool LED1status = LOW;

uint8_t LED2pin = D6;
bool LED2status = LOW;
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html> <html>
<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">
<title>LED Control</title>
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}
.button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}
.button-on {background-color: #1abc9c;}
.button-on:active {background-color: #16a085;}
.button-off {background-color: #34495e;}
.button-off:active {background-color: #2c3e50;}
{font-size: 14px;color: #888;margin-bottom: 10px;}
</style>
</head>
<body>
<h1>ESP8266 Web Server</h1>
<h3>Node Links : </h3>

<p>Board Restart Status: reset </p><a class=\"button button-off\" href=/led1on>reset</a>
<p>Board upload : </p><a class=\"button button-off\" href=/upload>upload</a>

</body>
</html>)rawliteral";

String SendHTML(uint8_t led1stat,uint8_t led2stat){
	String ptr = "<!DOCTYPE html> <html>\n";
	ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
	ptr +="<title>LED Control</title>\n";
	ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
	ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
	ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
	ptr +=".button-on {background-color: #1abc9c;}\n";
	ptr +=".button-on:active {background-color: #16a085;}\n";
	ptr +=".button-off {background-color: #34495e;}\n";
	ptr +=".button-off:active {background-color: #2c3e50;}\n";
	ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
	ptr +="</style>\n";
	ptr +="</head>\n";
	ptr +="<body>\n";
	ptr +="<h1>ESP8266 Web Server</h1>\n";
	ptr +="<h3>Using Station(STA) Mode</h3>\n";

	if(led1stat)
	{ptr +="<p>Board Restart Status: ON</p><a class=\"button button-off\" href=\"/led1off\">ON</a>\n";}
	else
	{ptr +="<p>Board Restart Status:: OFF</p><a class=\"button button-on\" href=\"/led1on\">OFF</a>\n";}

	if(led2stat)
	{ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";}
	else
	{ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";}

	ptr +="</body>\n";
	ptr +="</html>\n";
	return ptr;
}
void handle_OnConnect() {
	LED1status = LOW;
	LED2status = LOW;
	Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
	server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}

void handle_led1on() {
	LED1status = HIGH;
	Serial.println("GPIO7 Status: ON");
	server.send(200, "text/html", SendHTML(true,LED2status)); 
}

void handle_led1off() {
	LED1status = LOW;
	Serial.println("GPIO7 Status: OFF");
	server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_led2on() {
	LED2status = HIGH;
	Serial.println("GPIO6 Status: ON");
	server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_led2off() {
	LED2status = LOW;
	Serial.println("GPIO6 Status: OFF");
	server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_NotFound(){
	server.send(404, "text/plain", "Not found");
}
void browser_config()
{
	MDNS.begin(host);
	server.on("/", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", loginIndex);
			});
	//server.on("/", handle_OnConnect);
	server.on("/led1on", handle_led1on);
	server.on("/led1off", handle_led1off);
	server.on("/led2on", handle_led2on);
	server.on("/led2off", handle_led2off);
	server.on("/index_html", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", index_html);
			});
	server.on("/upload", HTTP_GET, []() {
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

void onData(String topic, byte *data, uint32_t length) {
	char str[length+1];
	os_memcpy(str, data, length);
	str[length] = '\0';
	Serial.println("received topic '"+topic+"' with data '"+(String)str+"'");
	StaticJsonDocument <256> doc;
	deserializeJson(doc,str);
	const char* sensor = doc["sensor"];
	long time = doc["time"];
	ClientData.NodeConfigs = doc["serverConfigs"];
	ClientData.SensorBits = doc["AllConfigs"];
	if(ClientData.SensorBits & DOOR_ENABLE) {
		ClientData.DoorStatus = doc["Value"];
		Serial.print("Door Status Received as: ");
		Serial.println(ClientData.DoorStatus);
	}

	if(ClientData.SensorBits & TEMP_ENABLE) {
		ClientData.h = doc["Humidity"];
		ClientData.t = doc["Temp"];
		ClientData.f = doc["F"];
		ClientData.hif = doc["HIF"];
		ClientData.hic = doc["HIC"];
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
	}
	if(ClientData.NodeConfigs & ARDUINO_BROKER_RESET_ENABLE) {
		uint8_t temp = doc["Value"];
		broker_reset_check = broker_reset_check | temp;
	}
	if(ClientData.NodeConfigs & ARDUINO_BROKER_CLIDATA_ENABLE) {
		Serial.println("Enabled client Data....");
		const char* sensor1 = doc["NodeIPAddress"];
		Serial.println(sensor1);
		sensor1 = doc["BrokerName"];
		Serial.println(sensor1);
		uint8_t totClients = doc["Cnumbers"];
		Serial.println(totClients);
		const char* addr;
		addr = doc["client0"];
		Serial.println(addr);
		addr = doc["client1"];
		Serial.println(addr);
		addr = doc["client2"];
		Serial.println(addr);
	}
}

void callback(char* topic, byte* payload, unsigned int length) {
	//Serial.println("received topic '"+topic+"' with data '"+(String)payload+"'");
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	if(strcmp(topic, "RxFromBroker") == 0) {
		Serial.print("Am in All Rx Payload : ");
		Serial.println((uint8_t)payload[3]);
		Serial.println(payload[3]);
		if((uint8_t)(payload[3] - '0') == 2)
			ESP.restart();
	} else
		onData(topic, payload, length);

}
void Ciritical_Door_event()
{
	Serial.println("Sending to gateway");
	nrf24.send((uint8_t *)&ClientData.DoorStatus, 1);
	nrf24.waitPacketSent();
}

void broker_1_subscribtion()
{
	client.subscribe("DoorSensorEvents");
	client.subscribe("ClientsData");
	client.subscribe("RxFromBroker");
}

void broker_2_subscribtion()
{
	client2.subscribe("DoorSensorEvents");
	client2.subscribe("ClientsData");
	client.subscribe("RxFromBroker");
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
			retry2 = 5;
			mqtt_broker_status_1 = 1;
			client.publish("BCoutTopic", "BrokerConsumer-1");
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
			client2.publish("BCoutTopic", "BrokerCLient-2");
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

void nrf_config()
{
	if (!nrf24.init()) 
		Serial.println("init failed");

	if (!nrf24.setChannel(3)) 
		Serial.println("setChannel failed");

	if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
		Serial.println("setRF failed");
}

BlynkTimer timer;

void notifyOnFire()
{
	Blynk.logEvent("First-E", "First-E is opened");
}

void setup() {

	pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
	pinMode(D5, INPUT_PULLUP);
	pinMode(12, OUTPUT);
	pinMode(ledpin,OUTPUT);

	Serial.begin(9600);
	Serial.println("Publisher: All Rx Door Events Node");

	nrf_config();

	wifi_scan_config();

	ota_config();

	ClientData.SensorBits = 0;
	ClientData.NodeConfigs = 0;

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

void process_payload0(byte buf)
{	
	Serial.println("proceesing byte 0....");
	Serial.println(buf);
	switch(buf) { //coming from blynk
		case 3:
		case 5:
			break;
		case 8:
			if (client.connected())
				client.publish("RxFromBroker", "0012");//ARDUINO_NODE_RESET_ENABLE;
			if (client2.connected())
				client2.publish("RxFromBroker", "0012");//ARDUINO_NODE_RESET_ENABLE;
			break;
		case 16:
			if (client.connected())
				client.publish("RxFromBroker", "0022");//ARDUINO_NODE_RESET_ENABLE;
			if (client2.connected())
				client2.publish("RxFromBroker", "0022");//ARDUINO_NODE_RESET_ENABLE;
			break;
		case 32:
			client.publish("RxFromBroker", "0042");//ARDUINO_NODE_RESET_ENABLE;
			client2.publish("RxFromBroker", "0042");//ARDUINO_NODE_RESET_ENABLE;
			break;
		case 64:
			client.publish("BrokerReset", "1");//ARDUINO_NODE_RESET_ENABLE;
			client2.publish("BrokerReset", "1");//ARDUINO_NODE_RESET_ENABLE;
			break;
		case 128:
			ESP.restart();
			break;
		default:
			break;
	}
}

void receive_data_from_mesh()
{
	uint8_t i = 0;

	if (nrf24.waitAvailableTimeout(1000)){  
		if (nrf24.recv(buf, &len)){
			//memcpy((uint8_t *)&ClientData, buf, len);
			if(i==0)
				process_payload0(buf[0]);
			if(i<len)
				Serial.println(buf[i++]);
			else
				return;
		}
		else
		{
			Serial.println("recv failed");
		}

	}
	else
	{
	}

}

void dev_events_check()
{
	if (ClientData.DoorStatus & DOOR_EVENT_1_MAIN)
	{
		//enable_config();
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

	if (ClientData.NodeConfigs & ARDUINO_BROKER_ASSISTANT_RESET_ENABLE)
		ClientData.NodeConfigs &=~ARDUINO_BROKER_ASSISTANT_RESET_ENABLE;

	receive_data_from_mesh();
}

void reconnect_dup() {
	if(!client.connected()) {
		clientId = "ESP8266Client-";
		clientId += String(random(0xffff), HEX);
		Serial.print(clientId);
		if (client.connect(clientId.c_str())) {
			Serial.println("connected");
			mqtt_broker_status_1 = 1;
			client.publish("outTopic", "hello world");
			broker_1_subscribtion();
		}
	}
	if(!client2.connected()) {
		clientId2 = "ESP8266Client-";
		clientId2 += String(random(0xffff), HEX);
		Serial.print(clientId2);
		if (client2.connect(clientId2.c_str())) {
			Serial.println("connected");
			mqtt_broker_status_2 = 1;
			client2.publish("outTopic", "hello world");
			broker_2_subscribtion();
		}
	}

}

void handle_func()
{
	if(LED1status)
		ESP.restart();
}
void loop()
{
	if ( WiFi.status() != WL_CONNECTED ) {
		ESP.restart();
	}

	server.handleClient();
	MDNS.update();

	broker_status_check();

	//reconnect_dup();

	ArduinoOTA.handle();

	config_time();

	dev_events_check();

	handle_func();
	client.loop();
	client2.loop();

	//Blynk.run();
	//timer.run();
}
