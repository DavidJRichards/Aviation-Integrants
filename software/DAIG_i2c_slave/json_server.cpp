/****************************************************************************************************************************
  AsyncTCPServer.ino

  For RP2040W with CYW43439 WiFi

  AsyncTCP_RP2040W is a library for the RP2040W with CYW43439 WiFi

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncTCP_RP2040W
  Licensed under GPLv3 license
 *****************************************************************************************************************************/

#include <AsyncTCP_RP2040W.h>
// credentials hidden from github
// includes MySSID and MyPASSWORD macros
#include "Wifi.h"
#include <vector>
#include <ArduinoJson.h>

extern float heading,ntos;
extern void heading2res(float);
extern void ntos2res(float);
extern bool i2c_override;

//const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
//DynamicJsonDocument doc(capacity);
StaticJsonDocument<200> doc;

bool new_data = false;
bool led_flash = false;

#define TCP_PORT          2055

static std::vector<AsyncClient*> clients; // a list to hold all clients

// uses macros included from Wifi.h
const char* ssid = MySSID; //Enter SSID
const char* pass = MyPASSWORD; //Enter Password

int status = WL_IDLE_STATUS;

// static IP of this server - must add to HyperIMU application on phone
// should to be on your local subnet
IPAddress serverIP(192, 168, 178, 112);

/* clients events */
static void handleError(void* arg, AsyncClient* client, int8_t error)
{
  (void) arg;

  Serial.printf("\nConnection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

#define REPLY_SIZE      64

static void handleData(void* arg, AsyncClient* client, void *data, size_t len)
{
//  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
//  DynamicJsonDocument doc(capacity);
  (void) arg;
  // sample json string used when testing decoder
  // note: unable to decode timestamp due to excessive size
  // char msg[256]="{\"os\":\"hyperimu\",\"Timestamp\":1691790229692,\"lsm6ds3c accelerometer\":[0.24556341767311096,-0.2919243574142456,9.816255569458008]}";

#if 0
  Serial.printf("\nData received from client %s \n", client->remoteIP().toString().c_str());
  Serial.write((uint8_t*)data, len);
#endif
  // Parse JSON object
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
//    client.stop();
    return;
  }
  new_data = true;
#if 0
  // Extract values
  Serial.println(F("\nResponse:"));
  Serial.println(doc["os"].as<const char*>());
//  Serial.println(doc["Timestamp"].as<long>());
#if oneplus
  Serial.println(doc["lsm6ds3c accelerometer"][0].as<float>(), 3);
  Serial.println(doc["lsm6ds3c accelerometer"][1].as<float>(), 3);
  Serial.println(doc["lsm6ds3c accelerometer"][2].as<float>(), 3);
#else  
  Serial.println(doc["accelerometer lsm6ds3-c"][0].as<float>(), 3);
  Serial.println(doc["accelerometer lsm6ds3-c"][1].as<float>(), 3);
  Serial.println(doc["accelerometer lsm6ds3-c"][2].as<float>(), 3);
#endif  
#endif

#if 0
  // reply to client
  if (client->space() > REPLY_SIZE && client->canSend())
  {
    char reply[REPLY_SIZE];
    sprintf(reply, "You've connected to AsyncTCPServer @ %s", serverIP.toString().c_str());
    client->add(reply, strlen(reply));
    client->send();
  }
#endif  
}

static void handleDisconnect(void* arg, AsyncClient* client)
{
  (void) arg;

  Serial.printf("\nClient %s disconnected\n", client->remoteIP().toString().c_str());
  i2c_override = false;
}

static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time)
{
  (void) arg;
  (void) time;

  Serial.printf("\nClient ACK timeout ip: %s\n", client->remoteIP().toString().c_str());
}


/* server events */
static void handleNewClient(void* arg, AsyncClient* client)
{
  (void) arg;

  Serial.printf("\nNew client has been connected to server, IP: %s", client->remoteIP().toString().c_str());

  // add to list
  clients.push_back(client);

  // register events
  client->onData(&handleData, NULL);
  client->onError(&handleError, NULL);
  client->onDisconnect(&handleDisconnect, NULL);
  client->onTimeout(&handleTimeOut, NULL);
  i2c_override = true;

}

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  serverIP = WiFi.localIP();
  Serial.print("Local IP Address: ");
  Serial.println(serverIP);
}

void json_server_setup()
{
  Serial.print("\nStart AsyncTCP_Server on ");
  Serial.print(BOARD_NAME);
  Serial.print(" with ");
  Serial.println(SHIELD_TYPE);
  Serial.println(ASYNCTCP_RP2040W_VERSION);

  ///////////////////////////////////

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");

    // don't continue
    while (true);
  }

  Serial.print(F("Connecting to SSID: "));
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);

  delay(1000);

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED)
  {
    led_flash = ! led_flash;
    digitalWrite(LED_BUILTIN, led_flash);

    delay(500);
  
    // Connect to WPA/WPA2 network
    status = WiFi.status();
}

  printWifiStatus();

  ///////////////////////////////////

  AsyncServer* server = new AsyncServer(TCP_PORT); // start listening on tcp port 7050

  server->onClient(&handleNewClient, server);
  server->begin();

  Serial.print(F("AsyncTCPServer is @ IP: "));
  Serial.print(serverIP);
  Serial.print(F(", port: "));
  Serial.println(TCP_PORT);
}


void json_server_loop()
{
  if(new_data)
  {
    //#ifdef USE_PWM
// note: sensor names seen here depend on phone    
#ifdef oneplus    
    heading = 9.0 * doc["lsm6ds3c accelerometer"][0].as<float>();
    ntos = 9.0 * doc["lsm6ds3c accelerometer"][1].as<float>();
#else
    heading = 9.0 * doc["accelerometer lsm6ds3-c"][0].as<float>();
    ntos = 9.0 * doc["accelerometer lsm6ds3-c"][1].as<float>();
#endif    
    heading2res(0);
    ntos2res(0);
    Serial.print(F("heading:"));
    Serial.print(heading, 1);
    Serial.print(F(", ntos:"));
    Serial.println(ntos, 1);
    //#endif
#if 0    
//    Serial.print(F("Min:-10, Max:10, Roll:"));
    Serial.print(F("Roll:"));
    Serial.print(doc["lsm6ds3c accelerometer"][0].as<float>(), 3);
    Serial.print(F(", Pitch:"));
    Serial.print(doc["lsm6ds3c accelerometer"][1].as<float>(), 3);
  //  Serial.print(F(", "));
  //  Serial.println(doc["lsm6ds3c accelerometer"][2].as<float>(), 3);
    Serial.println();
#endif    
    new_data = false;

  }
}
