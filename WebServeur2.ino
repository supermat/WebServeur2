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

  //char header[64];
char filename[64];
  int i, j;

  i=0;
  while (header[i++] != ' ') {
    ;
  }
  j=0;
  while (header[i] != ' ') {
    filename[j++] = header[i++];
  }
  filename[j] = 0;
  if (filename[0] == '/' && filename[1] == 0) {
    strcpy(filename, "index.htm");
  }
  char v_retour[j];
  sprintf (filename, "%s", filename);
  //String(filename).toCharArray(v_retour, j);
  return &v_retour[0];
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
                    //char* v_Filename = ExtractFileName(HTTP_req);
                    char *v_Filename;
                    v_Filename = HTTP_req + 5; // look after the "GET /" (5 chars)
                    // a little trick, look for the " HTTP/1.1" string and
                    // turn the first character of the substring into a 0 to clear it out.
                    (strstr(HTTP_req, " HTTP"))[0] = 0;

                      // print the file we want
                      Serial.println(v_Filename);
                      ShowWebPageInSD(client,v_Filename);
                    /*if (StrContains(HTTP_req, "GET / ")
                                 || StrContains(HTTP_req, "GET /index.htm")) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connnection: close");
                        client.println();
                        webFile = SD.open(v_Filename);        // open web page file
                    }
                    else if (StrContains(HTTP_req, "GET /page2.htm")) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connnection: close");
                        client.println();
                        webFile = SD.open(v_Filename);        // open web page file
                    }
                    else if (StrContains(HTTP_req, "GET /pic.jpg")) {
                        webFile = SD.open(v_Filename);
                        if (webFile) {
                            client.println("HTTP/1.1 200 OK");
                            client.println();
                        }
                    }*/
                    /*else if (strstr(clientline, "GET /") != 0) {
          // this time no space after the /, so a sub-file!
          char *filename;

          filename = clientline + 5; // look after the "GET /" (5 chars)
          // a little trick, look for the " HTTP/1.1" string and
          // turn the first character of the substring into a 0 to clear it out.
          (strstr(clientline, " HTTP"))[0] = 0;

          // print the file we want
          Serial.println(filename);

          if (! file.open(&root, filename, O_READ)) {
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<h2>File Not Found!</h2>");
            break;
          }

          Serial.println("Opened!");

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println();

          int16_t c;
          while ((c = file.read()) > 0) {
              // uncomment the serial to debug (slow!)
              //Serial.print((char)c);
              client.print((char)c);
          }
          file.close();
        } else {
          // everything else is a 404
          client.println("HTTP/1.1 404 Not Found");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<h2>File Not Found!</h2>");
        }*/

                    /*if (webFile) {
                        while(webFile.available()) {
                            client.write(webFile.read()); // send web page to client
                        }
                        webFile.close();
                    }*/
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