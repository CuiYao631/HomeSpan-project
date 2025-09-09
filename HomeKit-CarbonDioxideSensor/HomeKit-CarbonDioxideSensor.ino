/*
 * HomeKit CO2 Sensor with HomeSpan
 * 基于 HomeSpan 库的 HomeKit 二氧化碳传感器
 * 
 * 硬件需求:
 * - ESP32 开发板
 * - MQ-135 气体传感器 (A0接ADC引脚)
 * - 可选: LED指示器 (GPIO2)
 * - 可选: 蜂鸣器警报 (GPIO18)
 * 
 * 支持特性:
 * - CarbonDioxideDetected: 检测异常CO2水平
 * - CarbonDioxideLevel: 当前CO2浓度 (ppm)
 * - CarbonDioxidePeakLevel: 峰值CO2浓度 (ppm)
 * - StatusActive: 传感器工作状态
 * - StatusFault: 故障状态
 * - StatusLowBattery: 电池状态
 */

#include "HomeSpan.h"

// 硬件配置
#define CO2_SENSOR_PIN    A0      // MQ-135传感器模拟引脚
#define LED_PIN           2       // LED指示器引脚
#define BUZZER_PIN        18      // 蜂鸣器引脚

// 传感器配置
#define CO2_NORMAL_THRESHOLD    1000    // 正常CO2阈值 (ppm)
#define CO2_WARNING_THRESHOLD   2000    // 警告CO2阈值 (ppm)
#define CO2_DANGER_THRESHOLD    5000    // 危险CO2阈值 (ppm)

// CO2传感器服务类
struct DEV_CarbonDioxideSensor : Service::CarbonDioxideSensor {
  
  // HomeKit特性定义
  Characteristic::CarbonDioxideDetected carbonDioxideDetected{0};        // 异常检测
  Characteristic::CarbonDioxideLevel carbonDioxideLevel{400};            // CO2浓度
  Characteristic::CarbonDioxidePeakLevel carbonDioxidePeakLevel{400};    // 峰值CO2浓度
  Characteristic::StatusActive statusActive{1};                          // 传感器活跃状态
  Characteristic::StatusFault statusFault{0};                            // 故障状态
  Characteristic::StatusLowBattery statusLowBattery{0};                  // 低电量状态
  Characteristic::ConfiguredName configuredName{"CO2 Sensor"};           // 设备名称
  
  // 传感器数据
  float currentCO2Level = 400.0;      // 当前CO2浓度
  float peakCO2Level = 400.0;         // 峰值CO2浓度
  bool isAbnormal = false;            // 异常状态标志
  
  // 时间管理
  uint32_t lastSensorRead = 0;        // 上次传感器读取时间
  uint32_t lastHeartbeat = 0;         // 上次心跳时间
  uint32_t lastPeakReset = 0;         // 上次峰值重置时间
  
  // 构造函数
  DEV_CarbonDioxideSensor() : Service::CarbonDioxideSensor() {
    Serial.println("✓ CO2传感器服务已初始化");
    
    // 初始化硬件
    initSensor();
    
    // 读取初始传感器数据
    readSimulatedSensorData();
    
    Serial.println("✓ CO2传感器就绪");
  }
  
  // 初始化传感器硬件
  void initSensor() {
    // 设置引脚模式
    pinMode(CO2_SENSOR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    // 关闭蜂鸣器和LED
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    
    Serial.println("✓ CO2传感器硬件初始化完成");
  }
  
  // 主循环处理函数
  void loop() override {
    uint32_t currentTime = millis();
    
    // 每5秒读取一次传感器
    if (currentTime - lastSensorRead >= 5000) {
      readSimulatedSensorData();
      lastSensorRead = currentTime;
    }
    
    // 每30秒发送一次心跳
    if (currentTime - lastHeartbeat >= 30000) {
      sendHeartbeat();
      lastHeartbeat = currentTime;
    }
    
    // 每小时重置峰值
    if (currentTime - lastPeakReset >= 3600000) { // 1小时
      resetPeakLevel();
      lastPeakReset = currentTime;
    }
    
    // 更新LED和蜂鸣器状态
    updateIndicators();
  }
  
  // 读取模拟传感器数据
  void readSimulatedSensorData() {
    // 生成模拟CO2数据 (400-2000 ppm之间变化)
    static float baseCO2 = 450.0;
    static float trend = 0.5;
    
    // 随机变化
    float randomChange = (random(-100, 101) / 100.0) * 50; // ±50 ppm随机变化
    
    // 趋势变化
    baseCO2 += trend;
    if (baseCO2 > 1800) trend = -1.0;
    if (baseCO2 < 400) trend = 1.0;
    
    currentCO2Level = baseCO2 + randomChange;
    
    // 限制范围
    if (currentCO2Level < 300) currentCO2Level = 300;
    if (currentCO2Level > 3000) currentCO2Level = 3000;
    
    // 更新峰值
    if (currentCO2Level > peakCO2Level) {
      peakCO2Level = currentCO2Level;
      carbonDioxidePeakLevel.setVal(peakCO2Level);
      Serial.printf("📈 新CO2峰值: %.0f ppm\n", peakCO2Level);
    }
    
    // 检测异常水平
    updateAbnormalStatus();
    
    // 更新HomeKit特性
    carbonDioxideLevel.setVal(currentCO2Level);
    
    Serial.printf("🌪️  CO2浓度: %.0f ppm", currentCO2Level);
    if (isAbnormal) Serial.print(" ⚠️ 异常");
    Serial.println();
  }
  
  // 更新异常状态
  void updateAbnormalStatus() {
    bool previousAbnormal = isAbnormal;
    
    // 根据CO2浓度判断是否异常
    isAbnormal = (currentCO2Level > CO2_NORMAL_THRESHOLD);
    
    // 如果状态改变，更新HomeKit
    if (isAbnormal != previousAbnormal) {
      carbonDioxideDetected.setVal(isAbnormal ? 1 : 0);
      
      if (isAbnormal) {
        Serial.printf("⚠️  CO2浓度异常! %.0f ppm (阈值: %d ppm)\n", 
                     currentCO2Level, CO2_NORMAL_THRESHOLD);
      } else {
        Serial.println("✅ CO2浓度恢复正常");
      }
    }
  }
  
  // 更新指示器状态
  void updateIndicators() {
    // LED指示
    if (isAbnormal) {
      // 异常时LED闪烁
      digitalWrite(LED_PIN, (millis() / 500) % 2);
    } else {
      // 正常时LED常亮
      digitalWrite(LED_PIN, HIGH);
    }
    
    // 蜂鸣器警报
    if (currentCO2Level > CO2_DANGER_THRESHOLD) {
      // 危险级别：快速蜂鸣
      if ((millis() / 200) % 2) {
        digitalWrite(BUZZER_PIN, HIGH);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
      }
    } else if (currentCO2Level > CO2_WARNING_THRESHOLD) {
      // 警告级别：慢速蜂鸣
      if ((millis() / 1000) % 2) {
        digitalWrite(BUZZER_PIN, HIGH);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
      }
    } else {
      // 正常级别：关闭蜂鸣器
      digitalWrite(BUZZER_PIN, LOW);
    }
  }
  
  // 发送心跳信号
  void sendHeartbeat() {
    Serial.printf("💓 心跳 - CO2: %.0f ppm, 峰值: %.0f ppm, 状态: %s\n", 
                 currentCO2Level, peakCO2Level, 
                 isAbnormal ? "异常" : "正常");
  }
  
  // 重置峰值
  void resetPeakLevel() {
    peakCO2Level = currentCO2Level;
    carbonDioxidePeakLevel.setVal(peakCO2Level);
    Serial.printf("🔄 CO2峰值已重置为: %.0f ppm\n", peakCO2Level);
  }
  
  // 获取CO2质量等级描述
  String getCO2QualityDescription() {
    if (currentCO2Level < 400) return "极佳";
    else if (currentCO2Level < 600) return "良好"; 
    else if (currentCO2Level < 1000) return "一般";
    else if (currentCO2Level < 2000) return "较差";
    else if (currentCO2Level < 5000) return "差";
    else return "极差";
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("\n🏠 HomeKit CO2传感器启动中...");
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Sensors, "HomeKit CO2传感器");
  homeSpan.enableAutoStartAP();
  
  // 创建配件
  new SpanAccessory();
  
  // 添加必需的配件信息服务
  new Service::AccessoryInformation();
  new Characteristic::Name("CO2传感器");
  new Characteristic::Manufacturer("HomeSpan");
  new Characteristic::SerialNumber("CO2-001");
  new Characteristic::Model("MQ135-CO2");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();
  
  // 添加CO2传感器服务
  new DEV_CarbonDioxideSensor();
  
}

void loop() {
  homeSpan.poll();
}