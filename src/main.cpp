#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DNSServer.h>
#include <WiFiManager.h> 

#define SEALEVELPRESSURE_HPA (1013.25)

#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif

Adafruit_BME280 bme;  // I2C

const char* serverName = "https://froesmhs.com/miniespstation/post-data.php";
String apiKeyValue = "tPmAT5Ab3j7F9";
String sensorName = "BME280";
String sensorLocation = "CASA";

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

float temperature, humidity, pressure;

void getReadings(){
  // volt = analogRead(A0);
  //tensao da bateria +- entre 3.3 e 4.2(nao está calibrado, requer testes)
  // bat = map(volt, 420, 650, 0, 100);
  //esp "desliga" se tensao da bateria estiver muito baixa
  // if(bat <= 0){
    // ESP.deepSleep(0);
  // }
  // Serial.println(bat);
  // Serial.println(countVen);
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure();
}

void setup() {
  Serial.begin(115200);

  WiFiManager wifiManager;
  wifiManager.autoConnect("configurarEsp");
  
  bool status = bme.begin(0x76);
  if (!status) {
    Serial.println("o bme nao foi achado");
    while (1);
  }
  
  Serial.println("Conectado no Wifi com endereco IP: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("WiFi desconectado");
    }
    
    getReadings();
    Serial.println("enviando");

    // WiFiClient client;
    HTTPClient http;

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation
    client->setInsecure();

    http.begin(*client, serverName);

    http.addHeader("Content-Type", "application/json");
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpResponseCode = http.POST("{\"api_key\":\"" + apiKeyValue + "\",\"sensor\":\"bme280\",\"location\":\"casa\",\"temp\":\"" + temperature+ "\",\"humi\":\""+humidity+"\",\"press\":\""+ pressure+ "\"}");
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    http.end();
    lastTime = millis();
  }
}