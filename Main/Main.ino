#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_HX711.h>
#include <WiFi.h>
#include <credential.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi information
const char *ssid = STASSID;
const char *password = STAPSK;

// AirTable credential
const char *token = AIRTABLE_KEY;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int buttonPinA = 16;
const int buttonPinB = 17;
const int buttonPinC = 5;

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

typedef enum
{
  MAINSTATE,
  UPLOADCONF,
  WEIGHTCHANGE,
  UPLOADED,
  CROPCHANGE
} State;

State state;
float weight;              // Will replace with function I assume?
String date = "2/26/2026"; // to replace
String buttonText;
String weightText;
float measureWeight;
float offset;
boolean uploaded = false;

// Buttons
const unsigned long DEBOUNCE_DELAY = 300; // in milliseconds
volatile unsigned long lastPressTimeA = 0;
volatile unsigned long lastPressTimeB = 0;
volatile unsigned long lastPressTimeC = 0;

void setup()
{
  Serial.begin(9600);
  Serial.begin(115200);
  delay(500);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.display();
  delay(1000);
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.cp437(true); // Use full 256 char 'Code Page 437' font

  pinMode(buttonPinA, INPUT_PULLUP);
  pinMode(buttonPinB, INPUT_PULLUP);
  pinMode(buttonPinC, INPUT_PULLUP);
  attachInterrupt(buttonPinA, buttonISRA, RISING);
  attachInterrupt(buttonPinB, buttonISRB, RISING);
  attachInterrupt(buttonPinC, buttonISRC, RISING);

  // Load Cell Setup
  hx711_1.begin();
  hx711_2.begin();
  hx711_3.begin();
  hx711_4.begin();

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

  offset = 0;
  state = MAINSTATE;

  //  WiFi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void loop()
{
  display.clearDisplay();
  switch (state)
  {
  case MAINSTATE:
    weigh();
    weightText = String(weight) + " lb";
    if (weightText == "-0.00 lb")
    {
      weightText = "0.00 lb";
    }
    drawMainState();
    break;
  case UPLOADCONF:
    drawUploadConf();
    break;
  case WEIGHTCHANGE:
    drawWeightChange();
    break;
  case UPLOADED:
    if (!uploaded)
    {
      upload();
      uploaded = true;
    }
    drawUploaded();
    break;
  case CROPCHANGE:
    drawCropChange();
    break;
  }
  // Display Date
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH - date.length() * 6, 0);
  display.println(date);

  // Display Button Descriptions
  display.setCursor(0, 56);
  display.println(buttonText);
  display.display();
  delay(20);
}

// Interrupt Service Routines (ISR)
// Red Button
void ARDUINO_ISR_ATTR buttonISRA()
{
  unsigned long now = millis(); // current time
  if (now - lastPressTimeA > DEBOUNCE_DELAY)
  {
    State oldstate = state;
    switch (state)
    {
    case MAINSTATE:
      offset = measureWeight;
      break;
    case UPLOADCONF:
      state = MAINSTATE;
      break;
    case WEIGHTCHANGE:
      // change weight type to type 1 idk
      state = UPLOADED;
      break;
    case UPLOADED:
      state = MAINSTATE;
      uploaded = false;
      break;
    case CROPCHANGE:
      // go up to next crop
      break;
    }
  }
  lastPressTimeA = now;
}

// Blue Button
void ARDUINO_ISR_ATTR buttonISRB()
{
  unsigned long now = millis(); // current time
  if (now - lastPressTimeB > DEBOUNCE_DELAY)
  {
    State oldstate = state;
    switch (state)
    {
    case MAINSTATE:
      state = UPLOADCONF;
      break;
    case UPLOADCONF:
      //        state = WEIGHTCHANGE;
      // Tam changed this part for dry run
      state = UPLOADED;
      break;
    case WEIGHTCHANGE:
      // change weight type to type 2 idk
      state = UPLOADED;
      break;
    case UPLOADED:
      // N/A
      break;
    case CROPCHANGE:
      state = MAINSTATE;
      break;
    }
  }
  lastPressTimeB = now;
}

// Yellow Button
void ARDUINO_ISR_ATTR buttonISRC()
{
  unsigned long now = millis(); // current time
  if (now - lastPressTimeC > DEBOUNCE_DELAY)
  {
    State oldstate = state;
    switch (state)
    {
    case MAINSTATE:
      state = CROPCHANGE;
      break;
    case UPLOADCONF:
      weight *= -1;
      break;
    case WEIGHTCHANGE:
      // change weight type to type 3 idk
      state = UPLOADED;
      break;
    case UPLOADED:
      // n/a
      break;
    case CROPCHANGE:
      // go down in the crop scrolling
      break;
    }
  }
  lastPressTimeC = now;
}

// State Drawing Functions

// DONE
void drawMainState()
{
  buttonText = "ZERO  UPLOAD  NEWCROP";
  // Display Weight
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - weightText.length() * 12) / 2, 20);
  display.println(weightText);
}

//  NEED MORE SPACE (should be fine with big screen)
void drawUploadConf()
{
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - 6 * 12) / 2, 18);
  display.println("Upload");
  display.setCursor((SCREEN_WIDTH - (weightText.length() + 1) * 12) / 2, 35);
  display.println(weightText + "?");
  buttonText = "BACK    UPLOAD    NEG";
}

// NEED INFO ON WEIGHT TYPES
void drawWeightChange()
{
  display.setTextSize(1);
  display.setCursor((SCREEN_WIDTH - 18 * 6) / 2, 20);
  display.println("Select Weight Type");
  buttonText = "TYPE1   TYPE2   TYPE3";
}

void upload()
{
  // Add upload function here for now. We should reorganize code later
  HTTPClient https;
  // The URL is Base ID / Table ID
  https.begin("https://api.airtable.com/v0/appYERzq7g8wEpx5M/tblC6ko92LRuh9Qef/");
  // Set headers with AirTable Token
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Authorization", "Bearer " + String(token));
  // Create a payload package
  StaticJsonDocument<256> payload;
  JsonObject scale = payload.createNestedObject("fields");
  scale["Wieght"] = weight;
  scale["Crop"] = "Onion";
  // Prepare data for an API call
  String requestBody;
  serializeJson(payload, requestBody);
  // Send a request
  int httpResponseCode = https.POST(requestBody);
  Serial.printf("Response: %d\n", httpResponseCode);
  https.end();
  uploaded = true;
}

// DONE
void drawUploaded()
{
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - weightText.length() * 12) / 2, 18);
  display.println(weightText);
  display.setCursor((SCREEN_WIDTH - 8 * 12) / 2, 35);
  display.println("Uploaded");
  buttonText = "CONTINUE";
}

// NEED INFO ON CROPS
void drawCropChange()
{
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - 10 * 12) / 2, 20);
  display.println("CropChange");
  buttonText = " UP    SELECT    DOWN";
}

// Weight Collecting
void weigh()
{
  int32_t weight_1 = hx711_1.readChannelBlocking(CHAN_A_GAIN_128);
  int32_t weight_2 = hx711_2.readChannelBlocking(CHAN_A_GAIN_128);
  int32_t weight_3 = hx711_3.readChannelBlocking(CHAN_A_GAIN_128);
  int32_t weight_4 = hx711_4.readChannelBlocking(CHAN_A_GAIN_128);
  int64_t total_weight = weight_1 + weight_2 + weight_3 + weight_4;
  measureWeight = total_weight / (-428.0);
  measureWeight /= 453.592;
  weight = measureWeight - offset;
}
