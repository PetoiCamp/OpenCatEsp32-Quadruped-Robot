// 引入必要的库文件
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>

// WiFi凭据
String ssid = "";
String password = "";

// 创建Web服务器实例，监听80端口
WebServer server(80);

// 处理HTTP请求的函数
void handleCommand() {
  // 获取URL中的cmd参数
  String cmd = server.arg("cmd");

  // 如果没有cmd参数，返回400错误
  if (cmd == "") {
    server.send(400, "text/plain", "Missing cmd parameter");
    return;
  }

  // 在串口打印收到的命令
  Serial.print("Received command: ");
  Serial.println(cmd);

  // 处理命令并准备响应
  String response;

  // 根据不同的命令返回不同的响应
  if (cmd == "kwk" || cmd == "ksit") {
    response = cmd;  // 步行或坐下命令
  } else if (cmd == "A1") {
    response = "101";  // 模拟传感器读数
  } else if (cmd == "D1") {
    response = "1";  // 数字传感器读数
  } else if (cmd == "Ultrasonic") {
    response = "30cm";  // 超声波传感器距离
  } else if (cmd == "?") {
    response = "bittle_X";  // 设备标识
  } else {
    Serial.print("Custom_cmd:");  // 自定义命令处理
    Serial.println(response);
  }

  // 在串口打印响应内容
  Serial.print("Sending response: ");
  Serial.println(response);

  // 发送HTTP响应
  server.send(200, "text/plain", response);
}

// 通过串口配置WiFi的函数
bool configureWiFiViaSerial() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    if (input.startsWith("w%")) {
      // 解析WiFi配置字符串
      int firstDelimiter = input.indexOf('%', 2);                    // 找到第二个%的位置
      int secondDelimiter = input.indexOf('%', firstDelimiter + 1);  // 找到第三个%的位置

      if (firstDelimiter != -1 && secondDelimiter != -1) {
        ssid = input.substring(2, firstDelimiter);
        password = input.substring(firstDelimiter + 1, secondDelimiter);
        Serial.println(ssid);
        Serial.println(password);
        // 尝试连接WiFi
        WiFi.begin(ssid.c_str(), password.c_str());

        // 等待WiFi连接，最多等待10秒
        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 100) {
          delay(100);
          timeout++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.print("IP Address: ");
          Serial.println(WiFi.localIP().toString());
          return true;
        } else {
          Serial.println("connection failed");
          return false;
        }
      }
    }
  }
  return false;
}

// 修改WiFi设置函数
void setupWiFi() {
  // 等待串口配置
  // unsigned long startTime = millis();
  // while (millis() - startTime < 20000) {  // 等待5秒钟接收串口配置
  //   if (configureWiFiViaSerial()) {
  //     return;  // 如果通过串口配置成功，直接返回
  //   }
  // }


  // 如果没有收到串口配置，则使用WiFiManager
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);

  if (!wm.autoConnect("BittleWirelessCfg")) {
    Serial.println("配网失败，重启设备");
    delay(3000);
    ESP.restart();
  }
  Serial.println("已成功连接到WiFi");
  Serial.println(WiFi.localIP().toString());
}

// 设备启动时的初始化函数
void setup() {
  // 初始化串口通信，波特率115200
  Serial.begin(115200);

  Serial.println("UART init!");

  // 配置并连接WiFi
  setupWiFi();

  // 启用CORS（跨域资源共享）
  server.enableCORS(true);

  // 设置服务器路由
  server.on("/", HTTP_GET, handleCommand);

  // 启动Web服务器
  server.begin();
  Serial.println("HTTP server started");
}

// 主循环函数
void loop() {
  // 处理客户端请求
  server.handleClient();
}
