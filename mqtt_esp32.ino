#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

Servo servo_pin_22;
boolean servoRotationRequested = false;

#define RXp2 16
#define TXp2 17

// WiFi credentials
const char *ssid = "AndroidAPD46D";
const char *password = "tony9026";

// 定義 EMQX 上的客戶端 ID
String mqtt_ClientID = "stonez56_IOT_Station_";

// MQTT topics
const char *sub_topic = "stonez56/esp32s";
const char *pub_init_topic = "stonez56/esp32s_is_back";
const char *pub_pH_topic = "stonez56/pH";        // pH value topic
const char *pub_temp_topic = "stonez56/temp"; // Water temperature topic

// EMQX broker parameters
const char *mqtt_server = "broker.emqx.io";
const char *mqtt_userName = "emqx";
const char *mqtt_password = "public";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

String phValue;     // 用於存儲 pH 值的變量
String tempValue;   // 用於存儲水溫數據的變量

void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void startMotor() {
    if (!servoRotationRequested) {
        servo_pin_22.attach(22); // 重新連接伺服馬達
        servoRotationRequested = true;
        servo_pin_22.write(90); // 式旋轉伺服馬達到 90 度位置
        delay(3000); // 等待3秒
        servo_pin_22.write(0); // 返回到0度
        servoRotationRequested = false;
        Serial.println("Feed: on");
    }
}

void manualControl() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "r") {
            startMotor();
            delay(3000);  // 等待3秒，然後停止伺服馬達
            servo_pin_22.detach(); // 斷開伺服馬達的控制

            client.publish(sub_topic, "r");  // 在按下按鈕 "r" 時發佈到 "stonez56/esp32s" MQTT主題

        }
    }
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    payload[length] = '\0';
    String message = (char *)payload;

    if (strcmp(topic, sub_topic) == 0){
        if (message == "r"){
            startMotor();  // 啟動馬達的函數
            delay(3000);   // 等待3秒，然後停止伺服馬達
            servo_pin_22.detach(); // 斷開伺服馬達的控制
        }
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.println("Attempting EMQX MQTT connection...");
        mqtt_ClientID += String(ESP.getEfuseMac(), HEX); 

        if (client.connect(mqtt_ClientID.c_str(), mqtt_userName, mqtt_password))
        {
            Serial.print("Connected with Client ID: ");
            Serial.println(mqtt_ClientID);
            client.subscribe(sub_topic);  
            
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    phValue = "";     // 用於存儲 pH 值的變量
    tempValue = "";   // 用於存儲水溫數據的變量

    //servo_pin_22.attach(22);
}

void loop() {
    manualControl(); // 增加手動控制功能

    if (Serial2.available()) {
        String receivedData = Serial2.readString();
        
        // 尋找pH值、水溫和水位的開始索引
        int pHIndex = receivedData.indexOf("pH: ");
        int tempIndex = receivedData.indexOf("Temp: ");

        if (pHIndex >= 0 && tempIndex >= 0) {
            // 提取pH值
            phValue = receivedData.substring(pHIndex + 4, tempIndex);
            phValue.trim();
            Serial.print(phValue);
            // 提取水溫數據
            tempValue = receivedData.substring(tempIndex + 6);
            tempValue.trim();
            Serial.print(tempValue);

            // 在這裡處理pH值、水溫和水位數據
            // 例如，將它們發佈到MQTT主題中

            if (!client.connected()) {
                reconnect();
            }
            client.loop();

            unsigned long now = millis();
            if (now - lastMsg > 5000) {
                lastMsg = now;
                // 將pH值、水溫和水位發佈到MQTT主題
                client.publish(pub_pH_topic, phValue.c_str());             // 發佈pH值主題
                client.publish(pub_temp_topic, tempValue.c_str());         // 發佈水溫主題 
            }
        }
    }
}
