#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class ESP32SoundCloud{
    public:
        ESP32SoundCloud(const char* client_id, const char* client_secret);
        void begin();
        void beginWithSetToken(char* token, char* refresh, int expires);
        JsonDocument searchPlaylist(char* query, unsigned int limit, bool show_tracks = false);
        JsonDocument searchTrack(char* query, unsigned int limit);
        JsonDocument searchUser(char* query, unsigned int limit);
        JsonDocument getTrack(char* track_urn);
        JsonDocument streamTrack(char* track_urn);
        JsonDocument getPlaylist(char* playlist_urn, bool show_tracks = true);
        JsonDocument userTracks(char* user_urn, unsigned int limit);
        JsonDocument userLikedTracks(char* user_urn, unsigned int limit);
        JsonDocument customGet(char* url);
        String urlEncode(String str);

    private:
        JsonDocument getFromURL();
        bool checkToken();
        bool renewToken();
        bool createToken();
        WiFiClientSecure client;
        HTTPClient http;
        char* accessToken;
        char* refreshToken;
        const char* clientID;
        const char* clientSecret;
        unsigned long expirationTime;
        char postDataBuffer[512];
        char getURLBuffer[512];
};
