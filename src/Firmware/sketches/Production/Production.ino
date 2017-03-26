#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/*
 * Connection configuration
 *
 */

#define FIRMWARE_VERSION                        "1.0.0"

#define WIFI_SSID                               ""
#define WIFI_PASSWORD                           ""
#define WIFI_CONNECTION_RETRIES                 20

#define OPENHAB_REQUESTPATH                     "https://10.0.0.3/CMD?GroupName=TOGGLE"
#define OPENHAB_SERVER_TLS_FINGERPRINT          "XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX"
#define OPENHAB_USERNAME                        ""
#define OPENHAB_PASSWORD                        ""
#define OPENHAB_REQUEST_TIMEOUT                 5000

#define PIN_STATUSLED                           2

HTTPClient httpClient;
int connectionFailCouter = WIFI_CONNECTION_RETRIES;

void setup()
{
    Serial.begin(115200);

    setupPins();
    setupWifi();

    const bool wasSuccessful = sendRequest();

    if (wasSuccessful)
    {
        blinkStatusSendingRequest();
    }
    else
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

    bool wasSuccessful = true;

    while (WiFi.status() != WL_CONNECTED)
    {
        if (connectionFailCouter <= 0)
        {
            wasSuccessful = false;
            break;
        }

        Serial.print(".");

        blinkStatusConnecting();
        delay(300);

        connectionFailCouter--;
    }

    Serial.println();

    if (wasSuccessful)
    {
        Serial.print("Connected to Wi-Fi access point. Obtained IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.printf("Connection failed after %i tries! Giving up.\n", WIFI_CONNECTION_RETRIES);
        blinkStatusError();
        startDeepSleep();
    }
}

bool sendRequest()
{
    bool wasSuccessful = false;

    Serial.println("Sending request...");

    httpClient.begin(OPENHAB_REQUESTPATH, OPENHAB_SERVER_TLS_FINGERPRINT);
    httpClient.setAuthorization(OPENHAB_USERNAME, OPENHAB_PASSWORD);

    httpClient.setTimeout(OPENHAB_REQUEST_TIMEOUT);

    const int httpCode = httpClient.GET();

    if (httpCode > 0)
    {
        Serial.printf("Request successful with HTTP code: %i\n", httpCode);

        if (httpCode == HTTP_CODE_OK)
        {
            wasSuccessful = true;

            const String payload = httpClient.getString();
            Serial.println("Server response:");
            Serial.println(payload);
        }
    }
    else
    {
        Serial.printf("Request failed (error: '%s', error code: %i)!\n", httpClient.errorToString(httpCode).c_str(), httpCode);
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

void blinkStatusConnecting()
{
    blinkStatusLED(2, 100);
}

void blinkStatusSendingRequest()
{
    for (int i = 4; i < 50; i = (5*i) >> 2)
    {
        digitalWrite(PIN_STATUSLED, HIGH);
        delay(10 * i);

        digitalWrite(PIN_STATUSLED, LOW);
        delay(10 * i);
    }
}

void blinkStatusError()
{
    blinkStatusLED(10, 125);
}

void blinkStatusLED(const int times, const int waitDelayInMilliseconds)
{
    for (int i = 0; i < times; i++)
    {
        digitalWrite(PIN_STATUSLED, HIGH);
        delay(waitDelayInMilliseconds);

        digitalWrite(PIN_STATUSLED, LOW);
        delay(waitDelayInMilliseconds);
    }
}

void loop()
{
    // Nothing here
}
