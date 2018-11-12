/*
Example, transmit all received ArtNet messages (DMX) out of the serial port in plain text.

Stephan Ruloff 2016
https://github.com/rstephan

*/

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiUdp.h>
#include <ArtnetWifi.h>

#include <FastLED.h> 

//Wifi settings
const char* ssid = "cobbler";
const char* password = "nevermind";

// LED Strip
//const int numLeds = 64; // CHANGE if your setup has more or less LED's
#define NUM_LEDS 10
const int numberOfChannels = NUM_LEDS * 3; // Total number of DMX channels you want to receive (1 led = 3 channels)
#define DATA_PIN 12 //CHANGE according to dotstar wiring
#define CLOCK_PIN 13 // CHANGE according to wiring
CRGB leds[NUM_LEDS];

#define DEBUG true

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0;

bool sendFrame = 1;
int previousDataLength = 0;

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("My IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{

  if (DEBUG) {
    boolean tail = false;

    Serial.print("DMX: Univ: ");
    Serial.print(universe, DEC);
    Serial.print(", Seq: ");
    Serial.print(sequence, DEC);
    Serial.print(", Data (");
    Serial.print(length, DEC);
    Serial.print("): ");

    if (length > 16) {
      length = 16;
      tail = true;
    }
    // send out the buffer
    for (int i = 0; i < length; i++)
    {
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    if (tail) {
      Serial.print("...");
    }
    Serial.println();
  }
  sendFrame = 1;
  // set brightness of the whole strip
  FastLED.setBrightness(64);
  
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < NUM_LEDS)
    {
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
  previousDataLength = length;
  FastLED.show();
}


void paintColor(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    // Turn the LED on, then pause
    leds[i] = color;
  }
  FastLED.show();
}

void setup()
{
  Serial.begin(115200);
  ConnectWifi();
  artnet.begin();
  // FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, numLeds);
  FastLED.addLeds<DOTSTAR, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

  // Turn the LED on, then pause
  paintColor(CRGB::Red);
  delay(500);
  paintColor(CRGB::Green);
  delay(500);
  paintColor(CRGB::Blue);
  delay(500);
  // Now turn the LED off, then pause
  paintColor(CRGB::Black);
  delay(500);

  // onDmxFrame will execute every time a packet is received by the ESP32
  artnet.setArtDmxCallback(onDmxFrame);
}


void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
