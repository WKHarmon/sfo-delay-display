#include <FastLED.h>
#include <ESP8266WiFi.h>
using namespace std;

const char ssid[] = "EDITME";        // your network SSID (name)
const char pass[] = "EDITME";    // your network password (use for WPA, or use as key for WEP)

#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define WIFI_TIMEOUT 60 // in seconds
#define RETRY_TIMEOUT 15000 // in ms
#define REQUEST_INTERVAL 900000 // in ms (15 miunutes is 900000)
#define BRIGHTNESS_INTERVAL 0 // interval at which to adjust brightness in ms; set to 0 to disable sensor readings

#define FASTLED_ESP8266_RAW_PIN_ORDER
#define LIGHTSENSORPIN A0 //Ambient light sensor reading 
#define DATA_PIN 5
#define LED_TYPE WS2812
#define COLOR_ORDER RGB
#define MAX_BRIGHTNESS 20
#define MIN_BRIGHTNESS 3
#define MIN_LUX 10 // not really lux, we're using a raw sensor reading, maybe 15
#define MAX_LUX 250
#define NUM_LEDS 128
CRGB leds[NUM_LEDS];

short int temp;
String conditions;

#define HOST "www.fly.faa.gov"
#define URI "/flyfaa/xmlAirportStatus.jsp"

unsigned int loop_number = 0;

void setup() {
  // put your setup code here, to run once:
  delay(3000);
  Serial.begin(74880);
  while (!Serial) {
    // wait
  }
  if (BRIGHTNESS_INTERVAL > 0) pinMode(LIGHTSENSORPIN, INPUT);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  fill_gradient_RGB(leds, NUM_LEDS, CRGB::Blue, CRGB::Red);
  FastLED.show();
}

void draw(byte digit, CRGB color, byte * image){
  unsigned short int led;
  if (digit == 0) led = 0;
  else if (digit == 1) led = 64;

  for (byte i = 0; i < 8; i++) {
    for (byte a = 0; a < 8; a++) {
      if (image[i] >> a & 0x01) leds[led] = color;
      led++;
    }
  }
}

CRGB getColor(float delay) {
  CRGB color;

  // FastLED gradients crash the ESP8266 so we have to do it manually. There's probably a math function I'm too lazy to look up.
  // http://www.perbang.dk/rgbgradient/

  /*CRGBPalette16 temp_palette = temperature_gradient;*/

  if (delay == 0) color = 0x00FF00;
  /*else if (highTemp > 90) color = 0xFF0000;
  else {
    uint8_t colorIndex = ((highTemp - 40) / 50) * 255;
    color = ColorFromPalette(temp_palette, colorIndex);
  }*/
  else if (delay <= 10) color = 0x38FF00;
  else if (delay <= 20) color = 0x71FF00;
  else if (delay <= 30) color = 0xAAFF00;
  else if (delay <= 40) color = 0xE2FF00;
  else if (delay <= 50) color = 0xFFE200;
  else if (delay <= 60) color = 0xFFE200;
  else if (delay <= 70) color = 0xFFAA00;
  else if (delay <= 80) color = 0xFF7100;
  else if (delay <= 90) color = 0xFF3800;
  else color = 0xFF0000;
  return color;
}

void doLEDs(unsigned int delay) {
  byte * first_digit;
  byte * second_digit;

  // overwrite image based on priority
  byte IMAGES[][8] = {
  {
    B00010000,
    B00110000,
    B00010000,
    B00010000,
    B00010000,
    B00010000,
    B00010000,
    B00111000
  },{
    B00111000,
    B01000100,
    B00000100,
    B00000100,
    B00001000,
    B00010000,
    B00100000,
    B01111100
  },{
    B00111000,
    B01000100,
    B00000100,
    B00011000,
    B00000100,
    B00000100,
    B01000100,
    B00111000
  },{
    B00000100,
    B00001100,
    B00010100,
    B00100100,
    B01000100,
    B01111100,
    B00000100,
    B00000100
  },{
    B01111100,
    B01000000,
    B01000000,
    B01111000,
    B00000100,
    B00000100,
    B01000100,
    B00111000
  },{
    B00111000,
    B01000100,
    B01000000,
    B01111000,
    B01000100,
    B01000100,
    B01000100,
    B00111000
  },{
    B01111100,
    B00000100,
    B00000100,
    B00001000,
    B00010000,
    B00100000,
    B00100000,
    B00100000
  },{
    B00111000,
    B01000100,
    B01000100,
    B00111000,
    B01000100,
    B01000100,
    B01000100,
    B00111000
  },{
    B00111000,
    B01000100,
    B01000100,
    B01000100,
    B00111100,
    B00000100,
    B01000100,
    B00111000
  },{
    B00111000,
    B01000100,
    B01000100,
    B01000100,
    B01000100,
    B01000100,
    B01000100,
    B00111000
  },{
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000
  }};

  if (delay >= 100) {
    first_digit = IMAGES[9];
    second_digit = IMAGES[9];
  } else if (delay < 10) {
    first_digit = IMAGES[11];
    second_digit = IMAGES[delay];
  } else {
    unsigned short int ones = (delay%10);
    unsigned short int tens = ((delay/10)%10);
    first_digit = IMAGES[tens];
    second_digit = IMAGES[ones];
  }

  CRGB color;

  color = getColor(delay);
  draw(0, color, first_digit);
  draw(1, color, second_digit);
  FastLED.show();
}

unsigned short int parseDelay(String delayString) {
  unsigned short int hours;
  unsigned short int minutes;
  if (delayString.indexOf("hour") != -1) {
    hours = delayString.substring(0, delayString.indexOf("hour") - 1).toInt();
    Serial.print(hours);
    Serial.println(" hours");
  } else hours = 0;
  if (delayString.indexOf("minute") != -1) {
    minutes = delayString.substring(delayString.indexOf("minute") - 3, delayString.indexOf("minute") -1).toInt();
    Serial.print(minutes);
    Serial.println(" minutes");
  } else minutes = 0;
  return hours * 60 + minutes;
}

int getDelay(String url) {
  uint32_t t;
  char c;

  WiFiClientSecure client;

  //Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (!client.connect(HOST, 443)) {
    Serial.println("Connection failed!");
    client.stop();
    return -1;
  } else {
  /*  Serial.println("Connected ...");
    Serial.print("GET ");
    Serial.print(url);
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(HOST);
    Serial.println("User-Agent: Weather Clock client - contact kyle@kyleharmon.com");
    Serial.println("Accept: application/ld+json");
    Serial.println("Connection: close");
    Serial.println();*/
    // Make a HTTP request, and print it to console:
    client.print("GET ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(HOST);
    client.println("User-Agent: Weather Clock client - contact kyle@kyleharmon.com");
    //client.println("Accept: application/ld+json");
    client.println("Connection: close");
    client.println();
    client.flush();
    t = millis(); // start time
    
    //Serial.print("Getting data");

    while (!client.connected()) {
      if ((millis() - t) >= (READ_TIMEOUT * 1000)) {
        Serial.println("---Timeout---");
        client.stop();
        return -1;
      }
      Serial.print(".");
      delay(1000);
    }

    String currentLine = "";
    bool readingData = false;
    bool readingMaxDelay = false;
    bool readingAvgDelay = false;
    unsigned int highestMaxDelay = 0;
    unsigned int highestAvgDelay = 0;
    String avgDelay;
    String maxDelay;

    while (client.connected()) {
      if ((c = client.read()) >= 0) {
        currentLine += c;
        if (c == '\n') {
          currentLine = "";
        }
        if (currentLine.endsWith("SFO</ARPT>")) {
          readingData = true;
        }

        // Look for max delay and save it
        if (readingData && currentLine.endsWith("<Max>")) {
          Serial.println("Found a max delay");
          readingMaxDelay = true;
        }
        if (readingMaxDelay && currentLine.endsWith("<")) {
          readingMaxDelay = false;
          unsigned int currentDelay = parseDelay(maxDelay);
          if (currentDelay > highestMaxDelay) highestMaxDelay = currentDelay;
          maxDelay = "";
        }
        if (readingMaxDelay) {
          maxDelay += c;
        } 

        // Now do the same for Average delay
        if (readingData && currentLine.endsWith("<Avg>")) {
          Serial.println("Found an avg delay");
          readingAvgDelay = true;
        }
        if (readingAvgDelay && currentLine.endsWith("<")) {
          readingAvgDelay = false;
          unsigned int currentDelay = parseDelay(avgDelay);
          if (currentDelay > highestAvgDelay) highestAvgDelay = currentDelay;
          avgDelay = "";
        }
        if (readingAvgDelay) {
          avgDelay += c;
        } 
        if (currentLine.endsWith("</Delay>")) {
          readingData = false;
          readingMaxDelay = false;
          readingAvgDelay = false;
        }
        t = millis(); // Reset timeout clock
      } else if ((millis() - t) >= (READ_TIMEOUT * 1000)) {
        Serial.println("---Timeout---");
        client.stop();
        return -1;
      }
    }
    
    client.stop();
    
    if (highestAvgDelay > 0) {
      Serial.print("Returning AVG delay of ");
      Serial.print(highestAvgDelay);
      Serial.println(" minutes");
      return highestAvgDelay;
    } else if (highestMaxDelay > 0) {
      Serial.print("Returning MAX delay of ");
      Serial.print(highestMaxDelay);
      Serial.println(" minutes");
      return highestMaxDelay;
    } else { 
      Serial.println("Returning no delay.");
      return 0;
    }
  }
  return -1;
}

void loop() {
  // set LED brightness
  if (BRIGHTNESS_INTERVAL > 0) {
    float reading;
    byte brightness;

    reading = analogRead(LIGHTSENSORPIN);

    Serial.print("Light reading: ");
    Serial.print(reading);
    Serial.print(" raw, ");

    if (reading <= MIN_LUX) brightness = 0;
    else if (reading >= MAX_LUX) brightness = MAX_BRIGHTNESS;
    else {
      // Percentage in lux range * brightness range + min brightness
      float brightness_percent = (reading - MIN_LUX) / (MAX_LUX - MIN_LUX);
      brightness = brightness_percent * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;
    }

    Serial.print(brightness);
    Serial.println(" brightness");
    FastLED.setBrightness(brightness);
    FastLED.show();
  }

    // Connect to WiFi. We always want a wifi connection for the ESP8266
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.hostname("Weather Clock " + WiFi.macAddress());
    Serial.print("WiFi connecting..");
    WiFi.begin(ssid, pass);

    int c;

    // Wait up to 1 minute for connection...
    for (c = 0; (c < WIFI_TIMEOUT) && (WiFi.status() != WL_CONNECTED); c++) {
      Serial.write('.');
      delay(1000);
    }

    if (c >= WIFI_TIMEOUT) { // If it didn't connect within WIFI_TIMEOUT
      Serial.println("Failed. Will retry...");
      return;
    }
    Serial.println("OK!");
  }

  if (loop_number > (REQUEST_INTERVAL / BRIGHTNESS_INTERVAL) || loop_number == 0 || BRIGHTNESS_INTERVAL == 0) { // Do the new request at certain intervals
    Serial.println("Starting delay request.");
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Set everything to black just in case there is no report

    // get weather for Alameda
    int delay;
    delay = getDelay(URI);
    if (delay != -1) {
      doLEDs(delay); // 0 for Alameda
    }

    FastLED.show();
    if (BRIGHTNESS_INTERVAL > 0) loop_number = 0;
  }

  if (BRIGHTNESS_INTERVAL > 0) loop_number++;
  if (BRIGHTNESS_INTERVAL > 0) delay(BRIGHTNESS_INTERVAL);
  else delay(REQUEST_INTERVAL);
}