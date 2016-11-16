
#include <Ethernet.h>
#include <SPI.h>

/*
 * Using the freetronic accelerometer,
 *  0g = 1/2 supply rail.
 *  0g = 3.3v / 2 or 1.65v
 *  used in 1.5g mode
 *  so 800mV/g
 *  3.3 / 1023 = 0.0032v bit
 *  1.65 / 0.0032 = 515.625 = 1.65 - 1.65 = 0g, should work wright?
*/

byte mac[] = { 0x10, 0xBA, 0xD0, 0xBE, 0xEF, 0x00 };

IPAddress ip(192, 168, 2, 123);
IPAddress gateway( 192, 168, 2, 1);
IPAddress subnet( 255, 255, 255, 0);

EthernetServer server(80);

const int xSensor = 0;
const int ySensor = 1;
const int zSensor = 2;

int rawX;
int rawY;
int rawZ;

float voltX = 0.0f;
float voltY = 0.0f;
float voltZ = 0.0f;

float accX = 0.0f;
float maxX = 0.0f;
float lastX = 0.0f;

float accY = 0.0f;
float maxY = 0.0f;
float lastY = 0.0f;

float accZ = 0.0f;
float maxZ = 0.0f;
float lastZ = 0.0f;

float const blahh = (float)(3.3f / 1024);
float const fudge = 1.65;

void setup() {
  Serial.begin(9600);
  analogReference(EXTERNAL);
  delay(50);
  Serial.println("eQuake setup");
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.println("Ready");

}

void loop() {
  readSensors();
  clients();
}

void readSensors() {
  float tempAcc = 0.0f;

  rawX = analogRead(xSensor);
  rawY = analogRead(ySensor);
  rawZ = analogRead(zSensor);

  voltX = (float)(blahh * rawX);
  voltY = (float)(blahh * rawY);
  voltZ = (float)(blahh * rawZ);

  tempAcc = (float)(( voltX - fudge) / 0.8f );
  accX = tempAcc - accX;
  (maxX < accX) ? maxX = accX : maxX = maxX;

  tempAcc = (float)(( voltY - fudge ) / 0.8f );
  accY = tempAcc - accY;
  (maxY < accY ) ? maxY = accX : maxY = maxY;
  
  tempAcc = (float)(( voltZ - fudge ) / 0.8f );
  accZ = tempAcc - accZ;
  (maxZ < accZ ) ? maxZ = accZ : maxZ = maxZ;
  
}

//Could make this smarter.
void clients() {
  char c;
  EthernetClient client = server.available();

  if ( client ) {
    Serial.println("new client");
    bool blankLine = true;

    while ( client.connected() ) {
      if ( client.available() ) {

        c = client.read();

        if ( c == '\n' && blankLine  ) {

          delay(10);
          //respond
          Serial.println("responding");
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type: text/html");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<meta http-equiv=\"refresh\" content=\"1\" />"); 
          client.println("<body>");
          client.print("<h1>eQuake v0.1</h1>\n");
          client.print("<a href=\"https://github.com/murray1978/eQuake\">source</a>\n");
          client.print("<p>");

          client.print(" raw X "); client.println(rawX);
          client.print(" raw Y "); client.println(rawY);
          client.print(" raw Z "); client.println(rawZ);
          client.print("</br>"); client.println("</br>");

          client.print(" Volt x "); client.println(voltX);
          client.print(" Volt y "); client.println(voltY);
          client.print(" Volt z "); client.println(voltZ);
          client.print("</br>"); client.println("</br>");

          client.print("acc x "); client.println(accX);
          client.print("acc y "); client.println(accY);
          client.print("acc z "); client.println(accZ);
          client.print("</br>"); client.println("</br>");

          client.print("max x "); client.println(maxX);
          client.print("max y "); client.println(maxY);
          client.print("max z "); client.println(maxZ);
          client.print("</br>"); client.println("</br>");

          client.println("</p>");
          client.println("</body>");
          client.println("</html>");
          Serial.println("Done responding");
        }
        if ( c == '\n' ) {
          blankLine = true;
        }
        else if ( c != '\r' ) {
          blankLine = false;
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}

