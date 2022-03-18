#if 1
#include <ESP8266WiFi.h>
#include "namedMesh.h"
#include "NodeMcu_ESP8266_Gmail_Sender_by_FMT.h"  
#include <RH_NRF24.h>
#if 0
#define   MESH_SSID       "MCUServerNWAP"
#define   MESH_PASSWORD   "mcupasswd"
#define   MESH_PORT       5555
#endif
#define FIRST_MAIN_DOOR_STRING "1ST-MAIN-DOOREVENT"
#define FIRST_NORTH_DOOR_STRING "1ST-NORTH-DOOREVENT"
#define SECOND_MAIN_DOOR_STRING "2ND-MAIN-DOOREVENT"

#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04
#pragma region Globals

//replace the below two lines with your information
const char* ssid = "SHSIAAP2";                  // WIFI network name
const char* password = "XXXXX";              // WIFI network password
/*--------------------------------------------------*/
uint8_t connection_state = 0;                    
uint16_t reconnect_interval = 10000;             
#pragma endregion Globals

String recipientID;
String subject;
String body;

RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather

Scheduler userScheduler; // to control your personal task
namedMesh mesh;
String nodeName = "MCUServerNWDev"; // Name needs to be unique
Gsender *gsender = Gsender::Instance();
// Should be a message for us now
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
uint8_t data[2];
uint8_t len = sizeof(buf);

Task taskSendMessage( TASK_SECOND*10, TASK_FOREVER, []() {
    //String msg = String("MSG from: ") + nodeName ;
    //String to = "MCUServerNWDevice";
    //mesh.sendSingle(to, msg);
    //Serial.println("Server is Running ....\n");
}); // start with a one second interval

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


void setup()
{
    Serial.begin(9600);
    Serial.println("\nHello from Server....!\n");
    connection_state = WiFiConnect();
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect

    //Gsender *gsender = Gsender::Instance();
    
    //InputInfo(); //inputs receiver's gmail id, subject and body of the email
    CustomInfo(); // uncomment this line if you want to send the email using the custom information which are already set in the code; and comment the above line.

  if (!nrf24.init()) 
  {
    Serial.println("init failed");
 
  }
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(3)) 
  {
    Serial.println("setChannel failed");
  }
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)) {
    Serial.println("setRF failed");
  }
  //mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
#if 0
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.setName(nodeName); // This needs to be an unique name!
#endif
//  mesh.onReceive([](uint32_t from, String &msg) {
//    Serial.printf("\nRcvd MSGid from: %u, %s\n", from, msg.c_str());
//  });
//
//  mesh.onReceive([](String &from, String &msg) {
//    Serial.printf("\nRcvd MSG by name from: %s, %s\n", from.c_str(), msg.c_str());
//  });t
  #if 0
  mesh.onReceive([](String &from, String &msg) {
    //Serial.println("%s\n", msg.c_str());
    if(strcmp(msg.c_str(), FIRST_MAIN_DOOR_STRING) == 0) {
      Serial.println("FIRST_MAIN_DOOR_STRING Triggered ");
      data[0] = DOOR_EVENT_1_MAIN;
    }
    if (strcmp(msg.c_str(), FIRST_NORTH_DOOR_STRING) == 0) {
      Serial.println("FIRST_NORTH_DOOR_STRING Triggered ");
      data[0] |= DOOR_EVENT_1_NORTH;
    }
    if (strcmp(msg.c_str(), SECOND_MAIN_DOOR_STRING) == 0) {
      Serial.println("SECOND_MAIN_DOOR_STRING Triggered ");
      data[0] |= DOOR_EVENT_2_SECOND;
    }
  });
  
  mesh.onChangedConnections([]() {
    Serial.printf("\nChanged connection\n");
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
#endif
}

void event_send() {
      Serial.println("Sending to gateway");
      nrf24.send(data, 2);
      nrf24.waitPacketSent();
      data[0] = 0;
      if (nrf24.waitAvailableTimeout(1000))
      {
          if (nrf24.recv(buf, &len))
          {
            Serial.print("got reply: ");
            Serial.println((char*)buf);
          }
          else
          {
            Serial.println("recv failed");
          }
      }
      else
      {
        Serial.println("No reply.");
      }
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

void loop() {
  #if 0
  mesh.update();
  #endif
  if (nrf24.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (nrf24.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
      int first = (int)buf[0];
      Serial.println(first);
#if 1
      if(first & DOOR_EVENT_1_MAIN)
        data[0] |= DOOR_EVENT_1_MAIN;
      if(first & DOOR_EVENT_1_NORTH)
        data[0] |= DOOR_EVENT_1_NORTH;
       if(first & DOOR_EVENT_2_SECOND)
        data[0] |= DOOR_EVENT_2_SECOND;
#endif
    }
    else
    {
     
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply.");
    
  }
  delay(2000);
 
  if (data[0] & DOOR_EVENT_1_MAIN)
  {
    event_send();
    data[0] &=~DOOR_EVENT_1_MAIN;
    door1_sent_num ++;
  }
    if (data[0] & DOOR_EVENT_1_NORTH)
  {
    event_send();
    data[0] &=~DOOR_EVENT_1_NORTH;
    door2_sent_num++;
  }
    if (data[0] & DOOR_EVENT_2_SECOND)
  {
    event_send();
    data[0] &=~DOOR_EVENT_2_SECOND;
    door3_sent_num++;
  }
  if (door1_sent_num > 10) {
    mail_send();
    door1_sent_num = 0;
  }
  if (door2_sent_num > 10) {
    mail_send();
    door2_sent_num = 0;
  }
  if(door3_sent_num > 10) {
    mail_send();
    door3_sent_num = 0;
  }
}

#endif
