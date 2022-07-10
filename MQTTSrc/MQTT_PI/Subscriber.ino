#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.

const char* ssid = "XXXXX";
const char* password = "XXXX";
const char* mqtt_server = "192.168.29.32";

WiFiClient espClient;
PubSubClient client(espClient);
//PubSubClient client(server, 1883, callback, ethClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  char str[length+1];
  int i;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for ( i = 0; i < length; i++) {
    str[i]=(char)payload[i];
    Serial.print((char)payload[i]);
  }
 
  str[i] = 0;
  Serial.println();
  StaticJsonDocument <256> doc;
  deserializeJson(doc,str);
  const char* sensor = doc["sensor"];
  long time = doc["time"];
  int door = doc["Value"];
  Serial.print("Door Status Received as: ");
  Serial.println(door);

  // Switch on the LED if an 1 was received as first character
  if (/*(char)payload[0]*/(int) door == 1) {
    digitalWrite(BUILTIN_LED, HIGH);
       // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
      // Turn the LED off by making the voltage HIGH
      digitalWrite(BUILTIN_LED, LOW);
  }

}you

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  Serial.println("Subscriber...");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  //Serial.print("Millie: ");
  //Serial.println(now);
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
#if 0
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
#endif
    client.subscribe("DoorEvents");
  }
}
