/*
  Lora Shield- Yun Shield to log data into USB

  This is an example to show how to get sensor data from a remote Arduino via Wireless Lora Protocol
  The exampels requries below hardwares:
  1) Client Side: Arduino + Lora Shield (868Mhz) + DS18B20 (Temperature Sensor). 
  2) Server Side: Arduino + Lora Shield (868Mhz) + Yun Shield + USB flash. make sure the USB flash has
  this file datalog.csv in the data directory of root. 

  requrie below software:
  Radiohead library from:  http://www.airspayce.com/mikem/arduino/RadioHead/  

  Client side will get the temperature and keep sending out to the server via Lora wireless. 
  Server side will listin on the Lora wireless frequency, once it get the data from Client side, it will 
  turn on the LED and log the sensor data to a USB flash, 

  More about this example, please see:


*/

//Include required lib so Arduino can talk with the Lora Shield
#include <SPI.h>
#include <RH_RF95.h>

//Include required lib so Arduino can communicate with Yun Shield



// Singleton instance of the radio driver
RH_RF95 rf95;
int led = 4;
int reset_lora = 9;
String dataString = "";

void setup() 
{
  Serial.begin(9600);
  pinMode(led, OUTPUT); 
  pinMode(reset_lora, OUTPUT);     

  

  // reset lora module first. to make sure it will works properly
  digitalWrite(reset_lora, LOW);   
  delay(1000);
  digitalWrite(reset_lora, HIGH); 
  
  //while(!Serial);  // wait for Serial port to connect.
  //Serial.println("Log remote sensor data to USB flash\n");



  if (!rf95.init())
    Serial.println("init failed");  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // Need to change to 868.0Mhz in RH_RF95.cpp 
    RH_RF95::ModemConfig modem_config = {
  0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
  0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
  0x0c  // Reg 0x26: LowDataRate=On, Agc=On
};
rf95.setModemRegisters(&modem_config);
}

void loop()
{
  dataString="";
  if (rf95.available())
  {
    Serial.println("Get new message");
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(led, HIGH);
      //RH_RF95::printBuffer("request: ", buf, len);
      Serial.print("got message: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      //make a string that start with a timestamp for assembling the data to log:
      dataString += "nu";
      dataString += "  :  ";
      dataString += String((char*)buf);

      // Send a reply to client as ACK
      uint8_t data[] = "200 OK";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      // The FileSystem card is mounted at the following "/mnt/FileSystema1"
      // Make sure you have this file in your system


      digitalWrite(led, LOW);      
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}



