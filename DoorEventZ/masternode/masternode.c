#include <ESP8266WiFi.h>
#include "namedMesh.h"
#include "NodeMcu_ESP8266_Gmail_Sender_by_FMT.h"  
#include <RH_NRF24.h>
#define FIRST_MAIN_DOOR_STRING "1ST-MAIN-DOOREVENT"
#define FIRST_NORTH_DOOR_STRING "1ST-NORTH-DOOREVENT"
#define SECOND_MAIN_DOOR_STRING "2ND-MAIN-DOOREVENT"

#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04
#pragma region Globals

const char* ssid = "SHSIAAP2";
const char* password = "XXXXXXX";
uint8_t connection_state = 0;                    
uint16_t reconnect_interval = 10000;             
#pragma endregion Globals
String recipientID;
String subject;
String body;
RH_NRF24 nrf24(2, 4); 
WiFiServer server(80);
String header;
String output5State = "off";
String output4State = "off";
Scheduler userScheduler;
namedMesh mesh;
String nodeName = "MCUServerNWDev";
Gsender *gsender = Gsender::Instance();
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
uint8_t data[2];
uint8_t len = sizeof(buf);
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

void InputInfo(){
	Serial.println("\n");
	Serial.print("Recipient's Email ID : ");
	while(Serial.available() == 0);
	recipientID = Serial.readString();
	recipientID.replace("\n", "");
	Serial.println(recipientID);

	Serial.print("Subject              : ");
	while(Serial.available() == 0);
	subject = Serial.readString();
	subject.replace("\n", "");
	Serial.println(subject);

	Serial.print("Body                 : ");
	while(Serial.available() == 0);
	body = Serial.readString();
	for(int i = 0; i < body.length(); i++) {
		Serial.print((char)body.charAt(i));
		if (i % 65 == 0 && i != 0) {
			if(body.charAt(i) == ' '){
				Serial.println();
				Serial.print("                       ");
			}
			else{
				Serial.print("-\n");
				Serial.print("                       ");
			}
		}
	}
}

void CustomInfo(){
	recipientID = (String)"vanand1651962@gmail.com";
	subject = (String)"Door Events";
	body = (String)"Test mail just ignore - 2102";
}

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
	static uint16_t attempt = 0;
	Serial.print("Connecting to ");
	if(nSSID) {
		WiFi.begin(nSSID, nPassword);  
		Serial.println(nSSID);
	} else {
		WiFi.begin(ssid, password);
		Serial.println(ssid);
	}

	uint8_t i = 0;
	while(WiFi.status()!= WL_CONNECTED && i++ < 50)
	{
		delay(200);
		Serial.print(".");
	}
	++attempt;
	Serial.println("");
	if(i == 51) {
		Serial.print("Connection: TIMEOUT on attempt: ");
		Serial.println(attempt);
		if(attempt % 2 == 0)
			Serial.println("Check if access point available or SSID and Password\r\n");
		return false;
	}
	Serial.print("Connected to ");
	Serial.print(ssid);
	Serial.println(" successfully!");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	return true;
}

void Awaits()
{
	uint32_t ts = millis();
	while(!connection_state)
	{
		delay(50);
		if(millis() > (ts + reconnect_interval) && !connection_state){
			connection_state = WiFiConnect();
			ts = millis();
		}
	}
}


void setup()
{
	Serial.begin(9600);
	pinMode(5, OUTPUT);
	Serial.println("\nHello from Server....!\n");
	connection_state = WiFiConnect();
	if(!connection_state)
		Awaits();
	CustomInfo();

	if (!nrf24.init()) 
	{
		Serial.println("init failed");

	}
	if (!nrf24.setChannel(3)) 
		Serial.println("setChannel failed");
	if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
		Serial.println("setRF failed");
	server.begin();
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
uint8_t door1_sent_num = 0;
uint8_t door2_sent_num = 0;
uint8_t door3_sent_num = 0; 

void mail_send()
{
	if(gsender->Subject(subject)->Send(recipientID, body))
		Serial.println("Email sent");
	else {
		Serial.print("Error sending message: ");
		Serial.println(gsender->getError());
	}
}

void loop() {
	WiFiClient client = server.available();
	if (nrf24.waitAvailableTimeout(1000))
	{
		if (nrf24.recv(buf, &len))
		{
			Serial.print("got reply: ");
			Serial.println((char*)buf);
			int first = (int)buf[0];
			Serial.println(first);
			if(first & DOOR_EVENT_1_MAIN)
				data[0] |= DOOR_EVENT_1_MAIN;
			if(first & DOOR_EVENT_1_NORTH)
				data[0] |= DOOR_EVENT_1_NORTH;
			if(first & DOOR_EVENT_2_SECOND)
				data[0] |= DOOR_EVENT_2_SECOND;
		}
		else
			Serial.println("recv failed");
	}
	else
		Serial.println("No reply.");
	if (data[0] & DOOR_EVENT_1_MAIN)
	{
		event_send();
		data[0] &=~DOOR_EVENT_1_MAIN;
		door1_sent_num ++;
	}
	if (data[0] & DOOR_EVENT_1_NORTH)
	{
		event_send();
		data[0] &=~DOOR_EVENT_1_NORTH;
		door2_sent_num++;
	}
	if (data[0] & DOOR_EVENT_2_SECOND)
	{
		event_send();
		data[0] &=~DOOR_EVENT_2_SECOND;
		door3_sent_num++;
	}
	if (door1_sent_num > 10) {
		subject = (String)"Door Events - 1st MainDoor";
		body = (String)"Main Door Event";
		mail_send();
		digitalWrite(5, HIGH);
		door1_sent_num = 0;
	}
	if (door2_sent_num > 10) {
		subject = (String)"Door Events - 1st MainDoor North";
		body = (String)"Main North Door Event";
		mail_send();
		door2_sent_num = 0;
	}
	if(door3_sent_num > 10) {
		subject = (String)"Door Events - 2nd Floor";
		body = (String)"Second Floor Door Event";
		mail_send();
		door3_sent_num = 0;
	}
	if (client) {
		Serial.println("New Client.");
		String currentLine = ""; 
		currentTime = millis();
		previousTime = currentTime;
		while (client.connected() && currentTime - previousTime <= timeoutTime) {
			currentTime = millis();         
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				header += c;
				if (c == '\n') {
					if (currentLine.length() == 0) {
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println("Connection: close");
						client.println();

						if (header.indexOf("GET /5/on") >= 0) {
							Serial.println("GPIO 5 on");
							output5State = "on";
							digitalWrite(5, HIGH);
						} else if (header.indexOf("GET /5/off") >= 0) {
							Serial.println("GPIO 5 off");
							output5State = "off";
							digitalWrite(5, LOW);
						} else if (header.indexOf("GET /4/on") >= 0) {
							Serial.println("GPIO 4 on");
							output4State = "on";
						} else if (header.indexOf("GET /4/off") >= 0) {
							Serial.println("GPIO 4 off");
							output4State = "off";
						}

						client.println("<!DOCTYPE html><html>");
						client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
						client.println("<link rel=\"icon\" href=\"data:,\">");
						client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
						client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
						client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
						client.println(".button2 {background-color: #77878A;}</style></head>");

						client.println("<body><h1>ESP8266 Web Server</h1>");
						client.println("<p>GPIO 5 - State " + output5State + "</p>");
						if (output5State=="off") {
							client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
						} else {
							client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
						} 

						client.println("<p>GPIO 4 - State " + output4State + "</p>");
						if (output4State=="off") {
							client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
						} else {
							client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
						}
						client.println("</body></html>");

						client.println();
						break;
					} else { 
						currentLine = "";
					}
				} else if (c != '\r') {
					currentLine += c;
				}
			}
		}
		header = "";
		client.stop();
		Serial.println("Client disconnected.");
		Serial.println("");
	}
	delay(2000);
}
