#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

#define BUTTON_PIN 27

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;

struct Location {
  const char* name;
  float latitude;
  float longitude;
};

Location getLocation(int number) {
  switch (number) {
    case 0:
      return {"Arizona", 33.4484, -112.0740};
    case 1:
      return {"California", 36.7783, -119.4179};
    case 2:
      return {"Florida", 27.9944, -81.7603};
    case 3:
      return {"Texas", 31.0000, -100.0000};
    case 4:
      return {"Colorado", 39.5501, -105.7821};
    case 5:
      return {"Oregon", 43.8041, -120.5542};
    case 6:
      return {"North Carolina", 35.7596, -79.0193};
    case 7:
      return {"New York", 43.0000, -75.0000};
    default:
      return {"Arizona", 33.4484, -112.0740};
  }
}

int selectedLocation = -1;

struct Plant {
  const char* name;
  const char* state;
  float minTemp;
  float maxTemp;
  float minRain;
  float maxRain;
  float minPH;
  float maxPH;
};

Plant plants[] = {
  {"Saguaro", "Arizona", 15, 40, 100, 400, 6.0, 8.5},
  {"Blue Palo Verde", "Arizona", 15, 40, 100, 500, 6.0, 8.5},
  {"Brittlebush", "Arizona", 10, 40, 100, 450, 6.0, 8.5},
  {"Desert Marigold", "Arizona", 10, 38, 100, 500, 6.0, 8.5},
  {"Ocotillo", "Arizona", 12, 40, 100, 450, 6.5, 8.0},
  {"Creosote Bush", "Arizona", 10, 42, 50, 350, 6.0, 9.0},

  {"California Poppy", "California", 10, 30, 200, 700, 5.5, 8.0},
  {"Coast Live Oak", "California", 10, 30, 300, 1200, 5.0, 7.5},
  {"Toyon", "California", 8, 30, 300, 1000, 5.0, 7.5},
  {"Blue Elderberry", "California", 5, 32, 300, 1200, 5.0, 8.0},
  {"Matilija Poppy", "California", 10, 32, 250, 600, 6.0, 7.5},
  {"Western Redbud", "California", 8, 35, 350, 900, 6.0, 8.0},

  {"Saw Palmetto", "Florida", 15, 35, 700, 1800, 4.5, 7.0},
  {"Coontie", "Florida", 15, 32, 700, 1600, 5.0, 7.5},
  {"Firebush", "Florida", 15, 35, 800, 1800, 5.0, 7.5},
  {"American Beautyberry", "Florida", 12, 35, 900, 1700, 5.0, 7.0},
  {"Sand Pine", "Florida", 10, 34, 1000, 1600, 4.0, 6.5},

  {"Texas Bluebonnet", "Texas", 10, 32, 300, 900, 6.0, 8.0},
  {"Honey Mesquite", "Texas", 10, 40, 150, 700, 6.0, 8.5},
  {"Prickly Pear", "Texas", 10, 40, 150, 700, 6.0, 8.5},
  {"Huisache", "Texas", 15, 38, 300, 1000, 6.0, 8.5},
  {"Texas Persimmon", "Texas", 8, 36, 400, 850, 6.5, 8.0},
  {"Anacacho Orchid Tree", "Texas", 12, 38, 500, 900, 6.0, 8.0},

  {"Columbine", "Colorado", 2, 24, 400, 800, 6.0, 7.5},
  {"Rocky Mountain Juniper", "Colorado", -5, 28, 300, 600, 6.0, 8.0},
  {"Aspen Sunflower", "Colorado", 0, 25, 450, 900, 5.5, 7.0},
  {"Ponderosa Pine", "Colorado", -10, 30, 350, 750, 5.0, 7.0},

  {"Oregon Grape", "Oregon", 2, 26, 700, 1500, 5.0, 6.5},
  {"Sword Fern", "Oregon", 4, 24, 900, 2000, 4.5, 6.5},
  {"Salal", "Oregon", 5, 25, 800, 1800, 4.5, 6.0},
  {"Red Columbine", "Oregon", 3, 25, 700, 1400, 5.5, 7.5},

  {"Catawba Rhododendron", "North Carolina", 5, 28, 1000, 1800, 4.5, 6.0},
  {"Flowering Dogwood", "North Carolina", 4, 30, 900, 1400, 5.5, 7.0},
  {"Sweetbay Magnolia", "North Carolina", 8, 32, 1100, 1700, 5.0, 6.5},
  {"Flame Azalea", "North Carolina", 3, 26, 1000, 1600, 4.5, 6.0},

  {"Jack-in-the-Pulpit", "New York", 2, 26, 800, 1300, 5.5, 7.0},
  {"Bloodroot", "New York", 0, 24, 750, 1200, 6.0, 7.5},
  {"Wild Ginger", "New York", 2, 25, 850, 1400, 6.0, 7.5},
  {"Trillium", "New York", 3, 24, 800, 1300, 5.0, 6.5}
};

const int plantCount = sizeof(plants) / sizeof(plants[0]);

float averageTemperature = 0;
float annualRainfall = 0;
float soilPH = 0;

bool climateAvailable = false;
bool soilAvailable = false;

void showMessage(
  String line1,
  String line2 = "",
  String line3 = ""
) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  display.println(line2);
  display.println(line3);
  display.display();
}

void showStateScreen() {
  Location location = getLocation(selectedLocation);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ASTROFORAGE");
  display.println("----------------");
  display.println("SELECTED STATE:");
  display.setTextSize(2);
  display.println(location.name);
  display.setTextSize(1);
  display.println();
  display.println("Loading data...");
  display.display();
}

void showResultsScreen(int results[], int count) {
  Location location = getLocation(selectedLocation);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(location.name);

  if (!climateAvailable || !soilAvailable) {
    display.println("API data unavailable");
    display.print("NASA: ");
    display.println(climateAvailable ? "OK" : "FAILED");
    display.print("USDA: ");
    display.println(soilAvailable ? "OK" : "FAILED");
    display.println("Check Serial Monitor");
  }
  else if (count == 0) {
    display.println("No close matches");
    display.println("Fallback:");
    display.println("Dandelion");
  }
  else {
    display.println("Plant matches:");
    for (int i = 0; i < count; i++) {
      display.print(i + 1);
      display.print(". ");
      display.println(plants[results[i]].name);
    }
  }
  display.display();
}

void connectWiFi() {
  showMessage("AstroForage", "Connecting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi...");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    showMessage("WiFi connected!", "AstroForage ready");
    delay(1000);
  }
  else {
    Serial.println();
    Serial.println("WiFi connection failed!");
    showMessage("WiFi FAILED", "Check credentials");
    delay(2000);
  }
}

bool getNASAData(Location location) {
  Serial.println();
  Serial.println("Calling NASA POWER API...");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String url =
    "https://power.larc.nasa.gov/api/temporal/daily/point?"
    "parameters=T2M,PRECTOTCORR"
    "&community=AG"
    "&longitude=" + String(location.longitude, 4) +
    "&latitude=" + String(location.latitude, 4) +
    "&start=20240101"
    "&end=20241231"
    "&format=JSON";

  Serial.println("NASA URL:");
  Serial.println(url);

  if (!http.begin(client, url)) {
    Serial.println("NASA connection failed");
    return false;
  }

  http.setTimeout(60000);
  http.useHTTP10(true);

  int responseCode = http.GET();

  if (responseCode != HTTP_CODE_OK) {
    Serial.printf("NASA HTTP error: %d\n", responseCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  Serial.print("NASA response size: ");
  Serial.println(payload.length());
  http.end();

  if (payload.length() == 0) {
    Serial.println("NASA returned empty response");
    return false;
  }

  DynamicJsonDocument doc(100000);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("NASA JSON error: ");
    Serial.println(error.c_str());
    Serial.println("Response preview:");
    Serial.println(payload.substring(0, 300));
    return false;
  }

  JsonObject parameters = doc["properties"]["parameter"];
  JsonObject temperatures = parameters["T2M"];
  JsonObject rainfall = parameters["PRECTOTCORR"];

  if (temperatures.isNull() || rainfall.isNull()) {
    Serial.println("NASA parameters missing");
    return false;
  }

  float tempSum = 0;
  int tempCount = 0;
  float rainSum = 0;
  int rainCount = 0;

  for (JsonPair item : temperatures) {
    float value = item.value().as<float>();
    if (value > -900) {
      tempSum += value;
      tempCount++;
    }
  }

  for (JsonPair item : rainfall) {
    float value = item.value().as<float>();
    if (value >= 0) {
      rainSum += value;
      rainCount++;
    }
  }

  if (tempCount == 0 || rainCount == 0) {
    Serial.println("NASA returned no usable values");
    return false;
  }

  averageTemperature = tempSum / tempCount;
  annualRainfall = rainSum;

  Serial.println();
  Serial.println("NASA DATA:");
  Serial.print("Average temperature: ");
  Serial.print(averageTemperature);
  Serial.println(" C");
  Serial.print("Annual rainfall: ");
  Serial.print(annualRainfall);
  Serial.println(" mm");

  return true;
}

String urlEncode(String input) {
  String encoded = "";
  const char hex[] = "0123456789ABCDEF";

  for (unsigned int i = 0; i < input.length(); i++) {
    uint8_t c = input[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += (char)c;
    }
    else if (c == ' ') {
      encoded += '+';
    }
    else {
      encoded += '%';
      encoded += hex[c >> 4];
      encoded += hex[c & 0x0F];
    }
  }
  return encoded;
}

bool getUSDAData(Location location) {
  Serial.println();
  Serial.println("Calling USDA Soil API...");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  const char* endpoint = "https://SDMDataAccess.sc.egov.usda.gov/Tabular/post.rest";

  String point =
    "POINT(" +
    String(location.longitude, 5) + " " +
    String(location.latitude, 5) + ")";

  String query =
    "SELECT TOP 1 ch.ph1to1h2o_r "
    "FROM component co "
    "INNER JOIN chorizon ch ON ch.cokey = co.cokey "
    "WHERE co.mukey IN "
    "(SELECT mukey FROM SDA_Get_Mukey_from_intersection_with_WktWgs84('"
    + point +
    "')) "
    "AND ch.ph1to1h2o_r IS NOT NULL "
    "ORDER BY co.comppct_r DESC, ch.hzdept_r ASC";

  Serial.println("USDA query:");
  Serial.println(query);

  if (!http.begin(client, endpoint)) {
    Serial.println("USDA connection failed");
    return false;
  }

  http.setTimeout(60000);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String body = "query=" + urlEncode(query) + "&format=JSON%2BCOLUMNNAME";

  int responseCode = http.POST(body);

  if (responseCode != HTTP_CODE_OK) {
    Serial.printf("USDA HTTP error: %d\n", responseCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  Serial.print("USDA response size: ");
  Serial.println(payload.length());
  http.end();

  if (payload.length() == 0) {
    Serial.println("USDA returned empty response");
    return false;
  }

  DynamicJsonDocument doc(10000);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("USDA JSON error: ");
    Serial.println(error.c_str());
    Serial.println("USDA response preview:");
    Serial.println(payload.substring(0, 400));
    return false;
  }

  if (
    !doc.is<JsonArray>() ||
    doc.as<JsonArray>().size() < 2 ||
    doc[1].isNull() ||
    doc[1][0].isNull()
  ) {
    Serial.println("USDA returned no soil pH");
    Serial.println("USDA response:");
    Serial.println(payload);
    return false;
  }

  if (doc[1][0].is<const char*>()) {
    soilPH = atof(doc[1][0].as<const char*>());
  }
  else {
    soilPH = doc[1][0].as<float>();
  }

  if (soilPH <= 0 || soilPH > 14 || !isfinite(soilPH)) {
    Serial.println("Invalid soil pH");
    return false;
  }

  Serial.print("USDA soil pH: ");
  Serial.println(soilPH);

  return true;
}

bool closeToRange(
  float value,
  float minimum,
  float maximum,
  float tolerance
) {
  return value >= minimum - tolerance && value <= maximum + tolerance;
}

int findBestPlants(int results[], int maxResults) {
  int resultCount = 0;
  Location location = getLocation(selectedLocation);

  for (int i = 0; i < plantCount; i++) {
    if (strcmp(plants[i].state, location.name) != 0) {
      continue;
    }

    bool matches = true;

    if (!closeToRange(averageTemperature, plants[i].minTemp, plants[i].maxTemp, 3.0)) {
      matches = false;
    }

    if (!closeToRange(annualRainfall, plants[i].minRain, plants[i].maxRain, plants[i].maxRain * 0.20)) {
      matches = false;
    }

    if (!closeToRange(soilPH, plants[i].minPH, plants[i].maxPH, 0.5)) {
      matches = false;
    }

    if (matches && resultCount < maxResults) {
      results[resultCount] = i;
      resultCount++;
    }
  }
  return resultCount;
}

void runAstroForage() {
  Location location = getLocation(selectedLocation);

  Serial.println();
  Serial.println("========================");
  Serial.println("ASTROFORAGE START");
  Serial.println("========================");
  Serial.print("Selected state: ");
  Serial.println(location.name);

  showMessage("AstroForage", location.name, "Getting API data...");

  climateAvailable = false;
  soilAvailable = false;

  if (WiFi.status() == WL_CONNECTED) {
    climateAvailable = getNASAData(location);
    soilAvailable = getUSDAData(location);
  }
  else {
    Serial.println("WiFi is disconnected");
  }

  Serial.println();
  Serial.println("--- RESULTS ---");
  Serial.print("Location: ");
  Serial.println(location.name);

  int results[3];
  int count = 0;

  if (climateAvailable && soilAvailable) {
    count = findBestPlants(results, 3);
  }

  if (!climateAvailable || !soilAvailable) {
    Serial.println("API failure: no reliable match calculated.");
  }
  else if (count == 0) {
    Serial.println("No close plant matches.");
    Serial.println("Fallback recommendation: Dandelion");
    Serial.println("Dandelion is a fallback, not necessarily native.");
  }
  else {
    Serial.println("TOP PLANT MATCHES:");
    for (int i = 0; i < count; i++) {
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.println(plants[results[i]].name);
    }
  }

  showResultsScreen(results, count);

  Serial.println("========================");
  Serial.println("ASTROFORAGE FINISHED");
  Serial.println("========================");
}

bool buttonPressed() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
    lastButtonReading = reading;
  }

  if (millis() - lastDebounceTime >= debounceDelay) {
    if (reading != stableButtonState) {
      stableButtonState = reading;
      if (stableButtonState == LOW) {
        return true;
      }
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (true) {
      delay(1000);
    }
  }

  showMessage("ASTROFORAGE", "Starting...");
  delay(1000);

  connectWiFi();

  showMessage("ASTROFORAGE", "Press button", "to choose state");

  Serial.println();
  Serial.println("AstroForage ready.");
  Serial.println("Press button to select Arizona first.");
}

void loop() {
  if (buttonPressed()) {
    selectedLocation++;
    if (selectedLocation > 7) {
      selectedLocation = 0;
    }

    Location location = getLocation(selectedLocation);

    Serial.println();
    Serial.println("New location selected:");
    Serial.println(location.name);

    showStateScreen();
    delay(500);

    runAstroForage();
  }
}
