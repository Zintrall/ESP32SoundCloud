#include "ESP32SoundCloud.h"

ESP32SoundCloud::ESP32SoundCloud(const char* client_id, const char* client_secret){
    clientID = client_id;
    clientSecret = client_secret;
}

void ESP32SoundCloud::begin(){
    client.setInsecure();
    createToken();
    return;
}

bool ESP32SoundCloud::checkToken(){
    while (WiFi.status() != WL_CONNECTED);
    if ((expirationTime - millis())>0)return true;
    if (renewToken()) return true;
    return createToken();
}

bool ESP32SoundCloud::renewToken(){
    while (WiFi.status() != WL_CONNECTED);
    snprintf(postDataBuffer, sizeof(postDataBuffer),
                "grant_type=refresh_token&client_id=%s&client_secret=%s&refresh_token=%s",
                clientID, clientSecret, refreshToken);
    http.begin(client, "https://api.soundcloud.com/oauth2/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(postDataBuffer);
    if (httpResponseCode == 200) {
            String response = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, response);
            if (error) {
                Serial.print("JSON parsing failed: ");
                Serial.println(error.c_str());
                http.end();
                return false;
            }
            if (accessToken) free(accessToken);
            if (refreshToken) free(refreshToken);

            accessToken = strdup(doc["access_token"]);
            refreshToken = strdup(doc["refresh_token"]);
            expirationTime = millis() + ((doc["expires_in"].as<long>()-10) * 1000);
            Serial.println("Token refreshed successfully!");
            http.end();
            Serial.println(accessToken);
            Serial.println(refreshToken);
            Serial.println(expirationTime);
            return true;
    }
    http.end();
    return false;
}

bool ESP32SoundCloud::createToken(){
    while (WiFi.status() != WL_CONNECTED);
    snprintf(postDataBuffer, sizeof(postDataBuffer),
                "grant_type=client_credentials&client_id=%s&client_secret=%s",
                clientID, clientSecret);
    http.begin(client, "https://api.soundcloud.com/oauth2/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(postDataBuffer);
    if (httpResponseCode == 200) {
            String response = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, response);
            if (error) {
                Serial.print("JSON parsing failed: ");
                Serial.println(error.c_str());
                http.end();
                return false;
            }
            if (accessToken) free(accessToken);
            if (refreshToken) free(refreshToken);

            accessToken = strdup(doc["access_token"]);
            refreshToken = strdup(doc["refresh_token"]);
            expirationTime = millis() + ((doc["expires_in"].as<long>()-10) * 1000);
            Serial.println("Token created successfully!");
            http.end();
            Serial.println(accessToken);
            Serial.println(refreshToken);
            Serial.println(expirationTime);
            return true;
    }
    http.end();
    return false;
}

void ESP32SoundCloud::beginWithSetToken(char* token, char* refresh, int expires){
    client.setInsecure();
    accessToken = token;
    refreshToken = refresh;
    expirationTime = millis() + ((expires-10) * 1000);
}

char* ESP32SoundCloud::urlEncode(const String& str) {
    const char* unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    size_t outIndex = 0;
    for (size_t i = 0; i < str.length() && outIndex < sizeof(encode) - 1; i++) {
        char c = str.charAt(i);
        if (strchr(unreserved, c)) {
            if (outIndex < sizeof(encode) - 1) {
                encode[outIndex++] = c;
            }
        } else {
            if (outIndex < sizeof(encode) - 4) {
                snprintf(encode + outIndex, 4, "%%%02X", (unsigned char)c);
                outIndex += 3;
            } else {
                break;
            }
        }
    }
    encode[outIndex] = '\0';
    return encode;
}


JsonDocument ESP32SoundCloud::getFromURL() {
    checkToken();

    http.begin(client, getURLBuffer);

    http.addHeader("Authorization", String("Bearer ")+String(accessToken));

    int httpResponseCode = http.GET();
    JsonDocument doc;

    if (httpResponseCode == 200) {
        String response = http.getString();
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.print("JSON parsing failed: ");
            Serial.println(error.c_str());
        }
    } else {
        Serial.printf("HTTP Error: %d\n", httpResponseCode);
    }

    http.end();
    return doc;
}

JsonDocument ESP32SoundCloud::searchPlaylist(char* query, unsigned int limit, bool show_tracks){
    strcpy(getURLBuffer, "");
    const char* tracksStr = show_tracks ? "true" : "false";
    if (limit == 0){
        snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/playlists?q=%s&show_tracks=%s&linked_partitioning=true",
                 urlEncode(String(query)), tracksStr);
    }
    else{
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/playlists?q=%s&show_tracks=%s&limit=%u&linked_partitioning=true",
             urlEncode(String(query)), tracksStr, limit);}
    return getFromURL();
}

JsonDocument ESP32SoundCloud::searchTrack(char* query, unsigned int limit){
    strcpy(getURLBuffer, "");
    if (limit == 0){
        snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/tracks?q=%s&linked_partitioning=true",
                 urlEncode(String(query)));
    }
    else{
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/tracks?q=%s&limit=%u&linked_partitioning=true",
             urlEncode(String(query)), limit);}
    return getFromURL();
}

JsonDocument ESP32SoundCloud::searchUser(char* query, unsigned int limit){
    strcpy(getURLBuffer, "");
    if (limit == 0){
        snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/users?q=%s&linked_partitioning=true",
                 urlEncode(String(query)));
    }
    else{
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/users?q=%s&limit=%u&linked_partitioning=true",
             urlEncode(String(query)), limit);}
    return getFromURL();
}

JsonDocument ESP32SoundCloud::getTrack(char* track_urn){
    strcpy(getURLBuffer, "");
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/tracks/%s",
             urlEncode(String(track_urn)));
    return getFromURL();
}

JsonDocument ESP32SoundCloud::streamTrack(char* track_urn){
    strcpy(getURLBuffer, "");
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/tracks/%s/streams",
             urlEncode(String(track_urn)));
    return getFromURL();
}

JsonDocument ESP32SoundCloud::getPlaylist(char* playlist_urn, bool show_tracks){
    strcpy(getURLBuffer, "");
    const char* tracksStr = show_tracks ? "true" : "false";
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/playlists/%s?show_tracks=%s&linked_partitioning=true",
             urlEncode(String(playlist_urn)), tracksStr);
    return getFromURL();
}

JsonDocument ESP32SoundCloud::userTracks(char* user_urn, unsigned int limit){
    strcpy(getURLBuffer, "");
    if (limit == 0){
        snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/users/%s/tracks?linked_partitioning=true",
                 urlEncode(String(user_urn)));
    }
    else{
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/users/%s/tracks?limit=%u&linked_partitioning=true",
             urlEncode(String(user_urn)), limit);}
    return getFromURL();
}

JsonDocument ESP32SoundCloud::userLikedTracks(char* user_urn, unsigned int limit){
    strcpy(getURLBuffer, "");
    if (limit == 0){
        snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/users/%s/likes/tracks?linked_partitioning=true",
                 urlEncode(String(user_urn)));
    }
    else{
    snprintf(getURLBuffer, sizeof(getURLBuffer), "https://api.soundcloud.com/users/%s/likes/tracks?limit=%u&linked_partitioning=true",
             urlEncode(String(user_urn)), limit);}
    return getFromURL();
}

JsonDocument ESP32SoundCloud::customGet(char* url){
    strcpy(getURLBuffer, "");
    snprintf(getURLBuffer, sizeof(getURLBuffer), url);
    return getFromURL();
}
