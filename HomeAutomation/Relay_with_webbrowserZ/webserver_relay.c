#include <ESP8266WiFi.h> //ESP8266 Arduino library with built in functions

#define ssid "SHSIAAP2" // Replace with your network name
#define password "XXXXXXX" // Replace with your network password

WiFiServer server(443); // Web Server on port 443

String header;
String state1 = "Off";
String state2 = "Off";
String state3 = "Off";
String state4 = "Off";
String state5 = "Off";
String state6 = "Off";
String state7 = "Off";
String state8 = "Off";

int LED1 = 5;
int LED2 = 4;
int LED3 = 10;
int LED4 = 2;
int LED5 = 12;
int LED6 = 13;
int LED7 = 9;
int LED8 = 3;

void setup() { // only executes once

  Serial.begin(9600); // Initializing serial port
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  pinMode(LED7, OUTPUT);
  pinMode(LED8, OUTPUT);
  
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH);
  digitalWrite(LED8, HIGH);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // Connecting to WiFi network
  while (WiFi.status() != WL_CONNECTED)
  {
  delay(500);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  server.begin(); // Starting the web server
  Serial.println("Web server Initiated. Waiting for the ESP IP...");
  delay(10000);
  
  Serial.println(WiFi.localIP()); // Printing the ESP IP address
}

// runs over and over again
void loop() {
  // Searching for new clients
  WiFiClient client = server.available();
  
  if (client) {
  Serial.println("New client");
  boolean blank_line = true; // boolean to locate when the http request ends
  while (client.connected()) {
  if (client.available()) {
  char c = client.read();
  header += c;
  
  if (c == '\n' && blank_line) {
  Serial.print(header);
  
  // Finding the right credential string
  if (header.indexOf("cHJhdmVlbjoxMjM0") >= 0) { // 
    //successful login
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    // turns the LED on and off
    if (header.indexOf("GET / HTTP/1.1") >= 0) {
    Serial.println("Main Web Page");
    }
    else if (header.indexOf("GET /LED1on HTTP/1.1") >= 0) {
    Serial.println("GPIO 5 On");
    state1 = "On";
    digitalWrite(LED1, HIGH);
    }
    else if (header.indexOf("GET /LED1off HTTP/1.1") >= 0) {
    Serial.println("GPIO 5 Off");
    state1 = "Off";
    digitalWrite(LED1, LOW);
    }
    else if (header.indexOf("GET /LED2on HTTP/1.1") >= 0) {
    Serial.println("GPIO 4 On");
    state2 = "On";
    digitalWrite(LED2, HIGH);
    }
    else if (header.indexOf("GET /LED2off HTTP/1.1") >= 0) {
    Serial.println("GPIO 4 Off");
    state2 = "Off";
    digitalWrite(LED2, LOW);
    }
    else if (header.indexOf("GET /LED3on HTTP/1.1") >= 0) {
    Serial.println("GPIO 0 On");
    state3 = "On";
    digitalWrite(LED3, HIGH);
    }
    else if (header.indexOf("GET /LED3off HTTP/1.1") >= 0) {
    Serial.println("GPIO 0 Off");
    state3 = "Off";
    digitalWrite(LED3, LOW);
    }
    else if (header.indexOf("GET /LED4on HTTP/1.1") >= 0) {
    Serial.println("GPIO 2 On");
    state4 = "On";
    digitalWrite(LED4, HIGH);
    }
    else if (header.indexOf("GET /LED4off HTTP/1.1") >= 0) {
    Serial.println("GPIO 2 Off");
    state4 = "Off";
    digitalWrite(LED4, LOW); //same
    }
    else if (header.indexOf("GET /LED5on HTTP/1.1") >= 0) {
    Serial.println("GPIO 14 On");
    state5 = "On";
    digitalWrite(LED5, HIGH);
    }
    else if (header.indexOf("GET /LED5off HTTP/1.1") >= 0) {
    Serial.println("GPIO 14 Off");
    state5 = "Off";
    digitalWrite(LED5, LOW);
    }
    else if (header.indexOf("GET /LED6on HTTP/1.1") >= 0) {
    Serial.println("GPIO 12 on");
    state6 = "On";
    digitalWrite(LED6, HIGH);
    }
    else if (header.indexOf("GET /LED6off HTTP/1.1") >= 0) {
    Serial.println("GPIO 12 Off");
    state6 = "Off";
    digitalWrite(LED6, LOW);
    }
    else if (header.indexOf("GET /LED7on HTTP/1.1") >= 0) {
    Serial.println("GPIO 13 On");
    state7 = "On";
    digitalWrite(LED7, HIGH);
    }
    else if (header.indexOf("GET /LED7off HTTP/1.1") >= 0) {
    Serial.println("GPIO 13 Off");
    state7 = "Off";
    digitalWrite(LED7, LOW);
    }
    else if (header.indexOf("GET /LED8on HTTP/1.1") >= 0) {
    Serial.println("GPIO 15 On");
    state8 = "On";
    digitalWrite(LED8, HIGH);
    }
    else if (header.indexOf("GET /LED8off HTTP/1.1") >= 0) {
    Serial.println("GPIO 15 Off");
    state8 = "Off";
    digitalWrite(LED8, LOW);
    } 
  // Web page
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    //client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">");
    client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap-theme.min.css\">");
    client.println("</head><div class=\"container\">");
    client.println("<h1>Praveen Home's Automation</h1>");
    client.println("<h2>WebServer</h2>");
    client.println("<h3>LED 1: State: " + state1);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED1on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED1off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>");
    client.println("</div>");
    client.println("<h3>LED 2: State: " + state2);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED2on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED2off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>");
    client.println("</div>");
    client.println("<h3>LED 3: State: " + state3);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED3on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED3off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>");
    client.println("</div>");
    client.println("<h3>LED 4: State: " + state4);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED4on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED4off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>");
    client.println("</div>");
    client.println("<h3>LED 5: State: " + state5);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED5on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED5off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>");
    client.println("</div>");
    client.println("<h3>LED 6: State: " + state6);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED6on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED6off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>");
    client.println("</div>");
    client.println("<h3>LED 7: State: " + state7);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED7on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED7off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>");
    client.println("</div>");
    client.println("<h3>LED 8: State: " + state8);
    client.println("<div class=\"row\">");
    client.println("<div class=\"col-md-2\"><a href=\"/LED8on\" class=\"btn btn-block btn-lg btn-primary\" role=\"button\">ON</a></div>");
    client.println("<div class=\"col-md-2\"><a href=\"/LED8off\" class=\"btn btn-block btn-lg btn-info\" role=\"button\">OFF</a></div>"); 
    client.println("</div></div></html>");
    }
    else { // Http request fails for unauthorized users
      client.println("HTTP/1.1 401 Unauthorized");
      client.println("WWW-Authenticate: Basic realm=\"Secure\"");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<html>Authentication failed</html>");
    }
    header = "";
    break;
    }
    if (c == '\n') { // starts reading a new line
      blank_line = true;
    }
  else if (c != '\r') { // finds a character on the current line
    blank_line = false;
  }
  }
  }
  delay(1);
  client.stop(); // ending the client connection
  Serial.println("Client disconnected.");
  }
}
