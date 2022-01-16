#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
//#include "imagedata.h"
#include "WiFi.h"
#include <stdlib.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <HTTPClient.h>
#include <time.h>

const char ssid[] PROGMEM = "WIFI SSID GOES HERE";
const char password[] PROGMEM =  "WIFI PASSWORD GOES HERE";
const String accessToken = "HOMEASSISTANT LONG LIVED ACCESS TOKEN GOES HERE";
const char root_ca[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";  //REPLACE THIS WITH YOUR HOMEASSISTANT ROOT CA CERT
const String tempEntityId PROGMEM = "sensor.bedroom_environment_temperature";
const String humidityEntityId PROGMEM = "sensor.bedroom_environment_humidity";
const String equalisedEntityId PROGMEM = "automation.bedroom_humidity_equalised";
const String statesEndpoint PROGMEM = "HOME ASSISTANT URL & STATES ENDPOINT GOES HERE";
const String roomName PROGMEM = "MASTER BEDROOM";
const String description PROGMEM = "Environment Sensor";
String temperature = "";
String humidity = "";
String lastUpdatedDate = "";
String lastUpdatedTime = "";
bool equalised = false;

#define US_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1800        /* Time ESP32 will go to sleep (in seconds) */

void setup() {
  printf("Initialising ESP32...\r\n");
  DEV_Module_Init();
  
  //clearDisplay();
  
  printf("Connecting WiFi...\r\n");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    printf("Connecting to WiFi..\r\n");
  }
  printf("Connected to the WiFi network\r\n");

  updateSensors();

  printf("Initialising display...\r\n");
  EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);
  updateDisplay();
  //sleepDisplay();
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * US_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void updateSensors() {
  HTTPClient http;

  float temp = GetEntityStateFloat(&http, tempEntityId, true);
  temperature = temp;
  temperature += "C";
  
  float humid = GetEntityStateFloat(&http, humidityEntityId, false);
  humidity = humid;
  humidity += "%";

  equalised = GetEntityStateBool(&http, equalisedEntityId, false);
  
  http.end();
}

float GetEntityStateFloat(HTTPClient* http, String entityId, bool updateLastUpdated)
{
  String authHeader = "Bearer " + accessToken;
  int statusCode = 0;

  String tempUrl = statesEndpoint + entityId;
  http->begin(tempUrl, root_ca);
  http->addHeader(F("Authorization"), authHeader);
  statusCode = http->GET();
  if(statusCode == 200)
  {
    String payload = http->getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      printf("deserializeJson() failed: ");
      printf(error.c_str());
      printf("\r\n");
      return 0;
    }

    if(updateLastUpdated)
    {
      String updated = doc["last_updated"];
      struct tm tm = {0};
      strptime(updated.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
      
      char dateStringBuff[50]; //50 chars should be enough
      strftime(dateStringBuff, sizeof(dateStringBuff), "%d-%m-%Y", &tm);
      lastUpdatedDate = dateStringBuff;

      char timeStringBuff[50]; //50 chars should be enough
      strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &tm);
      lastUpdatedTime = timeStringBuff;
  }
        
    String state = doc["state"];
    return state.toFloat();
  }
  else
  {
    printf("Error on HTTP request\r\n");
  }

  return 0;
}

bool GetEntityStateBool(HTTPClient* http, String entityId, bool updateLastUpdated)
{
  String authHeader = "Bearer " + accessToken;
  int statusCode = 0;

  String tempUrl = statesEndpoint + entityId;
  http->begin(tempUrl, root_ca);
  http->addHeader(F("Authorization"), authHeader);
  statusCode = http->GET();
  if(statusCode == 200)
  {
    String payload = http->getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      printf("deserializeJson() failed: ");
      printf(error.c_str());
      printf("\r\n");      
      return false;
    }

    if(updateLastUpdated)
    {
      String updated = doc["last_updated"];
      struct tm tm = {0};
      strptime(updated.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
      
      char dateStringBuff[50]; //50 chars should be enough
      strftime(dateStringBuff, sizeof(dateStringBuff), "%d-%m-%Y", &tm); //%H:%M:%S
      lastUpdatedDate = dateStringBuff;

      char timeStringBuff[50]; //50 chars should be enough
      strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &tm);
      lastUpdatedTime = timeStringBuff;
    }

    String state = doc["state"];
    return state == "on";
  }
  else
  {
    printf("Error on HTTP request\r\n");
  }

  return false;
}

void clearDisplay() {
  printf("Clearing display\r\n");
  EPD_2IN13_V2_Clear();
  DEV_Delay_ms(2000);  
}

void updateDisplay() {
  printf("Updating display\r\n");

  UBYTE *BlackImage;
  UWORD Imagesize = ((EPD_2IN13_V2_WIDTH % 8 == 0) ? (EPD_2IN13_V2_WIDTH / 8 ) : (EPD_2IN13_V2_WIDTH / 8 + 1)) * EPD_2IN13_V2_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    printf("Failed to apply for black memory...\r\n");
    while (1);
  }

  printf("Paint_NewImage\r\n");
  Paint_NewImage(BlackImage, EPD_2IN13_V2_WIDTH, EPD_2IN13_V2_HEIGHT, 270, WHITE);
  Paint_SelectImage(BlackImage);
  Paint_SetMirroring(MIRROR_HORIZONTAL); //
  Paint_Clear(WHITE);

  printf("Drawing\r\n");
  Paint_SelectImage(BlackImage);
  Paint_Clear(WHITE);

  Paint_DrawString_EN(8, 8, roomName.c_str(), &Font16, WHITE, BLACK);
  Paint_DrawString_EN(8, 20, description.c_str(), &Font12, WHITE, BLACK);
  Paint_DrawLine(8, 34, 245, 34, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

  Paint_DrawString_EN(8, 45, temperature.c_str(), &Font24, WHITE, BLACK);
  Paint_DrawString_EN(8, 65, humidity.c_str(), &Font20, WHITE, BLACK);

  Paint_DrawLine(8, 90, 245, 90, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
  Paint_DrawString_EN(8, 98, lastUpdatedDate.c_str(), &Font12, WHITE, BLACK);
  Paint_DrawString_EN(8, 110, lastUpdatedTime.c_str(), &Font12, WHITE, BLACK);

  if(equalised)
  {
    Paint_DrawString_EN(200, 100, "[E]", &Font20, WHITE, BLACK);
  }

  EPD_2IN13_V2_Display(BlackImage);
  DEV_Delay_ms(2000);
}

void sleepDisplay() {
  printf("Putting display to sleep\r\n");
  EPD_2IN13_V2_Sleep();
}

void loop() {
  
}
