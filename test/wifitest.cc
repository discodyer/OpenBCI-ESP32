#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <esp_timer.h>
#include <NVS.h>

const char *ap_ssid = "OpenBCI";
const char *ap_password = "openbci123";

const int ledPin = 2; // LED连接到GPIO 2
const int maxRetry = 3;
const int retryInterval = 5000; // 重试间隔为5秒

int retryCount = 0;
esp_timer_handle_t timer; // 使用esp_timer_handle_t

AsyncWebServer server(80);

// 函数声明
void connectToWiFi();
void startAPMode();
void retryConnection(void *);
void setup();
void loop();
void clearInterval(esp_timer_handle_t timer); // 函数声明更新
void startWebServer();

void setup()
{
    Serial0.begin(115200);
    pinMode(ledPin, OUTPUT);

    NVS.begin();
    ssid = NVS.getString("ssid", ssid);
    password = NVS.getString("password", password);
    NVS.end();

    // 尝试连接Wi-Fi
    connectToWiFi();
}

void loop()
{
    // 在这里可以添加其他的处理逻辑
}

void connectToWiFi()
{
    Serial0.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < maxRetry)
    {
        delay(retryInterval);
        Serial0.printf("Connection failed. Retrying %d of %d...\n", attempts, maxRetry);
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        startWebServer();
        startAPMode();
        while (1)
        {
            ;
        }
    }
    else
    {
        Serial0.println("Connected to WiFi");
        Serial0.printf("IP address: %s \n", WiFi.localIP().toString());
        digitalWrite(ledPin, HIGH); // 点亮LED
    }
}

void startAPMode()
{
    Serial0.println("Starting AP mode...");
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress local_ip(10, 0, 0, 1);
    IPAddress gateway(10, 0, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    Serial0.println("AP mode started. Connect to OpenBCI with password openbci123, then visit http://10.0.0.1/");
}

void retryConnection(void *arg)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        retryCount++;
        Serial0.printf("Connection failed. Retrying %d of %d...\n", retryCount, maxRetry);

        if (retryCount >= maxRetry)
        {
            startAPMode();
            esp_timer_stop(timer);
            esp_timer_delete(timer);
        }
    }
    else
    {
        Serial0.println("Wi-Fi already connected.");
        digitalWrite(ledPin, HIGH); // 点亮LED
        esp_timer_stop(timer);
        esp_timer_delete(timer);
    }
}

void clearInterval(esp_timer_handle_t timer)
{
    esp_timer_stop(timer);
    esp_timer_delete(timer);
}

void startWebServer()
{
    // 配置Web服务器
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Hello, configure your Wi-Fi here!"); });
    server.begin();
}
