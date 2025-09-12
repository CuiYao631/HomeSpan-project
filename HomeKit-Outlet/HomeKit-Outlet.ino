/*
 * HomeKit Smart Outlet with HomeSpan
 * 基于 HomeSpan 的 HomeKit 智能插座
 *
 * 硬件需求:
 * - ESP32 开发板
 * - 4路继电器模块
 * - LED指示灯 (可选)
 * - 全局控制按钮 (IO0)
 *
 * 支持特性:
 * - 4个独立的插座开关控制
 * - On: 插座开关状态
 * - OutletInUse: 插座使用状态
 * - Name: 设备名称
 * - IO0按钮全开/全关控制
 *
 * 应用场景:
 * - 智能电源管理
 * - 多设备统一控制
 * - 远程开关控制
 * - 定时开关
 */

#include "HomeSpan.h"
#include <Arduino.h>

// 硬件引脚定义
#define OUTLET1_PIN   19      // 插座1控制引脚
#define OUTLET2_PIN   4      // 插座2控制引脚  
#define OUTLET3_PIN   5      // 插座3控制引脚
#define OUTLET4_PIN   18     // 插座4控制引脚
#define MASTER_BUTTON 0      // 全局控制按钮 (IO0)
#define LED_PIN       2     // LED状态指示灯

// 智能插座服务类
struct DEV_SmartOutlet : Service::Outlet {
  Characteristic::On outletOn;
  Characteristic::OutletInUse outletInUse{0};  // 默认未使用
  Characteristic::Name name;
  
  int controlPin;         // 控制引脚
  bool currentState;      // 当前开关状态
  String outletName;      // 插座名称
  
  DEV_SmartOutlet(int pin, const char* outletName) : Service::Outlet(), name(outletName) {
    this->controlPin = pin;
    this->outletName = String(outletName);
    this->currentState = false;
    
    // 设置特性
    outletOn.setVal(0);  // 默认关闭
    
    // 初始化硬件
    pinMode(controlPin, OUTPUT);
    digitalWrite(controlPin, LOW);  // 继电器低电平触发，默认关闭
  }
  
  boolean update() override {
    if (outletOn.updated()) {
      currentState = outletOn.getNewVal();
      
      // 控制继电器
      digitalWrite(controlPin, currentState ? HIGH : LOW);
      
      // 更新使用状态
      outletInUse.setVal(currentState ? 1 : 0);
    }
    return true;
  }
  
  // 设置插座状态
  void setOutletState(bool state) {
    if (state != currentState) {
      currentState = state;
      outletOn.setVal(state ? 1 : 0);
      outletInUse.setVal(state ? 1 : 0);
      digitalWrite(controlPin, state ? HIGH : LOW);
    }
  }
  
  // 获取当前状态
  bool getState() {
    return currentState;
  }
};

// 全局控制类
struct MasterController {
  DEV_SmartOutlet* outlets[4];
  bool lastButtonState;
  uint32_t lastButtonTime;
  bool allOutletsOn;  // 当前所有插座的状态
  
  MasterController(DEV_SmartOutlet* o1, DEV_SmartOutlet* o2, DEV_SmartOutlet* o3, DEV_SmartOutlet* o4) {
    outlets[0] = o1;
    outlets[1] = o2;
    outlets[2] = o3;
    outlets[3] = o4;
    
    lastButtonState = true;  // 按钮上拉，未按下时为高电平
    lastButtonTime = 0;
    allOutletsOn = false;
    
    pinMode(MASTER_BUTTON, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
  }
  
  void loop() {
    uint32_t currentTime = millis();
    bool buttonState = digitalRead(MASTER_BUTTON);
    
    // 检测按钮按下 (下降沿)
    if (!buttonState && lastButtonState && (currentTime - lastButtonTime > 200)) {
      lastButtonTime = currentTime;
      toggleAllOutlets();
    }
    
    lastButtonState = buttonState;
    updateStatusLED();
  }
  
  // 切换所有插座状态
  void toggleAllOutlets() {
    // 检查当前所有插座状态
    bool anyOutletOn = false;
    for (int i = 0; i < 4; i++) {
      if (outlets[i]->getState()) {
        anyOutletOn = true;
        break;
      }
    }
    
    // 如果有任何一个插座开启，则全部关闭；否则全部开启
    bool newState = !anyOutletOn;
    
    for (int i = 0; i < 4; i++) {
      outlets[i]->setOutletState(newState);
    }
    
    allOutletsOn = newState;
  }
  
  // 更新状态LED
  void updateStatusLED() {
    bool anyOutletOn = false;
    int onCount = 0;
    
    // 统计开启的插座数量
    for (int i = 0; i < 4; i++) {
      if (outlets[i]->getState()) {
        anyOutletOn = true;
        onCount++;
      }
    }
    
    if (!anyOutletOn) {
      // 全部关闭 - LED熄灭
      digitalWrite(LED_PIN, LOW);
    } else if (onCount == 4) {
      // 全部开启 - LED常亮
      digitalWrite(LED_PIN, HIGH);
    } else {
      // 部分开启 - LED闪烁
      uint32_t currentTime = millis();
      digitalWrite(LED_PIN, (currentTime / 500) % 2);
    }
  }
  

};

// 全局变量
DEV_SmartOutlet* outlet1;
DEV_SmartOutlet* outlet2; 
DEV_SmartOutlet* outlet3;
DEV_SmartOutlet* outlet4;
MasterController* masterController;

void setup() {
  Serial.begin(115200);
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Outlets, "HomeKit智能插座");
  homeSpan.enableAutoStartAP();
  
  // 创建主配件
  new SpanAccessory();
  
  // 添加配件信息服务
  new Service::AccessoryInformation();
  new Characteristic::Name("智能插座组");
  new Characteristic::Manufacturer("XcuiTech Inc.");
  new Characteristic::SerialNumber("OUTLET-001");
  new Characteristic::Model("SmartOutlet-4Ch");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();
  
  // 创建4个插座服务
  outlet1 = new DEV_SmartOutlet(OUTLET1_PIN, "插座1");
  outlet2 = new DEV_SmartOutlet(OUTLET2_PIN, "插座2");
  outlet3 = new DEV_SmartOutlet(OUTLET3_PIN, "插座3");
  outlet4 = new DEV_SmartOutlet(OUTLET4_PIN, "插座4");
  
  // 创建全局控制器
  masterController = new MasterController(outlet1, outlet2, outlet3, outlet4);
}

void loop() {
  homeSpan.poll();
  masterController->loop();
}