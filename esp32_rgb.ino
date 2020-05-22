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

// Load Wi-Fi library
#include <WiFi.h>

// Enter your network credentials here
const char* ssid     = "MyNetwork";
const char* password = "password";

// Set web server port number to 80
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
int resolution = 8;     // PWM duty cycle resolution (1 to 16 bits

// R, G and B Strings and Values 
String rString = "";   
String gString = "";
String bString = "";
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
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // set RGB colors
            String RGB = "";           
            int req = header.indexOf("GET /led/");
            if (req >= 0) {
                                          
              rString = "";
              rString += header[9];
              rString += header[10];
              rString += header[11];
                            
              gString = "";
              gString += header[12];
              gString += header[13];
              gString += header[14];
                            
              bString = "";
              bString += header[15];
              bString += header[16];
              bString += header[17];
              
              rValue = rString.toInt();
              gValue = gString.toInt();
              bValue = bString.toInt(); `           

              // printing out the final RGB values                     
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
          

            // Displaying HTML webpage
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<p>" + rString + " " +  gString + " " +  bString + "</p>");         
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
