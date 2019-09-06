#include <EspMQTTClient.h>

const String nickname = "sky-esp8266-03";

#define ledPin 2    // built in led
#define uPin D3   // touch pin 
#define dPin D4   // touch pin
#define fwdPin D5   // motor pin (up)
#define bwdPin D6   // motor pin (down)

int maxspeed = 250;  // max speed

// global variables for millis timer (heartbeat)
unsigned long startMillis;          
unsigned long currentMillis;
unsigned long period = 60000;   

EspMQTTClient client(
  "Skywalker",
  "beergarden",
  "192.168.66.8",  // MQTT Broker server ip
  "jgainey",       // Can be omitted if not needed
  "password",      // Can be omitted if not needed
  nickname.c_str() // Client name that uniquely identify your device
);

void setup()
{
  startMillis = millis();  //initial start time 
  
  Serial.begin(115200);
  Serial.println("Skywalker Motorized Blinds");
  
  // Optionnal functionnalities of EspMQTTClient : 
  client.enableDebuggingMessages();                    // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater("jgainey", "password");  // Enable the web updater. 
  //client.enableLastWillMessage("skywalker/lastwills" + nickname, "offline", false);

  pinMode(fwdPin, OUTPUT);
  pinMode(bwdPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(fwdPin, 1); 
  digitalWrite(bwdPin, 1); 
  digitalWrite(ledPin, 1); 
}

// This function is called once everything is connected (Wifi and MQTT)
void onConnectionEstablished()
{
  // Subscribe to topic and display received message to Serial
  client.subscribe("skywalker/esp8266/" + nickname, [](const String & payload) {
    Serial.println(payload);
    
      // message options   
      if (payload.startsWith("heartbeat") == true){
        period = atol(payload.substring(10).c_str());
        Serial.print("changing heartbeat interval to ");
        Serial.println(payload.substring(10));
      }
      
      if (payload == "up"){ // *** move up
        Serial.println("moving blind up");
        digitalWrite(bwdPin, 0);
        digitalWrite(fwdPin, 1);
      }
      
      if (payload == "down"){ // *** move down
        Serial.println("moving blind down");
        digitalWrite(bwdPin, 1);
        digitalWrite(fwdPin, 0);
      }
      
      if (payload == "stop"){ // *** stop
        Serial.println("stopping blinds");
        digitalWrite(bwdPin, 0);
        digitalWrite(fwdPin, 0);
      }
      
      if (payload == "bark"){ // *** acknowledge
        Serial.println("Woof Woof Woof");
        blink(3);
      }
  });

  // Publish a message to topic
  client.publish("skywalker/esp8266", nickname + " online", false);
}

void blink(int x){
  for (int i = 0; i <= x; i++) {
    digitalWrite(ledPin, 0); delay(50);
    digitalWrite(ledPin, 1); delay(50);
  }
}

void loop()
{
  client.loop(); 

  if (digitalRead(fwdPin) == 1 && digitalRead(bwdPin) == 0){
    blink(1);
    delay(1000);
  }
  if (digitalRead(fwdPin) == 0 && digitalRead(bwdPin) == 1){
    blink(3);
    delay(1000);
  }
  if (digitalRead(fwdPin) == 0 && digitalRead(bwdPin) == 0){
//    blink(1);
//    delay(1000);
  }

  if (digitalRead(uPin) == 0){
      digitalWrite(bwdPin, 0);
      digitalWrite(fwdPin, 1);
    }

  if (digitalRead(dPin) == 0){
      digitalWrite(bwdPin, 1);
      digitalWrite(fwdPin, 0);
    }
  
  currentMillis = millis();
  if (currentMillis - startMillis >= period)
  {
    
    if (client.isConnected() == true){
      //blink(3);
      client.publish("skywalker/esp8266/heartbeats", nickname + " heartbeat", false);
      Serial.println("heartbeat ... thump thump");
    }
    startMillis = currentMillis;  
  } 
  
}
