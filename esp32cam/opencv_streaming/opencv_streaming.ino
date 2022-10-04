#include "WifiCam.hpp"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

const char* WIFI_SSID = "SHSIAAP2";
const char* WIFI_PASS = "prem@123";
const char* host = "esp32";
unsigned long previousMillis = 0;
unsigned long interval = 10000;

WebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);
void serveJpg()
{
	auto frame = esp32cam::capture();
	if (frame == nullptr) {
		Serial.println("CAPTURE FAIL");
		server.send(503, "", "");
		return;
	}
	Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
			static_cast<int>(frame->size()));

	server.setContentLength(frame->size());
	server.send(200, "image/jpeg");
	WiFiClient client = server.client();
	frame->writeTo(client);
}

void handleJpgLo()
{
	if (!esp32cam::Camera.changeResolution(loRes)) {
		Serial.println("SET-LO-RES FAIL");
	}
	serveJpg();
}

void handleJpgHi()
{
	if (!esp32cam::Camera.changeResolution(hiRes)) {
		Serial.println("SET-HI-RES FAIL");
	}
	serveJpg();
}

void handleJpgMid()
{
	if (!esp32cam::Camera.changeResolution(midRes)) {
		Serial.println("SET-MID-RES FAIL");
	}
	serveJpg();
}

/*
 * Login page
 */

const char* loginIndex =
"<form name='loginForm'>"
"<table width='20%' bgcolor='A09F9F' align='center'>"
"<tr>"
"<td colspan=2>"
"<center><font size=4><b>ESP32 Login Page</b></font></center>"
"<br>"
"</td>"
"<br>"
"<br>"
"</tr>"
"<tr>"
"<td>Username:</td>"
"<td><input type='text' size=25 name='userid'><br></td>"
"</tr>"
"<br>"
"<br>"
"<tr>"
"<td>Password:</td>"
"<td><input type='Password' size=25 name='pwd'><br></td>"
"<br>"
"<br>"
"</tr>"
"<tr>"
"<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
"</tr>"
"</table>"
"</form>"
"<script>"
"function check(form)"
"{"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{"
"window.open('/serverIndex')"
"}"
"else"
"{"
" alert('Error Password or Username')/*displays error message*/"
"}"
"}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update'>"
"<input type='submit' value='Update'>"
"</form>"
"<div id='prg'>progress: 0%</div>"
"<script>"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
" $.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!')"
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>";

const char* passwd = "prem@123";

void scan()
{
	// WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0) {
		Serial.println("no networks found");
	}
	else {
		Serial.print(n);
		Serial.println(" networks found");
		for (int i = 0; i < n; ++i) {
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println();
			delay(10);
		}
	}
}

void wifi_scan_config()
{
	WiFi.disconnect();
	//scan();
	WiFi.persistent(false);
	wifiMulti.addAP("SHSIAAP2", passwd);
	wifiMulti.addAP("JioFiber5G", passwd);
	wifiMulti.addAP("JioFiber4g", passwd);
	wifiMulti.addAP("TP-Link_F524", passwd);
	wifiMulti.addAP("TP-Link_F524_5G", passwd);

	if (wifiMulti.run() == WL_CONNECTED) {
		Serial.print("WiFi connected: ");
		Serial.print(WiFi.SSID());
		Serial.print(" ");
		Serial.println(WiFi.localIP());
	} else {
		Serial.println("WiFi not connected!");
		ESP.restart();
	}
}

void  setup(){
	Serial.begin(115200);
	Serial.println("Hellow praveen...");
	{
		using namespace esp32cam;
		Config cfg;
		cfg.setPins(pins::AiThinker);
		cfg.setResolution(hiRes);
		cfg.setBufferCount(2);
		cfg.setJpeg(80);

		bool ok = Camera.begin(cfg);
		Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
	}

	wifi_scan_config();

/*

	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}
*/
	Serial.print("http://");
	Serial.println(WiFi.localIP());
	Serial.println("  /cam-lo.jpg");
	Serial.println("  /cam-hi.jpg");
	Serial.println("  /cam-mid.jpg");

	server.on("/cam-lo.jpg", handleJpgLo);
	server.on("/cam-hi.jpg", handleJpgHi);
	server.on("/cam-mid.jpg", handleJpgMid);
	/*use mdns for host name resolution*/
	if (!MDNS.begin(host)) { //http://esp32.local
		Serial.println("Error setting up MDNS responder!");
		while (1) {
			delay(1000);
		}
	}
	Serial.println("mDNS responder started");
	/*return index page which is stored in serverIndex */
	server.on("/", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", loginIndex);
			});
	server.on("/serverIndex", HTTP_GET, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/html", serverIndex);
			});
	/*handling uploading firmware file */
	server.on("/update", HTTP_POST, []() {
			server.sendHeader("Connection", "close");
			server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
			ESP.restart();
			}, []() {
			HTTPUpload& upload = server.upload();
			if (upload.status == UPLOAD_FILE_START) {
			Serial.printf("Update: %s\n", upload.filename.c_str());
			if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
			Update.printError(Serial);
			}
			} else if (upload.status == UPLOAD_FILE_WRITE) {
			/* flashing firmware to ESP*/
			if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
			Update.printError(Serial);
			}
			} else if (upload.status == UPLOAD_FILE_END) {
			if (Update.end(true)) { //true to set the size to the current progress
			Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
			} else {
			Update.printError(Serial); 
			}
			}
			});
	Serial.println("Config sill one left.. praveen latest");
	server.begin();
	Serial.println("Config done.. praveen latest");
}

void loop()
{
    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(1000);
    }
#if 0
	unsigned long currentMillis = millis();
	if((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
		Serial.println(millis());
		Serial.println("WiFi Lost... Restarting...");
		ESP.restart();
		previousMillis = currentMillis;
		delay(10000);
	}
#endif


	server.handleClient();
}
