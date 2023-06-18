#include <MQ135.h> // Gaz sensor library used to collect the CO2 value
#define FLAME A0 // connect AN pin of sensor to this pin
#define ANALOGPIN A1    //  connect AN pin of sensor to this pin
#define RZERO 206.85    //  Define RZERO Calibration Value
MQ135 gasSensor = MQ135(ANALOGPIN); // we set the R0 of our gaz sensor

// 
void setup(){

  PinInitialisation(); // Pin Initialisation of the two serial port communication

  TestESP32(); // Method used in order to test the connection with the ESP32 wifi card from the arduino

  ESP32Reset(); // Reset of the ESP32 wifi card to be sure that we initialise the correct configuration on it

  ClientMode(); // We select the client mode, the number 1 out of 3

  MultipleConnectionActivation(); // We activate the multiple connection option

  WifiConnection(); // We connecte the client to the AP hotspot wifi

  OpenTCPConnection(); // Connect to the TCP server initiate by the AP 

  Serial3.flush();
  delay(6000);

  Serial.println("Wifi SetUp phase done !");
  Serial.println("");  

  Serial.println("Loop Start");
}

void loop() {

  int fire = GetFlameState();
  float ppm = gasSensor.getPPM();

  Serial.print("ppm: ");
  Serial.println(ppm);
  Serial.print("fire: ");
  Serial.println(fire);

  String data = DataToSend(ppm,fire); // Method which requires the CO2 ppm and the fire value and return the string to send via the wifi

  SendData(data.c_str()); // Method used to send data to the Serial3 port

  delay(3000);  
  Serial.println("");
  Serial.println("Loop Re-Start");

}

/* 
Pin Initialisation of the two serial port communication
*/
void PinInitialisation()
{
  Serial.begin(115200);  // Initialize serial communication with Serial0
  Serial3.begin(115200); // Initialize serial communication with Serial1
  Serial.println("Set Up phase for the connectors:");
  pinMode(FLAME, INPUT);//define FLAME input pin
  delay(1000);
  Serial.println("Done !");
  Serial.println("");
}

/* 
Method used in order to test the connection with the ESP32 wifi card from the arduino
*/
void TestESP32()
{
  Serial.println("Wifi Set Up phase:");
  delay(1000);  // Open serial communications and wait for port to open:
  Serial.println("");
  Serial.println("Start");
  Serial3.flush(); // we flush the port to be sure it is empty
  delay(1000);

  Serial3.write("AT\r\n"); // we send a simple AT command
  Serial.println("Test  connection:");
  Serial.println(Serial3.readStringUntil('\n')); // we wait the OK response
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println("Test  connection done !");
  Serial.println("");
  delay(1000);
}

/* 
Reset of the ESP32 wifi card to be sure that we initialise the correct configuration on it
*/
void ESP32Reset()
{
  Serial.println("Reset of the ESP32:");
  Serial3.write("AT+RST\r\n"); // we send the reset command
  
  String incomingData = Serial3.readStringUntil('\n');
  String substring = "ready";

  while(1)
  {    
    if(incomingData.indexOf(substring) != -1)
    {
      Serial.println("Reset ESP32: " + incomingData);
      break;
    }
    delay(200);
    incomingData = Serial3.readStringUntil('\n');
    Serial3.flush();
  }
  
  substring = "WIFI CONNECTED";
  incomingData = Serial3.readStringUntil('\n');

  while(1)
  {    
    if(incomingData.indexOf(substring) != -1)
    {
      Serial.println("Reset ESP32: " + incomingData);
      break;
    }
    delay(200);
    incomingData = Serial3.readStringUntil('\n');
    Serial3.flush();
  }

  substring = "WIFI GOT IP";
  incomingData = Serial3.readStringUntil('\n');

  while(1)
  {    
    if(incomingData.indexOf(substring) != -1)
    {
      Serial.println("Reset ESP32: " + incomingData);
      break;
    }
    delay(200);
    incomingData = Serial3.readStringUntil('\n');
    Serial3.flush();
  }

  Serial.println("Reset of the ESP32 done !");
  Serial.println("");
  delay(2500);
}

/* 
We select the client mode, the number 1 out of 3
*/
void ClientMode()
{
  Serial.println("Choice mode 1:");
  Serial3.write("AT+CWMODE=1\r\n"); // we send the AT command
  Serial.println(Serial3.readStringUntil('\n')); // we wait the ready response
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println("Mode 1 selected!");
  Serial.println("");
  delay(3500);
}

/* 
We activate the multiple connection option
*/
void MultipleConnectionActivation()
{
  Serial.println("Mutiple connection activation");
  Serial3.write("AT+CIPMUX=1\r\n"); // we send the AT command
  Serial.println(Serial3.readStringUntil('\n')); 
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println("Mutiple connection activation done !");
  Serial.println("");
  delay(4000);
}

/*
We connecte the client to the AP hotspot wifi
*/
void WifiConnection()
{
  Serial.println("Connection to the WifiGRP1 Wifi !");
  Serial3.write("AT+CWJAP=\"WifiGRP1\",\"soleil01\"\r\n"); // we send the AT command containing the ssid and password
  
  String incomingData = Serial3.readStringUntil('\n');
  String substring = "OK";

  while(1)
  {    
    if(incomingData.indexOf(substring) != -1)
    {
      Serial.println("Wifi Connection: " + incomingData);
      incomingData = Serial3.readStringUntil('\n');
      break;
    }
    delay(200);
    incomingData = Serial3.readStringUntil('\n');
    Serial3.flush();
  }

  Serial.println("Connected to the WifiGRP1!");
  Serial.println("");
  delay(2000);
}

/*
Connect to the TCP server initiate by the AP 
*/
void OpenTCPConnection()
{
  Serial.println("open a TCP connection !");
  Serial3.write("AT+CIPSTART=0,\"TCP\",\"192.168.168.1\",333\r\n"); // we connect the TCP server on the channel 0, SSID and 333 port
  
  String incomingData = Serial3.readStringUntil('\n');
  String substring = "OK";

  while(1)
  {    
    Serial.println("TCP connection Response: " + incomingData);

    if(incomingData.indexOf(substring) != -1)
    {
      Serial.println("Wifi Connection: " + incomingData);
      incomingData = Serial3.readStringUntil('\n');
      Serial.println("Wifi Connection: " + incomingData);

      break;
    }
    delay(400);
    Serial3.write("AT+CIPSTART=0,\"TCP\",\"192.168.168.1\",333\r\n"); // we connect the TCP server on the channel 0, SSID and 333 port
    incomingData = Serial3.readStringUntil('\n');
    Serial3.flush();
  }

  Serial.println("Mutiple connection activation done !");
  Serial.println("");
  delay(3000);
}

/*
Return the value of the Flame sensor (0 or 1)
*/
int GetFlameState()
{
  int fire = digitalRead(FLAME);// read FLAME sensor

  if( fire == HIGH)
  {
    return fire = 1;
  }
  else
  {
    return fire = 0;
  }
}

/*
Method which requires the CO2 ppm and the fire value and return the string to send via the wifi
*/
String DataToSend(float ppm, int fire)
{
  // the goal here is to return a string composed of two parts
  // the first part is a float which contains 6 digits, then the "." and two digits after the points.
  // the second part is only one digit: 1=fire, 0=No fire
  // the two parts are separated by ":"

  String floatString = String(ppm);
  int decimalIndex = floatString.indexOf(".");
  int nbDigits = decimalIndex;

  String formattedFloat = "";

  if (nbDigits < 6) {
      for (int i = 0; i < 6 - nbDigits; i++) {
          formattedFloat += "0";
      }
  }

  formattedFloat += floatString.substring(0, decimalIndex);
  formattedFloat += ".";
  formattedFloat += floatString.substring(decimalIndex + 1);

  String data;

  data = (uint8_t*)formattedFloat.c_str(), formattedFloat.length();
  
  data = data + ":" + String(fire);

  return data.c_str(); // transform a String to a string
}

/*
Method used to send data to the Serial3 port
*/
void SendData(char* data)
{
  Serial3.write("AT+CIPSEND=0,11\r\n");
  Serial.println(Serial3.readStringUntil('\n')); 
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println(Serial3.readStringUntil('\n'));
  delay(100);
  Serial3.write(data);
  Serial.println(Serial3.readStringUntil('\n')); 
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println(Serial3.readStringUntil('\n'));
  Serial.println("Data send");
  Serial.println("");


  // String substring = "OK";
  // Serial3.write("AT+CIPSEND=0,11\r\n");
  // Serial3.write(data); 

  // String incomingData = Serial3.readStringUntil('\n');
  // Serial.println("CIPSEND: " + incomingData);  
  // while(1)
  // {    
  //   if(incomingData.indexOf(substring) != -1)
  //   {
  //     Serial.println("Commande send Status: " + incomingData);
  //     incomingData = Serial3.readStringUntil('\n');
  //     Serial.println("Out of the while");
  //     break;
  //   }

  //   incomingData = Serial3.readStringUntil('\n');
  // }

  //  while(1)
  // {    
  //   Serial.println("CIPSEND: " + incomingData);  
  //   if(incomingData.indexOf(substring) != -1)
  //   {
  //     Serial.println("Commande send Status: " + incomingData);
  //     incomingData = Serial3.readStringUntil('\n');
  //     break;
  //   }

  //   delay(200);
  //   incomingData = Serial3.readStringUntil('\n');
  //   Serial3.flush();
  //   Serial.println(" ");  
  // }







  // incomingData = Serial3.readStringUntil('\n'); 
  // substring = "SEND OK";

  // while(1)
  // {    
  //   if(incomingData.indexOf(substring) != -1)
  //   {
  //     Serial.println("Message Status: " + incomingData);
  //     incomingData = Serial3.readStringUntil('\n');
  //     break;
  //   }
  //   delay(200);
  //   incomingData = Serial3.readStringUntil('\n');
  //   Serial3.flush();
  // }
}
