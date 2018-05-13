/******
 Author :  Arunav Mallik Avi, 
 Department of Computer Science & Engineering, National University, Bagladesh.
 
 Description : Monitor and generates the Home weather activity and convert them into JSON data and send this JSON Data to the NodeMCU(ESP-8266) IoT Module.Mercury Droid 
 Android mobile application is used to read this JSON Data from the NodeMCU(ESP-8266) Server. This code also helpful for NodeMcu to connect 
 any wifi network without Hard coded Wifi Router or WifiHotspot "SSID name" and "Password", it fully suport the AutoConnectAp Services. it is easy to configure from any Wifi Capable devices 

 Fetures : 
 (1) Don't need to give Hard Coded Wifi Router SSID and Password. it Configure it self from "192.168.4.1" webserver basis of user given wifi router SSID name & Password. so it is fully Support
 Runtime Configuration
 
 (2) Monitor home weather activity. such as Celsius, Kelvin, Heat-Index, Humidity

 (3) Convert DHT11 Temperature sensor data in JSON format

 (4) Reduced Speed for DHT11 Sensor Data Calculation 


*******/

#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson
#include "DHT.h"

#define DHTPIN D5
#define DHTTYPE DHT11 

IPAddress ip(192, 168, 0, 107); //set static ip
IPAddress gateway(192, 168, 0, 1); //set getteway
IPAddress subnet(255, 255, 255, 0);//set subnet

//For Measuring Heat Index
#define c1 (-42.379)
#define c2 (2.04901523)
#define c3 (10.14333127)
#define c4 (-0.22475541)
#define c5 (-0.00683783)
#define c6 (-0.05481717)
#define c7 (0.00122874)
#define c8 (0.00085282)
#define c9 (-0.00000199)


DHT dht(DHTPIN, DHTTYPE);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputState = "off";

// Assign output variables to GPIO pins
char output[2] = "5";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(output, json["output"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  
  WiFiManagerParameter custom_output("output", "output", output, 2);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //Static ip Config
  wifiManager.setSTAStaticIPConfig(IPAddress(192,168,0,107), IPAddress(192,168,0,1), IPAddress(255,255,255,0));
  
  // set custom ip for portal
  //  wifiManager.setAPConfig(ip, gateway, subnet);
 

  //add all your parameters here
  wifiManager.addParameter(&custom_output);
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  
  strcpy(output, custom_output.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["output"] = output;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  // Initialize the output variables as outputs
  pinMode(atoi(output), OUTPUT);
  // Set outputs to LOW
  digitalWrite(atoi(output), LOW);;
  
  server.begin();
}

void loop(){
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  
// here 
// h = Humidity
// t = celsius
// k = kelvin
// f = fahrenheit
  
  // Read the first line of the request
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Conversion of Celsius to Kelvin
  float k = t + 273 ; 
  // Conversion of Celsius to fahrenheit
  float f = (t*9)/5 + 32;
  //Heat Index
  float heatIndex, heatIndexCelsius;
  heatIndex = c1+c2*(f)+c3*(h)+c4*(f)*(h)+c5*(pow(f,2))+c6*(pow(h,2))+c7*(pow(f, 2))*(h)+c8*(f)*(pow(h, 2))+c9*(pow(f, 2))*(pow(h, 2)); 
  heatIndexCelsius = ((((heatIndex)-32)*5)/9);
 
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  if (req.indexOf("/data") != -1){
    client.flush();
    client.println("HTTP/1.1 200 OK");           // This tells the browser that the request to provide data was accepted
    client.println("Access-Control-Allow-Origin: *");  //Tells the browser it has accepted its request for data from a different domain (origin).
    client.println("Content-Type: application/json;charset=utf-8");  //Lets the browser know that the data will be in a JSON format
    client.println("Server: Arduino");           // The data is coming from an Arduino Web Server (this line can be omitted)
    client.println("Connection: close");         // Will close the connection at the end of data transmission.
    client.println();                            // You need to include this blank line - it tells the browser that it has reached the end of the Server reponse header.
                               // This is tha starting bracket of the JSON data
    client.print("{\"temperature\": \"");
    client.print(t,0);  
    client.print("\", \"kelvin\": \""); 
    client.print(k,0);    
    client.print("\", \"fahrenheit\": \"");
    client.print(f,0);     
    client.print("\", \"heatindex\": \"");
    client.print(heatIndexCelsius,0);                
    client.print("\", \"Humidity\": \"");
    client.print(h,0);               
    client.print("\"}");                      
                      
  }
  else {
    client.flush();
    client.println("HTTP/1.1 200 OK");           // This tells the browser that the request to provide data was accepted
    client.println("Access-Control-Allow-Origin: *");  //Tells the browser it has accepted its request for data from a different domain (origin).
    client.println("Content-Type: application/json;charset=utf-8");  //Lets the browser know that the data will be in a JSON format
    client.println("Server: Arduino");           // The data is coming from an Arduino Web Server (this line can be omitted)
    client.println("Connection: close");         // Will close the connection at the end of data transmission.
    client.println();                            // You need to include this blank line - it tells the browser that it has reached the end of the Server reponse header.
                          // This is tha starting bracket of the JSON data
    client.print("{\"Response\": ");
    client.print("Invalid");                          
    client.print("}");                     
                       // This is the final bracket of the JSON data
  }
 
    delay(1);
    Serial.println("Client disconnected.");
    Serial.println("");
  }


