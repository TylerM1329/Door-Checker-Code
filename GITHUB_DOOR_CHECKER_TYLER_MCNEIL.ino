/*

Designed by Tyler McNeil
08/07/2024
This code is for demonstration purposes. It was created using an example code that built the basis of the communication portion of the system.
My additions make it specific to my application.

These are two seperate codes meant to run on their own ESP32 units. If using as reference, make sure to separate them

    {ESP-NOW Broadcast Slave Example}

    This sketch demonstrates how to receive broadcast messages from a master device using the ESP-NOW protocol.

    The master device will broadcast a message every 5 seconds to all devices within the network.

    The slave devices will receive the broadcasted messages. If they are not from a known master, they will be registered as a new master
    using a callback function.
*/

#include "ESP32_NOW.h"
#include "WiFi.h"

#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

#include <vector>

/* Definitions */

#define ESPNOW_WIFI_CHANNEL 6
#define LED_GREEN 2
#define LED_RED 4
/* Classes */

// Creating a new class that inherits from the ESP_NOW_Peer class is required.

class ESP_NOW_Peer_Class : public ESP_NOW_Peer {
public:
  // Constructor of the class
  ESP_NOW_Peer_Class(const uint8_t *mac_addr, uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(mac_addr, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Peer_Class() {}

  // Function to register the master peer
  bool add_peer() {
    if (!add()) {
      log_e("Failed to register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to print the received messages from the master
  void onReceive(const uint8_t *data, size_t len, bool broadcast) {
    Serial.printf("Received a message from master " MACSTR " (%s)\n", MAC2STR(addr()), broadcast ? "broadcast" : "unicast");
    Serial.printf("  Message: %s\n", (char *)data);

    //Reaction to messages
    if (strcmp((char *)data, "LED ON") == 0) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED,    LOW);
    } else if (strcmp((char *)data, "LED OFF") == 0) {
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
    }
    }
  };


/* Global Variables */

// List of all the masters. It will be populated when a new master is registered
std::vector<ESP_NOW_Peer_Class> masters;

/* Callbacks */

// Callback called when an unknown peer sends a message
void register_new_master(const esp_now_recv_info_t *info, const uint8_t *data, int len, void *arg) {
  if (memcmp(info->des_addr, ESP_NOW.BROADCAST_ADDR, 6) == 0) {
    Serial.printf("Unknown peer " MACSTR " sent a broadcast message\n", MAC2STR(info->src_addr));
    Serial.println("Registering the peer as a master");

    ESP_NOW_Peer_Class new_master(info->src_addr, ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

    masters.push_back(new_master);
    if (!masters.back().add_peer()) {
      Serial.println("Failed to register the new master");
      return;
    }
  } else {
    // The slave will only receive broadcast messages
    log_v("Received a unicast message from " MACSTR, MAC2STR(info->src_addr));
    log_v("Igorning the message");
  }
}

/* Main */

void setup() {
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  while (!Serial) {
    delay(10);
  }

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Slave");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Initialize the ESP-NOW protocol
  if (!ESP_NOW.begin()) {
    Serial.println("Failed to initialize ESP-NOW");
    Serial.println("Reeboting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  // Register the new peer callback
  ESP_NOW.onNewPeer(register_new_master, NULL);

  Serial.println("Setup complete. Waiting for a master to broadcast a message...");
}

void loop() {
  delay(1000);
}


/*
----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------
----------------------------------------BREAK-------------------------------------------
----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------
*/

/* Sending Unit */

#include "ESP32_NOW.h"
#include "WiFi.h"
#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

/* Definitions */
#define ESPNOW_WIFI_CHANNEL 6
#define BUTTON_PIN 2  // GPIO pin connected to the button

/* Classes */
// Creating a new class that inherits from the ESP_NOW_Peer class is required.
class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  // Constructor of the class using the broadcast address
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Broadcast_Peer() {
    remove();
  }

  // Function to properly initialize the ESP-NOW and register the broadcast peer
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to send a message to all devices within the network
  bool send_message(const uint8_t *data, size_t len) {
    if (!send(data, len)) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};

/* Global Variables */
uint32_t msg_count = 0;

// Create a broadcast peer object
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

/* Main */
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  pinMode(BUTTON_PIN, INPUT_PULLDOWN);  // Set the button pin as input with pull-down

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("Setup complete. Press the button to broadcast messages.");
}

void loop() {
    static uint32_t lastButtonPress = 0;
    static bool lastButtonState = false; // To keep track of the last button state

    // Read the current state of the button
    bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;

    // Check if button state has changed from last check
    if (buttonPressed != lastButtonState) {
        lastButtonState = buttonPressed; // Update the last button state

        if (buttonPressed) {
            if (millis() - lastButtonPress > 200) { // Basic debouncing
                lastButtonPress = millis();

                // Construct a message to turn on the LED
                const char* message = "LED ON";
                Serial.printf("Broadcasting message: %s\n", message);

                if (!broadcast_peer.send_message((uint8_t *)message, strlen(message) + 1)) {
                    Serial.println("Failed to broadcast message");
                }
            }
        } else {
            // Construct a message to turn off the LED
            const char* message = "LED OFF";
            Serial.printf("Broadcasting message: %s\n", message);

            if (!broadcast_peer.send_message((uint8_t *)message, strlen(message) + 1)) {
                Serial.println("Failed to broadcast message");
            }
        }
    }

    delay(100); // Small delay to prevent CPU hogging
}
