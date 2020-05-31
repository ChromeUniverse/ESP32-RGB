//
//  ESP32-based Wi-Fi RGB LED controller
// 
//  Written by Lucca Rodrigues 
//  YouTube: Lucca's Lab
//  https://www.youtube.com/channel/UCjpQ2w6Di2f-tyCiK6mVGlA
//  
//  Special thanks to:
//  * Rui Santos for his very informative ESP32 tutorials. 
//  https://randomnerdtutorials.com/
//  * Eduardo Quirino for help with this Arduino sketch.
//  https://github.com/qrno
//

#include <WiFi.h>

// Enter your network credentials here
const char* ssid     = "MyNetwork";
const char* password = "password";

// Set web server at port 80
WiFiServer server(80);

// Storing the HTTP request
String header;

// GPIOs 
const int rLed = 16;
const int gLed = 18;
const int bLed = 21;

// setting up PWM signal parameters for LED dimming 

int freq = 5000;        // PWM frequency (hertz)
int rChannel = 0;       // PWM channel (0 to 15)
int gChannel = 1;
int bChannel = 2;
int resolution = 8;     // PWM duty cycle resolution (1 to 16 bits)

// R, G and B Values 
int rValue = 0;
int gValue = 0;
int bValue = 0;

void setup() {
  Serial.begin(115200);

  // Setting GPIOs as outputs
  pinMode(rLed, OUTPUT);
  pinMode(gLed, OUTPUT);
  pinMode(bLed, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print ESP32 IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // LED dimming setup
  ledcSetup(rChannel, freq, resolution);
  ledcAttachPin(rLed, rChannel);
  ledcSetup(gChannel, freq, resolution);
  ledcAttachPin(gLed, gChannel);
  ledcSetup(bChannel, freq, resolution);
  ledcAttachPin(bLed, bChannel);
}

void loop(){
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if a new client connects,
    Serial.println("New Client.");
    String currentLine = "";                // 'currentLine' stores the current line of the request

    while (client.connected()) {
      if (client.available()) {             // checks if there are unread characters from the request

        char c = client.read();             // c stores the current character we are reading
        Serial.write(c);
        header += c;                        // we'll store the entire request in 'header'

        if (c == '\n') {
          if (currentLine.length() == 0) { 
            /* Note that we'll only enter this conditional with a double line break
            ('\n' on empty line) which signifies the end of an http request */

            /* HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            and a content-type, followed by a blank line */
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // if the request does not contain 'GET /led/' req will be set to -1
            int req = header.indexOf("GET /led/");
            if (req >= 0) {
                                          
              rValue = header.substring(9, 12).toInt();     // chars 9-11
              gValue = header.substring(12, 15).toInt();    // chars 12-14
              bValue = header.substring(15, 18).toInt();    // chars 15-18
              
              // parsed Values
              Serial.print("RGB: ");
              Serial.print(rValue);
              Serial.print(" ");
              Serial.print(gValue);
              Serial.print(" ");
              Serial.println(bValue);

              // adjusting LED brigthness
              ledcWrite(rChannel, rValue);
              ledcWrite(gChannel, gValue);
              ledcWrite(bChannel, bValue);
            }         
          
            // displaying HTML webpage
            client.println("<!DOCTYPE html><html>");
            client.println("<head><title>ESP 32</title></head>");
            client.println("<body>");
              client.print("<p>RGB: ");
              client.print(rValue);
              client.print(" ");
              client.print(gValue);
              client.print(" ");
              client.print(bValue);
              client.println("</p>");
            client.println("</body>");
            client.println("</html>");
            client.println();           // http response ends with blank line

            break;
          } else currentLine == "";

        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

    // Ends connection and clears the header
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
