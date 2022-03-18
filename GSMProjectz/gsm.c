void sendSMS(String message)
{
	// AT command to send SMS message
	Serial.print("AT+CMGF=1\r");
	delay(200);
	// recipient's mobile number, in international format
	Serial.println("AT + CMGS = \"XXXXXXXXXX\"");
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

void setup() {
	Serial.begin(9600);
	Serial.println("GSM Application Started");
}

void loop() {
	sendSMS("Hello from nodemcu");
	delay(10000);
}
