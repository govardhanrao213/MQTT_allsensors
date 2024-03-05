#include <WiFi.h>// enables network connection(local and internet)
#include <PubSubClient.h>// a client library for mqtt messaging//128 bytes size
#include <ArduinoJson.h>// a simple and efficient Json library.it supports serialization,deserialization,messagepack
#include <DHT.h>//provides a simple and easy-to-use interface to read temperature and humidity data from a DHT11 sensor.
#include "MQ135.h"//o provide functions and utilities to easily interface with the MQ-135 sensor module, allowing developers to read data from the sensor and integrate it 
#define sensorPin 18 // to define digital pin for gas sensor
#define DHT11PIN  19 //to define data pin for humidity sensor
#define MOISTURE_SENSOR 5  // Analog pin for soil moisture sensor
#define TRIGGER_PIN 12   // Pin for ultrasonic sensor trigger// used to trigger ultrasonic sound pulses
#define ECHO_PIN 13      // Pin for ultrasonic sensor echo// produces a pulse when the reflected signal is received
//DHT dht(DHTPIN, DHTTYPE);
DHT dht(DHT11PIN, DHT11);//DHT:data type representing the DHT sensor module//DHT11:it indicates that a DHT11 sensor is being used
//float humidity;
int soilMoisture;
int distance;

const char* ssid = "realme 6i";//service set identifier and it is wifi network name
const char* password = "98767890";//wifi network password
const char* mqtt_server = "broker.emqx.io";//MQTT broker address
//const char *broker = "broker.emqx.io"; // MQTT Broker address
//const char *topic = "sensor_data"; // MQTT topic to publish sensor data
const char *ClientID = "mqttx_d445d52a";//MQTT Client id 
const char *Username = "humi";//MQTT USERNAME
const char *Password = "public1";//MQTT Password
//variable named 'espClient' of type 'WiFiClient'
WiFiClient espClient;//it can be used to create connections, send data, and receive data from remote servers over the network.
PubSubClient client(espClient);//PubSubClient library, which facilitates MQTT communication in Arduino sketches.
// is to initialize an MQTT client object named client, using the espClient object for network communication
void setup_wifi()//begins by initializing the serial communication for debugging purposes using Serial.begin().
{
  delay(10);//10msec delay to print  below  wifi name and connect the wifi
  Serial.println();//print newline 
  Serial.print("Connecting to ");
  Serial.println(ssid);// print wifi name

  WiFi.begin(ssid, password);// used to initializes the WiFi library's network settings and provides the current status.

  while (WiFi.status() != WL_CONNECTED)//WL_CONNECTED:when connected to a WiFi network//WiFi.status():Return the connection status
  {
    delay(500);//produce 500 millisec
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");// wifi ip address
  Serial.println(WiFi.localIP());// wifi ip address
}
void callback(char* topic, byte* message, unsigned int length)// it is used to receive the subcribe message 
{
  Serial.print("Message received on topic: ");
  Serial.print(topic);// topic: a filter the broker uses in MQTT message deliveries or a matching mechanism between publishers and subscribers
  Serial.print(". Message: ");
  String messageTemp;//declare data type string and variable name is messageTemp

  for (int i = 0; i < length; i++)
  //iterates through each character in the message array, which represents the payload of the incoming MQTT message.
  {
    messageTemp += (char)message[i];//char message[i] is to be stored in messageTemp 
  }
  Serial.println(messageTemp);// print the subscribe message
}

void reconnect()// it is used to reconnect the MQTT connection in case it is connection failed.reconnection function help to connect MQTT. 
{
  while (!client.connected())//while loop runs if not equal to connect client(MQTT)
  {
    Serial.print("Attempting MQTT connection...");//to print Attempting MQTT connection...
    if (client.connect("mqttx_931d9cf0"))//connected to the respective client id
    {
      Serial.println("connected");// to print connected
      client.subscribe("esp32/test1");//is used to subscribe to a particular topic on a messaging protocol such as MQTT
    } 
    //if broker is not connected to clientid goes to else 
    else 
    {
      Serial.print("failed, rc=");//to print failed,reconnect
      Serial.print(client.state());//returns the current state of the client
      Serial.println(" try again in 5 seconds");
      delay(5000);//to produce 5 sec delay
    }
  }
}
void setup() //it to initialize variables, pin modes, start using libraries, etc. The setup() function will only run once, after each powerup or reset of the Arduino board.
{
  Serial.begin(115200);//initializes serial communication at a baud rate 
  setup_wifi();//sets up the Wi-Fi connection on an ESP32
  client.setServer(mqtt_server, 1883);//mqtt server address and port 1883
  client.setCallback(callback);//handling incoming messages in MQTT communication

   dht.begin();//initializes the DHT sensor library and prepares the sensor for reading 
  pinMode(MOISTURE_SENSOR, INPUT);//Configures the MOISTURE_SENSOR pin as an input
  pinMode(TRIGGER_PIN, OUTPUT);//Configures the TRIGGER_PIN as an output
  pinMode(ECHO_PIN, INPUT);//Configures the ECHO_PIN as an input
  pinMode(sensorPin, INPUT);//Configures the sensorPin as an input
}

void loop() {
  if (!client.connected())  // Check if MQTT client is not connected
  {
    reconnect();// Attempt to reconnect
  }
  long duration, distance;//Declares two variables duration and distance of type long. 
  int sensorValue;//Declares a variable sensorValue of type int
  float humidity = dht.readHumidity();//returns the humidity value (in percentage) measured by the DHT sensor.and stored in a variable humidity of type float 
  float temperature = dht.readTemperature();//Declares a variable temperature of type float and initializes it with the temperature reading using the readTemperature() function. 
  
  // Read soil moisture
  soilMoisture = digitalRead(MOISTURE_SENSOR);
  //soilMoisture = ( 100 - ( (soilMoisture/4095.00) * 100 ) );

  // Read distance
  //distance = getDistance();
 // long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);//sets the TRIGGER_PIN to a LOW state.
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);//The TRIGGER_PIN is set to a HIGH state, which triggers the sensor to send out an ultrasonic pulse.
  delayMicroseconds(10);//introduces a short delay of 10 microseconds 
  digitalWrite(TRIGGER_PIN, LOW);//The TRIGGER_PIN is set back to a LOW state to stop the transmission of the ultrasonic pulse
  duration = pulseIn(ECHO_PIN, HIGH);//measures the duration (in microseconds) for which the ECHO_PIN (which receives the reflected pulse) remains in a HIGH state. 
  distance = duration * 0.034 / 2;//The distance is calculated based on the duration of the pulse.//he distance is calculated as half of the duration multiplied by the speed of sound
 //measure gas
//int sensorValue;
  sensorValue = digitalRead(sensorPin);       // read digital input pin
 // Create JSON object
  StaticJsonDocument<200> jsonDoc;//StaticJsonDocument to store in the stack.staticJsonDocument to recommed for documents smaller than 1KB
  jsonDoc["humidity"] = humidity;//to store humidity values in jsondoc
  jsonDoc["temperature"] = temperature;//to store temperature values in jsondoc
  jsonDoc["moisture"] = soilMoisture;//to store moisture values in jsondoc
  jsonDoc["distance"] = distance;//to store distance values in jsondoc
  
  jsonDoc["gas "] = sensorValue;//to store gas value values in jsondoc
  // Serialize JSON to string
  String jsonString;// to declare the variable as string data type
  serializeJson(jsonDoc, jsonString);//allows you to insert preformatted chunks of JSON (or MessagePack) in a JsonObject or a JsonArray 

  // Print JSON string
  Serial.println(jsonString);

  delay(1000); // Adjust delay according to your needs//1000millisec=1sec
  client.loop();// If MQTT client is connected, continue with other tasks
  // For example, publishing sensor data or subscribing to topics
  client.publish("esp32/test", jsonString.c_str());//connect to a broker, and messages are published to topics. 
  delay(1000);//to produce 5sec delay
}
