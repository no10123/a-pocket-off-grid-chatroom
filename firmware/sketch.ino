#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>

const char* ssid = "pug palace";
const char* password = "";

#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 14

#define D20_PIN 15
#define D12_PIN 32
#define D10_PIN 33
#define D8_PIN 23
#define D6_PIN 26
#define D4_PIN 27

// screen width, height, reset, and adress
#define W 128
#define H 64
#define R -1
#define A 0x3C

Adafruit_SSD1306 display(W, H, &Wire, R);

void handleText(char* text) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.println(F(text));
  display.display();
}

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

void RollDie (int sides) {
  int result = random(1, sides + 1);
  char* r[] = {"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20"};
  handleText(r[result]);
  setColor(255,(255 * (result/sides)), 0);
  delay(500);
  setColor(0 ,0 , 0);
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

bool canRoll[6] = {};

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

  pinMode(D20_PIN, INPUT);
  pinMode(D12_PIN, INPUT);
  pinMode(D10_PIN, INPUT);
  pinMode(D8_PIN, INPUT);
  pinMode(D6_PIN, INPUT);
  pinMode(D4_PIN, INPUT);

  server.on("/", handleRoot);
  server.on("/color", handleColor);
  server.on("/sendChat", handleSendChat);
  server.on("/getChat", handleGetChat);
  server.begin();
  Serial.println("\n Web Server Sarted! ");
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, A)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  setColor(225,0,225);

  handleText("running"); 
  delay(2000);
  setColor(0,0,0);
  handleText("");
  //canRoll = {digitalRead(D20_PIN) == 1, digitalRead(D12_PIN) == 1, digitalRead(D10_PIN) == 1, digitalRead(D8_PIN) == 1, digitalRead(D6_PIN) == 1, digitalRead(D4_PIN) == 1};
  
}

void loop() {
  server.handleClient();
  Serial.print(digitalRead(D20_PIN));
  Serial.print(" | ");
  Serial.print(digitalRead(D12_PIN));
  Serial.print(" | ");
  Serial.print(digitalRead(D10_PIN));
  Serial.print(" | ");
  Serial.print(digitalRead(D8_PIN));
  Serial.print(" | ");
  Serial.print(digitalRead(D6_PIN));
  Serial.print(" | ");
  Serial.println(digitalRead(D4_PIN));

  if (false) {}
  else if (digitalRead(D20_PIN) == 0 && canRoll[0]) {RollDie(20);}
  else if (digitalRead(D12_PIN) == 0 && canRoll[1]) {RollDie(12);}
  else if (digitalRead(D10_PIN) == 0 && canRoll[2]) {RollDie(10);}
  else if (digitalRead(D8_PIN) == 0 && canRoll[3])  {RollDie(8);}
  else if (digitalRead(D6_PIN) == 0 && canRoll[4])  {RollDie(6);}
  else if (digitalRead(D4_PIN) == 0 && canRoll[5])  {RollDie(4);}
  else {
    if (false) {}
    else if (digitalRead(D20_PIN) == 1 && !canRoll[0]) {RollDie(20);canRoll[0] = true;}
    else if (digitalRead(D12_PIN) == 1 && !canRoll[1]) {RollDie(12);canRoll[1] = true;}
    else if (digitalRead(D10_PIN) == 1 && !canRoll[2]) {RollDie(10);canRoll[2] = true;}
    else if (digitalRead(D8_PIN) == 1 && !canRoll[3])  {RollDie(8);canRoll[3] = true;}
    else if (digitalRead(D6_PIN) == 1 && !canRoll[4])  {RollDie(6);canRoll[4] = true;}
    else if (digitalRead(D4_PIN) == 1 && !canRoll[5])  {RollDie(4);canRoll[5] = true;}
  }
}

//5 bread an hour is 60 / 5 which is 12 so 1 bread each 12 min.




