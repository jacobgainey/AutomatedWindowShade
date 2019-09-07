#include <EspMQTTClient.h>

// Electronic Materials (modules)
/ ------------------------------------------------------------------------------------------
// ESP8266 D1 mini (ESP8266MOD) WEMO
// 2 Way DC Motor Driver Module Speed Dual H-Bridge Replace Stepper L298N
// 12V Mini DC Metal Gear Motor with Gearwheel Shaft Diameter N20 200RPM
// TTP223 Touch button Module Capacitor type Single Channel Self Locking Touch switch sensor
// 4.75-23V To 1-17V Mini 360 DC-DC Buck Converter Step Down Module

const String nickname = "sky-esp8266-03";

#define ledPin 2    // built in led
#define uPin D3     // touch pin (move shades up)
#define dPin D4     // touch pin (move shdes down)
#define fwdPin D5   // motor pin (up)
#define bwdPin D6   // motor pin (down)

int maxspeed = 250;  // max speed

// global variables for millis timer (heartbeat)
unsigned long startMillis;          
unsigned long currentMillis;
unsigned long period = 60000;   

EspMQTTClient client(
  "Skywalker-IoT",
  "beergarden",
  "192.168.66.8",  // MQTT Broker server ip
  "jgainey",       // Can be omitted if not needed
  "password",      // Can be omitted if not needed
  nickname.c_str() // Client name 
);

void setup()
{
  startMillis = millis();  //initial start time 
  
  Serial.begin(115200);
  Serial.println("Skywalker Motorized Blinds");
  
  // Optionnal functionnalities of EspMQTTClient : 
  client.enableDebuggingMessages();                    // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater("jgainey", "password");  // Enable the web updater. 
  client.enableLastWillMessage("skywalker/lastwills/" + nickname, "offline", false);

  pinMode(fwdPin, OUTPUT);
  pinMode(bwdPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  // set high (WEMO D1 mini's are opposite?)
  digitalWrite(fwdPin, 1); 
  digitalWrite(bwdPin, 1); 
  digitalWrite(ledPin, 1); 
}

// --- private internal functions
void moveShadeUp(){
    digitalWrite(bwdPin, 0);
    digitalWrite(fwdPin, 1);
}
void moveShadeDown(){
    digitalWrite(bwdPin, 1);
    digitalWrite(fwdPin, 0);
}
void stopMovement(){
    digitalWrite(bwdPin, 0);
    digitalWrite(fwdPin, 0);
}

// standard ack  (blink led)
void blink(int x){
  for (int i = 0; i <= x; i++) {
    digitalWrite(ledPin, 0); delay(50);
    digitalWrite(ledPin, 1); delay(50);
  }
}

// This function is called once everything is connected (Wifi and MQTT)
void onConnectionEstablished()
{
  // Subscribe to topic and display received message to Serial
  client.subscribe("skywalker/sensors/esp8266/" + nickname, [](const String & payload) {
    Serial.println(payload);
    
      // --- watch mqtt topic for input ()
      if (payload == "up"){
        Serial.println("moving blind up");
        moveShadeUp();
      }
      
      if (payload == "down"){ // *** move down
        Serial.println("moving blind down");
        moveShadeUp();
      }
      
      if (payload == "stop"){ // *** stop
        Serial.println("stopping blinds");
        stopMovement();
      }
      
      if (payload == "bark"){ // *** acknowledge
        Serial.println("Woof Woof Woof");
        blink(3);
      }
  });

  // Publish a message to topic
  client.publish("skywalker/esp8266", nickname + " online", false);
}


void loop()
{
  // mqtt loop
  client.loop(); 

  if (digitalRead(fwdPin) == 1 && digitalRead(bwdPin) == 0){
    blink(1);
    delay(1000);
  }
  if (digitalRead(fwdPin) == 0 && digitalRead(bwdPin) == 1){
    blink(3);
    delay(1000);
  }
  
  // --- watch for touch pad input
  if (digitalRead(uPin) == 0){
      moveShadeUp();
    }

  if (digitalRead(dPin) == 0){
      moveShadeDown();
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
