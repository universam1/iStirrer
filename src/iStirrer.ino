#include "Globals.h"
#include "UbidotsESPMQTT.h"

uint32_t lastcheck, lastupload;
uint32_t lastupdRemote, lastupdPoti ;
uint32_t lastPotichck;

long previousMillis = 0;             // will store last time of the cycle end
volatile unsigned long duration = 0; // accumulates pulse width
volatile unsigned int pulsecount = 0;
volatile unsigned long previousMicros = 0;

int16_t rpm, set, lastADC;

Ubidots client(TOKEN, MQTTCLIENTNAME);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // for (int i=0;i<length;i++) {
  String s = String((char *)payload);
  uint16_t v = s.toInt();
  Serial.print(v);
  setSpeed(v, REMOTE);
  //}

  Serial.println();
}

uint8_t getLastActiveSource() {
  return lastupdRemote > lastupdPoti ? REMOTE : POTI;
  // if (lastupdRemote > lastupdPoti) return REMOTE;
  // else return POTI;
}

void subscribe() {
  client.ubidotsSubscribe(MQTTPUBLISHNAME, MQTTTHROTTLEID); // Insert the dataSource and Variable's Labels
}

void setSpeed(uint16_t spd, uint8_t source) { 
  if (source == REMOTE) lastupdRemote = millis();
  else lastupdPoti = millis();

  Serial.println(String("Src:")+source+String(" Spd:")+spd);

  uint16_t _spd = constrain(ANALOGRANGE * spd / 100.0, 0, ANALOGRANGE);
  analogWrite(Pin, _spd); 
}

void setup() {
  // nothing happens in setup
  Serial.begin(115000);
  pinMode(Pin, OUTPUT);
  pinMode(Freq, INPUT_PULLUP);
  digitalWrite(Freq, HIGH);
  attachInterrupt(Freq, myinthandler, FALLING);

  analogWriteRange(ANALOGRANGE);
  analogWriteFreq(ANALOGFREQ);
  analogWrite(Pin, ANALOGRANGE * STARTPOWER / 100);

  // client.wifiConnection(WIFINAME, WIFIPASS);
  WiFi.begin(WIFINAME, WIFIPASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 10000)
      break;
  }
  if (WiFi.status() == WL_CONNECTED) {
    client.begin(callback);
    subscribe();
    client.add(MQTTTHROTTLEID, STARTPOWER);
    client.ubidotsPublish(MQTTPUBLISHNAME);

    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  }
}


void loop() {

  if (millis() - lastPotichck > POTICHKINTERVAL) {
    lastPotichck = millis();

    uint16_t _adc = analogRead(A0);

    if (getLastActiveSource() == POTI ||
        abs(lastADC - _adc) > POTITRIGGER) {
      lastADC = _adc;
      setSpeed(_adc/10, POTI);
    }
  }

  if (Serial.available()) {
    set = Serial.parseInt();
    Serial.println(set);
    setSpeed(set, REMOTE);
  }

  if (micros() - lastcheck > INTERVALL) {
    lastcheck = micros();

    if (!client.connected()) {
      client.reconnect();
      subscribe();
    }
    // rpm = getRPMbyPulse();
    int16_t lastrpm = getRPMbyIRQ();

    if (abs(rpm - lastrpm) > TRIGGER || millis() - lastupload > UPLOADINTV) {
      lastupload = millis();
      rpm = lastrpm;
      client.add(MQTTSPEEDID, rpm);
      client.ubidotsPublish(MQTTPUBLISHNAME);

    }
  }
  client.loop();
}

// int16_t getRPMbyPulse() {
//   int16_t newrpm = 60e6 * 2 / (pulseIn(Freq, HIGH, 1e5));
//   if (newrpm == 0xFFFF)
//     newrpm = 0;
//   Serial.println(String("RPM: ") + rpm + String(" ") + (rpm - newrpm));
//   return newrpm;
// }

int16_t getRPMbyIRQ() {
  int16_t _RPM = 0;
  unsigned long _duration = duration;
  unsigned long _pulsecount = pulsecount;
  duration = 0; // clear counters
  pulsecount = 0;
  if (_pulsecount) {
    _RPM = 60e6 / 2.0 * _pulsecount / float(_duration);
  }

  // Freq *= _pulsecount; // calculate F
  // output time and frequency data to RS232
  // Serial.print(currentMillis);
  // Serial.print(" "); // separator!
  // Serial.print(_RPM);
  // Serial.print(" ");
  // // Serial.print(_pulsecount);
  // // Serial.print(" ");
  // // Serial.print(_duration);
  // // Serial.print(" ");
  // Serial.println((_RPM - rpm));

  // Serial.println(String("RPM: ") + rpm + String(" ") + ((int16_t)rpm -
  // newrpm));
  return _RPM;
}

void myinthandler() // interrupt handler
{
  unsigned long currentMicros = micros();
  duration += currentMicros - previousMicros;
  previousMicros = currentMicros;
  pulsecount++;
}
