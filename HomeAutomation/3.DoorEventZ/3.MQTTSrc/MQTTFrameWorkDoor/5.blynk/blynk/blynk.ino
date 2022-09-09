//Tech Trends Shameer
//Control LED Using Blynk 2.0/Blynk IOT

#define BLYNK_TEMPLATE_ID "TMPLZoeBRvSy"
#define BLYNK_DEVICE_NAME "MyHomeAutomation"
#define BLYNK_AUTH_TOKEN "Pec1K_-BT_WrW4Hkyi_1AINac3XyaHlo"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>  
#include <BlynkSimpleEsp8266.h>
BlynkTimer timer; 

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "SHSIAAP2";  // Enter your Wifi Username
char pass[] = "prem@123";  // Enter your Wifi password

int ledpin = D4;

void notifyOnFire()
{
	digitalWrite(D4, HIGH);
	Blynk.logEvent("first-e","Temp above 30 degree");
	//Blynk.notify("Alert : Fire in the House");
	delay(1000);
	digitalWrite(D4, LOW);
}

void setup()
{     
	Serial.begin(9600);
	Blynk.begin(auth, ssid, pass);    
	pinMode(ledpin,OUTPUT);
	timer.setInterval(2000L,notifyOnFire);
}

void loop()
{
	Blynk.run(); 
	timer.run();
}
