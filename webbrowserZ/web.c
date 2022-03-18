// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//extern std::string ipadd1 = "";

String cameraip = "";
String cameraip2 = "";
String cameraip3 = "";
String cameraip4 = "";
String cameraip5 = "";
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";
// Replace with your network credentials
const char* ssid = "SHSIAAP2";
const char* password = "XXXXXX";

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

uint8_t doorevent;
uint8_t doorevent_e;
uint8_t doorevent_s;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000;  
//  <link rel="fluid-icon" href="https://github.com/fluidicon.png" title="Prem's Home Automation">
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr, https://fonts.googleapis.com/css?family=Sofia&effect=fire" crossorigin="anonymous">
  <style>
    html {
     font-family: Serif;
     display: inline-block;
     margin: 0px auto;
     text-align: left;
    }
    h2 { font-size: 2.0rem; }
    p { font-size: 1.5rem; }
    .units { font-size: 1.0rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:left;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 Home Automation</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">°C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <h2>Prem Home Cameras from nodeMCU </h2>
  <p>
    <i class="fas fa-eye" style="color:#00add6;"></i>
    <span class="dht-labels">Cam-1</span>
    <span id="cameraip"></span>
  </p>
  <p>
    <i class="fas fa-eye" style="color:#00add6;"></i>
    <span class="dht-labels">Cam-2</span>
    <span id="cameraip2"></span>
  </p>
  <p>
    <i class="fas fa-eye" style="color:#00add6;"></i> 
    <span class="dht-labels">Cam-3</span>
    <span id="cameraip3"></span>
  </p>
  <p>
    <i class="fas fa-eye" style="color:#00add6;"></i>
    <span class="dht-labels">Cam-4</span>
    <span id="cameraip4"></span>
  </p>
  <p>
    <i class="fas fa-eye" style="color:#00add6;"></i>
    <span class="dht-labels">Cam-5</span>
    <span id="cameraip5"></span>
  </p>
  <h2>Prem Home Door Events</h2>
  <p>
    <i class="fas fa-bell" style="color:#00add6;" ></i>
    <span class="dht-labels">EVENTZERO</span>
    <span id="doorevent">%EVENT%</span>
  </p>
  <p>
    <i class="fas fa-bell" style="color:#00add6;"></i>
    <span class="dht-labels">EVENTONE</span>
    <span id="doorevent_e">%EVENT-E%</span>
  </p>
  <p>
    <i class="fas fa-bell" style="color:#00add6;"></i>
    <span class="dht-labels">EVENTTWO</span>
    <span id="doorevent_s">%EVENT-S%</span>
  </p>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input2: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input3: <input type="text" name="input3">
    <input type="submit" value="Submit">
  </form>
  <div class="bottom">
    <i class='fas fa-code-branch' style='font-size:36px;color:blue'></i>
    <span style="font-size:1.0rem;">Subscribe to </span>
    <a href="https://github.com/premembeddedcodedev/IOT" target="_blank" style="font-size:1.0rem;">The IoT Projects GitHub SourceCode</a>
  </div>
  </P>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cameraip").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/cameraip", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cameraip2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/cameraip2", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cameraip3").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/cameraip3", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cameraip4").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/cameraip4", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cameraip5").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/cameraip5", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("doorevent").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/doorevent", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("doorevent_e").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/doorevent_e", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("doorevent_s").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/doorevent_s", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Replaces placeholder with DHT values
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  else if(var == "EVENT-E"){
    return String(doorevent_e);
  }
  else if(var == "EVENT-S"){
    return String(doorevent_s);
  }
  else if(var == "EVENT"){
    return String(doorevent);
  }
  else if(var == "CAMERAIP"){
    return String(cameraip);
  }
  else if(var == "CAMERAIP2"){
    return String(cameraip2);
  }
  else if(var == "CAMERAIP3"){
    return String(cameraip3);
  }
  else if(var == "CAMERAIP4"){
    return String(cameraip4);
  }
  else if(var == "CAMERAIP5"){
    return String(cameraip5);
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
    
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
  cameraip = WiFi.localIP().toString().c_str();
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/doorevent", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(doorevent).c_str());
  });
  server.on("/doorevent_e", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(doorevent_e).c_str());
  });
  server.on("/doorevent_s", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(doorevent_s).c_str());
  });
  server.on("/cameraip", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", WiFi.localIP().toString().c_str());
    request->send_P(200, "text/plain", String(cameraip).c_str());
  });
  server.on("/cameraip2", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", WiFi.localIP().toString().c_str());
    request->send_P(200, "text/plain", String(cameraip2).c_str());
  });
  server.on("/cameraip3", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", WiFi.localIP().toString().c_str());
    request->send_P(200, "text/plain", String(cameraip3).c_str());
  });
  server.on("/cameraip4", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", WiFi.localIP().toString().c_str());
    request->send_P(200, "text/plain", String(cameraip4).c_str());
  });
  server.on("/cameraip5", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", WiFi.localIP().toString().c_str());
    request->send_P(200, "text/plain", String(cameraip5).c_str());
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  
  // Start server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = 10.1190;
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    float newH = 12.1190;
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
    doorevent  = 1;
    doorevent_e  = 2;
    doorevent_s  = 3;
    cameraip = "192.168.10.1";
    cameraip2 = "192.168.10.2";
    cameraip3 = "192.168.10.3";
    cameraip4 = "192.168.10.4";
    cameraip5 = "192.168.10.5";
  }
}
