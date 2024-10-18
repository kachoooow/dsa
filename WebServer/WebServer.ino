#include <SPI.h>
#include <Ethernet.h>

float izracunTemp(int vrednost) {
  float rezultat;
  rezultat = (vrednost / 1023.0 * 5000.0 - 400.0) / 19.53;
  return rezultat;
}

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x28};
IPAddress ip(10, 8, 128, 42);

// Initialize the Ethernet server library with the IP address and port you want to use
EthernetServer server(80);

// LED pin
const int ledPin = 2;
bool blinkLED = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Ensure LED is off initially

  // Start serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // Start the Ethernet connection and the server
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found. Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // Do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // Start the server
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("New client");
    String currentLine = ""; // Make a String to hold incoming data from the client

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n') {
          // If the current line is blank, you got two newline characters in a row.
          // That's the end of the client HTTP request, so send a response.
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close"); // The connection will be closed after completion of the response
            client.println("Refresh: 15"); // Refresh the page automatically every 15 seconds
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<style>");
            client.println("body { font-family: Arial, sans-serif; background-color: #f0f0f0; }");
            client.println("h1 { background-color: DodgerBlue; color: white; padding: 10px; }");
            client.println("button { padding: 10px 20px; margin: 5px; font-size: 16px; border-radius: 12px; }");
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>MOJ DOMACI STREZNIK</h1>");

            // Output the value of each analog input pin
            int sensorValue = analogRead(A1);
            float temperatura = izracunTemp(sensorValue);
            client.println("<p>Temperatura je: ");
            client.println(temperatura);
            client.println("</p>");
            delay(200); // Delay in between reads for stability
            int svetlost = analogRead(A0);
            client.println("<p>Svetlost je: ");
            client.println(svetlost);
            client.println("</p>");

            // Add buttons for LED control
            client.println("<br><br>");
            client.println("<button onclick=\"location.href='H'\">Vklopi LED</button>");
            client.println("<button onclick=\"location.href='L'\">Izklopi LED</button>");
            client.println("<button onclick=\"location.href='B'\">Utripanje LED</button>");

            client.println("</body>");
            client.println("</html>");
            break;
          }
          else {
            currentLine = "";
          }
        }
        else if (c != '\r') {
          currentLine += c; // Add character to currentLine

          // Check for the LED control commands
          if (currentLine.endsWith("GET /H")) {
            controlLED(HIGH);
            blinkLED = false;
          } else if (currentLine.endsWith("GET /L")) {
            controlLED(LOW);
            blinkLED = false;
          } else if (currentLine.endsWith("GET /B")) {
            blinkLED = true;
          }
        }
      }
    }
    // Give the web browser time to receive the data
    delay(1);
    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }

  // Blink LED if blink mode is enabled
  if (blinkLED) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void controlLED(int state) {
  digitalWrite(ledPin, state);
}
