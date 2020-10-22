#include "include/CaptivePortal.h"

const char *ssidAP = "EnergyCounterESP32 AP";

String indexPage(int num_ssid, String ssid_list[50]) {
  String header_page = R"(
    <!DOCTYPE HTML>
    <html>
    <head> <title>ESP32 Configuration Panel</title> </head>
    <body>
    <h1>Configuration panel</h1>
    <p>Energy counter configuration panel</p>
    <form action="/connect" method="post">
      SSID:<br>
      <select name="network">
     )";

  String auth_form = R"(
        </select> <br><br>
        
        Username:<br>
        <input type="username" name="username"> *only for WPA2 Enterprise networks<br>
        Password:<br>
        <input type="password" name="password"><br><br> )";
  String footer_page = R"(
    
    <h2>Configurazione broker</h2>
    Indirizzo ip broker:
    <input type="text" name="brokerip"><br>
    Topic appartamento 1: (ES: fila/numero)<br>
    <input type="text" name="topic1"><br>
    Topic appartamento 2: <br>
    <input type="text" name="topic2"><br>
    Topic appartamento 3:<br>
    <input type="text" name="topic3"><br>
    Topic appartamento 4:<br>
    <input type="text" name="topic4"><br><br>
    <input type="submit">
  </form>
  
  </body>
  </html>)";

  return header_page + getNetworks(num_ssid, ssid_list) + auth_form + footer_page;
}

void setupNetwork(){
  Serial.println("Starting AP mode...");
  WiFi.softAP(ssidAP);
}

void scanNetworks(int *num_ssid, String ssid_list[50]) {
  Serial.println("*** Scan Networks ***");
  *num_ssid = WiFi.scanNetworks();
  if ( *num_ssid == -1 ) {
    Serial.println("Nessuna rete wifi presente");
    while (true);
  }
  
  Serial.print("Reti trovate: ");
  Serial.println(*num_ssid);

  for (int thisNet = 0; thisNet < *num_ssid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    Serial.println(getEncryptionType(WiFi.encryptionType(thisNet)));
    ssid_list[thisNet] = WiFi.SSID(thisNet);
  }
}

boolean selectEncryptionType(String encryption, String ssid, String username, String password) {
  char ssidToCharArray[32];
  char usernameToCharArray[63];
  char passwordToCharArray[50];
  ssid.toCharArray(ssidToCharArray, ssid.length());
  if (encryption.equals("Open")) {
    return connectOpenNetwork(ssidToCharArray);
  }else if (encryption.equals("WPA2_Enterprise")) {
    username.toCharArray(usernameToCharArray, username.length()+1);
    password.toCharArray(passwordToCharArray, password.length()+1);
    return connectWpa2Enterprise(ssidToCharArray, usernameToCharArray, passwordToCharArray);
  }else {
    password.toCharArray(passwordToCharArray, password.length()+1);
    return connectWpa(ssidToCharArray, passwordToCharArray);
  }
}

String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (0):
      return "Open";
     case (1):
      return "WEP";
     case (2):
      return "WPA_PSK";
     case (3):
      return "WPA2_PSK";
     case (4):
      return "WPA_WPA2_PSK";
     case (5):
      return "WPA2_Enterprise";
     default:
      return "UNKNOWN";
  }
}

String getNetworks(int num_ssid, String ssid_list[50]) {
  String list;
  for (int i = 0; i < num_ssid; i++) {
    list += "<option value=' " + WiFi.SSID(i) + " '>" + WiFi.SSID(i) + ": " + getEncryptionType(WiFi.encryptionType(i)) + " </option>";
  }
  return list;
}

boolean connectOpenNetwork(char* ssid) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

boolean connectWpa(char* ssid, char* password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

boolean connectWpa2Enterprise(char* ssid, char* username, char* password) {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
  esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
  esp_wifi_sta_wpa2_ent_enable(&config); 
  WiFi.begin(ssid); //connect to wifi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

boolean saveSettingsToSdCard(String encryption, String ssid, String username, String password, char mqtt_server[],
  char outTopic_Ap1[], char outTopic_Ap2[], char outTopic_Ap3[], char outTopic_Ap4[]){
    Serial.println("Saving settings to SD card...");
    File configFile = SD.open("/connection.config", FILE_WRITE);
    
    if (configFile) {
      configFile.println(ssid.substring(0, ssid.length()-1));
      configFile.println(username);
      configFile.println(password);
      configFile.println(encryption);
      configFile.println(String(mqtt_server));
      configFile.println(String(outTopic_Ap1));
      configFile.println(String(outTopic_Ap2));
      configFile.println(String(outTopic_Ap3));
      configFile.println(String(outTopic_Ap4));
      configFile.close();
      Serial.println("Settings saved.");
    }else{
      Serial.println("Error opening connection.config for writing configuration");
      return false;
    }
}

boolean searchConfigurationFile(){
  return SD.exists("/connection.config")? true : false;
}
