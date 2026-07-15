#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "pug palace";
const char* password = "";

#define RED_PIN 5
#define GREEN_PIN 17
#define BLUE_PIN 16

WebServer server(80);

String History = "";

void setColor(int r, int g, int b) {
  ledcWrite(0,r);
  ledcWrite(1,g);
  ledcWrite(2,b);
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name'viewport' content='width=device-width, initial-scale=1.0'>
    <title>Document</title>
</head>
<body>
    <h1>welcome to my offgrid server.</h1>
    <div id='chat'></div>
    <input type='color' id='color'>
    <input type='text' id='username'>
    <input type='text' id='msg'>
    <script>
        const color = document.getElementById('color')
        const msg = document.getElementById('msg')
        const chat = document.getElementById('chat')
        const u = document.getElementById('username')
        msg.addEventListener('change', () => {
            chat.innerHTML = chat.innerHTML + `<p style='color: ${color.value}'>[${u.value}] ${msg.value}</p>`
            fetch(`/color?hex=${encodeURIComponent(color.value)}`)
            fetch(`/sendChat?user=${encodeURIComponent(u.value)}&msg=${encodeURIComponent(msg.value)}&color=${encodeURIComponent(color.value)}`);
            msg.value = ''
        })
        setInterval(() => {
            fetch('/getChat')
                .then(response => response.text())
                .then(data => {
                    chat.innerHTML = data;
                    // Auto-scroll to the bottom
                    chat.scrollTop = chat.scrollHeight; 
                });
        }, 1500);
    </script>
</body>
</html>)rawliteral");
}

void handleSendChat() {
  String user = server.arg("user");
  String msg = server.arg("msg");
  String c = server.arg("color");

  String newEntry = "<p style='color: " + c + "; margin: 2px 0;'><b>[" + user + "]</b> " + msg + "</p>";
  History += newEntry;
}

void handleGetChat() {
  server.send(200, "text/plain", History);
}

void handleColor() {
  String hexs = server.arg("hex");
  if (hexs.startsWith("#")) {
    hexs.remove(0,1);
  }
  long hex = strtol(hexs.c_str(), NULL, 16);
  byte r = (hex >> 16) & 0xFF;
  byte g = (hex >> 8) & 0xFF;
  byte b = hex & 0xFF;
  setColor(r,g,b);
  server.sendHeader("Location","/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("connecting to wifi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n Connected!, IP address: ");
  Serial.println(WiFi.localIP());
  ledcSetup(0, 5000, 8);
  ledcAttachPin(RED_PIN, 0);
  ledcSetup(1, 5000, 8);
  ledcAttachPin(GREEN_PIN, 1);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(BLUE_PIN, 2);

  server.on("/", handleRoot);
  server.on("/color", handleColor);
  server.on("/sendChat", handleSendChat);
  server.on("/getChat", handleGetChat);
  server.begin();
  Serial.println("\n Web Server Sarted! ");
}

void loop() {
  server.handleClient();
}






