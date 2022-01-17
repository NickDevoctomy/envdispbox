const char ssid[] PROGMEM = "WIFI SSID GOES HERE";
const char password[] PROGMEM =  "WIFI PASSWORD GOES HERE";
const String accessToken = "HOMEASSISTANT LONG LIVED ACCESS TOKEN GOES HERE";
const char root_ca[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"-----END CERTIFICATE-----\n";  //REPLACE THIS WITH YOUR HOMEASSISTANT ROOT CA CERT
const String tempEntityId PROGMEM = "sensor.temperature";
const String humidityEntityId PROGMEM = "sensor.humidity";
const String equalisedEntityId PROGMEM = "automation.equalised";
const String statesEndpoint PROGMEM = "HOME ASSISTANT URL & STATES ENDPOINT GOES HERE";
const String roomName PROGMEM = "ROOM NAME GOES HERE";
const String description PROGMEM = "SHORT DESCRIPTION GOES HERE";