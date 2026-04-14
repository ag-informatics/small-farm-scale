#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <credential.h>

// From credential.h, which is not uploaded to GitHub for security reasons.
// Please create this file with the following content:
// WiFi information
const char *ssid = STASSID;
const char *password = STAPSK;
// AirTable credential
const char *token = AIRTABLE_KEY;

void setupWiFi()
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    // Print the local IP address to confirm connection
    Serial.println(WiFi.localIP());
}

void upload(float weight)
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
}