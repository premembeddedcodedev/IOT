#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <Adafruit_Sensor.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
const char* ssid = "SHSIAAP";
const char* password = "prem@123";

int temp = 0;

// Time API endpoint
const char* timeApiUrl = "http://worldtimeapi.org/api/timezone/Asia/Kolkata";

String get_weekday(int n)
{
        switch(n) {
                case 1:
                        Serial.println("Monday");
                        return "Monday";
                        break;
                case 2:
                        Serial.println("Tuesday");
                        return "Tuesday";
                        break;
                case 3:
                        Serial.println("Wednesday");
                        return "Wednesday";
                        break;
                case 4:
                        Serial.println("Thursday");
                        return "Thursday";
                        break;
                case 5:
                        Serial.println("Friday");
                        return "Friday";
                        break;
                case 6:
                        Serial.println("Saturday");
                        return "Saturday";
                        break;
                case 7:
                        Serial.println("Sunday");
                        return "Sunday";
                        break;
                default:
                        break;
        };
        return " ";
}

void TCA9548A(uint8_t bus)
{
	Wire.beginTransmission(0x70);
	Wire.write(1 << bus);
	Wire.endTransmission();
}

void setup()
{
	Serial.begin(115200);

	TCA9548A(1);

	// Connect to Wi-Fi
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.println("Connecting to WiFi...");
	}
	Serial.println("Connected to WiFi");

	if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
		Serial.println(F("1 - SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}

	display.display();
	delay(2000);
	display.clearDisplay();

	TCA9548A(2);

	if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
		Serial.println(F("2 - SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}

	display.display();
	delay(2000);
	display.clearDisplay();
}

void loop()
{
	TCA9548A(1);

	display.clearDisplay();

	// Fetch current time from the time API
	String currentTimeStr = getTimeFromApi();

	// Parse the JSON response
	const size_t capacity = JSON_OBJECT_SIZE(2) + 100;
	DynamicJsonDocument doc(capacity);
	deserializeJson(doc, currentTimeStr);

	// Extract time values
	String datetime = doc["datetime"];
	String timezone = doc["timezone"];
	Serial.println(datetime);

        int weekday = doc["day_of_week"];
        Serial.println("weekday");
        Serial.println(weekday);

        String weekname = get_weekday(weekday);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0); // Adjust the position as needed
        display.print(weekname);

	// Adjust the time to the Kolkata time zone (GMT+5)
	const char* KolkataTimeZone = "Asia/Kolkata";

	//    int hours = datetime.substring(11, 13).toInt();
	//    int minutes = datetime.substring(14, 16).toInt();
	//    int seconds = datetime.substring(17, 19).toInt();


	int hours, minutes, seconds;
	parseTime(datetime, hours, minutes, seconds);
	Serial.print("Hours: ");
	Serial.println(hours);
	Serial.print("Minutes: ");
	Serial.println(minutes);
	Serial.print("Seconds: ");
	Serial.println(seconds);

	// Display hours and minutes with text size 3
	display.setTextSize(3);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(5, 10); // Adjust the position as needed
	if (hours < 10) {
		display.print("0"); // Add leading zero for single-digit hours
	}
	display.print(hours);
	display.print(":");
	if (minutes < 10) {
		display.print("0"); // Add leading zero for single-digit minutes
	}
	display.print(minutes);

	// Display seconds with text size 2
	display.setTextSize(2);
	display.setCursor(100, 15); // Adjust the position as needed
	if (seconds < 10) {
		display.print("0"); // Add leading zero for single-digit seconds
	}
	display.print(seconds);

	int check = hours;

	if(minutes == 40 && check == hours) {
		Serial.print("2nd display Seconds: ");
		Serial.println(temp);
		TCA9548A(2);
		display.setTextColor(WHITE);
		display.clearDisplay();
	
		display.setTextSize(1);
		display.setCursor(30,0);
		display.print("Iterations : ");
	
		display.setTextSize(1);
		display.setCursor(10,20);
		temp++;
		display.print(temp);
		display.display();
		check += 1;
	}

	display.display();
	delay(1000); // Update the display every second
}

String getTimeFromApi()
{
	WiFiClient client;

	// Make a GET request to the time API
	HTTPClient http;
	http.begin(client, timeApiUrl); // Use begin with WiFiClient
	int httpCode = http.GET();
	if (httpCode > 0) {
		if (httpCode == HTTP_CODE_OK) {
			String payload = http.getString();
			http.end();
			return payload;
		}
	} else {
		Serial.println("Failed to connect to time API");
	}
	http.end();
	return "";
}



void parseTime(const String &timeStr, int &hours, int &minutes, int &seconds) {
	// Find the position of 'T' in the string
	int tIndex = timeStr.indexOf('T');

	if (tIndex != -1) {
		// Extract the time substring after 'T'
		String timeSubStr = timeStr.substring(tIndex + 1);

		// Find the position of '+' in the time substring
		int plusIndex = timeSubStr.indexOf('+');

		if (plusIndex != -1) {
			// Extract the time part before '+'
			String timePart = timeSubStr.substring(0, plusIndex);

			// Split the time part into hours, minutes, and seconds
			hours = timePart.substring(0, 2).toInt();
			minutes = timePart.substring(3, 5).toInt();
			seconds = timePart.substring(6, 8).toInt();
		}
	}
}
