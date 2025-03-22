/*
API KEY->0f04193675eede9b72a706f355101b36
String Format->https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={API key}*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


// WiFi credentials
const char* ssid = "iPhone";
const char* password = "123456789";

// OpenWeatherMap API credentials
const char* apiKey = "0f04193675eede9b72a706f355101b36";  // Get from OpenWeatherMap
const float latitude = 20.2960;  
const float longitude = 85.8246;

String weatherURL = "http://api.openweathermap.org/data/2.5/weather?lat=" + 
                    String(latitude) + "&lon=" + String(longitude) + 
                    "&appid=" + apiKey + "&units=metric"; 

void getWeatherData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(weatherURL);  // Set URL
        int httpCode = http.GET();  // Make GET request
        
        if (httpCode > 0) {  // Check for a successful request
            String payload = http.getString();
            Serial.println("Weather Data:");
            Serial.println(payload);

            // Parse JSON
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                float temp = doc["main"]["temp"];
                int humidity = doc["main"]["humidity"];
                const char* description = doc["weather"][0]["description"];
                
                Serial.print("Temperature: ");
                Serial.print(temp);
                Serial.println("°C");
                
                Serial.print("Humidity: ");
                Serial.print(humidity);
                Serial.println("%");
                
                Serial.print("Description: ");
                Serial.println(description);
            } else {
                Serial.println("Failed to parse JSON");
            }
        } else {
            Serial.print("HTTP request failed, error: ");
            Serial.println(http.errorToString(httpCode));
        }
        
        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    
    getWeatherData();

}

void loop() {
    delay(60000);  
    getWeatherData();
}
