#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Your WiFi SSID";
const char* password = "Your WiFi Password";

// Define pin numbers
const int digitalPins[] = {1, 3, 5, 16, 17, 18, 19, 21, 22, 23};
const int analogPins[] = {2, 4, 12, 13, 14, 15, 25, 26, 27, 32, 33};

// Initialize AsyncWebServer object on port 8080
AsyncWebServer server(8080);

void setup() {
  // Initialize Serial port for debugging
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // Set digital pins to output mode
  for (int i = 0; i < sizeof(digitalPins) / sizeof(digitalPins[0]); i++) {
    pinMode(digitalPins[i], OUTPUT);
  }

  // Define server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String htmlContent = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>ESP32 GPIO Control</title><style>:root {--bg-color: #121212; --text-color: #ffffff; --button-bg: #00bcd4; --button-hover: #0097a7;} body{font-family:Arial, sans-serif;background-color:var(--bg-color);color:var(--text-color);margin:0;padding:20px;} .button{background-color:var(--button-bg);color:#ffffff;border:none;padding:10px 20px;text-align:center;text-decoration:none;display:inline-block;font-size:16px;border-radius:5px;cursor:pointer;margin-right:10px;} .button:hover{background-color:var(--button-hover);} .container{display:flex;justify-content:space-between;}.control-container{background-color:#212121;padding:20px;border-radius:10px;width:48%;}</style></head><body><h1>Welcome to ESP32 GPIO control</h1><div class=\"container\"><div class=\"control-container\"><h2>Digital Pins Control</h2>";

    // Add control elements for digital pins
    for (int i = 0; i < sizeof(digitalPins) / sizeof(digitalPins[0]); i++) {
      htmlContent += "<p>Pin D" + String(digitalPins[i]) + " (Digital) <div class=\"button-container\"><button class=\"button\" onclick=\"toggleDigitalPin(" + String(digitalPins[i]) + ")\">Toggle</button> <span id=\"digitalPinState" + String(digitalPins[i]) + "\">State: -</span></div></p>";
    }
    htmlContent += "</div><div class=\"control-container\"><h2>Analog Pins Control</h2>";

    // Add control elements for analog pins with sliders
    for (int i = 0; i < sizeof(analogPins) / sizeof(analogPins[0]); i++) {
      htmlContent += "<p>Pin A" + String(analogPins[i]) + " (Analog) <div class=\"slider-container\"><input type=\"range\" min=\"0\" max=\"100\" value=\"0\" class=\"slider\" data-pin=\"" + String(analogPins[i]) + "\"></div> <span id=\"analogPinValue" + String(analogPins[i]) + "\">Value: 0%</span></p>";
    }
    htmlContent += "</div></div><script>";

    // Add JavaScript function for controlling digital pins
    for (int i = 0; i < sizeof(digitalPins) / sizeof(digitalPins[0]); i++) {
      htmlContent += "function toggleDigitalPin(pin){fetch('/control?pin='+pin+'&value=toggle').then(response => response.text()).then(text => updateDigitalPinState(pin, text));}";
      htmlContent += "function updateDigitalPinState(pin, state){document.getElementById('digitalPinState'+pin).innerText = 'State: ' + state;}";
    }
    // Add JavaScript function for controlling analog pins and real-time updates
    htmlContent += "function updateAnalogPinValue(pin, value) { fetch('/analog?pin=' + pin + '&value=' + value).then(response => response.text()).then(text => { document.getElementById('analogPinValue' + pin).innerText = 'Value: ' + value + '%'; }); }";
    htmlContent += "var sliders = document.querySelectorAll('.slider');";
    htmlContent += "sliders.forEach(function(slider) {";
    htmlContent += "  slider.addEventListener('input', function() {";
    htmlContent += "    var pin = this.dataset.pin;";
    htmlContent += "    var value = this.value;";
    htmlContent += "    updateAnalogPinValue(pin, value);";
    htmlContent += "  });";
    htmlContent += "});";
    htmlContent += "</script></body></html>";

    request->send(200, "text/html", htmlContent);
  });

  // Route to control digital GPIO pins
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("pin") && request->hasParam("value")) {
      int pin = request->getParam("pin")->value().toInt();
      String value = request->getParam("value")->value();
      if (value == "toggle") {
        digitalWrite(pin, !digitalRead(pin));
        // Update pin state
        request->send(200, "text/plain", String(digitalRead(pin)));
      } else {
        digitalWrite(pin, value.toInt());
        // Update pin state
        request->send(200, "text/plain", value);
      }
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  // Route to control analog GPIO pins
  server.on("/analog", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("pin") && request->hasParam("value")) {
      int pin = request->getParam("pin")->value().toInt();
      int value = request->getParam("value")->value().toInt();
      int scaledValue = map(value, 0, 100, 0, 255);
      analogWrite(pin, scaledValue);
      // Update pin state
      request->send(200, "text/plain", String(value));
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Nothing to do in loop for this example
}
