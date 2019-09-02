#include <FastLED.h>

const int DATA_PIN = 2;
const int NUM_LEDS = 50;
const int DATA_LENGTH = NUM_LEDS * 3;
const int CHECKSUM_LENGTH = 4;
const int HEADER_LENGTH = 3;
const byte HEADER [HEADER_LENGTH] = { 0xbe, 0x31, 0xc4 };
CRGB leds[NUM_LEDS];

bool headerChecked = false;

bool checkHeader(byte bufferToCheck[HEADER_LENGTH]) {
  bool headerPasses = true;
  for (int i = 0; i < HEADER_LENGTH; i++) {
    if (bufferToCheck[i] != HEADER[i]) {
      headerPasses = false;
      break;
    }
  }
  return headerPasses;
}

int readChecksum(byte bufferToRead[4]) {
  int sum = bufferToRead[0] << 24 |
    bufferToRead[1] << 16 | 
    bufferToRead[2] <<8 | 
    bufferToRead[3];
  return sum;
}

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
}

void loop() {
  if (headerChecked) {
    if (Serial.available() >= DATA_LENGTH + CHECKSUM_LENGTH) {

      byte dataBuffer [DATA_LENGTH];
      Serial.readBytes(dataBuffer, DATA_LENGTH);

      int sum = 0;

      int ledIndex = 0;
      int colorIndex = 0;

      for (int dataIndex = 0; dataIndex < DATA_LENGTH; dataIndex++) {
        sum += dataBuffer[dataIndex];
        
        if (colorIndex == 0) {
          leds[ledIndex].r = dataBuffer[dataIndex];
          colorIndex++;
        }
        else if (colorIndex == 1) {
          leds[ledIndex].g = dataBuffer[dataIndex];
          colorIndex++;
        }
        else if (colorIndex == 2) {
          leds[ledIndex].b = dataBuffer[dataIndex];
          colorIndex = 0;
          ledIndex++;
        }
      }

      // checksum
      byte checksumBuffer[CHECKSUM_LENGTH];
      Serial.readBytes(checksumBuffer, CHECKSUM_LENGTH);
      int checksumNumber = readChecksum(checksumBuffer);
      if (checksumNumber == sum) {
        FastLED.show();
        Serial.println("Show frame!");
        headerChecked = false;
      }
      
    }
  }
  else {
    if (Serial.available() >= HEADER_LENGTH) {
      byte headerBuffer [HEADER_LENGTH];
      Serial.readBytes(headerBuffer, HEADER_LENGTH);
      if (checkHeader(headerBuffer)) {
        headerChecked = true;
      }
    }
  }
}
