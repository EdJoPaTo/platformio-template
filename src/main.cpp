#include <credentials.h>
#include <EspMQTTClient.h>
#include <MqttKalmanPublish.h>

#define CLIENT_NAME "espTemplate"
const bool MQTT_RETAINED = false;

EspMQTTClient mqttClient(
	WIFI_SSID,
	WIFI_PASSWORD,
	MQTT_SERVER,
	MQTT_USERNAME,
	MQTT_PASSWORD,
	CLIENT_NAME, // Client name that uniquely identify your device
	1883         // The MQTT port, default to 1883. this line can be omitted
);

#define BASE_TOPIC CLIENT_NAME "/"
#define BASE_TOPIC_SET BASE_TOPIC "set/"
#define BASE_TOPIC_STATUS BASE_TOPIC "status/"

#ifdef ESP8266
	#define LED_BUILTIN_ON LOW
	#define LED_BUILTIN_OFF HIGH
#else // for ESP32
	#define LED_BUILTIN_ON HIGH
	#define LED_BUILTIN_OFF LOW
#endif

MQTTKalmanPublish mkRssi(mqttClient, BASE_TOPIC_STATUS "rssi", MQTT_RETAINED, 12 * 5 /* every 5 min */, 10);

boolean on = true;
uint8_t mqttBri = 1;

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	Serial.println();

	mqttClient.enableDebuggingMessages();
	mqttClient.enableHTTPWebUpdater();
	mqttClient.enableOTA();
	mqttClient.enableLastWillMessage(BASE_TOPIC "connected", "0", MQTT_RETAINED);

	Serial.println("Setup done...");
}

void onConnectionEstablished()
{
	mqttClient.subscribe(BASE_TOPIC_SET "bri", [](const String &payload) {
		int value = strtol(payload.c_str(), 0, 10);
		mqttBri = max(1, min(255, value));
		mqttClient.publish(BASE_TOPIC_STATUS "bri", String(mqttBri), MQTT_RETAINED);
	});

	mqttClient.subscribe(BASE_TOPIC_SET "on", [](const String &payload) {
		boolean value = payload != "0";
		on = value;
		mqttClient.publish(BASE_TOPIC_STATUS "on", String(on), MQTT_RETAINED);
	});

	mqttClient.publish(BASE_TOPIC_STATUS "bri", String(mqttBri), MQTT_RETAINED);
	mqttClient.publish(BASE_TOPIC_STATUS "on", String(on), MQTT_RETAINED);
	mqttClient.publish(BASE_TOPIC "git-version", GIT_VERSION, MQTT_RETAINED);
	mqttClient.publish(BASE_TOPIC "connected", "2", MQTT_RETAINED);
}

void loop()
{
	mqttClient.loop();
	digitalWrite(LED_BUILTIN, mqttClient.isConnected() ? LED_BUILTIN_OFF : LED_BUILTIN_ON);
	if (!mqttClient.isWifiConnected())
	{
		return;
	}

	auto now = millis();

	static unsigned long nextMeasure = 0;
	if (now >= nextMeasure)
	{
		nextMeasure = now + 5000;
		long rssi = WiFi.RSSI();
		float avgRssi = mkRssi.addMeasurement(rssi);
		Serial.printf("RSSI          in dBm: %8ld    Average: %10.2f\n", rssi, avgRssi);
	}
}
