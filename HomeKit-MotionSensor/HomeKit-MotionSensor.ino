//HomeKit 人体存在传感器 (Motion Sensor)

#include "HomeSpan.h" 

// 引脚定义
#define PIR_SENSOR_PIN 0  // 连接PIR传感器的数字引脚
#define LED_PIN 2         // LED指示灯引脚 (ESP32内置LED通常为2)

struct DEV_MotionSensor : Service::MotionSensor {     // 运动传感器服务

  SpanCharacteristic *motionDetected;           // 运动检测特性 (必需)
  SpanCharacteristic *statusActive;             // 传感器活动状态 (可选)
  SpanCharacteristic *statusFault;              // 故障状态 (可选)
  SpanCharacteristic *statusLowBattery;         // 低电量状态 (可选)

  int pirPin;                                   // PIR传感器引脚

  DEV_MotionSensor(int pin) : Service::MotionSensor() {
    
    pirPin = pin;
    
    // 创建运动检测特性 (必需)
    motionDetected = new Characteristic::MotionDetected(0);  // 初始值为0 (未检测到运动)
    
    // 创建可选状态特性
    statusActive = new Characteristic::StatusActive(1);           // 1 = 传感器正常工作
    statusFault = new Characteristic::StatusFault(0);             // 0 = 无故障
    statusLowBattery = new Characteristic::StatusLowBattery(0);   // 0 = 电量正常
    
    pinMode(pirPin, INPUT);                     // 设置PIR引脚为输入模式
    
    Serial.print("配置运动传感器在引脚: ");
    Serial.println(pirPin);
  }

  void loop() {
    
    // 读取PIR传感器状态
    int motionState = digitalRead(pirPin);
    
    // 获取当前运动检测状态
    int currentMotion = motionDetected->getVal();
    
    // 如果状态发生变化，更新HomeKit
    if (motionState != currentMotion) {
      motionDetected->setVal(motionState);
      
      Serial.print("运动传感器状态更新: ");
      Serial.println(motionState ? "检测到运动" : "未检测到运动");
      
      // 可选：添加LED指示灯
      digitalWrite(LED_PIN, motionState);
    }
  }
};

void setup() {
  Serial.begin(115200);
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Sensors, "运动传感器");
  homeSpan.enableAutoStartAP();
  // 创建HomeKit配件
  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Manufacturer("XcuiTech Inc.");
      new Characteristic::SerialNumber("HS-PIR-001");
      new Characteristic::Model("PIR运动传感器");
      new Characteristic::FirmwareRevision("1.0");
      new Characteristic::HardwareRevision("1.0");
    
    // 添加运动传感器服务
    new DEV_MotionSensor(PIR_SENSOR_PIN);

  // 配置LED指示灯
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("HomeKit运动传感器已启动");
  Serial.println("等待配对...");
}

void loop() {
  homeSpan.poll();    // 处理HomeSpan事务
}
