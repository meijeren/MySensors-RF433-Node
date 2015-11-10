#include <MyTransportNRF24.h>
#include <MyHwATMega328.h>
#include <MySensor.h>
#include <SPI.h>
#include <NewRemoteTransmitter.h>
#include <Wire.h>
#include <SSD1306_text.h>

#define NODE_ID          165
#define NODE_VERSION     "1.1"
#define NODE_DESCRIPTION "KakuRelay"

#define OLED_RESET 5
SSD1306_text display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define KAKU_TX_PIN 4
#define NUMBER_OF_RELAYS 8 // Total number of attached relays
typedef struct 
{
  unsigned long address;
  unsigned char unit;
  bool dimmer;
} KakuParams;
KakuParams KAKU_PARAMS[NUMBER_OF_RELAYS] =
{
  {3873602, 0, false},
  {3873602, 1, false},
  {3873602, 2, false},
  {3873602, 3, false},
  {3873602, 4, false},
  {3873602, 5, false},
  {3873601, 0, false},
  {3873601, 1, false}
};


// NRFRF24L01 radio driver (set low transmit power by default)
MyTransportNRF24 radio(RF24_CE_PIN, RF24_CS_PIN, RF24_PA_LEVEL_GW);
//MyTransportRFM69 radio;
// Message signing driver (none default)
//MySigningNone signer;
// Select AtMega328 hardware profile
MyHwATMega328 hw;
// Construct MySensors library
MySensor gw(radio, hw);

void setup()
{
  display.init();
  display.clear();
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  // Initialize library and add callback for incoming messages
  gw.begin(incomingMessage, NODE_ID, false);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  //display.display();
  //delay(2000);
  //display.clearDisplay();
    display.setTextSize(2,1);       
    display.write("Hallo ILSE");

  // text display tests
  display.setTextSize(2,1);
  //display.setTextColor(WHITE);
  display.setCursor(2, 0);
  display.print("Beterschap");
  delay(2000);
  display.setCursor(4, 0);
  display.print("Wipneus");
  display.setCursor(6, 0);
  display.print("NodeId: ");
  display.print(gw.getNodeId());
  //display.display();
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo(NODE_DESCRIPTION, NODE_VERSION);

  // Fetch relay status
  for (int sensor = 1; sensor <= NUMBER_OF_RELAYS; sensor++) {
    // Register all sensors to gw (they will be created as child devices)
    gw.present(sensor, S_LIGHT);
  }
}


void loop()
{
  // Alway process incoming messages whenever possible
  gw.process();
}

void incomingMessage(const MyMessage &message)
{
  if ((message.sensor < 1) && (message.sensor >= NUMBER_OF_RELAYS))
  {
    return;
  }
  if (message.type == V_LIGHT)
  {
    // Create a transmitter on address 123, using digital pin 11 to transmit,
    // with a period duration of 260ms (default), repeating the transmitted
    // code 2^3=8 times.
    NewRemoteTransmitter transmitter(KAKU_PARAMS[message.sensor - 1].address, KAKU_TX_PIN, 260, 3);
    transmitter.sendUnit(KAKU_PARAMS[message.sensor - 1].unit, message.getBool());
    // Store state in eeprom
    gw.saveState(message.sensor, message.getBool());
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", kAKUs: ");
    Serial.print(KAKU_PARAMS[message.sensor - 1].address);
    Serial.print(", kAKUs: ");
    Serial.print(KAKU_PARAMS[message.sensor - 1].unit);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
    display.setCursor(2, 0);
    display.print(message.sensor);
    display.print(" -> ");
    display.println(message.getBool());
//    display.display();
  }
  else if (message.type == V_DIMMER)
  {
    NewRemoteTransmitter transmitter(KAKU_PARAMS[message.sensor - 1].address, KAKU_TX_PIN, 260, 3);
    transmitter.sendDim(KAKU_PARAMS[message.sensor - 1].unit, message.getByte());

  }
}

