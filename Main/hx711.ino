#include <Adafruit_HX711.h> // v.1.0.2 by Adrafruit

// Load Cell Pins
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

void setupLoadCell()
{
    // Load Cell Setup
    hx711_1.begin();
    hx711_2.begin();
    hx711_3.begin();
    hx711_4.begin();

    // Tare the scales
    for (uint8_t t = 0; t < 3; t++)
    {
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

int64_t readLoadCell()
{
    // Read each load cell and calculate total weight
    int32_t weight_1 = hx711_1.readChannelBlocking(CHAN_A_GAIN_128);
    int32_t weight_2 = hx711_2.readChannelBlocking(CHAN_A_GAIN_128);
    int32_t weight_3 = hx711_3.readChannelBlocking(CHAN_A_GAIN_128);
    int32_t weight_4 = hx711_4.readChannelBlocking(CHAN_A_GAIN_128);

    return weight_1 + weight_2 + weight_3 + weight_4;
}