#include <WiFi.h>
#include <ESP32SoundCloud.h>
#include <ArduinoJson.h>

#define WIFI_SSID
#define WIFI_PASS
#define SC_CLIENT_ID
#define SC_CLIENT_SECRET

ESP32SoundCloud sc(SC_CLIENT_ID, SC_CLIENT_SECRET);

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    while (Serial.readStringUntil('\n').length() == 0) {
        delay(10);
    }
    sc.beginWithSetToken();
}
JsonDocument result;

void loop(){
    String input = "";
    while (true) {
        int c = Serial.read();
        if (c < 0) {
            delay(10);
            continue;
        }
        Serial.write(c);
        if (c == '\n') break;
        input += (char)c;
    }
    if (input.length() > 0) {
        input.remove(input.length() - 1);
    }
    char* command = strdup(input.c_str());
    if(strncmp(command, "track ", 6) == 0) {
        char* query = command + 6;
        Serial.print("query: ");
        Serial.println(query);
        result = sc.searchTrack(query, 3);
    } else if(strncmp(command, "user ", 5) == 0) {
        char* query = command + 5;
        Serial.print("query: ");
        Serial.println(query);
        result = sc.searchUser(query, 3);
    }
    Serial.println();
    serializeJsonPretty(result, Serial);
    Serial.println();
}
