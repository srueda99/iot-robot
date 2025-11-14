#include <Arduino.h>
#include <Wire.h>
#include <ClosedCube_HDC1080.h>
#include <TinyGPS++.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "LoRaBoards.h"

// --- Configuración LoRa (opcional) ---
#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ 915.0
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER 17
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW 125.0
#endif

// --- Pines ---
static const int MY_I2C_SDA = 21;
static const int MY_I2C_SCL = 22;
static const int RXPin = 34;
static const int TXPin = 12;

// --- WiFi ---
const char *ssid = "UPBWiFi";
const char *password = "";

// --- Servidor ---
const char *serverUrl = "http://3.230.70.191:4040/data";

// --- Sensores ---
ClosedCube_HDC1080 hdc1080;
TinyGPSPlus gps;

// --- Variables globales ---
float avgTemp = 0.0;
float avgHum = 0.0;
unsigned long lastSendTime = 0;
const long sendInterval = 10000;

// --- Funciones auxiliares ---
float average(float *arr, int n)
{
  float sum = 0.0;
  for (int i = 0; i < n; i++)
    sum += arr[i];
  return sum / n;
}

void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}

bool connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando a UPBWiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  else
  {
    Serial.println("\nFallo al conectar WiFi");
    return false;
  }
}

bool sendDataToServer(float lat, float lon, float temp, float hum)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("No hay WiFi para enviar datos");
    return false;
  }

  HTTPClient http;
  http.setTimeout(10000);
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  // Crear JSON
  StaticJsonDocument<200> doc;
  doc["lat"] = round(lat * 10000) / 10000.0;
  doc["lon"] = round(lon * 10000) / 10000.0;
  doc["temp"] = round(temp * 10) / 10.0;
  doc["hum"] = round(hum * 10) / 10.0;

  String payload;
  serializeJson(doc, payload);

  Serial.println("Enviando POST:");
  Serial.println(payload);

  int httpCode = http.POST(payload);

  if (httpCode > 0)
  {
    String response = http.getString();
    Serial.printf("HTTP %d | Respuesta: %s\n", httpCode, response.c_str());
    http.end();
    return (httpCode == 200 || httpCode == 201);
  }
  else
  {
    Serial.printf("Error HTTP: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return false;
  }
}

// --- Setup ---
void setup()
{
  Serial.begin(115200);
  Wire.begin(MY_I2C_SDA, MY_I2C_SCL);

  Serial.println("Inicializando sensores...");

  // HDC1080
  hdc1080.begin(0x40);
  delay(20);

  // GPS
  Serial2.begin(9600, SERIAL_8N1, RXPin, TXPin);
  Serial.println("GPS iniciado");

  setupBoards();
  delay(1500);
#ifdef RADIO_TCXO_ENABLE
  pinMode(RADIO_TCXO_ENABLE, OUTPUT);
  digitalWrite(RADIO_TCXO_ENABLE, HIGH);
#endif

  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
  if (!LoRa.begin(CONFIG_RADIO_FREQ * 1000000))
  {
    Serial.println("LoRa no iniciado (opcional)");
  }
  else
  {
    LoRa.setTxPower(CONFIG_RADIO_OUTPUT_POWER);
    LoRa.setSignalBandwidth(CONFIG_RADIO_BW * 1000);
    LoRa.setSpreadingFactor(10);
    LoRa.setPreambleLength(16);
    LoRa.setSyncWord(0xAB);
    LoRa.disableCrc();
    LoRa.disableInvertIQ();
    LoRa.setCodingRate4(7);
    Serial.println("LoRa iniciado (opcional)");
  }

  // Conectar WiFi
  connectWiFi();
}

// --- Loop ---
void loop()
{
  unsigned long now = millis();

  if (now - lastSendTime >= sendInterval)
  {
    lastSendTime = now;

    float temps[10], hums[10];
    for (int i = 0; i < 10; i++)
    {
      temps[i] = hdc1080.readTemperature();
      hums[i] = hdc1080.readHumidity();
      smartDelay(200);
    }
    avgTemp = average(temps, 10);
    avgHum = average(hums, 10);

    // --- GPS ---
    smartDelay(1000);
    float lat = gps.location.isValid() ? gps.location.lat() : 0.0;
    float lon = gps.location.isValid() ? gps.location.lng() : 0.0;

    // --- MOSTRAR EN CONSOLA ---
    Serial.println("=======================");
    Serial.printf("GPS: Lat=%.6f Lon=%.6f\n", lat, lon);
    Serial.printf("Temp: %.1f °C | Hum: %.1f %%\n", avgTemp, avgHum);
    Serial.println("=======================");

    // --- ENVIAR AL SERVIDOR ---
    if (sendDataToServer(lat, lon, avgTemp, avgHum))
    {
      Serial.println("Datos enviados al servidor");
    }
    else
    {
      Serial.println("Fallo al enviar datos");
      // Reintento de WiFi
      if (WiFi.status() != WL_CONNECTED)
      {
        Serial.println("Reconectando WiFi...");
        connectWiFi();
      }
    }

    String mensaje = "GPS: Lat=" + String(lat, 6) + " Lon=" + String(lon, 6) +
                     "\nTemp: " + String(avgTemp, 1) + "C" +
                     "\nHum: " + String(avgHum, 1) + "%";

    LoRa.beginPacket();
    LoRa.print(mensaje);
    LoRa.endPacket();
    Serial.println("Enviado por LoRa (opcional)");
    Serial.println(mensaje);
    Serial.println();
  }

  // === MANTENER GPS VIVO ===
  smartDelay(100);
}