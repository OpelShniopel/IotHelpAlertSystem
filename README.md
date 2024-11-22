# IoT Help Alert System (IoT Pagalbos Iškvietimo Sistema)

A smart IoT-based emergency alert system built with ESP8266 that integrates with Home Assistant. The system enables users to send and cancel help requests through physical buttons or Home Assistant interface, with email notifications and LED status indicators.

## Features

- **Home Assistant Integration**
  - Appears as MQTT switches in Home Assistant UI
  - Automated workflows support
  - Real-time status updates in Home Assistant
- **Dual Button Interface**
  - Physical Help Request Button
  - Physical Cancel Request Button
- **MQTT Integration**
  - Home Assistant MQTT discovery
  - Real-time status updates
  - Remote triggering capability
- **Email Notifications**
  - Instant email alerts for help requests
  - Cancellation notifications
- **Visual Feedback**
  - Success LED indicator
  - Error LED indicator
- **Network Features**
  - Auto-reconnect WiFi capability
  - Reliable MQTT connection
  - SMTP email delivery

## Hardware Requirements

- ESP8266 development board
- 2× Push buttons
- 2× LEDs (for success and error indication)
- Resistors (as needed for LEDs)
- Breadboard and connecting wires

## Pin Configuration

```
GPIO5  (D1) - Help Request Button
GPIO13 (D7) - Cancel Request Button
GPIO4  (D2) - Success LED
GPIO14 (D5) - Error LED
```

## Software Dependencies

The following are required:

- Home Assistant instance
- Arduino IDE with ESP8266 support
- Libraries:
  - ESP8266WiFi
  - ESP_Mail_Client
  - PubSubClient

## Configuration

1. Create a `secrets.h` file with the following configuration:

```cpp
// WiFi Configuration
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// MQTT Configuration (Home Assistant)
#define MQTT_SERVER "your_home_assistant_ip"
#define MQTT_PORT 1883
#define MQTT_USERNAME "your_mqtt_username"
#define MQTT_PASSWORD "your_mqtt_password"

// Email Configuration
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define SENDER_EMAIL "your_email@gmail.com"
#define SENDER_PASSWORD "your_app_password"
#define RECIPIENT_EMAIL_1 "recipient@example.com"
```

## MQTT Topics

The system uses the following MQTT topics:

```
homeassistant/button/help/set         - Trigger help request
homeassistant/button/cancel_help/set  - Trigger cancellation
homeassistant/button/help/state       - Help button state
homeassistant/button/cancel_help/state - Cancel button state
```

## Home Assistant Configuration

Add the following to your `configuration.yaml`:

```yaml
mqtt:
  switch:
    - name: "Siūsti pagalbos pranešimą"
      state_topic: "homeassistant/button/help/state"
      command_topic: "homeassistant/button/help/set"
      payload_on: "ON"
      payload_off: "OFF"
      state_on: "ON"
      state_off: "OFF"
      retain: false

    - name: "Siūsti pranešimą, kad pagalbos nereikia"
      state_topic: "homeassistant/button/cancel_help/state"
      command_topic: "homeassistant/button/cancel_help/set"
      payload_on: "ON"
      payload_off: "OFF"
      state_on: "ON"
      state_off: "OFF"
      retain: false
```

This configuration creates two MQTT switches in Home Assistant:

1. "Siūsti pagalbos pranešimą" (Send Help Message) - For triggering help requests
2. "Siūsti pranešimą, kad pagalbos nereikia" (Send No Help Needed Message) - For cancelling help requests

## Installation

1. Clone this repository:

```bash
git clone https://github.com/yourusername/iot-help-alert-system.git
```

2. Install the required libraries in Arduino IDE:

   - Go to `Tools > Manage Libraries`
   - Install all dependencies listed above

3. Create `secrets.h` file with your configuration

4. Configure Home Assistant MQTT integration

5. Upload the code to your ESP8266 board

## Usage

1. **Via Home Assistant:**

   - Use the configured MQTT switches in Home Assistant UI:
     - "Siūsti pagalbos pranešimą" to send help request
     - "Siūsti pranešimą, kad pagalbos nereikia" to cancel request
   - Create automations based on switch states
   - Monitor switch states in real-time

2. **Physical Buttons:**

   - Press the Help button to send an emergency alert
   - Press the Cancel button to send a cancellation notice

3. **LED Indicators:**
   - Success LED blinks when messages are sent successfully
   - Error LED blinks when there are connection or sending issues
