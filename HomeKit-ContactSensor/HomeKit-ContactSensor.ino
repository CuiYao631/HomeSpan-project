/*
 * HomeKit Contact Sensor with HomeSpan
 * 基于 HomeSpan 库的 HomeKit 接触传感器
 * 
 * 硬件需求:
 * - ESP32 开发板
 * - 磁性开关/门窗传感器 (GPIO4)
 * - 可选: LED状态指示器 (GPIO2)
 * - 可选: 蜂鸣器警报 (GPIO18)
 * - 可选: 按钮测试开关 (GPIO19)
 * 
 * 支持特性:
 * - ContactSensorState: 接触检测状态 (检测到/未检测到)
 * - StatusActive: 传感器工作状态
 * - StatusFault: 故障状态
 * - StatusTampered: 篡改检测
 * - StatusLowBattery: 电池状态
 * - ConfiguredName: 设备名称
 * 
 * 应用场景:
 * - 门窗开关监控
 * - 安全防护系统
 * - 智能家居自动化
 * - 防盗报警系统
 */

#include "HomeSpan.h"

// 硬件配置
#define CONTACT_SENSOR_PIN    4       // 磁性开关传感器引脚
#define LED_PIN               2       // LED状态指示器引脚
#define BUZZER_PIN            18      // 蜂鸣器引脚
#define TEST_BUTTON_PIN       0      // 测试按钮引脚

// 传感器配置
#define DEBOUNCE_TIME         50      // 防抖动时间 (毫秒)
#define CONTACT_POLL_INTERVAL 100     // 传感器轮询间隔 (毫秒)

// 接触传感器服务类
struct DEV_ContactSensor : Service::ContactSensor {
  
  // HomeKit特性定义
  Characteristic::ContactSensorState contactSensorState{1};           // 接触状态 (默认未检测到)
  Characteristic::StatusActive statusActive{1};                       // 传感器活跃状态
  Characteristic::StatusFault statusFault{0};                         // 故障状态
  Characteristic::StatusTampered statusTampered{0};                   // 篡改状态
  Characteristic::StatusLowBattery statusLowBattery{0};               // 低电量状态
  Characteristic::ConfiguredName configuredName{"Contact Sensor"};    // 设备名称
  
  // 传感器状态
  bool currentContactState = false;     // 当前接触状态 (false=未接触, true=接触)
  bool lastContactState = false;        // 上次接触状态
  bool isSimulationMode = false;        // 模拟模式标志
  
  // 时间管理
  uint32_t lastContactRead = 0;         // 上次传感器读取时间
  uint32_t lastStateChange = 0;         // 上次状态变化时间
  uint32_t lastHeartbeat = 0;           // 上次心跳时间
  uint32_t contactChangeTime = 0;       // 接触状态变化时间
  
  // 统计信息
  uint32_t contactEvents = 0;           // 接触事件计数
  uint32_t totalOperationTime = 0;      // 总运行时间
  
  // 构造函数
  DEV_ContactSensor() : Service::ContactSensor() {
    Serial.println("✓ 接触传感器服务已初始化");
    
    // 初始化硬件
    initSensor();
    
    // 读取初始传感器状态
    readContactSensorState();
    
    Serial.println("✓ 接触传感器就绪");
  }
  
  // 初始化传感器硬件
  void initSensor() {
    // 设置引脚模式
    pinMode(CONTACT_SENSOR_PIN, INPUT_PULLUP);  // 内部上拉电阻
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(TEST_BUTTON_PIN, INPUT_PULLUP);
    
    // 初始化输出状态
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    
    Serial.println("✓ 接触传感器硬件初始化完成");
    Serial.println("📌 引脚配置:");
    Serial.printf("   - 传感器引脚: GPIO%d (上拉输入)\n", CONTACT_SENSOR_PIN);
    Serial.printf("   - LED引脚: GPIO%d\n", LED_PIN);
    Serial.printf("   - 蜂鸣器引脚: GPIO%d\n", BUZZER_PIN);
    Serial.printf("   - 测试按钮: GPIO%d\n", TEST_BUTTON_PIN);
  }
  
  // 主循环处理函数
  void loop() override {
    uint32_t currentTime = millis();
    
    // 定期读取传感器状态
    if (currentTime - lastContactRead >= CONTACT_POLL_INTERVAL) {
      readContactSensorState();
      lastContactRead = currentTime;
    }
    
    // 检查测试按钮
    checkTestButton();
    
    // 每30秒发送一次心跳
    if (currentTime - lastHeartbeat >= 30000) {
      sendHeartbeat();
      lastHeartbeat = currentTime;
    }
    
    // 更新LED和蜂鸣器状态
    updateIndicators();
    
    // 更新运行时间统计
    totalOperationTime = currentTime;
  }
  
  // 读取接触传感器状态
  void readContactSensorState() {
    bool rawState = digitalRead(CONTACT_SENSOR_PIN);
    
    // 传感器逻辑：LOW = 接触检测到, HIGH = 未接触
    bool newContactState = !rawState;  // 反转逻辑
    
    // 防抖动处理
    if (newContactState != lastContactState) {
      uint32_t currentTime = millis();
      
      if (currentTime - contactChangeTime >= DEBOUNCE_TIME) {
        // 状态确实发生了变化
        lastContactState = currentContactState;
        currentContactState = newContactState;
        contactChangeTime = currentTime;
        
        // 更新HomeKit特性
        updateContactState();
        
        // 更新统计
        if (currentContactState) {
          contactEvents++;
        }
        
        Serial.printf("🔄 接触状态变化: %s → %s (事件 #%d)\n", 
                     lastContactState ? "接触" : "分离",
                     currentContactState ? "接触" : "分离",
                     contactEvents);
      }
    } else {
      // 重置变化时间
      contactChangeTime = millis();
    }
  }
  
  // 更新HomeKit接触状态
  void updateContactState() {
    // HomeKit定义: 0 = DETECTED (接触), 1 = NOT_DETECTED (未接触)
    uint8_t homeKitState = currentContactState ? 0 : 1;
    contactSensorState.setVal(homeKitState);
    
    // 记录状态变化时间
    lastStateChange = millis();
    
    // 打印详细状态
    String statusText = currentContactState ? "🔗 接触检测到" : "📤 接触分离";
    String homeKitText = (homeKitState == 0) ? "DETECTED" : "NOT_DETECTED";
    
    Serial.printf("%s (HomeKit: %s)\n", statusText.c_str(), homeKitText.c_str());
    
    // 触发蜂鸣器提示
    if (currentContactState) {
      // 接触时短促蜂鸣
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
  
  // 检查测试按钮
  void checkTestButton() {
    static bool lastButtonState = true;
    static uint32_t lastButtonTime = 0;
    
    bool buttonState = digitalRead(TEST_BUTTON_PIN);
    uint32_t currentTime = millis();
    
    // 检测按钮按下 (下降沿)
    if (!buttonState && lastButtonState && (currentTime - lastButtonTime > 200)) {
      toggleSimulationMode();
      lastButtonTime = currentTime;
    }
    
    lastButtonState = buttonState;
  }
  
  // 切换模拟模式
  void toggleSimulationMode() {
    isSimulationMode = !isSimulationMode;
    
    if (isSimulationMode) {
      Serial.println("🔧 进入模拟测试模式");
      // 模拟接触状态变化
      currentContactState = !currentContactState;
      updateContactState();
    } else {
      Serial.println("🔧 退出模拟测试模式");
    }
    
    // 蜂鸣器确认
    for (int i = 0; i < (isSimulationMode ? 2 : 1); i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(150);
      digitalWrite(BUZZER_PIN, LOW);
      delay(150);
    }
  }
  
  // 更新指示器状态
  void updateIndicators() {
    uint32_t currentTime = millis();
    
    if (isSimulationMode) {
      // 模拟模式：LED快速闪烁
      digitalWrite(LED_PIN, (currentTime / 200) % 2);
    } else if (currentContactState) {
      // 接触状态：LED常亮
      digitalWrite(LED_PIN, HIGH);
    } else {
      // 分离状态：LED慢速闪烁
      digitalWrite(LED_PIN, (currentTime / 1000) % 2);
    }
  }
  
  // 发送心跳信号
  void sendHeartbeat() {
    uint32_t uptimeSeconds = totalOperationTime / 1000;
    uint32_t timeSinceLastChange = (millis() - lastStateChange) / 1000;
    
    Serial.printf("💓 心跳 - 状态: %s | 运行: %d秒 | 事件: %d | 上次变化: %d秒前\n", 
                 currentContactState ? "接触" : "分离",
                 uptimeSeconds,
                 contactEvents,
                 timeSinceLastChange);
                 
    // 报告传感器健康状态
    if (uptimeSeconds > 3600) { // 运行超过1小时
      Serial.println("✅ 传感器长期稳定运行");
    }
  }
  
  // 获取传感器状态描述
  String getContactDescription() {
    if (currentContactState) {
      return "接触检测 - 磁铁靠近/门窗关闭";
    } else {
      return "接触分离 - 磁铁远离/门窗打开";
    }
  }
  
  // 获取详细状态信息
  void printDetailedStatus() {
    Serial.println("📊 接触传感器详细状态:");
    Serial.printf("   当前状态: %s\n", getContactDescription().c_str());
    Serial.printf("   HomeKit值: %d (%s)\n", 
                 contactSensorState.getVal(), 
                 contactSensorState.getVal() == 0 ? "DETECTED" : "NOT_DETECTED");
    Serial.printf("   接触事件: %d次\n", contactEvents);
    Serial.printf("   运行时间: %d秒\n", totalOperationTime / 1000);
    Serial.printf("   模拟模式: %s\n", isSimulationMode ? "开启" : "关闭");
    Serial.printf("   传感器引脚值: %s\n", digitalRead(CONTACT_SENSOR_PIN) ? "HIGH" : "LOW");
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("\n🏠 HomeKit 接触传感器启动中...");
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Sensors, "HomeKit接触传感器");
  homeSpan.enableAutoStartAP();
  
  // 创建配件
  new SpanAccessory();
  
  // 添加必需的配件信息服务
  new Service::AccessoryInformation();
  new Characteristic::Name("接触传感器");
  new Characteristic::Manufacturer("HomeSpan");
  new Characteristic::SerialNumber("CONTACT-001");
  new Characteristic::Model("MagneticSwitch-V1");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();
  
  // 添加接触传感器服务
  new DEV_ContactSensor();
  
}

void loop() {
  homeSpan.poll();
}