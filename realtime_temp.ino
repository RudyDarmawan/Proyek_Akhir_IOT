#include <DHT.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <stdlib.h>

#define dhtPin D7
#define DHTTYPE DHT11

DHT dht (dhtPin, DHTTYPE);

int led_pins[] = {D1, D2, D3};

float suhu() { return dht.readTemperature(); }
float kelembapan() { return dht.readHumidity(); }

const char* ssid = "Redmi Note 9";
const char* password = "Tasyacaca19";

const char* mqtt_server = "riset.revolusi-it.com";

WiFiClient espClient;
PubSubClient client(espClient);
String messages;

void konek_wifi()
{
  WiFi.begin(ssid, password);
  Serial.println();
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
    i++;
    if (i == 15) {
      Serial.println();
      i = 0;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected"); // cetak wifi terkoneksi
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  // Mengambil Pesan dari mqqt server
  Serial.print("Pesan dari MQTT [");
  Serial.print(topic);
  Serial.print("] ");
  messages="";
  for (int i=0;i<length;i++) { 
    char receivedChar = (char)payload[i];
    messages=messages+(char)payload[i]; // ambil pesannya masukkan dalam string
  }
  Serial.println(messages);
}

void reconnect(const char* clientId) 
{
  // Ulang terus sampai terkoneksi
  while (!client.connected()) 
  {
    Serial.print("Menghubungkan [" + (String) clientId + "] ke MQTT Server : "+(String)mqtt_server);
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println(" [CONNECTED]");
    } 
    else  {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 3 detik...");
      delay(3000);
    }
  }
}

void SendMessages(const char* topic, String msg) {
  char attributes[100];
  msg.toCharArray(attributes, 100);
  client.publish(topic,attributes,true);
  Serial.print("[");
  Serial.print(topic);
  Serial.print("] : ");
  Serial.print(msg);
  Serial.println(" [TERKIRIM]");
}

void GetMessages(const char* topic){
  client.subscribe(topic);
}

void LevelIndicator(int level){
  if (level == 0) {
    digitalWrite(led_pins[0], LOW);
    digitalWrite(led_pins[1], LOW);
    digitalWrite(led_pins[2], LOW);
  }
  if (level == 1) {
    digitalWrite(led_pins[0], HIGH);
    digitalWrite(led_pins[1], LOW);
    digitalWrite(led_pins[2], LOW);
  }
  if (level == 2) {
    digitalWrite(led_pins[0], HIGH);
    digitalWrite(led_pins[1], HIGH);
    digitalWrite(led_pins[2], LOW);
  }
  if (level == 3) {
    digitalWrite(led_pins[0], HIGH);
    digitalWrite(led_pins[1], HIGH);
    digitalWrite(led_pins[2], HIGH);
  }
}

const char * StringToChar(String str) {
  return str.c_str();
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(15000);
  dht.begin(); // Memulai Sensor DHT11
  client.setServer(mqtt_server, 1883); // sambungkan client ke mqtt
  client.setCallback(callback); // interaksi callback
  
  for (int i = 0; i < sizeof(led_pins); i++) {
    pinMode(led_pins[i], OUTPUT);
  }
}

String id = "";
String topik_suhu = "";
String topik_kelem = "";
void loop() {
  if(WiFi.status() != WL_CONNECTED) { konek_wifi(); } // jika tidak konek wifi maka di reconnect

  if (!client.connected()) { 
    Serial.println("Timeout 15 Seconds");
    Serial.print("Input ClientID: ");
    id = Serial.readStringUntil('\n');
    Serial.println(id);
    Serial.print("Input Topic Suhu: ");
    topik_suhu = Serial.readStringUntil('\n');
    Serial.println(topik_suhu);
    Serial.print("Input Topic Kelembapan: ");
    topik_kelem = Serial.readStringUntil('\n');
    Serial.println(topik_kelem);
    
    if (id == "") {
      reconnect("G_211_19_0106/G_211_19_0131"); 
    }
    else {
      reconnect(StringToChar(id));
    }
  }
  else {
    float s = suhu();
    float k = kelembapan();

    Serial.println();
    if (topik_suhu == "") {
      SendMessages("iot/G_211_19_0106/G_211_19_0131", (String) s);
    }
    else {
      SendMessages(StringToChar(topik_suhu), (String) s);
    }
    if (topik_kelem == "") {
      SendMessages("iot/G_211_19_0106/G_211_19_0131-1", (String) k);
    }
    else {
      SendMessages(StringToChar(topik_kelem), (String) k);
    }

    if (s > 31) {
      LevelIndicator(3);
    }
    else if (s >= 30) {
      LevelIndicator(2);
    }
    else if (s >= 29) {
      LevelIndicator(1);
    }
    else {
      LevelIndicator(0);
    }
  }

  client.loop();
  delay(1000);
}
