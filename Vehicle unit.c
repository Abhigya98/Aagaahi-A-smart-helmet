#include <WiFi.h>
#include <Wire.h>
#include <BMA222.h>

char buffer[255];
char ssid[] = "******";
char password[] = "*******";
boolean alreadyConnected = false;
BMA222 AccSensor;
int flag = 0, Fall = 0, counter = 0, hel = 0;
String msg="",lt="",ln="";

#define relPin 31 
#define buzPin 32 
#define gsmPin 12

WiFiClient client;
WiFiServer port(23);

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
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
  port.begin();
  pinMode(relPin,OUTPUT);
  pinMode(buzPin,OUTPUT);
  pinMode(gsmPin,OUTPUT);
  AccSensor.begin();
  uint8_t chipID = AccSensor.chipID();
  Serial.print("chipID: ");
  Serial.println(chipID);
}

void loop()
{
  digitalWrite(gsmPin, LOW);
  client = port.available();
  if(client)
  {
    if(!alreadyConnected)
    {
      client.flush();
      Serial.println("We have a new client from port1");
      client.println("Hello, client1!");
      alreadyConnected = true;
    }
    if(client.available() > 0)
    {
      char buffer1[1];
      client.read((uint8_t*)buffer1, client.available());
      Serial.println("Recieved");
      Serial.println(buffer1);
      if(buffer1[0]=='1')
      {
        digitalWrite(relPin,HIGH);
        hel = 1;
      }
      else if(buffer1[0]=='0')
      {
        digitalWrite(relPin,LOW);
        hel = 0;
      }
    }
  }
  delay(2000);
  if(hel == 1)
  {
    fallDetect();
    if(Fall)
    {
      Serial.println("Bike has fallen");
      digitalWrite(buzPin,HIGH);
      digitalWrite(gsmPin,HIGH);
      getLocation();
      //float lat,lon;
      //lat=lt.toFloat()-0.000942;
      //lon=ln.toFloat()+0.000342;
      //lt=String(lat,6);
      //ln=String(lon,6);
      msg="Accident Alert !!! \nLocation: https://www.google.com/maps/place/"+lt+"N,"+ln+"E";
      sendMessage();
      delay(5000);
    }
    else
    {
      digitalWrite(buzPin,LOW);
    }
  }
}

void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm"); 
}

void fallDetect()
{
  float xZero = 1, yZero = 0, zZero = 66, Anet, pitch, roll, Xval, Yval, Zval, datax, datay, dataz;
  datax = AccSensor.readXData();
  Serial.print("X: ");
  Serial.print(datax);
  datay = AccSensor.readYData();
  Serial.print(" Y: ");
  Serial.print(datay);
  dataz = AccSensor.readZData();
  Serial.print(" Z: ");
  Serial.println(dataz);
  Xval = (datax- xZero)/16;
  Yval = (datay - yZero)/16;
  Zval = (dataz - zZero)/16;
  Anet = sqrt(sq(Xval) + sq(Yval) + sq(Zval));
  Serial.print("Anet: ");
  Serial.println(Anet);
  Serial.print("Xval:");
  Serial.print(Xval);
  Serial.print(" Yval:");
  Serial.print(Yval);
  Serial.print(" Zval:");
  Serial.print(Zval);
  pitch = atan(Xval/sqrt(pow(Yval,2) + pow(Zval,2)));
  roll = atan(Yval/sqrt(pow(Xval,2) + pow(Zval,2)));
  pitch = pitch * (180.0/PI);
  roll = roll * (180.0/PI);
  Serial.print("pitch: ");
  Serial.print(pitch);
  Serial.print(",");
  Serial.print("roll: ");
  Serial.print(roll);
  Serial.println();
  delay(2000);
  if(abs(pitch)==0.00 | abs(roll)==45.02)
  {
    flag=0;
  }
  else if(abs(pitch)<20 && abs(roll)<70)
  {
    Serial.println();
    Serial.println("Falling");
    counter++;
    flag=1;
  }
  if(flag==1)
  {
    if(counter>3)
      Fall=1;
  }
}

void getLocation()
{
  String locdata="";
  Serial.println("Getting Location from GSM SIM800L");
  Serial1.println("AT");
  Serial1.readString();
  Serial1.println("AT+CGATT =1");
  Serial1.readString();
  Serial1.println("AT+SAPBR =3,1,\"CONTYPE\",\"GPRS\"");
  Serial1.readString();
  Serial1.println("AT+SAPBR =3,1,\"APN\",\"RCMNET\"");
  Serial1.readString();
  Serial1.println("AT+SAPBR =1,1");
  Serial1.readString();
  Serial1.println("AT+CIPGSMLOC =1,1");
  Serial1.readString();
  while(locdata=="")
  {
    if(Serial1.available())
      locdata=Serial1.readString();
  }
  Serial1.println("AT+SAPBR =0,1");
  Serial1.readString();
  locdata=locdata.substring(16,57);
  lt=locdata.substring(10,19);
  ln=locdata.substring(0,9);
  Serial.println("Latitude:\t"+lt);
  Serial.println("Longitude:\t"+ln);
  Serial.println();
}

void sendMessage()
{
  Serial.println("Sending Message from GSM SIM800L");
  Serial.println(msg);
  Serial1.println("AT");
  Serial1.readString();
  Serial1.println("AT+CMGF=1");
  Serial1.readString();
  Serial1.println("AT+CMGS=\"+919794171026\"");
  Serial1.readString();
  Serial1.println(msg);
  Serial1.write(0x1a);
  Serial1.readString();
  Serial.println();
  Serial.println("Message Sent");
}
