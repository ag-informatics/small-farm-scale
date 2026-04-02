#include "Adafruit_HX711.h"

// Define the pins for the HX711 communication
const uint8_t DATA_PIN_1 = 18;  
const uint8_t CLOCK_PIN_1 = 19; 

const uint8_t DATA_PIN_2 = 32;  
const uint8_t CLOCK_PIN_2 = 33; 

const uint8_t DATA_PIN_3 = 25;  
const uint8_t CLOCK_PIN_3 = 26; 

const uint8_t DATA_PIN_4 = 27;  
const uint8_t CLOCK_PIN_4 = 14; 

Adafruit_HX711 hx711_1(DATA_PIN_1, CLOCK_PIN_1);
Adafruit_HX711 hx711_2(DATA_PIN_2, CLOCK_PIN_2);
Adafruit_HX711 hx711_3(DATA_PIN_3, CLOCK_PIN_3);
Adafruit_HX711 hx711_4(DATA_PIN_4, CLOCK_PIN_4);

void setup() {
  Serial.begin(115200);

  // wait for serial port to connect. Needed for native USB port only
  while (!Serial) {
    delay(10);
  }

  // Initialize the HX711
  hx711_1.begin();
  hx711_2.begin();
  hx711_3.begin();
  hx711_4.begin();

  // read and toss 3 values each
  Serial.println("Tareing....");
  for (uint8_t t=0; t<3; t++) {
    hx711_1.tareA(hx711_1.readChannelRaw(CHAN_A_GAIN_128));
    hx711_1.tareA(hx711_1.readChannelRaw(CHAN_A_GAIN_128));
    
    hx711_2.tareA(hx711_2.readChannelRaw(CHAN_A_GAIN_128));
    hx711_2.tareA(hx711_2.readChannelRaw(CHAN_A_GAIN_128));

    hx711_3.tareA(hx711_3.readChannelRaw(CHAN_A_GAIN_128));
    hx711_3.tareA(hx711_3.readChannelRaw(CHAN_A_GAIN_128));

    hx711_4.tareA(hx711_4.readChannelRaw(CHAN_A_GAIN_128));
    hx711_4.tareA(hx711_4.readChannelRaw(CHAN_A_GAIN_128));
  }
} 

void loop() {
  // Read from Channel A with Gain 128, can also try CHAN_A_GAIN_64 or CHAN_B_GAIN_32
  // since the read is blocking this will not be more than 10 or 80 SPS (L or H switch)
  int32_t weight_1 = hx711_1.readChannelBlocking(CHAN_A_GAIN_128);
  Serial.print("Load Cell 1: ");
  Serial.println(weight_1);
  
  int32_t weight_2 = hx711_2.readChannelBlocking(CHAN_A_GAIN_128);
  Serial.print("Load Cell 2: ");
  Serial.println(weight_2);

  int32_t weight_3 = hx711_3.readChannelBlocking(CHAN_A_GAIN_128);
  Serial.print("Load Cell 3: ");
  Serial.println(weight_3);

  int32_t weight_4 = hx711_4.readChannelBlocking(CHAN_A_GAIN_128);
  Serial.print("Load Cell 4: ");
  Serial.println(weight_4);

  int64_t total_weight = weight_1 + weight_2 + weight_3 + weight_4;
  Serial.print("Total weight: ");
  Serial.println(total_weight);

  float total_weight_grams = total_weight / (-428.0) ;
  Serial.print("Total weight (g): ");
  Serial.println(total_weight_grams);
  
  delay(500);
  Serial.println();
}
