#include <Adafruit_BME280.h>



/*
  get temp data and sent to the Lora Server

  This is an example to show how to get sensor data from a remote Arduino via Wireless Lora Protocol
  The exampels requries below hardwares:
  1) Client Side: Arduino + Lora Shield (868Mhz) + DS18B20 (Temperature Sensor).
  2) Server Side: Arduino + Lora Shield (868Mhz) + Yun Shield + USB flash. make sure the USB flash has
  this file datalog.csv in the data directory of root.

  requrie below software:
  Radiohead library from:  http://www.airspayce.com/mikem/arduino/RadioHead/
  OneWire library for Arduino
  DallasTemperature library for Arduino

  Client side will get the temperature and keep sending out to the server via Lora wireless.
  Server side will listin on the Lora wireless frequency, once it get the data from Client side, it will
  turn on the LED and log the sensor data to a USB flash,

*/
//Include required lib so Arduino can communicate with the temperature sensorDS18B20
// Singleton instance of the radio driver
#include <SPI.h>
#include <RH_RF95.h>
RH_RF95 rf95;
Adafruit_BME280 bme; // I2C

float data;
String datastring = "";
char databuf[35];
uint8_t dataoutgoing[35];

const int delayTime = 5000; //elke 5 seconden
int lastTime;
void setup()
{

  Serial.begin(9600);
  Serial.println("initializing");

  if (!rf95.init())
    Serial.println("init failed");
  RH_RF95::ModemConfig modem_config = {
    0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
    0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
    0x0c  // Reg 0x26: LowDataRate=On, Agc=On
  };
  rf95.setModemRegisters(&modem_config);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop()
{
  delay(delayTime);
  // Print Sending to rf95_server
  Serial.println("Sending to rf95_server");
  data = bme.readTemperature();
  datastring = bme.readTemperature();
  datastring += ";";
  datastring += bme.readPressure();
  datastring += ";";
  datastring += bme.readHumidity();
  datastring.toCharArray(databuf, 35);
  strcpy((char *)dataoutgoing, databuf);
  Serial.println(databuf);
  rf95.send(dataoutgoing, sizeof(dataoutgoing));
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t indatabuf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(indatabuf);

  if (rf95.waitAvailableTimeout(3000))
  {
    // Should be a reply message for us now
    if (rf95.recv(indatabuf, &len))
    {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);

      // Serial print "got reply:" and the reply message from the server
      Serial.print("got reply: ");
      Serial.println((char*)indatabuf);

    }
    else
    {
      //
      Serial.println("recv failed");
    }
  }
  else
  {
    // Serial print "No reply, is rf95_server running?" if don't get the reply .
    Serial.println("No reply, is rf95_server running?");
  }
  delay(400);
}

