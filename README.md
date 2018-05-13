# IoT Home weather monitoring system NodeMCU(ESP-8266MOD) Code

<b> Author :</b> <br> Arunav Mallik Avi, <br>
 Department of Computer Science & Engineering, National University, Bagladesh.
 
 <b> Description :</b><br> Monitor and generates the Home weather activity and convert them into JSON data and send this JSON Data to the NodeMCU(ESP-8266) IoT Module.Mercury Droid 
 Android mobile application is used to read this JSON Data from the NodeMCU(ESP-8266) Server. This code is also helpful for NodeMcu to connect 
 any wifi network without Hard coded Wifi Router or WifiHotspot "SSID name" and "Password", it fully support the AutoConnectAp Services. it is easy to configure from any Wifi Capable devices 
 <br>
<b>Instruments/Hardware : </b><br>
 (1) NodeMCU(ESP-8266MOD) IoT wifi Module
 (2) DHT-11(Temperature & Humidity Sensor)
 (3) An Android devices
 (4) A single cell power source
 
<b> Fetures : </b> <br> 
 (1) Don't need to give Hard Coded Wifi Router SSID and Password. it Configure it self from "192.168.4.1" webserver basis of user given wifi router SSID name & Password. so it is fully Support
 Runtime Configuration
 
 (2) Monitor home weather activity. such as Celsius, Kelvin, Heat-Index, Humidity

 (3) Convert DHT11 Temperature sensor data in JSON format

 (4) Reduced Speed for DHT11 Sensor Data Calculation 
