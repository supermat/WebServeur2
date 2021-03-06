/*--------------------------------------------------------------
  Program:      eth_websrv_SD_image

  Description:  Arduino web server that serves up a basic web
                page that displays an image.

  Hardware:     Arduino Uno and official Arduino Ethernet
                shield. Should work with other Arduinos and
                compatible Ethernet shields.
                2Gb micro SD card formatted FAT16

  Software:     Developed using Arduino 1.0.5 software
                Should be compatible with Arduino 1.0 +

                Requires index.htm, page2.htm and pic.jpg to be
                on the micro SD card in the Ethernet shield
                micro SD card socket.

  References:   - WebServer example by David A. Mellis and
                  modified by Tom Igoe
                - SD card examples by David A. Mellis and
                  Tom Igoe
                - Ethernet library documentation:
                  http://arduino.cc/en/Reference/Ethernet
                - SD Card library documentation:
                  http://arduino.cc/en/Reference/SD

  Date:         7 March 2013
  Modified:     17 June 2013

  Author:       W.A. Smith, http://startingelectronics.com
--------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
//#include "Dht11.h"
//#include <DHT/DHT.h>
byte dht11_dat[5];
#define dht11_pin 14
//#define DHTPIN 2     // what pin we're connected to
//#define DHTTYPE DHT11   // DHT 11
//DHT dht(DHTPIN, DHTTYPE);x

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   20

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 20); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer


// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    return (strstr(str, sfind) != 0);
}

char* ExtractFileName(char* header)
{

  char * filename = (char *) malloc (64);
//char filename[64];
  int i, j;

  i=0;
  while (header[i++] != ' ') {
    ;
  }
  i++;
  j=0;
  while (header[i] != ' ') {
    filename[j++] = header[i++];
  }
  filename[j] = 0;
  if (filename[0] == 0) {
    strcpy(filename, "index.htm");
  }
  //char v_retour[j];
  //sprintf (v_retour, "%s", filename);
  //String(filename).toCharArray(v_retour, j);
  //return &v_retour[0];
  return filename;
}

void ShowWebPageInSD(EthernetClient client,char *p_Filename)
{
    if(SD.exists(p_Filename))
    {
      File webFile;
      webFile = SD.open(p_Filename);        // open web page file
      if (webFile) {

          client.println("HTTP/1.1 200 OK");
          if (StrContains(p_Filename, ".jpg") || StrContains(p_Filename, ".ico"))
          {
              //RAS
          }
          else if(StrContains(p_Filename, ".htm"))
          {
              client.println("Content-Type: text/html");
              client.println("Connnection: close");
          }
          else
          {
              client.println("Content-Type: text/plain");
              client.println("Connnection: close");
          }
         client.println();

        while(webFile.available()) {
            client.write(webFile.read()); // send web page to client
        }
        webFile.close();
      }
      else
      {
        Serial.print("Erreur �� l'ouverture du fichier");
      }
    }
    else
    {
      Serial.println("Erreur 404");
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: text/html");
      client.println();
      client.println("<h2>Fichier non trouve!</h2>");
    }

}


byte read_dht11_dat()
{
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while (!digitalRead(dht11_pin));
    delayMicroseconds(30);
    if (digitalRead(dht11_pin) != 0 )
      bitSet(result, 7-i);
    while (digitalRead(dht11_pin));
  }
  return result;
}


  void dht11_read()
{

  byte i;// start condition

  digitalWrite(dht11_pin, LOW);
  delay(18);
  digitalWrite(dht11_pin, HIGH);
  delayMicroseconds(1);
  pinMode(dht11_pin, INPUT);
  delayMicroseconds(40);

  if (digitalRead(dht11_pin))
  {
    //Serial.println("dht11 start condition 1 not met"); // wait for DHT response signal: LOW
    delay(1000);
    return;
  }
  delayMicroseconds(80);
  if (!digitalRead(dht11_pin))
  {
    //Serial.println("dht11 start condition 2 not met");  //wair for second response signal:HIGH
    return;
  }

  delayMicroseconds(80);// now ready for data reception
  for (i=0; i<5; i++)
  {  dht11_dat[i] = read_dht11_dat();}  //recieved 40 bits data. Details are described in datasheet

  pinMode(dht11_pin, OUTPUT);
  digitalWrite(dht11_pin, HIGH);
  byte dht11_check_sum = dht11_dat[0]+dht11_dat[2];// check check_sum
  if(dht11_dat[4]!= dht11_check_sum)
  {
    //Serial.println("DHT11 checksum error");
  }
  /*Serial.print("Current humdity = ");
  Serial.print(dht11_dat[0], DEC);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(dht11_dat[2], DEC);
  Serial.println("C  ");*/
  delay(2000); //fresh time
}

// send the state of the switch to the web browser
void WS(EthernetClient cl, char* p_req)
{
  //Serial.println("Web Services");
    dht11_read();
    if (StrContains(p_req, "temp")){
            String s = String(dht11_dat[2],DEC);
    Serial.println(s);
        cl.println(s);
      }
     else if (StrContains(p_req, "humidite")){
         String s = String(dht11_dat[0],DEC);
    Serial.println(s);
            cl.println(s);
     }
     else{
        cl.println("?");
     }
}

void setup()
{
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);

    Serial.begin(9600);       // for debugging

    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR - Can't find index.htm file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");

    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients

     //dht.begin();

  pinMode(dht11_pin, OUTPUT);
  digitalWrite(dht11_pin, HIGH);
}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // print HTTP request character to serial monitor
                Serial.print(c);
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // open requested web page file
                    char* v_Filename = ExtractFileName(HTTP_req);
                    if(StrContains(v_Filename,"ws/"))
                    {
                        WS(client,v_Filename);
                    }
                    else
                    {
                      ShowWebPageInSD(client,v_Filename);
                    }
                      free (v_Filename); //On lib��re la m��moire allou��e
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                }
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}
