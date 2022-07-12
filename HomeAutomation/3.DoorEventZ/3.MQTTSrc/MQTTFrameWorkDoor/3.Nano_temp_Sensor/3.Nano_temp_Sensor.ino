#include <Arduino_JSON.h>
#include <ArduinoJson.h>
#include <RH_NRF24.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

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
RH_NRF24 nrf24(7, 8);
uint8_t data[2];
uint8_t len = 4;
int check_event = 0;
Adafruit_BME280 bme; // I2C
unsigned long delayTime;

float temp;
float pres;
float alt;
float hum;

void nrf_config()
{
	if (!nrf24.init())
		Serial.println("init failed");

	if (!nrf24.setChannel(3))
		Serial.println("setChannel failed");

	if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
		Serial.println("setRF failed");   
}

void tempsensor_config()
{
	unsigned status; 
	status = bme.begin(0x76, &Wire);
	if (!status) {
		Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
		Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
		Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
		Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
		Serial.print("        ID of 0x60 represents a BME 280.\n");
		Serial.print("        ID of 0x61 represents a BME 680.\n");
		while (1) delay(10);
	}

	Serial.println("-- Default Test --");
	delayTime = 1000;
}

void setup()
{
	Wire.begin(8);               
	Serial.begin(9600);
	Serial.println("NRF Send I2C send Data....");
	nrf_config();
	tempsensor_config();
}

void event_send()
{
	Serial.println("Sending to gateway");
	data[0] = random(20);
	data[1] = random(20);
	Serial.println(data[0]);
	Serial.println(data[1]);
	nrf24.send(data, 2);
	nrf24.waitPacketSent();
}

void i2c_transfer()
{
	char out[128];
	StaticJsonDocument<256> doc;
	doc["sensor"] = "Door";
	doc["time"] = random(20);
	doc["Value"] = 4;
	int b = serializeJson(doc, out);
	Wire.beginTransmission(8); // transmit to device #8
	Wire.write(out, sizeof(30));
	Wire.endTransmission();    // stop transmitting
}

void i2c_send()
{
	printValues();
	ClientData.MsgID = random(20);
	ClientData.DoorClient = random(20);
	ClientData.DoorStatus = random(20);
	ClientData.SpeakerStatus = random(20);
	sprintf(ClientData.Temparature, "%d", (int)temp);
	//Serial.print("Writing size and Data : ");
	//Serial.print(ClientData.Temparature);
	sprintf(ClientData.Pressure, "%d", (int)pres);
	//Serial.print("Writing size and Data : ");
	//Serial.print(ClientData.Pressure);
	sprintf(ClientData.Altitude, "%d", (int)alt);
	//Serial.print("Writing size and Data : ");
	//Serial.print(ClientData.Altitude);
	sprintf(ClientData.Humidity, "%d", (int)hum);
	//Serial.print("Writing size and Data : ");
	//Serial.print(ClientData.Humidity);
	Serial.print("Writing size and Data : ");
	Serial.println(sizeof(ClientData));
	Serial.print("MsgID: ");
	Serial.println(ClientData.MsgID);
	Serial.print("DoorClient: ");
	Serial.println(ClientData.DoorClient);
	Serial.print("DoorStatus: ");
	Serial.println(ClientData.DoorStatus);
	Serial.print("SpeakerStatus: ");
	Serial.println(ClientData.SpeakerStatus);
	Serial.print("PTemparature: ");
	sscanf(ClientData.Temparature, "%d", &ClientData.TemparatureF);
	Serial.println(ClientData.TemparatureF);
	Serial.print("PPressure: ");
	sscanf(ClientData.Pressure, "%d", &ClientData.PressureF);
	Serial.println(ClientData.PressureF);
	Serial.print("PAltitude: ");
	sscanf(ClientData.Altitude, "%d", &ClientData.AltitudeF);
	Serial.println(ClientData.AltitudeF);
	Serial.print("PHumidity: ");
	sscanf(ClientData.Humidity, "%d", &ClientData.HumidityF);
	Serial.println(ClientData.HumidityF);  
	Serial.println("\n--------------Data Sent perfect--------------\n");
	Serial.println();
	Wire.beginTransmission(8); // transmit to device #8
	Wire.write((uint8_t *)&ClientData, sizeof(ClientData));
	Wire.endTransmission();    // stop transmitting
}

void receive_data_from_mesh()
{
	uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

	uint8_t len = sizeof(struct message);
	uint8_t i = 0;
	if (nrf24.waitAvailableTimeout(5000)){  
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
			if(buf[0] > 0) {
				Serial.print("i2c message: ");
				Serial.println(buf[0]);
				Wire.beginTransmission(8); // transmit to device #8
				Wire.write(buf, 1);
				Wire.endTransmission(); 
			}
			memset(buf, 0, sizeof(buf));    
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

void loop() {
	receive_data_from_mesh();
	delay(5000);
	i2c_send();
}

void printValues() {
	temp = bme.readTemperature();
	pres = (bme.readPressure() / 100.0F);
	alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
	hum = bme.readHumidity();
#if 0
	Serial.print("Temperature = ");
	Serial.print(temp);
	Serial.println(" °C");

	Serial.print("Pressure = ");
	Serial.print(pres);
	Serial.println(" hPa");

	Serial.print("Approx. Altitude = ");
	Serial.print(alt);
	Serial.println(" m");

	Serial.print("Humidity = ");
	Serial.print(hum);
	Serial.println(" %");

	Serial.println();
#endif
}
