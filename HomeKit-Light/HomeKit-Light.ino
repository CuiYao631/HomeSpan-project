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
 * - GPIO0: 物理开关按钮 (短按开关/长按重置)
 * - GPIO32: HomeKit状态LED (显示连接状态)
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
#include <EasyButton.h>

// 硬件引脚定义
#define LIGHT_CONTROL_PIN  4    // 灯光控制引脚 (低电平触发开启)
#define PHYSICAL_SWITCH    0    // 物理开关按钮引脚
#define LED_INDICATOR      32   // LED指示灯引脚 (低电平点亮)
#define DEFAULT_SETUP_CODE "46637726"  // HomeKit默认配对码
#define DEFAULT_QR_ID      "DENG"      // HomeKit QR码ID 

const int duration = 5000;                  // Button长按触发时间
EasyButton button(PHYSICAL_SWITCH);         // 使用GPIO0初始化按钮

// 智能灯服务类
struct DEV_SmartLight : Service::LightBulb {
  Characteristic::On lightOn{0};  // 默认关闭
  Characteristic::Name name{"智能灯"};
  
  bool currentState;              // 当前灯光状态
  
  DEV_SmartLight() : Service::LightBulb() {
    currentState = false;
    
    // 初始化灯光控制引脚
    pinMode(LIGHT_CONTROL_PIN, OUTPUT);
    
    // 设置初始状态 (关闭) - 高电平关闭，低电平开启
    digitalWrite(LIGHT_CONTROL_PIN, HIGH);
    
    Serial.println("⚙️  智能灯服务初始化完成");
  }
  
  boolean update() override {
    if (lightOn.updated()) {
      currentState = lightOn.getNewVal();
      updateHardware();
    }
    return true;
  }
  
  void loop() override {
    // EasyButton的处理在主loop中进行
  }
  
  // 更新硬件状态
  void updateHardware() {
    // 控制灯光 (低电平触发开启)
    digitalWrite(LIGHT_CONTROL_PIN, currentState ? LOW : HIGH);
    
    // LED指示灯由HomeSpan管理，显示HomeKit连接状态
    // 如需要显示灯光状态，可启用下面代码:
    digitalWrite(LED_INDICATOR, currentState ? LOW : HIGH);
  }
  
  // 切换灯光状态
  void toggleLight() {
    currentState = !currentState;
    lightOn.setVal(currentState ? 1 : 0);
    updateHardware();
    Serial.printf("💡 灯光状态: %s\n", currentState ? "开启" : "关闭");
  }
};

// 全局指针，用于在回调函数中访问设备
DEV_SmartLight* smartLight;

// 按钮短按回调函数
void onPressed() {
  Serial.println("🔎 按钮短按检测到");
  if (smartLight) {
    smartLight->toggleLight();
  }
}

// 按钮长按回调函数
void onPressedForDuration() {
  Serial.println("🔄 按钮长按检测到 - 恢复出厂设置");
  // 恢复出厂设置
  homeSpan.processSerialCommand("F");
}

void setup() {
  Serial.begin(115200);

  // 初始化EasyButton
  button.begin();
  button.onPressed(onPressed);                         // 定义按键单按事件回调
  button.onPressedFor(duration, onPressedForDuration); // 定义按键长按事件回调
  
  // 配置HomeSpan
  homeSpan.setStatusPin(LED_INDICATOR);              // 状态LED
  homeSpan.setQRID(DEFAULT_QR_ID);                   // QR码ID
  homeSpan.setPairingCode(DEFAULT_SETUP_CODE);       // 默认配对码



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
  smartLight = new DEV_SmartLight();

}

void loop() {
  homeSpan.poll();
  button.read();  // 读取按钮状态
}