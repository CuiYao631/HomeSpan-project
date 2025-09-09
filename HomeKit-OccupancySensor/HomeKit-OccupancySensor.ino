//HomeKit 占用传感器 (Occupancy Sensor)
//支持多种传感器：PIR、雷达、红外等检测人员占用情况

#include "HomeSpan.h" 

// 引脚定义
#define OCCUPANCY_SENSOR_PIN 4    // 占用传感器引脚 (PIR/雷达等)
#define LED_PIN 2                 // LED状态指示灯引脚
#define BUTTON_PIN 0              // 可选：校准按钮引脚

struct DEV_OccupancySensor : Service::OccupancySensor {

  SpanCharacteristic *occupancyDetected;       // 占用检测特性 (必需)
  SpanCharacteristic *statusActive;            // 传感器活动状态 (可选)
  SpanCharacteristic *statusFault;             // 故障状态 (可选) 
  SpanCharacteristic *statusTampered;          // 防拆状态 (可选)
  SpanCharacteristic *statusLowBattery;        // 低电量状态 (可选)

  int sensorPin;                               // 传感器引脚
  int lastOccupancyState;                      // 上次占用状态
  unsigned long lastStateChangeTime;          // 状态变化时间
  unsigned long occupancyStartTime;           // 占用开始时间
  
  // 配置参数
  const unsigned long DEBOUNCE_DELAY = 200;   // 防抖延时 (毫秒)
  const unsigned long MIN_OCCUPANCY_TIME = 5000; // 最小占用时间 (5秒)
  
  DEV_OccupancySensor(int pin) : Service::OccupancySensor() {
    
    sensorPin = pin;
    lastOccupancyState = 0;
    lastStateChangeTime = 0;
    occupancyStartTime = 0;
    
    // 创建必需的占用检测特性
    occupancyDetected = new Characteristic::OccupancyDetected(0);  // 初始值：未检测到占用
    
    // 创建可选状态特性
    statusActive = new Characteristic::StatusActive(1);           // 1 = 传感器正常工作
    statusFault = new Characteristic::StatusFault(0);             // 0 = 无故障
    statusTampered = new Characteristic::StatusTampered(0);       // 0 = 未被拆卸
    statusLowBattery = new Characteristic::StatusLowBattery(0);   // 0 = 电量正常
    
    // 配置传感器引脚
    pinMode(sensorPin, INPUT);
    
    Serial.print("占用传感器配置在引脚: ");
    Serial.println(sensorPin);
    Serial.println("占用传感器已就绪，等待检测...");
  }

  void loop() {
    
    unsigned long currentTime = millis();
    
    // 读取传感器状态
    int currentSensorReading = digitalRead(sensorPin);
    
    // 防抖处理
    if (currentSensorReading != lastOccupancyState) {
      if (currentTime - lastStateChangeTime > DEBOUNCE_DELAY) {
        
        // 状态确实发生了变化
        if (currentSensorReading == 1) {
          // 检测到占用
          occupancyDetected->setVal(1);
          occupancyStartTime = currentTime;
          digitalWrite(LED_PIN, HIGH);  // 点亮LED
          
          Serial.println("✓ 检测到人员占用");
          
        } else {
          // 检测到空闲
          // 确保至少占用了最小时间才认为是真正的离开
          if (currentTime - occupancyStartTime > MIN_OCCUPANCY_TIME) {
            occupancyDetected->setVal(0);
            digitalWrite(LED_PIN, LOW);   // 熄灭LED
            
            Serial.println("○ 人员已离开，空间空闲");
          }
        }
        
        lastOccupancyState = currentSensorReading;
      }
      lastStateChangeTime = currentTime;
    }
    
    // 可选：定期检查传感器健康状态
    static unsigned long lastHealthCheck = 0;
    if (currentTime - lastHealthCheck > 60000) {  // 每分钟检查一次
      checkSensorHealth();
      lastHealthCheck = currentTime;
    }
  }
  
  // 传感器健康检查
  void checkSensorHealth() {
    // 简单的健康检查逻辑
    // 实际应用中可以添加更复杂的检查
    
    bool sensorOk = true;  // 假设传感器正常
    
    if (sensorOk) {
      statusActive->setVal(1);   // 传感器正常工作
      statusFault->setVal(0);    // 无故障
    } else {
      statusActive->setVal(0);   // 传感器故障
      statusFault->setVal(1);    // 有故障
      Serial.println("⚠️ 传感器健康检查失败!");
    }
  }
  
  // 可选：校准功能
  void calibrateSensor() {
    Serial.println("开始传感器校准...");
    
    // LED闪烁表示校准中
    for(int i = 0; i < 6; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
    
    // 重置状态
    lastOccupancyState = 0;
    occupancyDetected->setVal(0);
    
    Serial.println("传感器校准完成");
  }
};

void setup() {
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("HomeKit 占用传感器启动中...");
  Serial.println("=================================");
  
  // 配置LED和按钮引脚
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Sensors, "占用传感器");
  homeSpan.enableAutoStartAP();
  
  // 创建HomeKit配件
  new SpanAccessory();
  
    // 配件信息服务 (必需)
    new Service::AccessoryInformation();
      new Characteristic::Identify();                
      new Characteristic::Manufacturer("HomeSpan");
      new Characteristic::SerialNumber("HS-Occupancy-001");    
      new Characteristic::Model("智能占用传感器");
      new Characteristic::FirmwareRevision("1.0.0");
      new Characteristic::HardwareRevision("1.0.0");
    
    // 占用传感器服务
    new DEV_OccupancySensor(OCCUPANCY_SENSOR_PIN);

  Serial.println("HomeKit占用传感器已启动完成");
  Serial.println("请使用家庭应用添加此配件");
  Serial.println("传感器类型：占用检测 (Occupancy)");
  Serial.println("支持功能：人员占用检测、状态监控");
  
  // 启动指示 - LED快速闪烁
  for(int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
}

void loop() {
  
  // 运行HomeSpan主循环
  homeSpan.poll();
  
  // 检查校准按钮
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    // 按钮被按下，触发校准
    delay(50);  // 简单防抖
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("校准按钮被按下");
      // 这里可以调用校准函数
      // 注意：需要通过全局变量或其他方式访问传感器对象
    }
  }
  lastButtonState = currentButtonState;
}