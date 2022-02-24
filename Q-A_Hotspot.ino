#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char *ssid = "test";
const char *password = "password";
WiFiServer server(80);

String header;
String output5State = "off";
const int output5 = 5;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

int numQ = 3;
int numOp = 2;
String ques[3] = {"Question 1?","Question 2?","Question 3?"};
String ops[3][2] = {{"Q1Op1","Q1Op2"},{"Q2Op1","Q2Op2"},{"Q3Op1","Q3Op2"}};
int cor[3] = {0,1,1};

int curQ = 0;
int curA = -1;
int curC = 0;
int curW = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();
}

void loop(){
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();         
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /home/") >= 0){
              curQ = 0;
            }
            else if(header.indexOf("GET /Q") >=0){
              curQ = (int)header.charAt(header.indexOf("GET /Q")+6) - 48;
              curA = (int)header.charAt(header.indexOf("GET /Q"+(String)curQ+"/")+8) - 48;
              if(curQ > 1 && curQ < numQ+1)
                if(curA == cor[curQ-2]) curC+=1;
                else curW+=1;
            }
            else if(header.indexOf("GET /en/") >=0){
              curQ = numQ+1;
              curA = (int)header.charAt(header.indexOf("GET /en/")+8) - 48;
              if(curA == cor[curQ-2]) curC+=1;
              else curW+=1;
            }
            
            Serial.print("Cur Q: ");
            Serial.print(curQ);
            Serial.print("  Cur A: ");
            Serial.print(curA);            
            Serial.print("  Cur C: ");
            Serial.print(curC);            
            Serial.print("  Cur W: ");
            Serial.println(curW);
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head>");          
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head>");    
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            if(curQ == 0){
              client.println("<p>Lets Start the questions</p>");
              client.println("<p><a href=\"/Q1/0\"><button>Start</button></a></p>");
            }
            else if(curQ > 0 && curQ < numQ+1){
              client.println("<p>"+ques[curQ-1]+"</p>");
              if(curQ!=numQ)
                for(int i=0;i<numOp;i++)
                  client.println("<p><a href=\"/Q"+(String)(curQ+1)+"/"+(String)i+"\"><button>" +ops[curQ-1][i] +"</button></a></p>");
              else
                for(int i=0;i<numOp;i++)
                  client.println("<p><a href=\"/en/"+(String)i+"\"><button>" +ops[curQ-1][i] +"</button></a></p>");
            }
            else if(curQ == numQ+1){
              client.println("<p>Questions Done</p>");
              client.println("<p>Num Correct: "+(String)curC+"</p>");
              client.println("<p>Num Wrong: "+(String)curW+"</p>");
            }
            
            client.println("</body></html>");            
            client.println();
            break;
          } 
          else {
            currentLine = "";
          }
        }
        else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
