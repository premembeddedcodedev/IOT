#include <Wire.h>
#include <Arduino_JSON.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
struct message {
	uint8_t MsgID;
	uint8_t DoorClient;
	uint8_t DoorStatus;
	uint8_t SpeakerStatus;
	uint8_t Temparature[4];
	uint8_t Pressure[4];
	uint8_t Altitude[4];
	uint8_t Humidity[4];
	int TemparatureF;
	int PressureF;
	int AltitudeF;
	int HumidityF;
};
struct message ClientData;
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
//Adafruit_BME280 bme; // I2C
//int deviceID = EEPROM.read(0);
unsigned long delayTime;
int check_event = 0;
uint8_t data[28];
uint8_t buf[4];
uint8_t len = 4;
int messageID = 0;
void receiveEvent(int howMany) {
	Serial.print("Reading byte : ");
	Serial.println(howMany);
	uint8_t i= 0;
	while(i<howMany) {
		data[i] = Wire.read();
		//Serial.print("Data:");
		//Serial.println(data[i]);
		i++;
	}

/*	for ( i = 0; i < length; i++) {
		str[i]=(char)payload[i];
		Serial.print((char)payload[i]);
	}
	data[i] = 0;
	Serial.println();
	StaticJsonDocument <256> doc;
	deserializeJson(doc,data);
	const char* sensor = doc["sensor"];
	byte time = doc["time"];
	byte door = doc["Value"];
*/

	if(howMany <= 1) {
		check_event = 1;
	} else {
#if 1
		memcpy((struct message *)&ClientData, data, i);
		Serial.print("MsgID: ");
		Serial.println(ClientData.MsgID);
		Serial.print("DoorClient: ");
		Serial.println(ClientData.DoorClient);
		Serial.print("DoorStatus: ");
		Serial.println(ClientData.DoorStatus);
		Serial.print("SpeakerStatus: ");
		Serial.println(ClientData.SpeakerStatus);
		Serial.print("Temparature: ");
		sscanf(&data[4], "%f", &ClientData.TemparatureF);
		Serial.println(ClientData.TemparatureF);
		Serial.print("Pressure: ");
		sscanf(&data[8], "%f", &ClientData.PressureF);
		Serial.println(ClientData.PressureF);
		Serial.print("Altitude: ");
		sscanf(&data[12], "%f", &ClientData.AltitudeF);
		Serial.println(ClientData.AltitudeF);
		Serial.print("Humidity: ");
		sscanf(&data[16], "%f", &ClientData.TemparatureF);
		Serial.println(ClientData.HumidityF);
		Serial.println("\n--------------Data Received perfect--------------\n");
#endif
	}
}

void display_config()
{
	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
		Serial.println(F("SSD1306 allocation failed"));
		for(;;); // Don't proceed, loop forever
	}

	// Clear the buffer
	display.clearDisplay();
	display.display();

	testdrawstyles();    // Draw 'stylized' characters
}

void setup() {
	Serial.begin(9600);
	Wire.begin(8); 
	while(!Serial);    // time to get serial running
	Serial.println(F("BME280 test - Original"));
	Serial.println(F("praveen"));
	Wire.onReceive(receiveEvent); 

	display_config();

	Serial.println();
}

void loop() {
	delay(2000);
	display.clearDisplay();
	if(check_event == 1) {
		if(data[0] >= 1) {
			display.setCursor(0,0);
			display.setTextSize(1);
			display.setTextColor(SSD1306_WHITE);
			display.print("DoorStatus: ");
			display.println(data[0]);
			display.display();
			messageID = 0;     
		}
		check_event = 0;
	} else {
		display.setCursor(0,0);
		display.setTextSize(1);
		display.setTextColor(SSD1306_WHITE);
		//display.println("Message Received....!");
		display.print("messageID = ");
		display.println(messageID);
		display.print("DoorClient: ");
		display.println(ClientData.DoorClient);
		display.print("DoorStatus: ");
		display.println(ClientData.DoorStatus);
		display.print("SpeakerStatus: ");
		display.println(ClientData.SpeakerStatus);
		display.print("Temparature: ");
		display.println(ClientData.TemparatureF);
		display.print("Pressure: ");
		display.println(ClientData.PressureF);
		display.print("Altitude: ");
		display.println(ClientData.AltitudeF);
		display.print("Humidity: ");
		display.println(ClientData.HumidityF);
		display.display();
		messageID++;
	}

	if(messageID > 32767)
		messageID = 0;
}

void testdrawstyles(void) {
	display.clearDisplay();
	display.setCursor(0,0);
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	//display.println("Start");
	//display.display();
	//delay(2000);

	display.setCursor(50,27);
	display.println("Application init....");
	display.display();
	delay(1000);

	//display.setCursor(100,55);
	//display.println("End");
	//display.display();
	//delay(2000);

	//display.setCursor(0,55);
	//display.println("bottom left");
	//display.display();
	//delay(2000);

	//display.setCursor(72,0);
	//display.println("top right");
	//display.display();
}
