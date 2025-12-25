# ESP32SoundCloud

A lightweight ESP32 library to interact with the SoundCloud API — search tracks, users, playlists, and retrieve streaming or metadata responses as ArduinoJson `JsonDocument` objects. The library focuses on simplicity and small footprint so you can quickly add SoundCloud functionality to your ESP32 projects.

---

## Features

- OAuth2 client helpers for SoundCloud (token renewal/creation handled internally)
- Search endpoints for tracks, users, and playlists
- Retrieve full track / playlist metadata
- Return results as `ArduinoJson` `JsonDocument` for easy parsing on-device
- Ability to set tokens manually via `beginWithSetToken()` (for development)
- Uses `client.setInsecure()` internally so TLS/SSL certificates are not validated (no root CA required). If you need certificate validation, modify the library to use a root CA with the internal `WiFiClientSecure`.

---

## Installation

### PlatformIO
Add the following to your `platformio.ini` file:
```ini
lib_deps =
    https://github.com/Zintrall/ESP32SoundCloud.git
```

---

## Dependencies

- `ArduinoJson` (v7.x) — for JSON parsing
- `WiFi` — ESP32 built-in WiFi
- `HTTPClient` — ESP32 built-in HTTP client
- `WiFiClientSecure` — for TLS connections
---

## Quick start

You will need a SoundCloud client ID and client secret (OAuth2 credentials). You can obtain these from https://soundcloud.com/you/apps. If you don't have an app registered yet, you can request credentials through the help chatbot at https://help.soundcloud.com.

### 1. Create the instance

Create the object in your sketch passing the `client_id` and `client_secret`:

```cpp
#include <WiFi.h>
#include <ESP32SoundCloud.h>
#include <ArduinoJson.h>

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"
#define SC_CLIENT_ID "your_client_id"
#define SC_CLIENT_SECRET "your_client_secret"

ESP32SoundCloud sc(SC_CLIENT_ID, SC_CLIENT_SECRET);
```

### 2. Connect to WiFi and call `begin()` (or use tokens)

The `begin()` method prepares internal state and can be used to initialize any required internals before making API calls. Note: `begin()` calls `client.setInsecure()` on the internal `WiFiClientSecure`, so the library does not validate TLS/SSL certificates. This simplifies setup because no root CA is required, but it is insecure for production use — if you require certificate validation, modify the library to provide and use a root CA.

If you already have valid OAuth token (access token, refresh token, and expiry), use `beginWithSetToken()` to supply them to the library instead of calling `begin()` — do NOT call `begin()` after `beginWithSetToken()`. Calling `begin()` will request a new token from SoundCloud and may overwrite tokens you provided.

```cpp
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

    sc.begin();

    // For development you can use sc.beginWithSetToken(token, refreshToken, expiresSeconds); instead of sc.begin();
}
```

### 3. Example: Search and print results

This example searches tracks and prints the raw JSON response. The library returns `JsonDocument` objects — you can adapt sizes or copy fields to smaller documents if memory is constrained.

```cpp
void setup() {
    // Your setup ...

    // Search for tracks matching "lofi" (limit 5)
    JsonDocument doc = sc.searchTrack((char*)"lofi", 5);

    // Print results
    Serial.println("Search results:");
    serializeJsonPretty(doc, Serial);
}
```

---

## API reference

This section documents the public interface found in `ESP32SoundCloud.h`.

### Constructor

`ESP32SoundCloud(const char* client_id, const char* client_secret)`

Create the library instance with your SoundCloud app credentials.

Example:
```cpp
ESP32SoundCloud sc("your_client_id", "your_client_secret");
```

### Public methods

- `void begin()`
  - Initialize internal state. Call after creating the instance and connecting to WiFi. This method calls `client.setInsecure()` on the internal `WiFiClientSecure`, so TLS/SSL certificates are not validated by default (no root CA required). Change the implementation if you need proper certificate validation.

- `void beginWithSetToken(char* token, char* refresh, int expires)`
  - Supply an already-obtained access token, refresh token, and expiry (seconds) to the library. Use this only in development if you do not want the library to create/renew tokens automatically at startup.
  - Do not call `begin()` after `beginWithSetToken()` — `begin()` will create/renew tokens and may overwrite the tokens you provided.

- `JsonDocument searchPlaylist(char* query, unsigned int limit, bool show_tracks = false)`
  - Search for playlists by query. Set `show_tracks` to true to include a list of playlist tracks.

- `JsonDocument searchTrack(char* query, unsigned int limit)`
  - Search tracks by query.

- `JsonDocument searchUser(char* query, unsigned int limit)`
  - Search users by query.

- `JsonDocument getTrack(char* track_urn)`
  - Get data for a single track (provide a track URN or id string).

- `JsonDocument streamTrack(char* track_urn)`
  - Get stream URL for playing a track.

- `JsonDocument getPlaylist(char* playlist_urn, bool show_tracks = true)`
  - Retrieve playlist and tracks (if requested).

- `JsonDocument userTracks(char* user_urn, unsigned int limit)`
  - Get tracks uploaded by a user.

- `JsonDocument userLikedTracks(char* user_urn, unsigned int limit)`
  - Get tracks liked/favorited by a user.

- `JsonDocument customGet(char* url)`
  - Perform a GET request to a custom SoundCloud URL, with the access_token Authorization (useful for endpoints not directly wrapped).

- `String urlEncode(String str)`
  - URL-encode a string for safe inclusion in queries.

---

## Notes on token handling

The library includes helpers to create, renew, and validate OAuth2 tokens. Those internal helpers are used automatically when you make requests unless you provide tokens manually with `beginWithSetToken()`.

- `beginWithSetToken(token, refresh, expires)` — set tokens manually at startup and avoid automatic token creation by the library
- Important: If you call `begin()` it will attempt to create/renew tokens automatically on startup.
- The library tries to ensure tokens are valid before requests; if a token is expired it will attempt to renew it using the `refresh_token`.

---

## Memory note

- SoundCloud responses can be large. Be mindful of `ArduinoJson` document sizes.
- TLS and HTTPS (via `WiFiClientSecure`) increase RAM/flash usage; ensure your ESP32 project has sufficient memory.

---

## License

MIT License - see [LICENSE](LICENSE) file for details.

---

## Contributing

Contributions are welcome. The library is incomplete, so there are a lot of features that need to be added.
