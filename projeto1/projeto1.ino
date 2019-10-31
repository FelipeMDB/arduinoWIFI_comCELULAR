#include "WiFiEsp.h"

// Emulate Serial1 on pins 6/7 if not present
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1a(8, 9); // RX, TX
//#endif

char ssid[] = "TSE_FELIPEBRITTO";         // your network SSID (name)
char pass[] = "RA:18200";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received

WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
  Serial.begin(115200);   // initialize serial for debugging
   while (!Serial) ;
  Serial1a.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial1a);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield nao esta presente");
    while (true); // don't continue
  }

  Serial.print("Tentando inicializar AP ");
  Serial.println(ssid);

  // "settar IP do PC"
  IPAddress localIp(177, 220, 18, 102);
  WiFi.configAP(localIp);
  
  // start access point
  status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);

  Serial.println("Ponto de acesso inicializado");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
  Serial.println("Servidor inicializado");
}


void loop()
{
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    Serial.println("Novo cliente");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        char c = client.read();               // read a byte, then
        buf.push(c);                          // push it to the ring buffer
        Serial.print(c);
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n\r\n")){
          sendHttpResponse(client);
          Serial.println("Fechando"); 
          break;
        }

        if (buf.endsWith("GET /LG?")){
          digitalWrite(2, LOW);
          buf.reset();
          Serial.println(" Ligado");
        }
        else if (buf.endsWith("GET /DS?")){
          digitalWrite(2, HIGH);
          buf.reset();
          Serial.println(" Desligado");
        }
        
      }
    }
    
    // give the web browser time to receive the data
    delay(10);

    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}


void printWifiStatus()
{
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, connect to ");
  Serial.print(ssid);
  Serial.print(" and open a browser to http://");
  Serial.println(ip);
  Serial.println();
}





void sendHttpResponse(WiFiEspClient client)
{
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // the connection will be closed after completion of the response // refresh the page automatically every 20 sec
    "\r\n");
  client.print("<!DOCTYPE HTML>\r\n");
  client.print("<html>\r\n");
  client.print("<body style=\"background: yellow\">\r\n");
  client.print("<center>\r\n");
  client.print("<h1>Liga e desliga com Wifi</h1>\r\n");
  client.print("<br>\r\n");
   // \"/H\"
  client.print(" <br><br><br><form method=\"get\" action=\"LG\"><button style=\"background: green\" type=\"submit\">LIGAR</button></form>");
  client.print(" <br><br><br><form method=\"get\" action=\"DS\"><button style=\"background: red\" type=\"submit\">DESLIGAR</button></form>");
  client.print("</center>\r\n");
  client.print("</body>\r\n");
  client.print("</html>\r\n");
 
 
}
