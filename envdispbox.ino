#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "WiFi.h"
#include <stdlib.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <HTTPClient.h>
#include <time.h>
//#include "configtemplate.h"   // Create a copy of this and name it config.h, put your config in there
#include "config.h"             // This is configured to be ignored by git

String authBearer = "";
String temperature = "";
String humidity = "";
String lastUpdatedDate = "";
String lastUpdatedTime = "";
bool equalised = false;

#define US_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1800        /* Time ESP32 will go to sleep (in seconds) */
#define WIFI_CONNECT_TIMEOUT_SECONDS 20

void setup() {
  printf("Initialising ESP32...\r\n");
  DEV_Module_Init(); 
  connectWiFi();    //If this fails to connect after allowed timeout, it will put the ESP32 into deep sleep
  updateSensors(); 
  updateDisplay();  
  deepSleep();
}

void deepSleep()
{
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * US_TO_S_FACTOR);
  esp_deep_sleep_start();  
}

void connectWiFi()
{
  printf("Connecting WiFi...\r\n");
  unsigned long startedAt = millis();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    unsigned long elapsed = millis() - startedAt;
    if(elapsed > 0 && elapsed / 1000 >= WIFI_CONNECT_TIMEOUT_SECONDS)
    {
      //Failed to connect to wifi after timeout period, put back to sleep
      deepSleep();
    }
    
    printf("Connecting to WiFi..\r\n");
  }
  printf("Connected to the WiFi network\r\n");  
}

void updateSensors() {
  HTTPClient http;
  authBearer = "Bearer " + accessToken;

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
  printf("Getting entity float state: ");
  printf(entityId.c_str());
  printf("\r\n");  
  int statusCode = 0;

  String tempUrl = statesEndpoint + entityId;
  http->begin(tempUrl, root_ca);
  AddAuthHeader(http, authBearer);
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
    printf("Retrieved entity float state: ");
    printf(state.c_str());
    printf("\r\n");    
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
  printf("Getting entity bool state: ");
  printf(entityId.c_str());
  printf("\r\n");
  int statusCode = 0;

  String tempUrl = statesEndpoint + entityId;
  http->begin(tempUrl, root_ca);
  AddAuthHeader(http, authBearer);
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

    String state = doc["state"]; //!!! this isn't correct, [attributes][current]? (0/1)
    printf("Retrieved entity bool state: ");
    printf(state.c_str());
    printf("\r\n");
    return state == "on";
  }
  else
  {
    printf("Error on HTTP request\r\n");
  }

  return false;
}

void updateDisplay() {
  printf("Updating display\r\n");

  printf("Initialising display...\r\n");
  EPD_2IN13_V2_Init(EPD_2IN13_V2_FULL);

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
}

void AddAuthHeader(HTTPClient* http, String authBearer)
{
  http->addHeader(F("Authorization"), authBearer);  
}

void loop() {
  // Don't need to do anything here as we want the ESP32 to deep sleep straight after each update for the specified period
}
