#include <Arduino_JSON.h>
#include <RH_NRF24.h>
#include "painlessMesh.h"
#include "namedMesh.h"
#include <EEPROM.h>

#define MESH_SSID       "MCUServerNWAP"
#define MESH_PASSWORD   "mcupasswd"
#define MESH_PORT       5555

#define DOOR_EVENT_1_MAIN 0x01
#define DOOR_EVENT_1_NORTH 0x02
#define DOOR_EVENT_2_SECOND 0x04

int nodeNumber;
String readings;
String nodeName = "MCUServerNWDev"; // Name needs to be unique

String to1 = "1stFloorMCU"; //nodename in the server
String to2 = "2ndFloorMCU"; //nodename in the server
String to1_N = "1stFloorMCU_North"; //nodename in the server

int deviceID = EEPROM.read(0);
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

namedMesh mesh;
RH_NRF24 nrf24(2, 4); // use this for NodeMCU Amica/AdaFruit Huzzah ESP8266 Feather
Scheduler userScheduler; // to control your personal task

#define NUMCLIENTS 3
#define DOOR_EVENT_2 0x03

/* SensorBits Enablement */
#define TEMP_ENABLE (1<<1)
#define LIGHT_ENABLE (1<<2)
#define SPEAKER_ENABLE (1<<3)
#define MOVEMENT_ENABLE (1<<4)
#define PHOTOS_ENABLE (1<<5)
#define NOISE_ENABLE (1<<6)
#define GAS_ENABLE (1<<7)
#define AIR_ENABLE (1<<8)
#define NODE_1	(1<<9)
#define NODE_2	(1<<10)
#define NODE_3	(1<<11)

struct message {
	/* Data get it from DoorClient */
	int SensorBits;
	int DoorClient;
	int DoorStatus;
	int SpeakerStatus;
	int Temparature;
	double Pressure;
	double Altitude;
	int Humidity;
	int LightStatus;
	int MovmentStatus;
	int CapturePhotos;
	int NoiseStatus;
	int GasStatus;
	int AirQualityStatus;

	/* Data get it from Email Node */
	uint8_t NodeNumber;
	uint8_t SpeakerEnable;
	uint8_t TempSensorEnable;
	uint8_t LightSensorEnable;
	uint8_t MovementSensorEnable;
	uint8_t PhotosEnable;
	uint8_t NoiseSensorEnable;
	uint8_t GasSensorEnable;
	uint8_t AirQualitySensorEnable;
};

struct message ClientData;
struct message Clients[NUMCLIENTS];

uint8_t len = sizeof(struct message);
void KeepAlive() ; 
String getReadings();

Task taskSendMessage(TASK_SECOND * 300, TASK_FOREVER, &KeepAlive);

String EnableConfigDataForDoor(int NodeNumber)
{
	JSONVar jsonReadings;
	jsonReadings["Node"] = NodeNumber;
	jsonReadings["SensorBits"] = ClientData.SensorBits;
	readings = JSON.stringify(jsonReadings);
	return readings;
}

String DoorCLientHealthStatus()
{
	JSONVar jsonReadings;
	jsonReadings["DoorClientHelathStatus"] = 1;
	readings = JSON.stringify(jsonReadings);
	return readings;
}

String &getClient(int NodeNumber)
{
	if(NodeNumber == DOOR_EVENT_1_MAIN)
		return to1;
	
	if(NodeNumber == DOOR_EVENT_1_NORTH)
		return to1_N;

	if(NodeNumber == DOOR_EVENT_2_SECOND)
		return to2;
}

void sendMessage(int NodeNumber)
{
	String msg = EnableConfigDataForDoor(NodeNumber);
	mesh.sendSingle(getClient(NodeNumber), msg);
}

void KeepAlive() {
	String msg = DoorCLientHealthStatus();
	mesh.sendBroadcast(msg);
}

void receivedCallback( uint32_t from, String &msg )
{
#ifdef DEBUG_LEVEL_2
	Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
#endif
	
	Serial.println("======== Received Data...==========");

	JSONVar myObject = JSON.parse(msg.c_str());
	//memset(&ClientData, 0, sizeof(struct message));
	ClientData.DoorClient = myObject["Node"];;
	ClientData.DoorStatus = myObject["DoorStatus"];
	
	if(ClientData.TempSensorEnable == 1) {
		ClientData.Temparature = myObject["Temp"];
		ClientData.Humidity = myObject["Hum"];
		ClientData.Pressure = myObject["Pres"];
		ClientData.Altitude = myObject["Alt"];
	}
	
	if(ClientData.SpeakerEnable == 1)
		ClientData.SpeakerStatus = myObject["SpeakerStatus"];

	if(ClientData.LightSensorEnable == 1) 
		ClientData.LightStatus = myObject["LightStatus"];

	if(ClientData.MovementSensorEnable == 1)
		ClientData.MovmentStatus = myObject["MovmentStatus"];
	
	if(ClientData.PhotosEnable == 1)
		ClientData.CapturePhotos = myObject["CapturePhotos"];
	
	if(ClientData.NoiseSensorEnable == 1)
		ClientData.NoiseStatus = myObject["NoiseStatus"];

	if(ClientData.GasSensorEnable == 1)
		ClientData.GasStatus = myObject["GasStatus"];
	
	if(ClientData.AirQualitySensorEnable == 1)
		ClientData.AirQualityStatus = myObject["AirQualityStatus"];

#ifdef MYDEBUG
	Serial.print("Node: ");
	Serial.println(ClientData.DoorClient);
	Serial.print("Client Event: ");
	Serial.println(ClientData.DoorStatus);
	Serial.print("Temperature: ");
	Serial.print(ClientData.Temparature);
	Serial.println(" C");
	Serial.print("Humidity: ");
	Serial.print(ClientData.Humidity);
	Serial.println(" %");
	Serial.print("Pressure: ");
	Serial.print(ClientData.Pressure);
	Serial.println(" hpa");
	Serial.print("Altitude: ");
	Serial.print(ClientData.Altitude);
	Serial.println(" hpa");
	Serial.print("SpeakerStatus : ");
	Serial.println(ClientData.SpeakerStatus);
	Serial.print("LightStatus : ");
	Serial.println(ClientData.LightStatus);
	Serial.print("MovmentStatus : ");
	Serial.println(ClientData.MovmentStatus);
	Serial.print("CapturePhotos : ");
	Serial.println(ClientData.CapturePhotos);
	Serial.print("NoiseStatus : ");
	Serial.println(ClientData.NoiseStatus);
	Serial.print("GasStatus : ");
	Serial.println(ClientData.GasStatus);
	Serial.print("AirQualityStatus : ");
	Serial.println(ClientData.AirQualityStatus);
#endif

	if(ClientData.DoorClient == DOOR_EVENT_1_MAIN)
		memcpy(&Clients[DOOR_EVENT_1_MAIN], &ClientData, sizeof(struct message));
	if(ClientData.DoorClient == DOOR_EVENT_1_NORTH)
		memcpy(&Clients[DOOR_EVENT_1_NORTH], &ClientData, sizeof(struct message));
	if(ClientData.DoorClient == DOOR_EVENT_2)
		memcpy(&Clients[DOOR_EVENT_2], &ClientData, sizeof(struct message));
}

void send_to_master()
{
	nrf24.send((uint8_t *)&ClientData, sizeof(struct message));
	nrf24.waitPacketSent();
}

void Ciritical_Door_event()
{
#ifdef DEBUG_LEVEL
	Serial.println("Sending to gateway");
#endif
	nrf24.send((uint8_t *)&ClientData.DoorStatus, 4);
	nrf24.waitPacketSent();
}

void debug_prints()
{
#ifdef DEBUG_LEVEL
	Serial.print("MSGID: ");
	Serial.println(ClientData. MsgID);
	Serial.print("DoorClient: ");
	Serial.println(ClientData.DoorClient);
	Serial.print("DoorStatus: ");
	Serial.println(ClientData.DoorStatus);
	Serial.print("SpeakerStatus: ");
	Serial.println(ClientData.SpeakerStatus);
	Serial.print("Temparature: ");
	Serial.println(ClientData.Temparature);
	Serial.print("Pressure: ");
	Serial.print(ClientData.Pressure);
	Serial.print("Altitude: ");
	Serial.print(ClientData.Altitude);
	Serial.print("Humidity: ");
	Serial.println(ClientData.Humidity);
#endif
}

int GetNodeNumber(int Data)
{
	if ((Data >> 9) & 0x1)
		ClientData.NodeNumber = DOOR_EVENT_1_MAIN;	
	if ((Data >> 10) & 0x1)
		ClientData.NodeNumber = DOOR_EVENT_1_NORTH;	
	if ((Data >> 11) & 0x1)
		ClientData.NodeNumber = DOOR_EVENT_2_SECOND;	

	return ClientData.NodeNumber;
}

void ExtractEmailNodeData()
{
	if(buf[0] & TEMP_ENABLE)
		ClientData.SensorBits |= ClientData.TempSensorEnable;
	if(buf[0] & LIGHT_ENABLE)
		ClientData.SensorBits |= ClientData.LightSensorEnable;
	if(buf[0] & SPEAKER_ENABLE)
		ClientData.SensorBits |= ClientData.SpeakerEnable;
	if(buf[0] & MOVEMENT_ENABLE)
		ClientData.SensorBits |= ClientData.MovementSensorEnable;
	if(buf[0] & PHOTOS_ENABLE)
		ClientData.SensorBits |= ClientData.PhotosEnable;
	if(buf[0] & NOISE_ENABLE)
		ClientData.SensorBits |= ClientData.NoiseSensorEnable;
	if(buf[0] & GAS_ENABLE)
		ClientData.SensorBits |= ClientData.GasSensorEnable;
	if(buf[0] & AIR_ENABLE)
		ClientData.SensorBits |= ClientData.AirQualitySensorEnable;

	sendMessage(GetNodeNumber(ClientData.SensorBits));
}

void receive_data_from_mesh()
{
	uint8_t i = 0;
	if (nrf24.waitAvailableTimeout(1000)){  
		if (nrf24.recv(buf, &len)){
#ifdef DEBUG
			Serial.print("got message: ");
			Serial.println(len);
#endif
			ExtractEmailNodeData();
#ifdef DEBUG
			memcpy((uint8_t *)&ClientData, buf, sizeof(struct message));
			debug_prints();
			send_to_master();
#endif
#ifdef DEBUG
			i = 0;
			while(i<len) {
				Serial.print(buf[i]);
				Serial.print(" ");
				i++;
			}
#endif
			Serial.println();
		}
		else
		{
			Serial.println("recv failed");
		}

	}
	else
	{
#ifdef DEBUG_LEVEL_2
		Serial.println("No message, is nrf24_server running?");
#endif
	}

}

/* auto nodes = mesh.getNodeList(true);
String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
if(mesh.isConnected(target))
*/

void loop() 
{
	mesh.update();
	if (ClientData.DoorStatus & DOOR_EVENT_1_MAIN)
	{
		Ciritical_Door_event();
		ClientData.DoorStatus &=~DOOR_EVENT_1_MAIN;
	}
	if (ClientData.DoorStatus & DOOR_EVENT_1_NORTH)
	{
		Ciritical_Door_event();
		ClientData.DoorStatus &=~DOOR_EVENT_1_NORTH;
	}
	if (ClientData.DoorStatus & DOOR_EVENT_2_SECOND)
	{
		Ciritical_Door_event();
		ClientData.DoorStatus &=~DOOR_EVENT_2_SECOND;
	}
	receive_data_from_mesh();
}

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

void mesh_config()
{
	mesh.setDebugMsgTypes( ERROR | STARTUP ); 
	mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
	mesh.setName(nodeName);
	mesh.onReceive(&receivedCallback);
	mesh.onNewConnection(&newConnectionCallback);
	mesh.onChangedConnections(&changedConnectionCallback);
	mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void nrf_config()
{
	if (!nrf24.init()) 
		Serial.println("init failed");

	if (!nrf24.setChannel(3)) 
		Serial.println("setChannel failed");

	if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
		Serial.println("setRF failed");
}

void setup()
{
	Serial.begin(9600);
	Serial.print("Mesh Trasceiver Started, ID: ");
	Serial.println(deviceID);
	while (!Serial); 
	nrf_config();
	Serial.println("NRF config Completed");
	mesh_config();
	Serial.println("Mesh congiuration completed");
	userScheduler.addTask(taskSendMessage);
	taskSendMessage.enable();
}
