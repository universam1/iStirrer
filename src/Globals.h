#define REMOTE 0
#define POTI 1
#define POTITRIGGER 100
#define POTICHKINTERVAL 50


#define TOKEN "razefumml"   // Your Ubidots TOKEN
#define WIFINAME "meiWaifai"                       // Your SSID
#define WIFIPASS "subbersicret"                    // Your Wifi Pass
#define MQTTCLIENTNAME "iStirrer01" 
// Your MQTT Client Name, it must be unique so we recommend to
// choose a random ASCCI name
#define MQTTSPEEDID "speed"
#define MQTTTHROTTLEID "set"
#define MQTTPUBLISHNAME MQTTCLIENTNAME

int Pin = BUILTIN_LED; // LED connected to digital pin 9
int Freq = D1;
#define INTERVALL 500 * 1000
#define TRIGGER 4
#define UPLOADINTV 60e3

#define ANALOGRANGE 40
#define ANALOGFREQ 20e3
#define STARTPOWER 80
