#include <Arduino.h>
#include <Wire.h>
#include <ClosedCube_HDC1080.h>
#include <TinyGPS++.h>
#include <LoRa.h>
#include "LoRaBoards.h"

// --- Configuración LoRa ---
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
static const int RXPin = 34; // RX GPS (desde TX del módulo)
static const int TXPin = 12; // TX GPS (hacia RX del módulo)

// --- Sensores ---
ClosedCube_HDC1080 hdc1080;
TinyGPSPlus gps;

// --- Variables globales ---
float avgTemp = 0.0;
float avgHum = 0.0;

// --- Funciones auxiliares ---
float average(float *arr, int n)
{
  float sum = 0.0;
  for (int i = 0; i < n; i++)
    sum += arr[i];
  return sum / n;
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial2.available())
    {
      gps.encode(Serial2.read());
    }
  } while (millis() - start < ms);
}

// --- Setup ---
void setup()
{
  Serial.begin(115200);
  Wire.begin(MY_I2C_SDA, MY_I2C_SCL);

  Serial.println("Inicializando sensores y LoRa...");

  // Inicializar HDC1080
  hdc1080.begin(0x40);
  delay(20);

  // Inicializar GPS
  Serial2.begin(9600, SERIAL_8N1, RXPin, TXPin);
  Serial.println("GPS iniciado");

  // Configurar LoRa
  setupBoards();
  delay(1500);
#ifdef RADIO_TCXO_ENABLE
  pinMode(RADIO_TCXO_ENABLE, OUTPUT);
  digitalWrite(RADIO_TCXO_ENABLE, HIGH);
#endif

  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
  if (!LoRa.begin(CONFIG_RADIO_FREQ * 1000000))
  {
    Serial.println("Error iniciando LoRa");
    while (1)
      ;
  }

  LoRa.setTxPower(CONFIG_RADIO_OUTPUT_POWER);
  LoRa.setSignalBandwidth(CONFIG_RADIO_BW * 1000);
  LoRa.setSpreadingFactor(10);
  LoRa.setPreambleLength(16);
  LoRa.setSyncWord(0xAB);
  LoRa.disableCrc();
  LoRa.disableInvertIQ();
  LoRa.setCodingRate4(7);

  Serial.println("LoRa iniciado correctamente");
}

// --- Loop ---
void loop()
{
  // Leer 10 muestras del HDC1080
  float temps[10];
  float hums[10];
  for (int i = 0; i < 10; i++)
  {
    temps[i] = hdc1080.readTemperature();
    hums[i] = hdc1080.readHumidity();
    smartDelay(200);
  }

  avgTemp = average(temps, 10);
  avgHum = average(hums, 10);

  // Procesar GPS
  while (Serial2.available())
    gps.encode(Serial2.read());
  float lat = gps.location.lat();
  float lon = gps.location.lng();

  // Mostrar en Serial
  Serial.println("=======================");
  Serial.print("GPS: Lat = ");
  Serial.print(lat, 6);
  Serial.print(" Lon = ");
  Serial.println(lon, 6);
  Serial.print("Temp: ");
  Serial.print(avgTemp, 2);
  Serial.println(" C");
  Serial.print("Hum: ");
  Serial.print(avgHum, 2);
  Serial.println(" %");
  Serial.println("=======================");

  // Armar mensaje LoRa
  String mensaje = "GPS: Lat=" + String(lat, 6) + " Lon=" + String(lon, 6) + "\nTemp: " + String(avgTemp, 2) + "C" + "\nHum: " + String(avgHum, 2) + "%";

  // Enviar por LoRa
  LoRa.beginPacket();
  LoRa.print(mensaje);
  LoRa.endPacket();

  Serial.println("Paquete LoRa enviado:");
  Serial.println(mensaje);
  Serial.println();

  // Espera 5 segundos antes del siguiente envío
  smartDelay(5000);
}