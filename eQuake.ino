/*
 * eQuake
 * arduino/android quake tool thing(at the moment) 
 * 
 * TODO
 *  Timestamp,
 *  Datalogging,
 *  FTP download of data, may need to go MEGA.
 *  Graphing of data see http://www.geonet.org.nz/quakes/drums for an idea. And http://www.tutorialspark.com/html5/HTML5_Canvas_Graphs_Charts.php
*/
#include <Ethernet.h>
#include <SPI.h>

//set to zero if you have not done http://forum.arduino.cc/index.php?topic=82416.0 
#define ETHERHACK 1

/*
 * Using the freetronic accelerometer,
 *  0g = 1/2 supply rail.
 *  0g = 3.3v / 2 or 1.65v
 *  used in 1.5g mode
 *  so 800mV/g
 *  3.3 / 1023 = 0.0032v bit
 *  1.65 / 0.0032 = 515.625 = 1.65 - 1.65 = 0g, should work wright?
*/
//Please change this for your setup, your nic may have a hardcoded MAC address
byte mac[] = { 0x10, 0xBA, 0xD0, 0xBE, 0xEF, 0x00 };
byte rip[] = {0, 0, 0, 0}; //client IP address

//Sett ip, gateway and subnet to your local values.
//do not use DHCP if you plan to use port forwarding on your router. 
IPAddress ip(192, 168, 2, 123);
IPAddress gateway( 192, 168, 2, 1);
IPAddress subnet( 255, 255, 255, 0);

//HTTP port
EthernetServer server(80);
//FTP port, NOT YET IMPLIMENTED
EthernetServer FTPServer(21);

//Accelerometer sensors are on analog pins..
const int xSensor = 0;
const int ySensor = 1;
const int zSensor = 2;

//Sensor data.
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

float const blahh = (float)(3.3f / 1024); //blahh its a real value honestly.
float const halfVolt = 1.65;
float const gVolt = 0.8f;

void setup() {

  Serial.begin(9600);
  //3.3 0-1024 resolution
  analogReference(EXTERNAL);    
  delay(50);
  Serial.println("eQuake setup"); 
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.println("Ready");

}

/*
 * TODO, sensor data rate 10Hz
 */ 
void loop() {
  readSensors();
  clients();
}

/*
 * TODO Sensor data neededs to be checked over.
*/
void readSensors() {
  float tempAcc = 0.0f;

  rawX = analogRead(xSensor);
  rawY = analogRead(ySensor);
  rawZ = analogRead(zSensor);

  voltX = (float)(blahh * rawX);
  voltY = (float)(blahh * rawY);
  voltZ = (float)(blahh * rawZ);

  /*
   * g = (V - 1.65) / 0.8
   * Took to long to figure that bit out.
   */
  tempAcc = (float)(( voltX - halfVolt) / gVolt );
  accX = tempAcc - accX;
  (maxX < accX) ? maxX = accX : maxX = maxX;

  tempAcc = (float)(( voltY - halfVolt ) / gVolt );
  accY = tempAcc - accY;
  (maxY < accY ) ? maxY = accX : maxY = maxY;

  tempAcc = (float)(( voltZ - halfVolt ) / gVolt );
  accZ = tempAcc - accZ;
  (maxZ < accZ ) ? maxZ = accZ : maxZ = maxZ;

}

/*
 * React to a clients HTTP GET request
 * Simply checks if input[0] == '\n'
 * Works OK with upto 3 clients so far.
 * TODO Graphing of data, see http://www.geonet.org.nz/quakes/drums
 */
void clients() {

  EthernetClient client = server.available();

  if ( client ) {
    Serial.print("new client");

    //Display IP address from http://forum.arduino.cc/index.php?topic=82416.0 
#ifdef ETHERHACK
    client.getRemoteIP(rip);
    for ( int bcount = 0; bcount < 4; bcount++) {
      Serial.print(rip[bcount], DEC );
      if ( bcount < 3 ) Serial.print('.');
    }
    Serial.println();
#endif
    /*
     * Same as the example programs except
     */
    while ( client.connected() ) {
      if ( client.available() ) {

        /*
         * Since this is a simple server we respone on ever <cr><lf> pair
         * 
         */
        if ( client.read() == '\r') { //only <cr> in the HTTP request
          delay(10);

          //respond
          Serial.println("responding");
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type: text/html");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<meta http-equiv=\"refresh\" content=\"5\" />"); //Refresh ever 30seconds, change for quicker or longer refreshness
          client.println("<head>");
          client.println("<style>body{ margin: 10px; padding: 0px;}</style>");
          client.println("</head>");
          client.println("<body>");
          client.print("<h1>eQuake v0.2</h1>\n");
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
          break; //must break other wise data is resent for other requests ie "GET /favicon.ico HTTP/1.1"
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}

