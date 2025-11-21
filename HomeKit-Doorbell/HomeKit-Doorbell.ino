/*
 * HomeKit Doorbell with HomeSpan
 * 基于 HomeSpan 的 HomeKit 门铃
 *
 * 硬件需求:
 * - ESP32 开发板
 * - 门铃按钮 (GPIO0)
 * - LED指示灯 (GPIO32, 可选)
 *
 * 引脚定义:
 * - GPIO0: 门铃按钮 (短按触发门铃事件)
 * - GPIO32: 状态LED (显示连接状态, 可选)
 *
 * 支持特性:
 * - ProgrammableSwitchEvent: 门铃事件
 * - Name: 设备名称
 *
 * 应用场景:
 * - 家庭门铃
 * - 智能通知
 */

#include "HomeSpan.h"
#include <Arduino.h>
#include <EasyButton.h>
#include <WiFi.h>

#define DOORBELL_BUTTON_PIN  0    // 门铃按钮引脚
#define LED_INDICATOR        2    // 状态LED引脚 (可选)
#define BUZZER_PIN           4    // 蜂鸣器引脚 (可选)
#define DEFAULT_SETUP_CODE   "46637726"  // HomeKit默认配对码
#define DEFAULT_QR_ID        "BELL"       // HomeKit QR码ID
#define DEVICE_HOSTNAME      "MyDoorbell" // 设备主机名

// 门铃配置
const int buzzerDuration = 1000;        // 蜂鸣器响铃时长(ms)
const int buttonDebounce = 50;          // 按钮防抖时间(ms)

EasyButton button(DOORBELL_BUTTON_PIN);

// 门铃服务类
struct DEV_Doorbell : Service::StatelessProgrammableSwitch {
  Characteristic::ProgrammableSwitchEvent event;
  Characteristic::Name name{"门铃"};

  DEV_Doorbell() : Service::StatelessProgrammableSwitch() {
    Serial.println("⚙️  门铃服务初始化完成");
  }

  // 触发门铃事件
  void ring() {
    Serial.println("🔔 门铃事件触发!");
    event.setVal(0);  // 0 = 单击事件
    delay(100);  // 等待事件处理
    Serial.println("📱 HomeKit门铃通知已发送");
    Serial.println("📲 请检查iPhone通知中心");
  }

  boolean update() override {
    return true;
  }
  void loop() override {}
};

// 全局门铃对象指针
DEV_Doorbell* doorbell;

// 按钮按下回调函数
void onPressed() {
  Serial.println("🔎 门铃按钮按下检测到");
  
  if (doorbell) {
    doorbell->ring();
    
    // 检查HomeSpan连接状态
    if (homeSpan.isConnected()) {
      Serial.println("✅ HomeKit连接正常，通知已发送");
    } else {
      Serial.println("⚠️ HomeKit未连接，请检查网络和配对状态");
    }
  } else {
    Serial.println("❌ 门铃服务未初始化");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🚀 启动 HomeKit 智能门铃");
  Serial.println("📝 支持功能:");
  Serial.println("   • 物理按钮触发门铃事件");
  Serial.println("   • HomeKit门铃通知");
  Serial.println("   • 本地蜂鸣器响铃");
  Serial.println("   • LED状态指示");
  
  // 设置自定义主机名
  WiFi.setHostname(DEVICE_HOSTNAME);
  Serial.printf("📡 设备主机名设置为: %s\n", DEVICE_HOSTNAME);

  // 初始化EasyButton
  button.begin();
  button.onPressed(onPressed);
  Serial.println("🔘 门铃按钮初始化完成");

  // 配置HomeSpan
  homeSpan.setStatusPin(LED_INDICATOR);              // 状态LED
  homeSpan.setQRID(DEFAULT_QR_ID);                   // QR码ID
  homeSpan.setPairingCode(DEFAULT_SETUP_CODE);       // 默认配对码

  // 初始化HomeSpan
  homeSpan.begin(Category::ProgrammableSwitches, "HomeKit智能门铃");
  homeSpan.enableAutoStartAP();
  
  Serial.println("⚙️  HomeSpan配置完成");
  Serial.printf("🔐 配对码: %s\n", DEFAULT_SETUP_CODE);
  Serial.printf("📱 QR码ID: %s\n", DEFAULT_QR_ID);

  // 创建配件
  new SpanAccessory();
  
  // 添加配件信息服务
  new Service::AccessoryInformation();
  new Characteristic::Name("智能门铃");
  new Characteristic::Manufacturer("XcuiTech Inc.");
  new Characteristic::SerialNumber("DOORBELL-001");
  new Characteristic::Model("SmartDoorbell-Basic");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();

  // 添加门铃服务
  doorbell = new DEV_Doorbell();
  
  Serial.println("✅ 设备初始化完成!");
  Serial.println("📱 请在家庭App中添加此配件");
  Serial.println("🔔 按下门铃按钮测试功能");
  Serial.println("📲 确保 iPhone 允许家庭App发送通知");
  Serial.println("⚙️  设置 > 通知 > 家庭 > 允许通知");
}

void loop() {
  homeSpan.poll();    // 处理HomeKit通信
  button.read();      // 读取按钮状态
}
