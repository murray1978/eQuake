
#include <Ethernet.h>
#include <SPI.h> //for baro sensor.

byte mac[] = { 0x10, 0xBA, 0xD0, 0xBE, 0xEF, 0x00 };

IPAddress ip(192, 168, 2, 123);
IPAddress gateway( 192, 168, 2, 1);
IPAddress subnet( 255, 255, 255, 0);

EthernetServer server(80);

void setup() {
  Serial.begin(9600);
  delay(50);
  Serial.println("eQuake setup");
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.println("Ready");
}



void loop() {
  
  clients();
}

void clients(){
  EthernetClient client = server.available();
  if( client ){
    Serial.println("Hello recieved");
    bool blankLine = true;
    char c = client.read();
    Serial.println(c);
    if( c == 'G' && blankLine ){
      //respond
      //Serial.println("responding");
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type: text/html");
      client.println();
      client.print("<h1>Hello</h1>");
    }
    if( c == '\n' ){
      blankLine = true;
    }
    else if( c != '\r' ){
      blankLine = false;
    }
  }
  delay(1);
  client.stop();
}

