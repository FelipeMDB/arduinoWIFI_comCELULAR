#include "WiFiEsp.h"
#include "SoftwareSerial.h"
SoftwareSerial Serial1a(8, 9); // RX, TX  inicia porta para recepção e transmissão
//#endif

char ssid[] = "TSE_FELIPEBRITTO";         // SSID da rede (nome)
char pass[] = "RA:18200";        // senha da rede 
int status = WL_IDLE_STATUS;     // status do WIfi
int reqCount = 0;                // numero de respostas recebidas

WiFiEspServer server(80); //porta que será utilizada

// ringBuffer para aumentar a memória e diminuir a quantidade de memória gasta
RingBuffer buf(8);

void setup()
{
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
  Serial.begin(115200);   // inicializa porta serial para debbug
   while (!Serial) ;
  Serial1a.begin(9600);    // inicialização serial para o módulo ESP
  WiFi.init(&Serial1a);    // inicialização do módulo ESP

  // verifica se o "Wifi shield" está conectado
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield nao esta presente");
    while (true); // para programa
  }

  Serial.print("Tentando inicializar AP ");
  Serial.println(ssid);

  // "settar IP do PC"
  IPAddress localIp(177, 220, 18, 102);
  WiFi.configAP(localIp);
  
  // inicializa o ponto de acesso
  status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);

  Serial.println("Ponto de acesso inicializado");
  printWifiStatus();
  
  // inicializa o servidor web na porta 80
  server.begin();
  Serial.println("Servidor inicializado");
}


void loop()
{
  WiFiEspClient client = server.available();  // permite conexão com clientes disponíveis

  if (client) {                               // ao receber um cliente
    Serial.println("Novo cliente");             // printa a mensagem na porta serial
    buf.init();                               // inicializa o buffer
    while (client.connected()) {              // loop enquanto o cliente estiver conectado
      if (client.available()) {               // recebeu dados do cliente
        char c = client.read();               // lê os dados
        buf.push(c);                          // push nas informações
        Serial.print(c);
        
        // fim do HTTP response
        if (buf.endsWith("\r\n\r\n")){
          sendHttpResponse(client);
          Serial.println("Fechando"); 
          break;
        }


        //codigo para açoes vindas da pagina HTML

        //se a resposta termina com "LG" deve-se ligar o rele
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
    
    // tempo para o browser receber os dados
    delay(10);

    // fecha a conexao
    client.stop();
    Serial.println("Cliente desconectado");
  }
}


//metodo para vizualizacao do status do Wifi
void printWifiStatus()
{
  // print o Wifi Shield e o endereço de IP
  IPAddress ip = WiFi.localIP();
  Serial.print("Endereço de IP: ");
  Serial.println(ip);

  // printa pra onde esta sendo mandado no browser (o IP e como se conectar a ele)
  Serial.println();
  Serial.print("Para ver esta pagina funcionando, conecte-se a ");
  Serial.print(ssid);
  Serial.print(" e abra uma pagina para http://");
  Serial.println(ip);
  Serial.println();
}



//codigo da pagina HTML

void sendHttpResponse(WiFiEspClient client)
{
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"  // conexão será fechada depois de rodar todo o código // reinicia a pagina a cada 20seg
    "\r\n");

    //corpo da pagina
  client.print("<!DOCTYPE HTML>\r\n");
  client.print("<html>\r\n");
  client.print("<body style=\"background: yellow\">\r\n");
  client.print("<center>\r\n");
  client.print("<h1>Liga e desliga com Wifi</h1>\r\n");
  client.print("<br>\r\n");
   // \"/H\"
  client.print(" <br><br><br><form method=\"get\" action=\"LG\"><button style=\"background: green\" type=\"submit\">LIGAR</button></form>");  //botao para ligar
  client.print(" <br><br><br><form method=\"get\" action=\"DS\"><button style=\"background: red\" type=\"submit\">DESLIGAR</button></form>"); //botao para desligar
  client.print("</center>\r\n");
  client.print("</body>\r\n");
  client.print("</html>\r\n");
 
 
}
