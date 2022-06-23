#include "namedMesh.h"
#include <Arduino_JSON.h>
#include "painlessMesh.h"

#define MESH_SSID       "MCUServerNWAP"
#define MESH_PASSWORD   "mcupasswd"
#define MESH_PORT       5555

#define FIRST_MAIN_DOOR_EVENT 0x01
#define FIRST_MAIN_NORTH_DOOR_EVENT 0x02
#define SECOND_NORTH_DOOR_EVENT 0x04

#define D5 14

/* SensorBits Enablement */
#define TEMP_ENABLE 	1
#define LIGHT_ENABLE 	2
#define SPEAKER_ENABLE  3
#define MOVEMENT_ENABLE 4
#define PHOTOS_ENABLE   5
#define NOISE_ENABLE 	6
#define GAS_ENABLE	7
#define AIR_ENABLE	8
#define NODE_1		9
#define NODE_2		10
#define NODE_3		11

struct ClientData {
	uint8_t MsgID;
	int DoorClient;
	int DoorStatus;
	uint8_t SpeakerStatus;
	int Temparature;
	double Pressure;
	double Altitude;
	int Humidity;
	int LightStatus;
	int MovementStatus;
	int CapturePhotos;
	int NoiseStatus;
	int GasStatus;
	int AirQualityStatus;
};

struct ClientData DoorClient;
int SensorBits;
int SoundEnable;
int SpeakerStatus;
int TempSensorEnable;
int TempSensorStatus;
int HealthEnable;
int HealthStatus;
int LightEnable;
int LightStatus;
int MovementEnable;
int MovementStatus;
int CaptureEnable;
int CaptureStatus;
int NoiseEnable;
int NoiseStatus;
int GasEnable;
int GasStatus;
int AirQualityEnable;
int AirQualityStatus;
int NodeEnable;
int event = 0;

Scheduler userScheduler;
namedMesh  mesh;

String to = "MCUServerNWDev"; 

#ifdef DOOR_CLIENT
String NodeName = "1stFloorMCU";
int NodeNumber = 1;
#elif DOOR_CLIENT2
String NodeName = "1stFloorMCU_North";
int NodeNumber = 2;
#else
String NodeName = "2ndFloorMCU";
int NodeNumber = 3;
#endif

String readings;

void sendMessage(); 
String SendDoorConfigs(); 

Task taskSendMessage(TASK_SECOND * 2 , TASK_FOREVER, &sendMessage);

void newConnectionCallback(uint32_t nodeId) {
#ifdef DEBUG_LEVEL_2
	Serial.printf("New Connection, nodeId = %u\n", nodeId);
#endif
}

void changedConnectionCallback() {
#ifdef DEBUG_LEVEL_2
	Serial.printf("Changed connections\n");
#endif
}

void nodeTimeAdjustedCallback(int32_t offset) {
#ifdef DEBUG_LEVEL_2
	Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
#endif
}

String SendDoorConfigs()
{
	JSONVar jsonReadings;
	jsonReadings["Node"] = NodeNumber;
	jsonReadings["DoorStatus"] = event;

	if(TempSensorStatus == 1){ 
		jsonReadings["Temp"] = DoorClient.Temparature;
		jsonReadings["Hum"] = DoorClient.Humidity;
		jsonReadings["Pres"] = DoorClient.Pressure;
		jsonReadings["Alt"] = DoorClient.Altitude;
	}

	if(SpeakerStatus == 1)
		jsonReadings["SpeakerStatus"] = DoorClient.SpeakerStatus;

	if(LightStatus == 1)
		jsonReadings["LightStatus"] = DoorClient.LightStatus;

	if(MovementStatus == 1)
		jsonReadings["MovmentStatus"] = DoorClient.MovementStatus;

	if(CaptureStatus == 1)
		jsonReadings["CapturePhotos"] = DoorClient.CapturePhotos;

	if(NoiseStatus == 1)
		jsonReadings["NoiseStatus"] = DoorClient.NoiseStatus;

	if(GasStatus == 1)
		jsonReadings["GasStatus"] = DoorClient.GasStatus;

	if(AirQualityStatus == 1)
		jsonReadings["AirQualityStatus"] = DoorClient.AirQualityStatus;

	readings = JSON.stringify(jsonReadings);

	return readings;
}

String DoorgetReadings()
{
	JSONVar jsonReadings;

	jsonReadings["Node"] = NodeNumber;
	jsonReadings["DoorStatus"] = event;

	readings = JSON.stringify(jsonReadings);

	return readings;
}

void sendMessage() 
{
	if (digitalRead(D5) == 1){
#ifdef MYDEBUG
		Serial.println("Door is open");
#endif
#ifdef DOOR_CLIENT
		event = FIRST_MAIN_DOOR_EVENT;
#elif DOOR_CLIENT2
		event = FIRST_MAIN_NORTH_DOOR_EVENT;
#else
		event = SECOND_NORTH_DOOR_EVENT;
#endif
		String msg = DoorgetReadings();
		mesh.sendSingle(to, msg);
	} else {
		String msg = SendDoorConfigs();
		mesh.sendSingle(to, msg);
	}
}

void ExtractMeshNodeData(int SensorBits)
{
	if(SensorBits >> TEMP_ENABLE)
		TempSensorEnable = 1;
	if(SensorBits >> LIGHT_ENABLE)
		LightEnable = 1;
	if(SensorBits >> SPEAKER_ENABLE)
		SoundEnable = 1;
	if(SensorBits >> MOVEMENT_ENABLE)
		MovementEnable = 1;
	if(SensorBits >> PHOTOS_ENABLE)
		CaptureEnable = 1;
	if(SensorBits >> NOISE_ENABLE)
		NoiseEnable = 1;
	if(SensorBits >> GAS_ENABLE)
		GasEnable = 1;
	if(SensorBits >> AIR_ENABLE)
		AirQualityEnable = 1;
	if(SensorBits >> NODE_1)
		NodeEnable = 1;
	if(SensorBits >> NODE_2)
		NodeEnable = 2;
	if(SensorBits >> NODE_3)
		NodeEnable = 3;
}

void receivedCallback( uint32_t from, String &msg )
{
#ifdef DEBUG_LEVEL_2
	Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
#endif
	JSONVar myObject = JSON.parse(msg.c_str());
	int node = myObject["Node"];
	SensorBits = myObject["SensorBits"];

	ExtractMeshNodeData(SensorBits);

#ifdef MYDEBUG
	Serial.print("Node: ");
	Serial.println(node);
	Serial.print("SoundEnable: ");
	Serial.println(SoundEnable);
	Serial.print("TempSensorEnable");
	Serial.println(TempSensorEnable);
	Serial.print("LightEnable");
	Serial.print(LightEnable);
	Serial.print("MovementEnable");
	Serial.print(MovementEnable);
	Serial.print("PhotosEnable");
	Serial.print(CaptureEnable);
	Serial.print("NoiseEnable");
	Serial.print(NoiseEnable);
	Serial.print("GasEnable");
	Serial.print(GasEnable);
	Serial.print("AirQualityEnable");
	Serial.print(AirQualityEnable);
	Serial.print("NodeEnable");
	Serial.print(NodeEnable);
#endif

}

void mesh_config()
{
	mesh.setDebugMsgTypes(ERROR|STARTUP);
	mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
	mesh.setName(NodeName);
	mesh.onNewConnection(&newConnectionCallback);
	mesh.onReceive(&receivedCallback);
	mesh.onChangedConnections(&changedConnectionCallback);
	mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void DoorClientFillData()
{
	DoorClient.Temparature = random(20);
#ifdef MYDEBUG
	Serial.print("Temperature: ");
	Serial.print(DoorClient.Temparature);
	Serial.println(" C");
#endif
	DoorClient.Humidity = random(20);
#ifdef MYDEBUG
	Serial.print("Humidity: ");
	Serial.print(DoorClient.Humidity);
	Serial.println(" %");
#endif
	DoorClient.Pressure = random(20);
#ifdef MYDEBUG
	Serial.print("Pressure: ");
	Serial.print(DoorClient.Pressure);
	Serial.println(" hpa");
#endif
	DoorClient.Altitude = random(20);
#ifdef MYDEBUG
	Serial.print("Altitude: ");
	Serial.print(DoorClient.Altitude);
	Serial.println(" hpa");

	Serial.print("SpeakerStatus: ");
#endif
	DoorClient.SpeakerStatus = random(20);
#ifdef MYDEBUG
	Serial.println(DoorClient.SpeakerStatus);
	Serial.print("LightStatus: ");
#endif
	DoorClient.LightStatus = random(20);
#ifdef MYDEBUG
	Serial.println(DoorClient.LightStatus);
	Serial.print("MovmentStatus: ");
#endif
	DoorClient.MovementStatus = random(20);
#ifdef MYDEBUG
	Serial.println(DoorClient.MovementStatus);
	Serial.print("CapturePhotos: ");
#endif
	DoorClient.CapturePhotos = random(20);
#ifdef MYDEBUG
	Serial.println(DoorClient.CapturePhotos);
	Serial.print("NoiseStatus: ");
#endif
	DoorClient.NoiseStatus = random(20);
#ifdef MYDEBUG
	Serial.println(DoorClient.NoiseStatus);
	Serial.print("GasStatus: ");
#endif
	DoorClient.GasStatus = random(20);
#ifdef MYDEBUG
	Serial.println(DoorClient.GasStatus);
	Serial.print("AirQualityStatus: ");
#endif
	DoorClient.AirQualityStatus = random(20);
#ifdef MYDEBUG
	Serial.println(DoorClient.AirQualityStatus);
#endif
}

void setup()
{
	Serial.begin(9600); 
	pinMode(D5, INPUT_PULLUP);
	Serial.print(NodeName);
	Serial.println(" :: am Available - PV");
	mesh_config();
	Serial.println("Mesg Config done");
	userScheduler.addTask(taskSendMessage);
	taskSendMessage.enable();
}

void loop() {
	mesh.update();


	if(SoundEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Sound is disabled as per instructions.....");
#endif
		SoundEnable = 90; // 90 Represents Disable
		SpeakerStatus = 90; // 90 Represents Disable
	}
	if(SoundEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Sound is enabled as per instructions.....");
#endif
		SoundEnable = 0;
		SpeakerStatus = 1;
		sendMessage();
	}

	if(TempSensorEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send Temp Readings...");
#endif
		TempSensorEnable = 90; // 90 Represents Disable
		TempSensorStatus = 90; // 90 Represents Disable
	}

	if(TempSensorEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send Temp Readings...");
#endif
		TempSensorEnable = 0;
		TempSensorStatus = 1;
		sendMessage();
	}

	if(HealthEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send HealthStatus Readings...");
#endif
		HealthEnable = 90; // 90 Represents Disable
		HealthStatus = 90; // 90 Represents Disable
	}

	if(HealthEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send HealthStatus Readings...");
#endif
		HealthEnable = 0;
		HealthStatus = 1;
		sendMessage();
	}
	if(LightEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send LightStatus Readings...");
#endif
		LightEnable = 90; // 90 Represents Disable
		LightStatus = 90; // 90 Represents Disable
	}

	if(LightEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send LightStatus Readings...");
#endif
		LightEnable = 0;
		LightStatus = 1;
		sendMessage();
	}
	if(MovementEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send MovementStatus Readings...");
#endif
		MovementEnable = 90; // 90 Represents Disable
		MovementStatus = 90; // 90 Represents Disable
	}

	if(MovementEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send MovementStatus Readings...");
#endif
		MovementEnable = 0;
		MovementStatus = 1;
		sendMessage();
	}
	if(CaptureEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send CaptureStatus Readings...");
#endif
		CaptureEnable = 90; // 90 Represents Disable
		CaptureStatus = 90; // 90 Represents Disable
	}

	if(CaptureEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send CaptureStatus Readings...");
#endif
		CaptureEnable = 0;
		CaptureStatus = 1;
		sendMessage();
	}
	if(NoiseEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send NoiseStatus Readings...");
#endif
		NoiseEnable = 90; // 90 Represents Disable
		NoiseStatus = 90; // 90 Represents Disable
	}

	if(NoiseEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send NoiseStatus Readings...");
#endif
		NoiseEnable = 0;
		NoiseStatus = 1;
		sendMessage();
	}
	if(GasEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send GasStatus Readings...");
#endif
		GasEnable = 90; // 90 Represents Disable
		GasStatus = 90; // 90 Represents Disable
	}

	if(GasEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send GasStatus Readings...");
#endif
		GasEnable = 0;
		GasStatus = 1;
		sendMessage();
	}
	if(AirQualityEnable == 0) {
#ifdef MYDEBUG
		Serial.println("Disable to Send AQStatus Readings...");
#endif
		AirQualityEnable = 90; // 90 Represents Disable
		AirQualityStatus = 90; // 90 Represents Disable
	}

	if(AirQualityEnable == 1) {
#ifdef MYDEBUG
		Serial.println("Ready to Send AQStatus Readings...");
#endif
		AirQualityEnable = 0;
		AirQualityStatus = 1;
		sendMessage();
	}

	DoorClientFillData();

	delay(2000);
}
