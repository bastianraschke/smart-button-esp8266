#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/*
 * Connection configuration
 *
 */

#define FIRMWARE_VERSION                        "1.0.0"

#define WIFI_SSID                               ""
#define WIFI_PASSWORD                           ""

#define OPENHAB_REQUESTPATH                     "https://10.0.0.3/CMD?Group_All_LightSwitches=TOGGLE"
#define OPENHAB_SERVER_TLS_FINGERPRINT          ""
#define OPENHAB_USERNAME                        ""
#define OPENHAB_PASSWORD                        ""


#define UPDATE_PATH                             "/update"
#define UPDATE_USERNAME                         "admin"
#define UPDATE_PASSWORD                         ""

#define PIN_STATUSLED                           2

HTTPClient httpClient;

void setup()
{
    Serial.begin(115200);
    delay(100);

    setupPins();
    setupWifi();

    const bool wasSuccessful = sendRequest();

    if (wasSuccessful == false)
    {
        blinkStatusError();
    }
    
    startDeepSleep();
}

void setupPins()
{
    pinMode(PIN_STATUSLED, OUTPUT);
}

void setupWifi()
{
    Serial.printf("Connecting to to Wi-Fi access point '%s'...\n", WIFI_SSID);

    // Do not store Wi-Fi config in SDK flash area
    WiFi.persistent(false);

    // Disable auto Wi-Fi access point mode
    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        blinkStatusConnecting();

        delay(500);
        Serial.print(F("."));
    }

    Serial.println();

    Serial.print(F("Connected to Wi-Fi access point. Obtained IP address: "));
    Serial.println(WiFi.localIP());
}

bool sendRequest()
{
    bool wasSuccessful = false;

    Serial.println("Sending request...");

    blinkStatusSendingRequest();

    httpClient.begin(OPENHAB_REQUESTPATH, OPENHAB_SERVER_TLS_FINGERPRINT);
    httpClient.setAuthorization(OPENHAB_USERNAME, OPENHAB_PASSWORD);

    // httpClient.setTimeout();

    const int httpCode = httpClient.GET();

    if (httpCode > 0)
    {
        Serial.printf("Request successful with HTTP code: %i\n", httpCode);

        if (httpCode == HTTP_CODE_OK)
        {
            wasSuccessful = true;

            String payload = httpClient.getString();
            Serial.println("Server response:");
            Serial.println(payload);
        }
    }
    else
    {
        Serial.printf("Request failed (error: '%s', error code %i)!\n", httpClient.errorToString(httpCode).c_str(), httpCode);
    }

    httpClient.end();

    return wasSuccessful;
}

void startDeepSleep()
{
    Serial.println("Shutting down. Going to deep sleep...");
    ESP.deepSleep(0);

    Serial.println("Deep sleep failed!");

    while(true)
    {
        blinkStatusError();
    }
}

void blinkStatusLED(const int times, const int waitDelayInMilliseconds)
{
    for (int i = 0; i < times; i++)
    {
        // Enable LED
        digitalWrite(PIN_STATUSLED, LOW);
        delay(waitDelayInMilliseconds);

        // Disable LED
        digitalWrite(PIN_STATUSLED, HIGH);
        delay(waitDelayInMilliseconds);
    }
}

void blinkStatusConnecting()
{
    blinkStatusLED(2, 100);
}

void blinkStatusSendingRequest()
{
    for (int i = 4; i < 50; i=(5*i) >> 2) {
        digitalWrite(PIN_STATUSLED, HIGH);   // turn the LED off
        delay(10*i);               // wait
        digitalWrite(PIN_STATUSLED, LOW);    // turn the LED on
        delay(10*i);               // wait
    }
}

void blinkStatusError()
{
    blinkStatusLED(10, 125);
}

void loop()
{
    // Nothing here
}
