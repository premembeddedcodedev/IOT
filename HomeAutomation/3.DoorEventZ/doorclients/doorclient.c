#include "namedMesh.h"

#define MESH_SSID       "MCUServerNWAP"
#define MESH_PASSWORD   "mcupasswd"
#define MESH_PORT       5555
#define FIRST_MAIN_DOOR_STRING "1ST-MAIN-DOOREVENT"
#define FIRST_NORTH_DOOR_STRING "1ST-NORTH-DOOREVENT"
#define SECOND_MAIN_DOOR_STRING "2ND-MAIN-DOOREVENT"

Scheduler userScheduler; // to control your personal task
namedMesh  mesh;

String nodeName = "2ndFloorMCU"; // Name needs to be unique

Task taskSendMessage( TASK_SECOND*2, TASK_FOREVER, []() {
    //String msg = String("MSG from: ") + nodeName;
    if (digitalRead(D5) == 1){
      //String msg = "1ST-MAIN-DOOREVENT";
      String msg = SECOND_MAIN_DOOR_STRING;
      String to = "MCUServerNWDev"; //nodename in the server
      mesh.sendSingle(to, msg);
    }
}); // start with a one second interval

void setup() {
  Serial.begin(9600); 
  pinMode(D5, INPUT_PULLUP);
  
  Serial.println("\n Hello from Client(SECOND_MAIN_DOOR_STRING)....\n");

  //mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.setName(nodeName); // This needs to be an unique name! 

//  mesh.onReceive([](uint32_t from, String &msg) {
//    Serial.printf("\n Rcvd MSGid from: %u, %s\n", from, msg.c_str());
//  });
//
//  mesh.onReceive([](String &from, String &msg) {
//    Serial.printf("\n Rcvd MSG by name from: %s, %s\n", from.c_str(), msg.c_str());
//  });
  //function prints the message sender (from) and the content of the message (msg.c_str()).
  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("%s", msg.c_str());
  });
  //function runs whenever a connection changes on the network (when a node joins or leaves the network).
  mesh.onChangedConnections([]() {
    Serial.printf("\n Changed connection\n");
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
//  if (digitalRead(D5) == 1){
//    Serial.println("Door is open");
//  }
//  delay(1000);
}
