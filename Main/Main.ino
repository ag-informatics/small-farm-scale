#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int buttonPinA = 16;
const int buttonPinB = 17;
const int buttonPinC = 5;

typedef enum state_t
{
  MAINSTATE,
  UPLOADCONF,
  WEIGHTCHANGE,
  UPLOADING,
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

// Buttons
const unsigned long DEBOUNCE_DELAY = 300; // in milliseconds
volatile unsigned long lastPressTimeA = 0;
volatile unsigned long lastPressTimeB = 0;
volatile unsigned long lastPressTimeC = 0;

void setup()
{
  Serial.begin(115200);

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

  setupLoadCell(); // from hx711.ino
  setupWiFi();     // from upload.ino
}

void loop()
{
  int64_t total_weight = readLoadCell(); // For calibration purpose
  display.clearDisplay();
  switch (state)
  {
  case MAINSTATE:
    weigh();
    weightText = String(weight) + " lb";
    // weightText = String(total_weight); // For debug
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
  case UPLOADING:
    drawUploading(); // There is a bug at this step.
    upload(weight);
    state = UPLOADED; // Transition to UPLOADED state after upload is done
    break;
  case UPLOADED:
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
      // Tam changed this part for dry run
      // state = WEIGHTCHANGE;
      state = UPLOADING;
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

void drawUploading()
{
  // Upload will take 2-3 seconds.
  // This state will inform users
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - 9 * 12) / 2, 20);
  display.println("Uploading");
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
  int64_t total_weight = readLoadCell(); // from hx711.ino
  measureWeight = total_weight / (-428.0);
  measureWeight /= 453.592;
  weight = measureWeight - offset;
}
