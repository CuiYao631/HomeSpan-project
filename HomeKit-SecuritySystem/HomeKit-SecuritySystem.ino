/*
 * HomeKit Security System with HomeSpan
 * 基于 HomeSpan 的 HomeKit 安全系统
 *
 * 硬件需求:
 * - ESP32 开发板
 * - 可选: LED 指示灯 (GPIO2)
 * - 可选: 门磁/红外传感器 (GPIO0)
 *
 * 支持特性:
 * - SecuritySystemCurrentState: 当前安全状态
 * - SecuritySystemTargetState: 目标安全状态
 * - SecuritySystemAlarmType: 报警类型
 * - StatusFault: 故障状态
 * - StatusTampered: 防拆状态
 * - Name: 设备名称
 *
 * 场景示例:
 * - 家庭防盗报警
 * - 门窗入侵检测
 * - 智能布防/撤防
 * - 夜间模式立即布防
 */

#include "HomeSpan.h"
#include <Arduino.h>

// 硬件引脚定义
#define LED_PIN   2      // LED指示灯引脚
#define SENSOR_PIN 0     // 门磁/红外传感器引脚

// 安全系统服务类
struct DEV_SecuritySystem : Service::SecuritySystem {
  Characteristic::SecuritySystemCurrentState currentState{3}; // 默认撤防
  Characteristic::SecuritySystemTargetState targetState{3};   // 默认撤防
  Characteristic::SecuritySystemAlarmType alarmType{0};       // 默认无报警
  Characteristic::StatusFault statusFault{0};                 // 故障状态
  Characteristic::StatusTampered statusTampered{0};           // 防拆状态
  Characteristic::Name name{"安全系统"};

  bool isArmed = false;
  bool isTriggered = false;
  bool isArming = false;          // 添加正在设防标志
  uint32_t armingStartTime = 0;   // 设防开始时间
  uint32_t lastSensorCheck = 0;

  DEV_SecuritySystem() : Service::SecuritySystem() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("✓ 安全系统服务已初始化");
  }

  boolean update() override {
    if (targetState.updated()) {
      uint8_t tState = targetState.getNewVal();
      Serial.printf("🔄 安全系统目标状态: %d\n", tState);
      
      if (tState == 0 || tState == 1) {
        // 开始布防过程（在家/外出布防需要延时）
        isArming = true;
        isArmed = false;
        armingStartTime = millis();
        currentState.setVal(tState);  // 立即设置为目标状态
        Serial.printf("⏰ 开始布防过程，目标状态: %d\n", tState);
      } else if (tState == 2) {
        // 夜间模式立即布防
        isArming = false;
        isArmed = true;
        currentState.setVal(2);
        Serial.println("🌙 夜间模式已启用，立即布防");
      } else if (tState == 3) {
        // 撤防
        isArming = false;
        isArmed = false;
        isTriggered = false;
        currentState.setVal(3);
        alarmType.setVal(0);
        Serial.println("🔓 系统已撤防");
      }
      
      digitalWrite(LED_PIN, (tState != 3) ? HIGH : LOW);
    }
    return true;
  }

  void loop() override {
    uint32_t now = millis();
    
    // 处理布防延时
    if (isArming && !isArmed && now - armingStartTime >= 5000) {  // 5秒延时
      isArming = false;
      isArmed = true;
      Serial.println("🔒 系统布防完成");
    }
    
    // 只有在完全布防后才检查传感器
    if (isArmed && !isArming && now - lastSensorCheck > 200) {
      lastSensorCheck = now;
      bool sensorTripped = digitalRead(SENSOR_PIN) == LOW;
      if (sensorTripped && !isTriggered) {
        // 触发报警
        isTriggered = true;
        currentState.setVal(4); // 4=报警
        alarmType.setVal(1);    // 1=报警触发 (HomeKit只支持0-1)
        
        Serial.println("🚨 入侵检测，重要警报触发！");
      } else if (!sensorTripped && isTriggered) {
        // 恢复正常
        isTriggered = false;
        currentState.setVal(targetState.getVal());
        alarmType.setVal(0);
        Serial.println("✅ 报警解除，系统恢复正常。");
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("\n🔒 HomeKit 安全系统启动中...");
  homeSpan.begin(Category::SecuritySystems, "HomeKit安全系统");
  homeSpan.enableAutoStartAP();

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Name("安全系统");
  new Characteristic::Manufacturer("XcuiTech Inc.");
  new Characteristic::SerialNumber("SEC-001");
  new Characteristic::Model("SecuritySystem-Pro");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();

  new DEV_SecuritySystem();
}

void loop() {
  homeSpan.poll();
}
