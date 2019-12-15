/*
 * Test eines Dallas 18B20 im eingebauten Node mit Display 5110
 */

#include <LCD5110_Graph.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 8

LCD5110 myGLCD(7,6,5,2,4);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;

extern uint8_t SmallFont[];
extern unsigned char TinyFont[];

float temp;
uint8_t* bm;
int pacy;

void setup()
{
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  sensors.begin();
  

}

void loop()
{
  myGLCD.clrScr();
  myGLCD.print("18B20 Test", CENTER, 0);
  if (!sensors.getAddress(tempDeviceAddress, 0)) {
    myGLCD.print(F("Error"),CENTER,10);
    myGLCD.print(F("connnecting"),CENTER,20);
    myGLCD.print(F("to Sensor."),CENTER,30);
  } else {
    myGLCD.print(F("Temperature : "),0,20);
    sensors.setWaitForConversion(false);
    sensors.setResolution(tempDeviceAddress, 9);
    sensors.requestTemperatures();
    delay(75);
    temp = sensors.getTempCByIndex(0);
    myGLCD.printNumF(temp,3,30,30);
  }
  myGLCD.update();
  delay(3000);
}
