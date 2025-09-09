//HomeKit 空气质量传感器 (Air Quality Sensor)
//支持多种空气质量指标监测：PM2.5, PM10, VOC, 臭氧等

#include "HomeSpan.h"

// 引脚定义
#define VOC_SENSOR_PIN A0     // VOC传感器模拟引脚 (MQ135等)
#define LED_PIN 2             // LED状态指示灯
#define BUZZER_PIN 5          // 可选：蜂鸣器报警

struct DEV_AirQualitySensor : Service::AirQualitySensor {

  // 必需特性
  SpanCharacteristic *airQuality;           // 空气质量主观评价 (必需)
  
  // 可选的颗粒物特性
  SpanCharacteristic *pm25Density;         // PM2.5密度
  SpanCharacteristic *pm10Density;         // PM10密度
  
  // 可选的气体特性
  SpanCharacteristic *vocDensity;          // 挥发性有机化合物密度
  SpanCharacteristic *ozoneDensity;        // 臭氧密度
  SpanCharacteristic *no2Density;          // 二氧化氮密度
  SpanCharacteristic *so2Density;          // 二氧化硫密度
  
  // 状态特性
  SpanCharacteristic *statusActive;        // 传感器活动状态
  SpanCharacteristic *statusFault;         // 故障状态
  SpanCharacteristic *statusTampered;      // 防拆状态
  SpanCharacteristic *statusLowBattery;    // 低电量状态

  // 传感器数据
  struct AirQualityData {
    float pm25;
    float pm10;
    float voc;
    float ozone;
    float no2;
    float so2;
    uint8_t quality;
  } currentData;

  // 配置参数
  unsigned long lastReadTime;
  const unsigned long READ_INTERVAL = 30000;  // 30秒读取间隔
  const unsigned long WARM_UP_TIME = 30000;   // 30秒预热时间

  DEV_AirQualitySensor() : Service::AirQualitySensor() {
    
    // 创建必需的空气质量特性
    airQuality = new Characteristic::AirQuality(0);  // 初始值：未知
    
    // 创建颗粒物特性 (范围 0-1000 µg/m³)
    pm25Density = new Characteristic::PM25Density(0);
    pm10Density = new Characteristic::PM10Density(0);
    
    // 创建气体特性 (范围 0-1000 µg/m³)
    vocDensity = new Characteristic::VOCDensity(0);
    ozoneDensity = new Characteristic::OzoneDensity(0);
    no2Density = new Characteristic::NitrogenDioxideDensity(0);
    so2Density = new Characteristic::SulphurDioxideDensity(0);
    
    // 创建状态特性
    statusActive = new Characteristic::StatusActive(1);          // 正常工作
    statusFault = new Characteristic::StatusFault(0);            // 无故障
    statusTampered = new Characteristic::StatusTampered(0);      // 未被拆卸
    statusLowBattery = new Characteristic::StatusLowBattery(0);  // 电量正常
    
    // 初始化传感器
    initSensors();
    
    Serial.println("空气质量传感器已配置完成");
    Serial.println("支持监测项目：PM2.5, PM10, VOC, O₃, NO₂, SO₂");
  }

  void initSensors() {
    // 配置引脚
    pinMode(VOC_SENSOR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    // 初始化数据结构
    memset(&currentData, 0, sizeof(currentData));
    lastReadTime = 0;
    
    Serial.println("传感器初始化完成（使用模拟数据），等待预热...");
    
    // 预热指示
    for(int i = 0; i < 6; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(250);
      digitalWrite(LED_PIN, LOW);
      delay(250);
    }
  }

  void loop() {
    
    unsigned long currentTime = millis();
    
    // 检查是否需要读取传感器数据
    if (currentTime - lastReadTime >= READ_INTERVAL) {
      
      // 读取各种传感器数据 (使用模拟数据)
      readSimulatedSensorData();
      calculateAirQuality();
      
      // 更新HomeKit特性
      updateHomeKitCharacteristics();
      
      // 更新LED状态指示
      updateStatusLED();
      
      // 检查是否需要报警
      checkAlarms();
      
      lastReadTime = currentTime;
      
      // 输出调试信息
      printSensorData();
    }
    
    // 定期健康检查
    static unsigned long lastHealthCheck = 0;
    if (currentTime - lastHealthCheck > 300000) {  // 每5分钟检查
      performHealthCheck();
      lastHealthCheck = currentTime;
    }
  }

  void readSimulatedSensorData() {
    // 生成模拟的传感器数据，用于测试HomeKit功能
    static float baseValues[] = {25.0, 45.0, 150.0, 15.0, 8.0, 5.0}; // 基础值
    static float variations[] = {0.8, 0.9, 1.1, 0.7, 1.2, 0.6};      // 变化系数
    
    // 添加一些随机波动，模拟真实传感器
    float randomFactor = 1.0 + (random(-20, 21) / 100.0);  // ±20%的随机波动
    
    currentData.pm25 = baseValues[0] * variations[0] * randomFactor;
    currentData.pm10 = baseValues[1] * variations[1] * randomFactor;
    currentData.voc = baseValues[2] * variations[2] * randomFactor;
    currentData.ozone = baseValues[3] * variations[3] * randomFactor;
    currentData.no2 = baseValues[4] * variations[4] * randomFactor;
    currentData.so2 = baseValues[5] * variations[5] * randomFactor;
    
    // 确保数值不为负
    if (currentData.pm25 < 0) currentData.pm25 = 0;
    if (currentData.pm10 < 0) currentData.pm10 = 0;
    if (currentData.voc < 0) currentData.voc = 0;
    if (currentData.ozone < 0) currentData.ozone = 0;
    if (currentData.no2 < 0) currentData.no2 = 0;
    if (currentData.so2 < 0) currentData.so2 = 0;
    
    // 每次读取后微调基础值，模拟环境变化
    for (int i = 0; i < 6; i++) {
      variations[i] += (random(-5, 6) / 1000.0);  // 微小调整
      if (variations[i] < 0.5) variations[i] = 0.5;
      if (variations[i] > 1.5) variations[i] = 1.5;
    }
    
    Serial.printf("模拟传感器数据 - PM2.5: %.1f µg/m³, PM10: %.1f µg/m³, VOC: %.1f µg/m³\n", 
                 currentData.pm25, currentData.pm10, currentData.voc);
  }

  void calculateAirQuality() {
    // 根据各项指标综合计算空气质量等级
    uint8_t quality = 1; // 默认优秀
    
    // PM2.5标准 (中国标准)
    if (currentData.pm25 > 75) quality = 5;       // 差
    else if (currentData.pm25 > 55) quality = 4;  // 较差
    else if (currentData.pm25 > 35) quality = 3;  // 一般
    else if (currentData.pm25 > 15) quality = 2;  // 良好
    
    // PM10标准
    if (currentData.pm10 > 150) quality = max(quality, (uint8_t)5);
    else if (currentData.pm10 > 100) quality = max(quality, (uint8_t)4);
    else if (currentData.pm10 > 75) quality = max(quality, (uint8_t)3);
    else if (currentData.pm10 > 50) quality = max(quality, (uint8_t)2);
    
    // VOC标准
    if (currentData.voc > 500) quality = max(quality, (uint8_t)5);
    else if (currentData.voc > 300) quality = max(quality, (uint8_t)4);
    else if (currentData.voc > 200) quality = max(quality, (uint8_t)3);
    else if (currentData.voc > 100) quality = max(quality, (uint8_t)2);
    
    currentData.quality = quality;
  }

  void updateHomeKitCharacteristics() {
    // 更新所有HomeKit特性
    airQuality->setVal(currentData.quality);
    pm25Density->setVal(currentData.pm25);
    pm10Density->setVal(currentData.pm10);
    vocDensity->setVal(currentData.voc);
    ozoneDensity->setVal(currentData.ozone);
    no2Density->setVal(currentData.no2);
    so2Density->setVal(currentData.so2);
    
    Serial.printf("HomeKit已更新 - 空气质量等级: %d\n", currentData.quality);
  }

  void updateStatusLED() {
    // 根据空气质量等级设置LED颜色/闪烁模式
    switch(currentData.quality) {
      case 1: // 优秀 - 绿色常亮
        digitalWrite(LED_PIN, HIGH);
        break;
      case 2: // 良好 - 慢闪
        digitalWrite(LED_PIN, (millis() / 1000) % 2);
        break;
      case 3: // 一般 - 快闪
        digitalWrite(LED_PIN, (millis() / 500) % 2);
        break;
      case 4: // 较差 - 很快闪
        digitalWrite(LED_PIN, (millis() / 200) % 2);
        break;
      case 5: // 差 - 常亮报警
        digitalWrite(LED_PIN, HIGH);
        break;
      default: // 未知
        digitalWrite(LED_PIN, LOW);
        break;
    }
  }

  void checkAlarms() {
    // 检查是否需要蜂鸣器报警
    bool shouldAlarm = false;
    
    if (currentData.pm25 > 75 || currentData.pm10 > 150 || currentData.voc > 500) {
      shouldAlarm = true;
    }
    
    if (shouldAlarm) {
      // 间歇性蜂鸣报警
      if ((millis() / 2000) % 2) {
        tone(BUZZER_PIN, 1000, 200);
      }
    }
  }

  void performHealthCheck() {
    // 传感器健康检查
    bool sensorOK = true;
    
    // 检查传感器是否响应
    if (currentData.pm25 == 0 && currentData.pm10 == 0 && currentData.voc == 0) {
      sensorOK = false;
      Serial.println("⚠️ 传感器可能故障 - 所有读数为0");
    }
    
    // 更新状态
    statusActive->setVal(sensorOK ? 1 : 0);
    statusFault->setVal(sensorOK ? 0 : 1);
  }

  void printSensorData() {
    Serial.println("=== 空气质量监测数据 ===");
    Serial.printf("空气质量等级: %d ", currentData.quality);
    
    switch(currentData.quality) {
      case 1: Serial.println("(优秀)"); break;
      case 2: Serial.println("(良好)"); break;
      case 3: Serial.println("(一般)"); break;
      case 4: Serial.println("(较差)"); break;
      case 5: Serial.println("(差)"); break;
      default: Serial.println("(未知)"); break;
    }
    
    Serial.printf("PM2.5: %.1f µg/m³\n", currentData.pm25);
    Serial.printf("PM10:  %.1f µg/m³\n", currentData.pm10);
    Serial.printf("VOC:   %.1f µg/m³\n", currentData.voc);
    Serial.printf("O₃:    %.1f µg/m³\n", currentData.ozone);
    Serial.printf("NO₂:   %.1f µg/m³\n", currentData.no2);
    Serial.printf("SO₂:   %.1f µg/m³\n", currentData.so2);
    Serial.println("========================");
  }
};

void setup() {
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=========================================");
  Serial.println("HomeKit 智能空气质量监测站启动中...");
  Serial.println("=========================================");
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Sensors, "空气质量监测站");
  homeSpan.enableAutoStartAP();
  
  // 创建HomeKit配件
  new SpanAccessory();
  
    // 配件信息服务
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Manufacturer("HomeSpan");
      new Characteristic::SerialNumber("HS-AirQuality-001");
      new Characteristic::Model("智能空气质量监测站");
      new Characteristic::FirmwareRevision("1.0.0");
      new Characteristic::HardwareRevision("1.0.0");
    
    // 空气质量传感器服务
    new DEV_AirQualitySensor();

  Serial.println("HomeKit空气质量监测站启动完成!");
  Serial.println("请使用家庭应用添加此配件");
  Serial.println("监测项目：PM2.5, PM10, VOC, O₃, NO₂, SO₂");
  Serial.println("空气质量等级：1=优秀, 2=良好, 3=一般, 4=较差, 5=差");
  
  // 启动完成指示
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    delay(300);
  }
}

void loop() {
  homeSpan.poll();
}