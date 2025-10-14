/*
 * HomeKit Smart Light with HomeSpan
 * 基于 HomeSpan 的 HomeKit 智能灯
 *
 * 硬件需求:
 * - ESP32 开发板
 * - 继电器模块或LED灯
 * - 物理开关按钮
 * - LED指示灯
 *
 * 引脚定义:
 * - GPIO4: 灯光控制 (低电平触发开启)
 * - GPIO0: 物理开关按钮
 * - GPIO32: LED指示灯 (低电平点亮)
 *
 * 支持特性:
 * - On: 灯光开关状态
 * - Name: 设备名称
 * - 物理按钮控制
 * - LED状态指示
 *
 * 应用场景:
 * - 智能家居照明
 * - 远程灯光控制
 * - 物理开关备份
 */

#include "HomeSpan.h"
#include <Arduino.h>

// 硬件引脚定义
#define LIGHT_CONTROL_PIN  4    // 灯光控制引脚 (低电平触发开启)
#define PHYSICAL_SWITCH    0    // 物理开关按钮引脚
#define LED_INDICATOR      32   // LED指示灯引脚 (低电平点亮)

// 智能灯服务类
struct DEV_SmartLight : Service::LightBulb {
  Characteristic::On lightOn{0};  // 默认关闭
  Characteristic::Name name{"智能灯"};
  
  bool currentState;              // 当前灯光状态
  bool lastButtonState;           // 上次按钮状态
  uint32_t lastButtonTime;        // 上次按钮按下时间
  
  DEV_SmartLight() : Service::LightBulb() {
    currentState = false;
    lastButtonState = true;  // 按钮上拉，未按下时为高电平
    lastButtonTime = 0;
    
    // 初始化硬件引脚
    pinMode(LIGHT_CONTROL_PIN, OUTPUT);
    pinMode(PHYSICAL_SWITCH, INPUT_PULLUP);
    pinMode(LED_INDICATOR, OUTPUT);
    
    // 设置初始状态 (关闭)
    digitalWrite(LIGHT_CONTROL_PIN, HIGH);  // 高电平关闭灯光
    digitalWrite(LED_INDICATOR, HIGH);      // 高电平熄灭LED指示灯
  }
  
  boolean update() override {
    if (lightOn.updated()) {
      currentState = lightOn.getNewVal();
      updateHardware();
    }
    return true;
  }
  
  void loop() override {
    checkPhysicalSwitch();
  }
  
  // 更新硬件状态
  void updateHardware() {
    // 控制灯光 (低电平触发开启)
    digitalWrite(LIGHT_CONTROL_PIN, currentState ? LOW : HIGH);
    
    // 控制LED指示灯 (低电平点亮)
    digitalWrite(LED_INDICATOR, currentState ? LOW : HIGH);
  }
  
  // 检查物理开关
  void checkPhysicalSwitch() {
    uint32_t currentTime = millis();
    bool buttonState = digitalRead(PHYSICAL_SWITCH);
    
    // 检测按钮按下 (下降沿) 并防抖
    if (!buttonState && lastButtonState && (currentTime - lastButtonTime > 200)) {
      lastButtonTime = currentTime;
      toggleLight();
    }
    
    lastButtonState = buttonState;
  }
  
  // 切换灯光状态
  void toggleLight() {
    currentState = !currentState;
    lightOn.setVal(currentState ? 1 : 0);
    updateHardware();
  }
};

void setup() {
  Serial.begin(115200);
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Lighting, "HomeKit智能灯");
  homeSpan.enableAutoStartAP();
  
  // 创建配件
  new SpanAccessory();
  
  // 添加配件信息服务
  new Service::AccessoryInformation();
  new Characteristic::Name("智能灯");
  new Characteristic::Manufacturer("XcuiTech Inc.");
  new Characteristic::SerialNumber("LIGHT-001");
  new Characteristic::Model("SmartLight-Basic");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();
  
  // 添加智能灯服务
  new DEV_SmartLight();
}

void loop() {
  homeSpan.poll();
}