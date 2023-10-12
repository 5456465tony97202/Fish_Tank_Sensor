#include <WiFi.h>
#include <PubSubClient.h>

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
const char *pub_water_level_topic = "stonez56/water"; // Water level topic

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
String waterLevel;

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

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    payload[length] = '\0';
    String message = (char *)payload;

    // Handle received MQTT messages here
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.println("Attempting EMQX MQTT connection...");
        mqtt_ClientID += String(random(0xffff), HEX); // 此處不再需要轉換

        if (client.connect(mqtt_ClientID.c_str(), mqtt_userName, mqtt_password))
        {
            Serial.print("Connected with Client ID: ");
            Serial.println(mqtt_ClientID);
            // 在這裡添加訂閱主題的程式碼
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

    // 在這裡聲明變數，不要加String
    phValue = "";     // 用於存儲 pH 值的變量
    tempValue = "";   // 用於存儲水溫數據的變量
    waterLevel = "";
}

void loop() {
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

            // 提取水位數據
            //waterLevel = receivedData.substring(waterLevelIndex + 4);
            //waterLevel.trim();

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
                //client.publish(pub_water_level_topic, waterLevel.c_str()); // 發佈水位主題
            }
        }
    }
    delay(3000);
}
