// Define a valid radiochannel here
#define RADIOCHANNEL 90
// This node: Use octal numbers starting with "0": "041" is child 4 of node 1
#define NODE 04
// Sleeptime during the loop in ms -> if 0 ATMega always busy
#define RADIO_CE_PIN 10
// The CS Pin of the Radio module
#define RADIO_CSN_PIN 9
//define some sleeptime as default values
// The pin of the statusled
//#define STATUSLED A2
#define STATUSLED A2
#define STATUSLED_ON HIGH
#define STATUSLED_OFF LOW
// The pin of the relais
#define RELAIS1 4
#define RELAIS2 5
#define RELAIS_ON LOW
#define RELAIS_OFF HIGH

// The outputpin for batterycontrol for the voltagedivider
#define VMESS_OUT 5
// The inputpin for batterycontrol
#define VMESS_IN A0
// Sleeptime when network is busy

// ------ End of configuration part ------------

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <sleeplib.h>
#include <Vcc.h>

ISR(WDT_vect) { watchdogEvent(); }
const float VccCorrection = 1.0/1.0;  // Measured Vcc by multimeter divided by reported Vcc
Vcc vcc(VccCorrection);

// Structure of our payload
struct payload_t {
  uint16_t  orderno;      // the orderno as primary key for our message for the nodes
  uint16_t  flags;        // a field for varies flags
                          // flags are defined as:
                          // 0x01: if set: last message, node goes into sleeptime1 else: goes into sleeptime2
  uint8_t   sensor1;      // internal address of sensor1
  uint8_t   sensor2;      // internal address of sensor2
  uint8_t   sensor3;      // internal address of sensor3
  uint8_t   sensor4;      // internal address of sensor4
  float     value1;       // value of sensor1
  float     value2;       // value of sensor2
  float     value3;       // value of sensor3
  float     value4;       // value of sensor4
};

payload_t payload;

enum radiomode_t { radio_sleep, radio_listen } radiomode = radio_sleep;
enum sleepmode_t { sleep1, sleep2, sleep3, sleep4} sleepmode = sleep1, next_sleepmode = sleep2;

RF24NetworkHeader   rxheader;
RF24NetworkHeader   txheader(0);
boolean             init_finished = false;
unsigned int        networkup = 0;
uint16_t            orderno_p1, orderno_p2;
boolean             low_voltage_flag = false;
//Some Var for restore after sleep of display
float               field1_val, field2_val, field3_val, field4_val;
float               cur_voltage;
uint8_t             n_update = 0;

// Usage: radio(CE_pin, CS_pin)
RF24 radio(RADIO_CE_PIN,RADIO_CSN_PIN);

// Network uses that radio
RF24Network network(radio);

float action_loop(unsigned char channel, float value) {
  float retval = value;
    switch (channel) {
      case 1:
        //****
        // insert here: payload.value=[result from sensor]
       break;
      case 21:
        if ( value > 0.5 ) {
          digitalWrite(RELAIS1, RELAIS_ON);
        } else {
          digitalWrite(RELAIS1, RELAIS_OFF);
        }
       break;
      case 22:
        if ( value > 0.5 ) {
          digitalWrite(RELAIS2, RELAIS_ON);
        } else {
          digitalWrite(RELAIS2, RELAIS_OFF);
        }
       break;
      case 31:
        //****
        // insert here: action = payload.value
        // Switch the StatusLED ON or OFF
        if ( value > 0.5 ) {
          digitalWrite(STATUSLED,STATUSLED_ON);
        } else {
          digitalWrite(STATUSLED,STATUSLED_OFF);
        }
       break;
      case 101:
      // battery voltage
        retval = cur_voltage;
        break;
      case 116:
      // Voltage devider
        vcc.m_correction = value;
        break;
      case 118:
      // init_finished (=1)
        init_finished = ( value > 0.5);
        break;
//      default:
      // Default: just send the paket back - no action here  
    }  
    return retval;
}  

void setup(void) {
  unsigned long last_send=millis();
  pinMode(STATUSLED, OUTPUT);
  pinMode(RELAIS1, OUTPUT);
  pinMode(RELAIS2, OUTPUT);
  digitalWrite(STATUSLED,STATUSLED_ON);
  digitalWrite(RELAIS1,RELAIS_ON);
  digitalWrite(RELAIS2,RELAIS_ON);
  SPI.begin();
  radio.begin();
  cur_voltage = vcc.Read_Volts();
  network.begin(RADIOCHANNEL, NODE);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  // initialisation beginns
  // initialisation beginns
  delay(1000);
  digitalWrite(STATUSLED,STATUSLED_OFF); 
  digitalWrite(RELAIS1,RELAIS_OFF);
  digitalWrite(RELAIS2,RELAIS_OFF);
}

void loop(void) {
  network.update();
  if ( network.available() ) {
    network.read(rxheader,&payload,sizeof(payload));
    payload.value1 = action_loop(payload.sensor1, payload.value1);
    payload.value2 = action_loop(payload.sensor2, payload.value2);
    payload.value3 = action_loop(payload.sensor3, payload.value3);
    payload.value4 = action_loop(payload.sensor4, payload.value4);
    txheader.type=rxheader.type;
    network.write(txheader,&payload,sizeof(payload));    
  } 
  delay(50);
}
