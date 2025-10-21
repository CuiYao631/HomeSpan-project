/*
 * HomeKit Programmable Switches with HomeSpan
 * 基于 HomeSpan 的 HomeKit 可编程开关
 *
 * 硬件需求:
 * - ESP32 开发板
 * - 4个物理按钮开关
 * - LED指示灯
 * - 上拉电阻（内置）
 *
 * 引脚定义:
 * - GPIO0: 开关1 (单击/双击/长按)
 * - GPIO2: 开关2 (单击/双击/长按)
 * - GPIO4: 开关3 (单击/双击/长按)
 * - GPIO5: 开关4 (单击/双击/长按)
 * - GPIO32: HomeKit状态LED (显示连接状态)
 *
 * 支持特性:
 * - ProgrammableSwitchEvent: 单击(0)/双击(1)/长按(2)
 * - Name: 设备名称
 * - 多个独立开关
 * - LED状态指示
 *
 * 应用场景:
 * - 智能场景控制
 * - 多功能按钮
 * - 自动化触发器
 * - 智能家居控制面板
 */

#include "HomeSpan.h"
#include <Arduino.h>
#include <EasyButton.h>

// 硬件引脚定义
#define SWITCH1_PIN       26     // 开关1引脚
#define SWITCH2_PIN       25     // 开关2引脚
#define SWITCH3_PIN       35     // 开关3引脚
#define SWITCH4_PIN       4     // 开关4引脚
#define LED_INDICATOR     32    // LED指示灯引脚
#define DEFAULT_SETUP_CODE "46637726"  // HomeKit默认配对码
#define DEFAULT_QR_ID      "PSWS"      // HomeKit QR码ID

// 按钮事件定义
#define SINGLE_PRESS      0     // 单击
#define DOUBLE_PRESS      1     // 双击
#define LONG_PRESS        2     // 长按

// 按钮配置
const int longPressDuration = 3000;     // 长按触发时间(ms)
const int doubleClickGap = 300;         // 双击间隔时间(ms)

// 创建EasyButton对象
EasyButton switch1(SWITCH1_PIN);
EasyButton switch2(SWITCH2_PIN);
EasyButton switch3(SWITCH3_PIN);
EasyButton switch4(SWITCH4_PIN);

// 可编程开关服务类
struct DEV_ProgrammableSwitch : Service::StatelessProgrammableSwitch {
  Characteristic::ProgrammableSwitchEvent switchEvent;
  Characteristic::Name name;
  
  int switchNumber;               // 开关编号
  
  DEV_ProgrammableSwitch(int switchNum, const char* switchName) : 
    Service::StatelessProgrammableSwitch(),
    switchNumber(switchNum) {
    
    name.setString(switchName);
    
    Serial.printf("⚙️  可编程开关%d (%s) 初始化完成\n", switchNumber, switchName);
  }
  
  // 触发开关事件
  void triggerEvent(int eventType) {
    switchEvent.setVal(eventType);
    
    const char* eventNames[] = {"单击", "双击", "长按"};
    Serial.printf("🔘 开关%d 触发: %s (值: %d)\n", 
                  switchNumber, eventNames[eventType], eventType);
  }
  
  boolean update() override {
    // 可编程开关不需要处理update事件
    return true;
  }
  
  void loop() override {
    // 按钮处理在主loop中进行
  }
};

// 全局开关对象指针
DEV_ProgrammableSwitch* programmableSwitch1;
DEV_ProgrammableSwitch* programmableSwitch2;
DEV_ProgrammableSwitch* programmableSwitch3;
DEV_ProgrammableSwitch* programmableSwitch4;

// 开关1事件回调函数
void onSwitch1Pressed() {
  Serial.println("🔎 开关1 单击");
  if (programmableSwitch1) {
    programmableSwitch1->triggerEvent(SINGLE_PRESS);
  }
}

void onSwitch1DoubleClick() {
  Serial.println("🔎 开关1 双击");
  if (programmableSwitch1) {
    programmableSwitch1->triggerEvent(DOUBLE_PRESS);
  }
}

void onSwitch1LongPress() {
  Serial.println("🔎 开关1 长按");
  if (programmableSwitch1) {
    programmableSwitch1->triggerEvent(LONG_PRESS);
  }
}

// 开关2事件回调函数
void onSwitch2Pressed() {
  Serial.println("🔎 开关2 单击");
  if (programmableSwitch2) {
    programmableSwitch2->triggerEvent(SINGLE_PRESS);
  }
}

void onSwitch2DoubleClick() {
  Serial.println("🔎 开关2 双击");
  if (programmableSwitch2) {
    programmableSwitch2->triggerEvent(DOUBLE_PRESS);
  }
}

void onSwitch2LongPress() {
  Serial.println("🔎 开关2 长按");
  if (programmableSwitch2) {
    programmableSwitch2->triggerEvent(LONG_PRESS);
  }
}

// 开关3事件回调函数
void onSwitch3Pressed() {
  Serial.println("🔎 开关3 单击");
  if (programmableSwitch3) {
    programmableSwitch3->triggerEvent(SINGLE_PRESS);
  }
}

void onSwitch3DoubleClick() {
  Serial.println("🔎 开关3 双击");
  if (programmableSwitch3) {
    programmableSwitch3->triggerEvent(DOUBLE_PRESS);
  }
}

void onSwitch3LongPress() {
  Serial.println("🔎 开关3 长按");
  if (programmableSwitch3) {
    programmableSwitch3->triggerEvent(LONG_PRESS);
  }
}

// 开关4事件回调函数
void onSwitch4Pressed() {
  Serial.println("🔎 开关4 单击");
  if (programmableSwitch4) {
    programmableSwitch4->triggerEvent(SINGLE_PRESS);
  }
}

void onSwitch4DoubleClick() {
  Serial.println("🔎 开关4 双击");
  if (programmableSwitch4) {
    programmableSwitch4->triggerEvent(DOUBLE_PRESS);
  }
}

void onSwitch4LongPress() {
  Serial.println("🔎 开关4 长按");
  if (programmableSwitch4) {
    programmableSwitch4->triggerEvent(LONG_PRESS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🚀 启动 HomeKit 可编程开关");
  Serial.println("📝 支持功能:");
  Serial.println("   • 4个独立可编程开关");
  Serial.println("   • 单击/双击/长按检测");
  Serial.println("   • HomeKit场景控制");
  Serial.println("   • LED状态指示");
  
  // 初始化所有按钮
  switch1.begin();
  switch1.onPressed(onSwitch1Pressed);
  switch1.onSequence(2, doubleClickGap, onSwitch1DoubleClick);
  switch1.onPressedFor(longPressDuration, onSwitch1LongPress);
  
  switch2.begin();
  switch2.onPressed(onSwitch2Pressed);
  switch2.onSequence(2, doubleClickGap, onSwitch2DoubleClick);
  switch2.onPressedFor(longPressDuration, onSwitch2LongPress);
  
  switch3.begin();
  switch3.onPressed(onSwitch3Pressed);
  switch3.onSequence(2, doubleClickGap, onSwitch3DoubleClick);
  switch3.onPressedFor(longPressDuration, onSwitch3LongPress);
  
  switch4.begin();
  switch4.onPressed(onSwitch4Pressed);
  switch4.onSequence(2, doubleClickGap, onSwitch4DoubleClick);
  switch4.onPressedFor(longPressDuration, onSwitch4LongPress);
  
  // 配置HomeSpan
  homeSpan.setStatusPin(LED_INDICATOR);              // 状态LED
  homeSpan.setQRID(DEFAULT_QR_ID);                   // QR码ID
  homeSpan.setPairingCode(DEFAULT_SETUP_CODE);       // 默认配对码
  
  // 初始化HomeSpan
  homeSpan.begin(Category::ProgrammableSwitches, "HomeKit可编程开关");
  homeSpan.enableAutoStartAP();
  
  Serial.println("⚙️  HomeSpan配置完成");
  Serial.printf("🔐 配对码: %s\n", DEFAULT_SETUP_CODE);
  Serial.printf("📱 QR码ID: %s\n", DEFAULT_QR_ID);
  
  // 创建配件
  new SpanAccessory();
  
  // 添加配件信息服务
  new Service::AccessoryInformation();
  new Characteristic::Name("可编程开关");
  new Characteristic::Manufacturer("XcuiTech Inc.");
  new Characteristic::SerialNumber("PSWS-001");
  new Characteristic::Model("ProgrammableSwitch-4Channel");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();
  
  // 添加4个可编程开关服务
  programmableSwitch1 = new DEV_ProgrammableSwitch(1, "场景开关1");
  programmableSwitch2 = new DEV_ProgrammableSwitch(2, "场景开关2");
  programmableSwitch3 = new DEV_ProgrammableSwitch(3, "场景开关3");
  programmableSwitch4 = new DEV_ProgrammableSwitch(4, "场景开关4");
  
  Serial.println("✅ 设备初始化完成!");
  Serial.println("📱 请在家庭App中添加此配件");
  Serial.println("🎯 操作说明:");
  Serial.println("   • 单击: 触发场景1");
  Serial.println("   • 双击: 触发场景2");
  Serial.println("   • 长按: 触发场景3");
}

void loop() {
  homeSpan.poll();
  
  // 读取所有按钮状态
  switch1.read();
  switch2.read();
  switch3.read();
  switch4.read();
}