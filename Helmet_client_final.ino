#include <WiFi.h>
#include <Wire.h>
 
char ssid[] = "Aakriti";
char password[] = "aakriti143";

uint16_t port = 23;
IPAddress server(192,168,43,90);
WiFiClient client;

#define alcPin 32
#define irPin 31
#define buzPin 10
int alc, ir, flag=0;

void setup()
{
  Serial.begin(9600);
  pinMode(alcPin,INPUT);
  pinMode(irPin,INPUT);
  pinMode(buzPin,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  Serial.print("Attempting to connect to Network named: ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
  while( WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  while (WiFi.localIP() == INADDR_NONE)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nIP Address obtained");
  printWifiStatus();
  Serial.println("Attempting to connect to server");
  uint8_t tries = 0;
  while(client.connect(server, port) == false)
  {
    Serial.print(".");
    if (tries++ > 100)
    {
      Serial.println("\nThe server isn't responding");
      while(1);
    }
    delay(100);
  }
  Serial.println("\nConnected to the server!");
}

void printWifiStatus()
{
  Serial.print("Network Name: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void loop()
{
  alc=digitalRead(alcPin);
  ir=digitalRead(irPin);
  if(ir==1 && alc==HIGH)
  {
    digitalWrite(GREEN_LED,HIGH);
    digitalWrite(RED_LED,LOW);
    digitalWrite(buzPin,LOW);
    int buffer1=1;
    client.println(buffer1);
    Serial.print("Sent: ");
    Serial.println(buffer1);
  }
  else
  {
    digitalWrite(GREEN_LED,LOW);
    digitalWrite(RED_LED,HIGH);
    digitalWrite(buzPin,HIGH);
    int buffer1=0;
    client.println(buffer1);
    Serial.print("Sent: ");
    Serial.println(buffer1);
  }
  delay(1000);
  if(client.available())
  {
    char buffer[255] = {0};
    client.read((uint8_t*)buffer, client.available());
    Serial.print("Received: ");
    Serial.println(buffer);
  }
}
