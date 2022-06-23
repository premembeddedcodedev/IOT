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

#define DOOR_EVENT_MAIN_1_SOUND_ENABLE 0x1
#define DOOR_EVENT_1_NORTH_SOUND_ENABLE 0x2
#define DOOR_EVENT_2_SECOND_SOUND_ENABLE 0x4

#pragma region Globals
const char* ssid = "XXXX";                  // WIFI network name
const char* password = "XXXX";              // WIFI network password
uint8_t connection_state = 0;                    
uint16_t reconnect_interval = 10000;             
#pragma endregion Globals

String recipientID;
String subject;
String body;
int led = 16;     // LED pin
int button = 5; // push button is connected
int temp = 0;    // temporary variable for reading the button pin status

RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather
struct message {
	uint8_t MsgID;
	uint8_t DoorClient;
	uint8_t DoorStatus;
	uint8_t SpeakerStatus;
	uint8_t Temparature;
	uint8_t Pressure[4];
	uint8_t Altitude[4];
	uint8_t Humidity;
};
struct message ClientData;
Scheduler userScheduler; // to control your personal task
namedMesh mesh;
String nodeName = "MCUServerNWDev"; // Name needs to be unique
Gsender *gsender = Gsender::Instance();
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
uint8_t data[2];
uint8_t sound[2];
uint8_t len = RH_NRF24_MAX_MESSAGE_LEN;

Task taskSendMessage( TASK_SECOND*10, TASK_FOREVER, []() {}); // start with a one second interval

void sendSMS(String message)
{
	// AT command to send SMS message
	Serial.print("AT+CMGF=1\r");
	delay(200);
	// recipient's mobile number, in international format
	Serial.println("AT + CMGS = \"+919989855099\"");
	delay(200);
	Serial.println(message);
	// message to send
	delay(200);
	// End AT command with a ^Z, ASCII code 26
	Serial.println((char)26);
	delay(200);
	Serial.println();
	// give module time to send SMS
	delay(100);
}

void InputInfo(){ // function to input recipient's email ID, subject and body of the email through the serial monitor
	Serial.println("\n");
	Serial.print("Recipient's Email ID : ");
	while(Serial.available() == 0);
	recipientID = Serial.readString(); // reads from the serial monitor
	recipientID.replace("\n", "");
	Serial.println(recipientID);

	Serial.print("Subject              : ");
	while(Serial.available() == 0);
	subject = Serial.readString(); // reads from the serial monitor
	subject.replace("\n", "");
	Serial.println(subject);

	Serial.print("Body                 : ");
	while(Serial.available() == 0);
	body = Serial.readString(); // reads from the serial monitor
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

void CustomInfo(){ // function to set custom settings
	//replace the below three lines with your information
	recipientID = (String)"vanand1651962@gmail.com"; // enter the email ID of the recipient
	subject = (String)"Door Events"; // enter the subject of the email
	body = (String)"Test mail just ignore - 2102"; // enter the body of the email
	/*--------------------------------------------------------------*/
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
	pinMode(led, OUTPUT);   // declare LED as output
	pinMode(button, INPUT); // declare push button as input
	digitalWrite(led, LOW);
	nrf_config();
	Serial.println("\nHello from WebssServer....!\n");
	pinMode(5, OUTPUT);
	connection_state = WiFiConnect();
	if(!connection_state)  // if not connected to WIFI
		Awaits();          // constantly trying to connect
	CustomInfo(); // uncomment this line if you want to send the email using the custom information which are already set in the code; and comment the above line.
}

void event_send() {
	Serial.println("Sending to gateway");
	sound[0] |= DOOR_EVENT_MAIN_1_SOUND_ENABLE;
	nrf24.send(sound, 2);
	nrf24.waitPacketSent();
	sound[0] = 0;
}

uint8_t door1_sent_num = 0;
uint8_t door2_sent_num = 0;
uint8_t door3_sent_num = 0; 

void mail_send()
{
	if(gsender->Subject(subject)->Send(recipientID, body)) { // sends the email using a single line function
		Serial.println("Email sent"); // message confirmation

	} else {
		Serial.print("Error sending message: ");
		Serial.println(gsender->getError()); // prints out the exact error if the email wasn't successfully sent
	}
}

void debug_prints()
{
	Serial.print("MSGID: ");
	Serial.println(ClientData. MsgID);
	Serial.print("DoorClient: ");
	Serial.println(ClientData. DoorClient);
	Serial.print("DoorStatus: ");
	Serial.println(ClientData. DoorStatus);
	Serial.print("SpeakerStatus: ");
	Serial.println(ClientData. SpeakerStatus);
	Serial.print("Temparature: ");
	Serial.println(ClientData. Temparature);

	Serial.print("Pressure: ");
	Serial.print(ClientData. Pressure[0]);
	Serial.print(ClientData. Pressure[1]);
	Serial.print(ClientData. Pressure[2]);
	Serial.println(ClientData. Pressure[3]);

	Serial.print("Altitude: ");
	Serial.print(ClientData. Altitude[0]);
	Serial.print(ClientData. Altitude[1]);
	Serial.print(ClientData. Altitude[2]);
	Serial.println(ClientData. Altitude[3]);

	Serial.print("Humidity: ");
	Serial.println(ClientData. Humidity);
}

void Send_Data_to_Display_Client()
{
	nrf24.send((uint8_t *)&ClientData, sizeof(struct message));
	nrf24.waitPacketSent();
}
#define DEBUG 1
void receive_data_from_mesh()
{
	uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
	uint8_t len = sizeof(struct message);
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
	receive_data_from_mesh();
}
