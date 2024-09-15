#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"

// Wi-Fi
const char* ssid = "hala saied";
const char* password = "4121941462751955";

// To send Emails using Gmail on port 465 (SSL)
#define emailSenderAccount    "sara.p852.2019@gmail.com"
#define emailSenderPassword   "xyli wbez phap fdta"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[ALERT] ESP32 Temperature"

// Default Recipient Email Address
String inputMessage = "sara.p852.2019@gmail.com";
String enableEmailChecked = "checked";
String inputMessage2 = "true";

// Sensors
#define DHTPIN 14
#define DHTTYPE DHT11

#define BMP_SDA 21
#define BMP_SCL 22

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
AsyncWebServer server(80);

float temperature_threshold = 25.0;  // Set your initial temperature threshold
float humidity_threshold = 50.0;     // Set your initial humidity threshold
float pressure_threshold = 1000.0;   // Set your initial pressure threshold
float altitude_threshold = 50.0;
float humidity,  temperature,  pressure, altitude, altitudeAbs;

const char index_html[] PROGMEM = R"rawliteral(
 <!DOCTYPE HTML><html><head>
  <title>Weather Station</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>Temperature</h2> 
  <h3>%Temperatured% &deg;C</h3>
  <h2>Humidity</h2> 
  <h3>%Humidityd%</h3>
  <h2>Pressure</h2> 
  <h3>%Pressured%</h3>
  <h2>Altitude</h2> 
  <h3>%Altituded%</h3>
  <h2>ESP Email Notification</h2>
  <form action="/get">
    Email Address <input type="email" name="email_input" value="%EMAIL_INPUT%" required><br>
    Enable Email Notification <input type="checkbox" name="enable_email_input" value="true" %ENABLE_EMAIL%><br>
    Temperature Threshold <input type="number" step="0.1" name="Temperature_threshold_input" value="%TTHRESHOLD%" required><br>
    Humidity Threshold <input type="number" step="0.1" name="Humidity_threshold_input" value="%HTHRESHOLD%" required><br>
    Pressure Threshold <input type="number" step="0.1" name="Pressure_threshold_input" value="%PTHRESHOLD%" required><br>
    Altitude Threshold <input type="number" step="0.1" name="Altitude_threshold_input" value="%ATHRESHOLD%" required><br>
    <input type="submit" value="Submit">
  </form>
  
</body>
</html>

)rawliteral";
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Replaces placeholder with DS18B20 values
String processor(const String& var){
  //Serial.println(var);
  if(var == "Temperatured"){
    return String(temperature);
  }
  if(var == "Humidityd"){
    return String(humidity);
  }
  if(var == "Pressured"){
    return String(pressure);
  }
  if(var == "Altituded"){
    return String(altitudeAbs);
  }
  else if(var == "EMAIL_INPUT"){
    return inputMessage;
  }
  else if(var == "ENABLE_EMAIL"){
    return enableEmailChecked;
  }
  else if(var == "TTHRESHOLD"){
    return String(temperature_threshold);
  }
  else if(var == "HTHRESHOLD"){
    return String(humidity_threshold);
  }
  else if(var == "PTHRESHOLD"){
    return String(pressure_threshold);
  }
  else if(var == "ATHRESHOLD"){
    return String(altitude_threshold);
  }
  
  return String();
}
// Flag variable to keep track if email notification was sent or not
bool tempemailSent = false;
bool humemailSent = false;
bool presemailSent = false;
bool altemailSent = false;

const char* PARAM_INPUT_1 = "email_input";
const char* PARAM_INPUT_2 = "enable_email_input";
const char* PARAM_INPUT_3 = "Temperature_threshold_input";
const char* PARAM_INPUT_4 = "Humidity_threshold_input";
const char* PARAM_INPUT_5 = "Pressure_threshold_input";
const char* PARAM_INPUT_6 = "Altitude_threshold_input";

unsigned long previousMillis = 0;     
const long interval = 5000;    

// The Email Sending data object contains config and data to send
SMTPData smtpData;


void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("Connected to WiFi!");
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Connect to BMP180
  Wire.begin(BMP_SDA, BMP_SCL);
  while (!bmp.begin())
  {
    Serial.println("Could not find BMP180 sensor, check wiring!");
    delay(1000);
  }
  Serial.println("Connected to BMP180!");

  // Connect to DHT11
  dht.begin();
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  //web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  
  // Receive an HTTP GET request at <ESP_IP>/get?email_input=<inputMessage>&enable_email_input=<inputMessage2>&threshold_input=<inputMessage3>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        enableEmailChecked = "checked";
      }
      else {
        inputMessage2 = "false";
        enableEmailChecked = "";
      }
      // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage3>
      if (request->hasParam(PARAM_INPUT_3)) {
        temperature_threshold = request->getParam(PARAM_INPUT_3)->value().toFloat();
      }
      if (request->hasParam(PARAM_INPUT_4)) {
        humidity_threshold = request->getParam(PARAM_INPUT_4)->value().toFloat();
      }
      if (request->hasParam(PARAM_INPUT_5)) {
        pressure_threshold = request->getParam(PARAM_INPUT_5)->value().toFloat();
      }
      if (request->hasParam(PARAM_INPUT_6)) {
        altitude_threshold = request->getParam(PARAM_INPUT_6)->value().toFloat();
      }
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    Serial.println(temperature_threshold);
    Serial.println(humidity_threshold);
    Serial.println(pressure_threshold);
    Serial.println(altitude_threshold);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {


  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
    temperature = (float)dht.readTemperature();
    humidity = dht.readHumidity();
    pressure = bmp.readPressure() / 100.0; // in hPa
    altitude = bmp.readAltitude();
    altitudeAbs = abs(altitude); // Take the absolute value

    // Check if temperature is above threshold and if it needs to send the Email alert
    if(temperature > temperature_threshold && inputMessage2 == "true" && !tempemailSent){
      String emailMessage = String("Temperature above threshold. Current temperature: ") + 
                            String(temperature) + String("C");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        tempemailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((temperature < temperature_threshold) && inputMessage2 == "true" && tempemailSent) {
      String emailMessage = String("Temperature below threshold. Current temperature: ") + 
                            String(temperature) + String(" C");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        tempemailSent = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }

    // Check if Humidity is above threshold and if it needs to send the Email alert
    if(humidity > humidity_threshold && inputMessage2 == "true" && !humemailSent){
      String emailMessage = String("Humidity above threshold. Current humidity: ") + 
                            String(humidity) + String("%");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        humemailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((humidity < humidity_threshold) && inputMessage2 == "true" && humemailSent) {
      String emailMessage = String("Humidity below threshold. Current humidity: ") + 
                            String(humidity) + String("%");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        humemailSent = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }


    // Check if temperature is above threshold and if it needs to send the Email alert
    if(pressure > pressure_threshold && inputMessage2 == "true" && !presemailSent){
      String emailMessage = String("Pressure above threshold. Current pressure: ") + 
                            String(pressure) + String(" hPa");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        presemailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((pressure < pressure_threshold) && inputMessage2 == "true" && presemailSent) {
      String emailMessage = String("Pressure below threshold. Current pressure: ") + 
                            String(pressure) + String(" hPa");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        presemailSent = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }


    // Check if temperature is above threshold and if it needs to send the Email alert
    if(altitudeAbs > altitude_threshold && inputMessage2 == "true" && !altemailSent){
      String emailMessage = String("Temperature above threshold. Current temperature: ") + 
                            String(altitudeAbs) + String(" M");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        altemailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if((altitudeAbs < altitude_threshold) && inputMessage2 == "true" && altemailSent) {
      String emailMessage = String("Altitude below threshold. Current altitude: ") + 
                            String(altitudeAbs) + String(" M");
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        altemailSent = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Altitude: ");
  Serial.print(altitudeAbs);
  Serial.println(" meters");
  Serial.println(" ********************************* ");

  }
 
}








bool sendEmailNotification(String emailMessage){
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be 
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);

  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  return true;
}

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}