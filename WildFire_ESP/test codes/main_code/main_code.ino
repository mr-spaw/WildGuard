#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>  // ESP32 NVS Library for persistent storage

// WiFi Credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// OpenWeatherMap API Key (Replace with your API key)
String apiKey = "YOUR_OPENWEATHER_API_KEY";
String weatherURL;

// UART Configuration
HardwareSerial GPS_UART(2);  // UART2 for GPS (RX=16, TX=17)
HardwareSerial UART3(1);     // UART3 for sending weather data (RX=18, TX=19)

// Non-Volatile Storage
Preferences preferences;

// Default (Fallback) Location
float latitude = 20.2961;   // Default Latitude (Change as needed)
float longitude = 85.8245;  // Default Longitude (Change as needed)
bool gpsConnected = false;  // Flag to check GPS connection

void setup() {
    Serial.begin(115200);    
    GPS_UART.begin(9600, SERIAL_8N1, 16, 17);  // UART2 for GPS
    UART3.begin(9600, SERIAL_8N1, 18, 19);     // UART3 for sending weather data

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nWiFi Connected!");

    // Load last known location from NVS
    preferences.begin("gps_data", false);
    latitude = preferences.getFloat("lat", latitude);
    longitude = preferences.getFloat("lon", longitude);
    preferences.end();

    Serial.printf("Loaded Last Location: Lat = %.6f, Lon = %.6f\n", latitude, longitude);
}

void loop() {
    // Read GPS Data
    if (readGPS()) {
        Serial.println("GPS Data Received!");
        saveLastLocation(latitude, longitude);  // Save new location to ROM
    } else {
        Serial.println("GPS Not Connected! Using Last Known Location.");
    }

    // Form Weather API Request
    weatherURL = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(latitude) +
                 "&lon=" + String(longitude) + "&units=metric&appid=" + apiKey;

    // Fetch Weather Data
    String weatherData = fetchWeather(weatherURL);

    // Send Weather Data via UART3
    UART3.println(weatherData);
    
    // Debug Output
    Serial.println("Sent Weather Data: " + weatherData);
    
    delay(60000); // Fetch weather every 60 seconds
}

// Function to read GPS data and update latitude & longitude
bool readGPS() {
    String gpsData = "";
    while (GPS_UART.available()) {
        char c = GPS_UART.read();
        gpsData += c;
    }

    // Assume we parse NMEA sentences to extract latitude & longitude
    // Dummy logic to check if GPS is providing valid data
    if (gpsData.indexOf("GPGGA") != -1) {  
        // Replace with actual NMEA parsing
        latitude = 12.3456;   // Parsed Latitude
        longitude = 98.7654;  // Parsed Longitude
        gpsConnected = true;
        return true;
    } else {
        gpsConnected = false;
        return false;
    }
}

// Function to save last known GPS location to ROM
void saveLastLocation(float lat, float lon) {
    preferences.begin("gps_data", false);
    preferences.putFloat("lat", lat);
    preferences.putFloat("lon", lon);
    preferences.end();
    Serial.printf("Saved Location to ROM: Lat = %.6f, Lon = %.6f\n", lat, lon);
}

// Function to fetch weather data
String fetchWeather(String url) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        http.end();
        return parseWeatherData(payload);
    } else {
        http.end();
        return "ERROR_FETCH";
    }
}

// Function to parse JSON and format essential data
String parseWeatherData(String jsonPayload) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonPayload);

    float temp = doc["main"]["temp"];
    int humidity = doc["main"]["humidity"];
    float wind_speed = doc["wind"]["speed"];
    String weather_desc = doc["weather"][0]["description"].as<String>();

    // Formatted UART3 Message
    String formattedData = "TEMP:" + String(temp) + "C|HUM:" + String(humidity) +
                           "%|WIND:" + String(wind_speed) + "m/s|DESC:" + weather_desc;

    return formattedData;
}