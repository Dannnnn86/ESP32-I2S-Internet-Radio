#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "Audio.h"
#include <WebServer.h>    // NEW: For the web page
#include <Preferences.h>  // NEW: To save WiFi permanently

// ================== PIN CONFIG ==================
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC  26

#define I2C_SDA 32
#define I2C_SCL 33

// --- BUTTON PINS FROM IMAGE ---
#define BTN_VOL_UP   14
#define BTN_VOL_DOWN 13
#define BTN_CH_UP    4
#define BTN_CH_DOWN  5

// ================== OBJECTS ==================
Adafruit_SH1106G display(128, 64, &Wire, -1);
Audio audio;
WebServer server(80);     // NEW: Web server on port 80
Preferences prefs;        // NEW: Permanent storage

// ================== WIFI STATE ==================
String ssid_str = "";
String pass_str = "";

// ================== STREAM & STATE ==================
const char* streams[] = {
    "http://media-ice.musicradio.com/ClassicFMMP3",
    "http://azura.easyrock.com.ph/listen/easy_rock_manila/radio.mp3",
    "http://azura.loveradio.com.ph/listen/love_radio_manila/radio.mp3"
};

int currentStation = 0;
int volume = 12; // Start at a moderate level (0-21)
String currentTitle = "Waiting...";

bool hasDisplay = false;
bool isPlaying = false;
unsigned long lastAttemptTime = 0;
const int retryInterval = 10000;

// ================== DISPLAY ==================
void updateDisplay(String msg) {
    Serial.println("OLED: " + msg);
    if (hasDisplay) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        
        display.setCursor(0, 0);
        display.println("ESP32 Radio");
        display.print("Vol: "); display.print(volume);
        display.print(" | CH: "); display.println(currentStation + 1);
        display.println("--------------------");
        display.println(msg);
        display.display();
    }
}

// ================== NEW: WEB PAGE LOGIC ==================
void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head>";
  html += "<body><h2>WiFi Config</h2><form action='/save' method='POST'>";
  html += "SSID: <br><input type='text' name='s'><br>";
  html += "Password: <br><input type='password' name='p'><br><br>";
  html += "<input type='submit' value='Connect'></form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  String new_ssid = server.arg("s");
  String new_pass = server.arg("p");
  
  prefs.begin("wifi_store", false);
  prefs.putString("n_ssid", new_ssid);
  prefs.putString("n_pass", new_pass);
  prefs.end();
  
  server.send(200, "text/html", "Settings Saved! Restarting...");
  delay(2000);
  ESP.restart();
}

// ================== SETUP ==================
void setup() {
    Serial.begin(115200);
    
    pinMode(BTN_VOL_UP, INPUT_PULLUP);
    pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
    pinMode(BTN_CH_UP, INPUT_PULLUP);
    pinMode(BTN_CH_DOWN, INPUT_PULLUP);

    Wire.begin(I2C_SDA, I2C_SCL);

    if (display.begin(0x3C, true)) {
        hasDisplay = true;
        display.setContrast(255);
        display.clearDisplay();
        display.display();
    }

    // Load saved WiFi
    prefs.begin("wifi_store", true);
    ssid_str = prefs.getString("n_ssid", "");
    pass_str = prefs.getString("n_pass", "");
    prefs.end();

    // IF NO WIFI SAVED OR CH_DOWN HELD: Start Portal
    if (ssid_str == "" || digitalRead(BTN_CH_DOWN) == LOW) {
        updateDisplay("WiFi Setup Mode\nConnect phone to:\nESP32_Radio_Setup\nVisit 192.168.4.1");
        WiFi.softAP("ESP32_Radio_Setup");
        server.on("/", handleRoot);
        server.on("/save", handleSave);
        server.begin();
        while(true) { server.handleClient(); }
    }

    updateDisplay("Connecting WiFi...");

    WiFi.begin(ssid_str.c_str(), pass_str.c_str());
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
        delay(500);
        Serial.print(".");
        timeout++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        updateDisplay("WiFi Failed!\nHold CH DOWN to\nreset WiFi.");
        delay(5000);
    } else {
        updateDisplay("WiFi Connected!");
        audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
        audio.setVolume(volume); 
        audio.setBufsize(100000, 0);
        updateDisplay("Loading CH 1...");
        audio.connecttohost(streams[currentStation]);
    }

    lastAttemptTime = millis();
}

// ================== LOOP ==================
void loop() {
    audio.loop();

    static unsigned long lastBtnTime = 0;
    if (millis() - lastBtnTime > 250) { 
        if (digitalRead(BTN_VOL_UP) == LOW && volume < 21) {
            volume++;
            audio.setVolume(volume);
            updateDisplay(currentTitle);
            lastBtnTime = millis();
        }
        if (digitalRead(BTN_VOL_DOWN) == LOW && volume > 0) {
            volume--;
            audio.setVolume(volume);
            updateDisplay(currentTitle);
            lastBtnTime = millis();
        }
        if (digitalRead(BTN_CH_UP) == LOW) {
            currentStation = (currentStation + 1) % 3;
            updateDisplay("Switching CH...");
            audio.connecttohost(streams[currentStation]);
            lastBtnTime = millis();
        }
        if (digitalRead(BTN_CH_DOWN) == LOW) {
            currentStation = (currentStation - 1 + 3) % 3;
            updateDisplay("Switching CH...");
            audio.connecttohost(streams[currentStation]);
            lastBtnTime = millis();
        }
    }

    if (!isPlaying && (millis() - lastAttemptTime > retryInterval)) {
        updateDisplay("Stream Failed!\nRetrying...");
        audio.stopSong();
        delay(200);
        audio.connecttohost(streams[currentStation]);
        lastAttemptTime = millis();
    }
}

// ================== CALLBACKS ==================
void audio_info(const char *info) {
    if (strstr(info, "failed") || strstr(info, "could not") || strstr(info, "404")) {
        isPlaying = false;
    }
    if (strstr(info, "stream ready") || strstr(info, "format")) {
        isPlaying = true;
    }
}

void audio_showstation(const char *info) {
    if (strlen(info) > 0) {
        isPlaying = true;
        currentTitle = String(info);
        updateDisplay(currentTitle);
    }
}

void audio_showstreamtitle(const char *info) {
    if (strlen(info) > 0) {
        currentTitle = String(info);
        updateDisplay(currentTitle);
    }
}
