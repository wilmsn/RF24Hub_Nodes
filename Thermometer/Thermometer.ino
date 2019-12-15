/*
A thermometer.
Can be used with a display or only as a sensor without display

!!!!! On Branch gateway !!!!!!!

*/
//****************************************************
// My definitions for my nodes based on this sketch
// Select only one at one time !!!!
//#define AUSSENTHERMOMETER
//#define AUSSENTHERMOMETER2
//#define SCHLAFZIMMERTHERMOMETER
#define BASTELZIMMERTHERMOMETER
//#define KUECHETHERMOMETER
//#define WOHNZIMMERTHERMOMETER
//#define ANKLEIDEZIMMERTHERMOMETER
//#define GAESTEZIMMERTHERMOMETER
//****************************************************
//          Define node general settings
//  Can be overwritten in individual settings later
//****************************************************
// Dummy values, be sure to overwrite later!!
#define EEPROM_VERSION 0
#define RF24NODE 0
//****************************************************
#define RF24CHANNEL     10
// Delay between 2 transmission in ms
#define RF24SENDDELAY 50
// Delay between 2 transmission in ms
#define RF24RECEIVEDELAY 50
// Sleeptime in ms !! 
// (valid: 10.000 ... 3.600.000)
#define RF24SLEEPTIME   60000
// Max Number of Heartbeart Messages to send !!
#define RF24SENDLOOPCOUNT 10
// Max Number of Receiveloops !!
#define RF24RECEIVELOOPCOUNT 10
// number of empty loop after sending data
// valid: 0...9
#define RF24EMPTYLOOPCOUNT  0
// Voltage Faktor will be divided by 100 (Integer !!)!!!!
#define VOLTAGEFACTOR 100
// Add a constant number her (will be divided by 100)!!!
#define VOLTAGEADDED 0
// Kontrast of the display
#define DISPLAY_KONTRAST 65;
// Define low voltage level on processor
// below that level the thermometer will be switched off 
// until the battery will be reloaded
#define LOWVOLTAGELEVEL 2.8
// Change the versionnumber to store new values in EEPROM
// Set versionnumber to "0" to disable 
// #define EEPROM_VERSION 2
// 4 voltages for the battery (empty ... full)
#define U0 3.6
#define U1 3.7
#define U2 3.8
#define U3 3.9
#define U4 4.0
// set X0 and Y0 of battery symbol ( is 10 * 5 pixel )
#define BATT_X0 74
#define BATT_Y0 0
// set X0 and Y0 of antenna symbol ( is 10 * 10 pixel )
#define ANT_X0 74
#define ANT_Y0 6
// set X0 and Y0 of thermometer symbol ( is 3 * 6 pixel )
#define THERM_X0 74
#define THERM_Y0 17
// set X0 and Y0 of waiting symbol ( is 6 * 6 pixel )
#define WAIT_X0 78
#define WAIT_Y0 17
// The CE Pin of the Radio module
#define RADIO_CE_PIN 10
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
// The pin of the statusled
#define STATUSLED 3
#define STATUSLED_ON HIGH
#define STATUSLED_OFF LOW
#define ONE_WIRE_BUS 8
#define RECEIVEDELAY 100

//*****************************************************
//    Individual settings
//-----------------------------------------------------
#if defined(AUSSENTHERMOMETER)
#define BME280
#define RF24NODE        02
#define STATUSLED       7
#define LOWVOLTAGELEVEL 1
#define EEPROM_VERSION  3

#endif
//-----------------------------------------------------
#if defined(AUSSENTHERMOMETER2)
#define BMP_280
#define RF24NODE        01
#define STATUSLED       3
#define LOWVOLTAGELEVEL 1
#define EEPROM_VERSION  2

#endif
//-----------------------------------------------------
#if defined(SCHLAFZIMMERTHERMOMETER)
#define DALLAS_18B20
#define DISPLAY_5110
#define RF24NODE        015
#define EEPROM_VERSION  3
#define VOLTAGEADDED    55
 
#endif
//-----------------------------------------------------
#if defined(BASTELZIMMERTHERMOMETER)
#define DALLAS_18B20
#define DISPLAY_5110
#define RF24NODE        100
#define EEPROM_VERSION  3
#define VOLTAGEADDED    55
#define DALLAS_RESOLUTION  9 
#endif
//-----------------------------------------------------
#if defined(KUECHETHERMOMETER)
#define DALLAS_18B20
#define DISPLAY_5110
#define RF24NODE        03
#define EEPROM_VERSION  3
#define VOLTAGEADDED    55
 
#endif
//-----------------------------------------------------
#if defined(ANKLEIDEZIMMERTHERMOMETER)
#define DALLAS_18B20
#define DISPLAY_5110
#define RF24NODE        025
#define EEPROM_VERSION  3
#define VOLTAGEADDED    55
 
#endif
//-----------------------------------------------------
#if defined(GAESTEZIMMERTHERMOMETER)
#define DALLAS_18B20
//#define DISPLAY_5110
#define RF24NODE        055
#define EEPROM_VERSION  3
#define VOLTAGEADDED    55
 
#endif
//-----------------------------------------------------
//*****************************************************
// ------ End of configuration part ------------

#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <sleeplib.h>
#include <Vcc.h>
#include <EEPROM.h>
#include "zahlenformat.h"

#if defined(DISPLAY_5110)
#define HAS_DISPLAY
#include <LCD5110_Graph.h>
#endif

#if defined(DALLAS_18B20)
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#if defined(BME280)
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#endif

#if defined(BMP_280)
#include <BMP280.h>
#endif

// ----- End of Includes ------------------------

Vcc vcc(1.0);

ISR(WDT_vect) { watchdogEvent(); }

#if defined(HAS_DISPLAY)
#if defined(DISPLAY_5110)
LCD5110 myGLCD(7,6,5,2,4);
extern uint8_t SmallFont[];
extern uint8_t BigNumbers[];
#endif
#endif

#if defined(DALLAS_18B20)
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensor(&oneWire);
DeviceAddress tempDeviceAddress;
float temp;
#endif

#if defined(BME280)
Adafruit_BME280 sensor;
float temp, pres, humi;
#endif

#if defined(BMP_280)
BMP280 bmp(0x76);
float temp, pres;
#endif

// Structure of our payload
struct payload_t {
  uint8_t     node_id;         
  uint8_t     msg_id;          
  uint8_t     msg_type;        
  uint8_t     msg_flags;       
  uint8_t     orderno;         
  uint8_t     network_id;      
  uint8_t     reserved1;      
  uint8_t     reserved2;      
  uint32_t    data1;         
  uint32_t    data2;         
  uint32_t    data3;         
  uint32_t    data4;         
  uint32_t    data5;         
  uint32_t    data6;         
};
payload_t payload;    

struct eeprom_t {
   uint8_t  versionnumber;
   uint32_t sleeptime;
   uint16_t sendloopcount;
   uint16_t receiveloopcount;
   uint16_t emptyloopcount;
   uint16_t voltagefactor;
   int      voltageadded;
   uint16_t node;
   uint8_t  channel;
   int      display_contrast;
};
eeprom_t eeprom;

#if defined(HAS_DISPLAY)
boolean             display_down = false;
boolean             monitormode = false;
//Some Var for restore after sleep of display
float               field1_val, field2_val, field3_val, field4_val;
#endif
boolean             low_voltage_flag = false;
float               cur_voltage;
uint16_t            loopcount;
uint16_t            receiveloopcount;
uint16_t            sendloopcount;
long int            sleep_kor_time;
uint32_t            last_send;
uint8_t             msg_id;


// nRF24L01(+) radio attached using Getting Started board 
// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);
uint8_t rx_address1[] = { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc};
uint8_t  tx_address[] = { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc};


void get_sensordata(void) {
#if defined(DALLAS_18B20)
  sensor.setWaitForConversion(false);
  sensor.setResolution(tempDeviceAddress, DALLAS_RESOLUTION);
  sensor.requestTemperatures(); // Send the command to get temperatures
  sleep4ms(800);
  delay(10);
  temp=sensor.getTempCByIndex(0);
#endif
#if defined(BME280)
  sensor.takeForcedMeasurement();
  sleep4ms(800);
  delay(10);
  temp=sensor.readTemperature();
  pres=pow(((95*0.0065)/(sensor.readTemperature()+273.15)+1),5.257)*sensor.readPressure()/100.0;
  humi=sensor.readHumidity();
#endif
#if defined(BMP_280)
  bmp.startSingleMeasure();
  sleep4ms(800);
  delay(10);
  temp = bmp.readTemperature();
  pres = bmp.readPressureAtSealevel(91);
#endif
}

float action_loop(uint32_t data) {
  uint8_t channel = getChannel(data);
  float value = getValue_f(data);
  float retval = value;
    switch (channel) {
#if defined(HAS_DISPLAY)
      case 21:
        // Set field 1
        field1_val = value;
        print_field(field1_val,1);
       break;
      case 22:
        // Set field 2
        field2_val= value;
        print_field(field2_val,2);
       break;
      case 23:
        // Set field 3
        field3_val= value;
        print_field(field3_val,3);
       break;
      case 24:
        // Set field 4
        field4_val= value;
        print_field(field4_val,4);
       break;
      case 31:
        // Displaylight ON <-> OFF
        if (value < 0.5) {
          digitalWrite(STATUSLED,STATUSLED_OFF); 
        } else  {
          digitalWrite(STATUSLED,STATUSLED_ON);
        }
       break;
      case 41:
        // Display Sleepmode ON <-> OFF
        display_sleep(value < 0.5);
       break;
#endif
      case 110:
#if defined(HAS_DISPLAY)
#if defined(DISPLAY_5110)
        if (value > 0.5 || value < 101) {
          eeprom.display_contrast=(uint8_t)value;
          myGLCD.setContrast(eeprom.display_contrast);
          EEPROM.put(0, eeprom);
        }
        retval = (float)eeprom.sleeptime;
#endif
#endif
      break;
      case 111:
      // sleeptime in ms!
        if (value > 9999 && value < 3600000) {
          eeprom.sleeptime=(uint32_t)value;
          EEPROM.put(0, eeprom);
        }
        retval = (float)eeprom.sleeptime;
        break;
      case 112:
      // sendloopcount - number sendloop befor giving up 
        if (value > 0.5 || value < 21) {
          eeprom.sendloopcount=(uint16_t)value;
          EEPROM.put(0, eeprom);
        }
        retval = (float)eeprom.sendloopcount;
        break;
      case 113:
      // receiveloopcount - number of receivloops befor giving up.
        if (value > 0.5 || value < 21) {
          eeprom.receiveloopcount=(uint16_t)value;
          EEPROM.put(0, eeprom);
        }
        retval = (float)eeprom.receiveloopcount;
        break;
      case 114:
      // emptyloopcount - only loop 0 will transmit all other loops will only read and display
        if (value >= 0.5 && value < 21) {
          eeprom.emptyloopcount=(uint16_t)value;
          EEPROM.put(0, eeprom);
        } 
        retval = (float)eeprom.emptyloopcount;
        break;
      case 115:
      // sleep korrektion faktor in ms - will only be used once!
        if ((value > 0.5 && value < 600001) || (value < -0.5 && value > -600001)) {
          sleep_kor_time = (long int)value;
        }
        retval = sleep_kor_time;
        break;
      case 116:
      // Voltagefactor - will be divided by 100
        if (value > 10 && value < 1000) {
          eeprom.voltagefactor=(uint16_t)value;
          EEPROM.put(0, eeprom);
        }
        retval = (float)eeprom.voltagefactor;
        break;
      case 117:
      // Voltageadded - will be divided by 100
        if (value > -300 && value < 300) {
          eeprom.voltageadded=(int)value;
          EEPROM.put(0, eeprom);
        }
        retval = (float)eeprom.voltageadded;
        break;
#if defined(HAS_DISPLAY)
      case 118:
          if ( value > 0.5 && value < 1.5 ) {
            monitor(eeprom.sleeptime/2); 
            monitormode = true;
          } else {
            monitormode = false;
          }
        break;
#endif
    }  
    return calcTransportValue_f(channel,retval);
}  

void setup(void) {
  pinMode(STATUSLED, OUTPUT);     
  digitalWrite(STATUSLED,STATUSLED_ON); 
  EEPROM.get(0, eeprom);
  if (eeprom.versionnumber != EEPROM_VERSION && EEPROM_VERSION > 0) {
    eeprom.versionnumber = EEPROM_VERSION;
    eeprom.sleeptime = RF24SLEEPTIME;
    eeprom.sendloopcount = RF24SENDLOOPCOUNT;
    eeprom.receiveloopcount = RF24RECEIVELOOPCOUNT;
    eeprom.emptyloopcount = RF24EMPTYLOOPCOUNT;
    eeprom.voltagefactor = VOLTAGEFACTOR;
    eeprom.voltageadded = VOLTAGEADDED;
    eeprom.node = RF24NODE;
    eeprom.channel = RF24CHANNEL; 
    eeprom.display_contrast = DISPLAY_KONTRAST;
    EEPROM.put(0, eeprom);
  }
  SPI.begin();
  radio.begin();
#if defined(DALLAS_18B20)
  sensor.begin(); 
#endif
#if defined(BME280)
  sensor.begin(); 
#endif
#if defined(BMP_280)
  bmp.begin(); 
#endif
#if defined(HAS_DISPLAY)
#if defined(DISPLAY_5110)
  myGLCD.InitLCD();
  myGLCD.setContrast(eeprom.display_contrast);
  myGLCD.clrScr();
#endif
#endif
  radio.setDataRate( RF24_250KBPS );
  radio.setPALevel( RF24_PA_MAX ) ;
  radio.setRetries(15, 15);
  radio.openWritingPipe(tx_address);
  radio.openReadingPipe(1,rx_address1);
  radio.setAutoAck(false);
  delay(1000);
  digitalWrite(STATUSLED,STATUSLED_OFF); 
#if defined(HAS_DISPLAY)
  monitor(15000);
  draw_antenna(ANT_X0, ANT_Y0);
  draw_therm(THERM_X0, THERM_Y0);
#endif
  loopcount = 0;
  last_send = 0;
  msg_id = 1;
}

#if defined(HAS_DISPLAY)
void monitor(uint32_t delaytime) {
  const char string_1[] PROGMEM = "Temp: ";
  const char string_2[] PROGMEM = "Ubatt: ";
  const char string_3[] PROGMEM = "Sleep: ";
  const char string_4[] PROGMEM = "send/rec/empty: ";
  const char string_5[] PROGMEM = "RF24 Network: ";
  const char string_6[] PROGMEM = "Node: ";
  const char string_7[] PROGMEM = "Channel: ";
#if defined(DISPLAY_5110)
  myGLCD.setFont(SmallFont);
  get_sensordata();
  myGLCD.print(string_1, 0, 0);
  myGLCD.printNumF(temp,1, 30, 0);
  cur_voltage = (vcc.Read_Volts()+((float)eeprom.voltageadded/100.0))*((float)eeprom.voltagefactor)/100.0;
  float mes_voltage = vcc.Read_Volts();
  myGLCD.print(string_2, 0, 10);
  myGLCD.printNumF(cur_voltage,1, 40, 10);
  myGLCD.printNumF(mes_voltage,1, 65, 10);
  myGLCD.print(string_3, 0, 20);
  myGLCD.printNumI(eeprom.sleeptime, 45, 20);
  myGLCD.print(string_4, 0, 30);
  myGLCD.printNumI(eeprom.sendloopcount, 0, 40);
  myGLCD.print("/", 20, 40);
  myGLCD.printNumI(eeprom.receiveloopcount, 30, 40);
  myGLCD.print("/", 60, 40);
  myGLCD.printNumI(eeprom.emptyloopcount, 70, 40);
  myGLCD.update();
  sleep4ms(delaytime);
  delay(10);
  myGLCD.clrScr();
  myGLCD.print(string_5, 0, 0);
  myGLCD.print(string_6, 0, 10);
  myGLCD.printNumI(eeprom.node, 62, 10);
  myGLCD.print(string_7, 0, 20);
  myGLCD.printNumI(eeprom.channel, 60, 20);
  myGLCD.update();
  sleep4ms(delaytime);
  delay(10);
  myGLCD.clrScr();
#endif  
}

void display_sleep(boolean dmode) {
  display_down = dmode;
  if ( dmode ) { // Display go to sleep
#if defined(DISPLAY_5110)
    myGLCD.enableSleep(); 
#endif
  } else {
    if ( ! low_voltage_flag ) {  
#if defined(DISPLAY_5110)
      myGLCD.disableSleep(); 
#endif
        get_sensordata();
        print_field(field1_val,1);
        print_field(field2_val,2);
        print_field(field3_val,3);
        print_field(field4_val,4);
    }
  }  
}

void draw_therm(byte x, byte y) {
  if ( ! display_down ) {
#if defined(DISPLAY_5110)
    myGLCD.drawRect(x+1,y,x+1,y+3);
    myGLCD.drawRect(x,y+4,x+2,y+5);
#endif
  }
}

void wipe_therm(byte x, byte y) {
  if ( ! display_down ) {
    for (byte i=x; i<x+3; i++) {
      for (byte j=y; j<y+6; j++) {
#if defined(DISPLAY_5110)
        myGLCD.clrPixel(i,j);
#endif
      }
    }
  }
}

void draw_temp(float t) {
  int temp_abs, temp_i, temp_dez_i; 
  if ( ! display_down ) {
#if defined(DISPLAY_5110)
    if (t < 0.0) {
      temp_abs=t*-1;
      myGLCD.drawRect(0,10,9,12);
    } else {
      temp_abs=t;
    }      
    temp_i=(int)temp_abs;
    temp_dez_i=t*10-temp_i*10;
    for(byte i=0; i<74; i++) {
      for (byte j=0; j<25; j++) {
        myGLCD.clrPixel(i,j);
      }
    }
    if (temp_i < 100) {
      myGLCD.setFont(BigNumbers);
      if ( temp_i < 10 ) {
        myGLCD.printNumI(temp_i, 20, 0);        
      } else {
        myGLCD.printNumI(temp_i, 10, 0);
      }
      myGLCD.drawRect(40,20,43,23);
      myGLCD.printNumI(temp_dez_i, 45, 0);
      myGLCD.drawRect(61,2,64,5);
    } else {
      myGLCD.drawRect(15,10,24,12);
      myGLCD.drawRect(30,10,39,12);
      myGLCD.drawRect(45,10,54,12);
    }
    myGLCD.update();
#endif
  }
}


void print_field(float val, int field) {
  int x0, y0;
  if ( ! display_down ) {
#if defined(DISPLAY_5110)
    switch (field) {
      case 1: x0=0; y0=25; break;
      case 2: x0=42; y0=25; break;
      case 3: x0=0; y0=36; break;
      case 4: x0=42; y0=36; break;
    }
    for (int i=x0; i < x0+42; i++) {
      for (int j=y0; j< y0+12; j++) {
        myGLCD.clrPixel(i,j);
      }
    }
    myGLCD.drawRect(x0,y0,x0+41,y0+11);
    myGLCD.setFont(SmallFont);
    if ( val > 100 ) {
      if (val+0.5 > 1000) { 
       myGLCD.printNumI(val, x0+9, y0+3);
      } else {
       myGLCD.printNumI(val, x0+12, y0+3);
      }    
    } else {
      if (val >= 10) {
        myGLCD.printNumF(val,1, x0+9, y0+3);
      } else {
        myGLCD.printNumF(val,2, x0+9, y0+3);
      }      
    }
    myGLCD.update();
#endif
  }
}

void draw_battery_filled(int x, int y) {
#if defined(DISPLAY_5110)
  myGLCD.setPixel(x,y); 
  myGLCD.setPixel(x,y+1); 
  myGLCD.setPixel(x,y+2); 
  myGLCD.setPixel(x+1,y); 
  myGLCD.setPixel(x+1,y+1); 
  myGLCD.setPixel(x+1,y+2); 
#endif
}

void draw_battery(int x, int y, float u) {
  if ( ! display_down ) {
#if defined(DISPLAY_5110)
    // Clear the drawing field
    for (byte i=x; i<=x+9; i++) {
      for (byte j=y; j<=y+5; j++) {
        myGLCD.clrPixel(i,j);
      }
    }
    // Drawing a symbol of an battery
    // Size: 10x5 pixel
    // at position x and y
    myGLCD.drawRect(x+2,y,x+9,y+4);
    myGLCD.drawRect(x,y+1,x+1,y+3);
    if ( u > U1 ) draw_battery_filled(x+8,y+1); else myGLCD.drawLine(x+3,y,x+7,y+4);
    if ( u > U2 ) draw_battery_filled(x+6,y+1);
    if ( u > U3 ) draw_battery_filled(x+4,y+1);
    if ( u > U4 ) draw_battery_filled(x+2,y+1);
    myGLCD.update();
#endif
  }
}

void draw_antenna(int x, int y) {
  if ( ! display_down ) {
#if defined(DISPLAY_5110)
    // Drawing a symbol of an antenna
    // Size: 10x10 pixel
    // at position x and y
    myGLCD.setPixel(x+7,y+0);
    myGLCD.setPixel(x+1,y+1);
    myGLCD.setPixel(x+8,y+1);
    myGLCD.setPixel(x+0,y+2);
    myGLCD.setPixel(x+3,y+2);
    myGLCD.setPixel(x+6,y+2);
    myGLCD.setPixel(x+9,y+2);
    myGLCD.setPixel(x+0,y+3);
    myGLCD.setPixel(x+2,y+3);
    myGLCD.setPixel(x+7,y+3);
    myGLCD.setPixel(x+9,y+3);
    myGLCD.setPixel(x+0,y+4);
    myGLCD.setPixel(x+2,y+4);
    myGLCD.setPixel(x+4,y+4);
    myGLCD.setPixel(x+5,y+4);
    myGLCD.setPixel(x+7,y+4);
    myGLCD.setPixel(x+9,y+4);
    myGLCD.setPixel(x+0,y+5);
    myGLCD.setPixel(x+2,y+5);
    myGLCD.setPixel(x+4,y+5);
    myGLCD.setPixel(x+5,y+5);
    myGLCD.setPixel(x+7,y+5);
    myGLCD.setPixel(x+9,y+5);
    myGLCD.setPixel(x+0,y+6);
    myGLCD.setPixel(x+3,y+6);
    myGLCD.setPixel(x+4,y+6);
    myGLCD.setPixel(x+5,y+6);
    myGLCD.setPixel(x+6,y+6);
    myGLCD.setPixel(x+9,y+6);
    myGLCD.setPixel(x+1,y+7);
    myGLCD.setPixel(x+4,y+7);
    myGLCD.setPixel(x+5,y+7);
    myGLCD.setPixel(x+8,y+7);
    myGLCD.setPixel(x+4,y+8);
    myGLCD.setPixel(x+5,y+8);
    myGLCD.setPixel(x+4,y+9);
    myGLCD.setPixel(x+5,y+9);
    myGLCD.update();
#endif
  }
}   
 
void wipe_antenna(int x, int y) {
  if ( ! display_down ) {
#if defined(DISPLAY_5110)
    for (int i=x; i<x+10; i++) {
      for (int j=y; j<y+10; j++) {
        myGLCD.clrPixel(i,j);
      }
    }
    myGLCD.update();
#endif
  }
}  
#endif
  
void loop(void) {
  delay(10);  
  cur_voltage = (vcc.Read_Volts()+((float)eeprom.voltageadded/100.0))*((float)eeprom.voltagefactor)/100.0;
  low_voltage_flag = (vcc.Read_Volts() <= LOWVOLTAGELEVEL);
  if ((! low_voltage_flag) || (last_send > 3600000)) {
#if defined(HAS_DISPLAY)
    if ( ! monitormode ) {
      draw_battery(BATT_X0,BATT_Y0,cur_voltage);
      draw_therm(THERM_X0, THERM_Y0);
    }
#endif
    get_sensordata();
#if defined(HAS_DISPLAY)
    draw_temp(temp);
    wipe_therm(THERM_X0, THERM_Y0);
#endif
    if ( loopcount == 0) {
      // Clear the RX buffer !!!
      radio.flush_rx();      
#if defined(HAS_DISPLAY)
      draw_antenna(ANT_X0, ANT_Y0);
#endif
      radio.powerUp();
      radio.startListening();
      delay(10);
      payload.orderno = 0;
      payload.msg_id = ++msg_id;
      payload.data1 = calcTransportValue_f(101, cur_voltage);
#if defined(DALLAS_18B20)
      payload.data2 = calcTransportValue_f(1,temp);
      payload.data3 = 0;
      payload.data4 = 0;
      payload.data5 = 0;
      payload.data6 = 0;
#endif
#if defined(BME280)
      payload.data2 = calcTransportValue_f(1, temp);
      payload.data3 = calcTransportValue_f(2, pres);
      payload.data4 = calcTransportValue_f(3, humi);
      payload.data5 = 0;
      payload.data6 = 0;
#endif
#if defined(BMP_280)
      payload.data2 = calcTransportValue_f(1, temp);
      payload.data3 = calcTransportValue_f(2, pres);
      payload.data4 = 0;
      payload.data5 = 0;
      payload.data6 = 0;
#endif
      payload.msg_type = 51;
      receiveloopcount = 0;
      sendloopcount = 0;
      while ( sendloopcount < eeprom.sendloopcount ) {
        if ( radio.available() ) {
          sendloopcount = eeprom.sendloopcount;
          while ( receiveloopcount < eeprom.receiveloopcount ) {
            if ( radio.available() ) {
              radio.read(&payload,sizeof(payload));
              if ( payload.msg_type == 52 ) {
                receiveloopcount = eeprom.receiveloopcount;
              } else {
                if (payload.data1 >0) payload.data1 = action_loop(payload.data1);
                if (payload.data2 >0) payload.data2 = action_loop(payload.data2);
                if (payload.data3 >0) payload.data3 = action_loop(payload.data3);
                if (payload.data4 >0) payload.data4 = action_loop(payload.data4);
                if (payload.data5 >0) payload.data5 = action_loop(payload.data5);
                if (payload.data6 >0) payload.data6 = action_loop(payload.data6);
              }
              if ((payload.msg_flags & 0x01) == 0x01 ) {
                receiveloopcount = eeprom.receiveloopcount;
              }
              radio.stopListening();
              radio.write(&payload,sizeof(payload));
              radio.startListening();
              delay(RF24RECEIVEDELAY);
            }
            receiveloopcount++;
          }
        } else {
              radio.stopListening();
              radio.write(&payload,sizeof(payload));
              radio.startListening();
            delay(RF24SENDDELAY);
            last_send = 0;
        }
        sendloopcount++;
      }
    }
    radio.stopListening();
#if defined(HAS_DISPLAY)
    wipe_antenna(ANT_X0, ANT_Y0);
#endif
    radio.powerDown();
  } else {
#if defined(HAS_DISPLAY)
    display_sleep(true);
#endif
  }
  if ( sleep_kor_time != 0 ) {
    long int tempsleeptime = eeprom.sleeptime + sleep_kor_time;
#if defined(HAS_DISPLAY)
    if ( monitormode ) {
      monitor(tempsleeptime/2);
    } else {
#endif
      sleep4ms(tempsleeptime);
#if defined(HAS_DISPLAY)
    }
#endif
    sleep_kor_time=0;
  } else {
#if defined(HAS_DISPLAY)
    if ( monitormode ) {
      monitor(eeprom.sleeptime/2);
    } else {
#endif
      sleep4ms(eeprom.sleeptime);
#if defined(HAS_DISPLAY)
    }
#endif
  } 
  last_send += eeprom.sleeptime;
  loopcount++;
  if (loopcount > eeprom.emptyloopcount) loopcount=0;
}
