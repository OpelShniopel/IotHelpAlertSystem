#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>
#include <PubSubClient.h>

#include "secrets.h"

// Pin definitions
#define SEND_HELP_BUTTON_PIN 5       // GPIO5 for D1
#define CANCEL_HELP_BUTTON_PIN 13    // GPIO13 for D7
#define SUCCESS_LED_PIN 4            // GPIO4 for D2
#define ERROR_LED_PIN 14             // GPIO14 for D5

// MQTT topics
const char *help_topic_set = "homeassistant/button/help/set";
const char *cancel_help_topic_set = "homeassistant/button/cancel_help/set";
const char *help_topic_state = "homeassistant/button/help/state";
const char *cancel_help_topic_state = "homeassistant/button/cancel_help/state";

// Global objects for WiFi and MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Global objects for SMTP (email)
SMTPSession smtp;
SMTP_Message message;
Session_Config configSMTP;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    initializeWiFi();
    initializeMQTT();
    initializePins();
    configureSMTP();
    configureNTP();
}

void loop()
{
    if (!mqttClient.connected())
    {
        reconnectMQTT();
    }
    mqttClient.loop();

    handleButtonPress();
    delay(100); // kad nesprogtu sistema
}

// Initializes the WiFi connection
void initializeWiFi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
}

// Initializes the MQTT client and connects to the MQTT broker
void initializeMQTT()
{
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    reconnectMQTT();
}

// Attempts to reconnect to the MQTT broker
void reconnectMQTT()
{
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect with username and password
        if (mqttClient.connect("ESP8266Client", MQTT_USERNAME, MQTT_PASSWORD))
        {
            Serial.println("connected");
            // Subscribe to topics after successful connection
            mqttClient.subscribe(help_topic_set);
            mqttClient.subscribe(cancel_help_topic_set);
            mqttClient.publish(help_topic_state, "OFF");
            mqttClient.publish(cancel_help_topic_state, "OFF");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

// Callback function for MQTT messages
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    payload[length] = '\0';
    String message = String((char *) payload);

    if (String(topic) == help_topic_set && message == "ON")
    {
        setupEmailMessage("REIKALINGA PAGALBA!", "Labai reikia pagalbos, tiesiog žiauriai\nTestas");
        sendEmail();
    }
    else if (String(topic) == cancel_help_topic_set && message == "ON")
    {
        setupEmailMessage("PAGALBOS NEREIKIA!", "Buvo išsiųstas klaidingas pranešimas\nTestas");
        sendEmail();
    }
}

// Configures SMTP (email) settings
void configureSMTP()
{
    MailClient.networkReconnect(true);

    smtp.debug(1);

    smtp.callback(smtpCallback);

    configSMTP.server.host_name = SMTP_HOST;
    configSMTP.server.port = SMTP_PORT;
    configSMTP.login.email = SENDER_EMAIL;
    configSMTP.login.password = SENDER_PASSWORD;
    configSMTP.login.user_domain = "";
}

// Configures NTP (Network Time Protocol) settings for SMTP
void configureNTP()
{
    configSMTP.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    configSMTP.time.gmt_offset = 2;
    configSMTP.time.day_light_offset = 0;
}

// Sets up the email message to be sent
void setupEmailMessage(String subject, String textMessage)
{
    message.sender.name = F("ESP8266");
    message.sender.email = SENDER_EMAIL;
    message.subject = subject.c_str();
    message.addRecipient(F("Dovydas"), RECIPIENT_EMAIL_1);

    message.text.content = textMessage.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
}

// Initializes the pins for buttons and LEDs
void initializePins()
{
    pinMode(SEND_HELP_BUTTON_PIN, INPUT);
    pinMode(CANCEL_HELP_BUTTON_PIN, INPUT);
    pinMode(SUCCESS_LED_PIN, OUTPUT);
    pinMode(ERROR_LED_PIN, OUTPUT);
}

// Handles button presses and triggers email sending
void handleButtonPress()
{
    if (digitalRead(SEND_HELP_BUTTON_PIN) == HIGH)
    {
        mqttClient.publish(help_topic_state, "ON");
        setupEmailMessage("REIKALINGA PAGALBA!", "Labai reikia pagalbos, tiesiog žiauriai\nTestas");
        sendEmail();
        mqttClient.publish(help_topic_state, "OFF");
    }
    if (digitalRead(CANCEL_HELP_BUTTON_PIN) == HIGH)
    {
        mqttClient.publish(cancel_help_topic_state, "ON");
        setupEmailMessage("PAGALBOS NEREIKIA!", "Buvo išsiųstas klaidingas pranešimas\nTestas");
        sendEmail();
        mqttClient.publish(cancel_help_topic_state, "OFF");
    }
}

// Sends the email
void sendEmail()
{
    if (!connectToSMTP())
    {
        return;
    }

    if (!MailClient.sendMail(&smtp, &message))
    {
        ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(),
                        smtp.errorReason().c_str());
        blinkLED(ERROR_LED_PIN, 5);
    }
    else
    {
        blinkLED(SUCCESS_LED_PIN, 5);
    }
}

// Connects to the SMTP server
bool connectToSMTP()
{
    if (!smtp.connect(&configSMTP))
    {
        ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(),
                        smtp.errorCode(), smtp.errorReason().c_str());
        blinkLED(ERROR_LED_PIN, 5);
        return false;
    }
    if (!smtp.isLoggedIn())
    {
        Serial.println("\nNot yet logged in.");
    }
    else
    {
        if (smtp.isAuthenticated())
            Serial.println("\nSuccessfully logged in.");
        else
            Serial.println("\nConnected with no Auth.");
    }
    return true;
}

// Blinks an LED on a specified pin for a specified duration
void blinkLED(int ledPin, int duration)
{
    for (int i = 0; i < duration; i++)
    {
        digitalWrite(ledPin, HIGH);
        delay(500);
        digitalWrite(ledPin, LOW);
        delay(500);
    }
}

// Callback function to get the Email sending status
void smtpCallback(SMTP_Status status)
{
    // Print the sending result
    Serial.println(status.info());

    // Print the sending result
    if (status.success())
    {
        Serial.println("-------------------------");
        ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
        ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
        Serial.println("-------------------------\n");

        for (size_t i = 0; i < smtp.sendingResult.size(); i++)
        {
            // Get the result item
            SMTP_Result result = smtp.sendingResult.getItem(i);

            ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
            ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
            ESP_MAIL_PRINTF("Date/Time: %s\n",
                            MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
            ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
            ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
        }
        Serial.println("-------------------------\n");

        // Clear sending result to manage memory usage
        smtp.sendingResult.clear();
    }
}
