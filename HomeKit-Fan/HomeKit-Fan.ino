/*
 * HomeKit Smart Fan with HomeSpan
 * 基于 HomeSpan 库的 HomeKit 智能风扇控制器
 * 
 * 硬件需求:
 * - ESP32 开发板
 * - 直流电机 (PWM控制) 或 继电器控制的风扇
 * - 电机驱动模块 (L298N/L9110等)
 * - 可选: LED状态指示器 (GPIO2)
 * - 可选: 旋转编码器 (调速控制)
 * - 可选: 按钮控制 (手动开关)
 * 
 * 支持特性:
 * - Active: 风扇开关状态
 * - CurrentFanState: 当前风扇工作状态
 * - TargetFanState: 目标风扇状态 (手动/自动)
 * - RotationDirection: 旋转方向控制
 * - RotationSpeed: 速度调节 (0-100%)
 * - SwingMode: 摆动/摇头功能
 * - LockPhysicalControls: 物理控制锁定
 * - ConfiguredName: 设备名称
 * 
 * 应用场景:
 * - 客厅吊扇控制
 * - 卧室台式风扇
 * - 工业通风扇
 * - 智能空气循环
 */

#include "HomeSpan.h"
#include <Arduino.h>

// 硬件配置
#define FAN_PWM_PIN           5       // 风扇PWM控制引脚
#define FAN_DIR_PIN           18      // 风扇方向控制引脚
#define SWING_SERVO_PIN       19      // 摆动舵机控制引脚
#define LED_PIN               2       // LED状态指示器引脚
#define BUTTON_PIN            0       // 手动控制按钮引脚
#define ENCODER_CLK_PIN       21      // 旋转编码器CLK引脚
#define ENCODER_DT_PIN        22      // 旋转编码器DT引脚

// PWM配置
#define PWM_CHANNEL           0       // PWM通道
#define PWM_FREQUENCY         1000    // PWM频率 (Hz)
#define PWM_RESOLUTION        8       // PWM分辨率 (8位=0-255)

// 风扇控制配置
#define MIN_SPEED_THRESHOLD   15      // 最小运行速度阈值 (%)
#define SWING_RANGE           90      // 摆动角度范围 (度)

// 智能风扇服务类
struct DEV_SmartFan : Service::Fan {
  
  // HomeKit特性定义
  Characteristic::Active active{0};                                 // 风扇开关状态
  Characteristic::CurrentFanState currentFanState{0};               // 当前风扇状态
  Characteristic::TargetFanState targetFanState{0};                 // 目标风扇状态
  Characteristic::RotationDirection rotationDirection{0};           // 旋转方向
  Characteristic::RotationSpeed rotationSpeed{0};                   // 旋转速度
  Characteristic::SwingMode swingMode{0};                          // 摆动模式
  Characteristic::LockPhysicalControls lockPhysicalControls{0};    // 物理控制锁定
  Characteristic::ConfiguredName configuredName{"Smart Fan"};       // 设备名称
  
  // 风扇状态变量
  bool isActive = false;                // 风扇是否激活
  bool isManualMode = true;             // 手动模式标志
  bool isClockwise = true;              // 顺时针方向
  bool isSwinging = false;              // 摆动状态
  bool isPhysicalLocked = false;        // 物理控制锁定状态
  
  // 控制变量
  int currentSpeed = 0;                 // 当前速度 (0-100%)
  int targetSpeedValue = 0;             // 目标速度值
  int swingPosition = 90;               // 当前摆动位置 (度)
  int swingDirection = 1;               // 摆动方向 (1/-1)
  
  // 时间管理
  uint32_t lastSpeedUpdate = 0;         // 上次速度更新时间
  uint32_t lastSwingUpdate = 0;         // 上次摆动更新时间
  uint32_t lastButtonCheck = 0;         // 上次按钮检查时间
  uint32_t lastHeartbeat = 0;           // 上次心跳时间
  
  // 统计信息
  uint32_t totalRunTime = 0;            // 总运行时间
  uint32_t speedChanges = 0;            // 速度变化次数
  
  // 构造函数
  DEV_SmartFan() : Service::Fan() {
    Serial.println("✓ 智能风扇服务已初始化");
    
    // 初始化硬件
    initFanHardware();
    
    // 设置初始状态
    updateFanHardware();
    
    Serial.println("✓ 智能风扇就绪");
  }
  
  // 初始化风扇硬件
  void initFanHardware() {
    // 配置PWM输出 (使用新的ESP32 PWM API)
    ledcAttach(FAN_PWM_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
    
    // 设置引脚模式
    pinMode(FAN_DIR_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
    pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
    
    // 初始化输出状态
    digitalWrite(FAN_DIR_PIN, HIGH);  // 默认顺时针
    digitalWrite(LED_PIN, LOW);
    ledcWrite(FAN_PWM_PIN, 0);        // 风扇停止
    
    // 初始化舵机 (如果使用摆动功能)
    initSwingServo();
    
    Serial.println("✓ 风扇硬件初始化完成");
    Serial.printf("   - PWM引脚: GPIO%d (频率: %dHz)\n", FAN_PWM_PIN, PWM_FREQUENCY);
    Serial.printf("   - 方向引脚: GPIO%d\n", FAN_DIR_PIN);
    Serial.printf("   - 摆动引脚: GPIO%d\n", SWING_SERVO_PIN);
  }
  
  // 初始化摆动舵机
  void initSwingServo() {
    // 使用PWM配置舵机 (新的ESP32 PWM API)
    ledcAttach(SWING_SERVO_PIN, 50, 16);  // 50Hz, 16位分辨率用于舵机控制
    
    // 设置舵机到中间位置
    setServoPosition(90);
  }
  
  // 设置舵机位置 (0-180度)
  void setServoPosition(int angle) {
    // 舵机PWM计算: 0.5ms-2.5ms 对应 0-180度
    // 50Hz = 20ms周期, 16位分辨率 = 65536
    int pulseWidth = map(angle, 0, 180, 1638, 8192);  // 0.5ms-2.5ms
    ledcWrite(SWING_SERVO_PIN, pulseWidth);
  }
  
  // HomeKit更新处理函数
  boolean update() override {
    bool hasChanged = false;
    
    // 检查Active状态变化
    if (active.updated()) {
      isActive = active.getNewVal();
      hasChanged = true;
      Serial.printf("🔄 风扇开关: %s\n", isActive ? "开启" : "关闭");
    }
    
    // 检查目标风扇状态变化
    if (targetFanState.updated()) {
      isManualMode = (targetFanState.getNewVal() == 0);  // 0=MANUAL, 1=AUTO
      hasChanged = true;
      Serial.printf("🎛️  风扇模式: %s\n", isManualMode ? "手动" : "自动");
    }
    
    // 检查旋转方向变化
    if (rotationDirection.updated()) {
      isClockwise = (rotationDirection.getNewVal() == 0);  // 0=CLOCKWISE, 1=COUNTERCLOCKWISE
      hasChanged = true;
      Serial.printf("🔄 旋转方向: %s\n", isClockwise ? "顺时针" : "逆时针");
    }
    
    // 检查速度变化
    if (rotationSpeed.updated()) {
      int newSpeed = rotationSpeed.getNewVal();
      if (newSpeed != currentSpeed) {
        currentSpeed = newSpeed;
        targetSpeedValue = newSpeed;
        speedChanges++;
        hasChanged = true;
        Serial.printf("⚡ 风扇速度: %d%%\n", currentSpeed);
      }
    }
    
    // 检查摆动模式变化
    if (swingMode.updated()) {
      isSwinging = (swingMode.getNewVal() == 1);  // 0=DISABLED, 1=ENABLED
      hasChanged = true;
      Serial.printf("↔️  摆动模式: %s\n", isSwinging ? "开启" : "关闭");
    }
    
    // 检查物理控制锁定变化
    if (lockPhysicalControls.updated()) {
      isPhysicalLocked = (lockPhysicalControls.getNewVal() == 1);
      hasChanged = true;
      Serial.printf("🔒 物理控制: %s\n", isPhysicalLocked ? "锁定" : "解锁");
    }
    
    // 如果有变化，更新硬件
    if (hasChanged) {
      updateFanHardware();
      updateFanState();
    }
    
    return true;  // 返回true表示更新成功
  }
  
  // 主循环处理函数
  void loop() override {
    uint32_t currentTime = millis();
    
    // 处理摆动功能
    if (isSwinging && currentTime - lastSwingUpdate >= 50) {  // 50ms更新一次
      updateSwingPosition();
      lastSwingUpdate = currentTime;
    }
    
    // 检查手动按钮 (如果未锁定)
    if (!isPhysicalLocked && currentTime - lastButtonCheck >= 100) {
      checkManualButton();
      lastButtonCheck = currentTime;
    }
    
    // 检查旋转编码器 (如果未锁定)
    if (!isPhysicalLocked) {
      checkRotaryEncoder();
    }
    
    // 定期心跳报告
    if (currentTime - lastHeartbeat >= 30000) {  // 30秒
      sendHeartbeat();
      lastHeartbeat = currentTime;
    }
    
    // 更新LED状态指示
    updateStatusLED();
    
    // 更新运行时间统计
    if (isActive && currentSpeed > 0) {
      totalRunTime = currentTime;
    }
  }
  
  // 更新风扇硬件状态
  void updateFanHardware() {
    // 设置风扇方向
    digitalWrite(FAN_DIR_PIN, isClockwise ? HIGH : LOW);
    
    // 设置风扇速度
    if (isActive && currentSpeed >= MIN_SPEED_THRESHOLD) {
      // 将0-100%映射到PWM值 (考虑最小启动速度)
      int pwmValue = map(currentSpeed, MIN_SPEED_THRESHOLD, 100, 64, 255);
      ledcWrite(FAN_PWM_PIN, pwmValue);
    } else {
      // 停止风扇
      ledcWrite(FAN_PWM_PIN, 0);
    }
    
    Serial.printf("🔧 硬件更新 - 速度: %d%%, 方向: %s, PWM: %d\n", 
                 currentSpeed, 
                 isClockwise ? "顺时针" : "逆时针",
                 isActive && currentSpeed >= MIN_SPEED_THRESHOLD ? map(currentSpeed, MIN_SPEED_THRESHOLD, 100, 64, 255) : 0);
  }
  
  // 更新风扇状态特性
  void updateFanState() {
    // 更新当前风扇状态
    uint8_t newState;
    if (!isActive || currentSpeed == 0) {
      newState = 0;  // INACTIVE
    } else if (currentSpeed < MIN_SPEED_THRESHOLD) {
      newState = 1;  // IDLE
    } else {
      newState = 2;  // BLOWING
    }
    
    if (currentFanState.getVal() != newState) {
      currentFanState.setVal(newState);
      
      String stateText[] = {"停止", "待机", "运行"};
      Serial.printf("📊 风扇状态: %s\n", stateText[newState].c_str());
    }
  }
  
  // 处理摆动位置更新
  void updateSwingPosition() {
    // 更新摆动位置
    swingPosition += (swingDirection * 2);  // 每次移动2度
    
    // 检查边界并反向
    if (swingPosition >= (90 + SWING_RANGE/2)) {
      swingPosition = 90 + SWING_RANGE/2;
      swingDirection = -1;
    } else if (swingPosition <= (90 - SWING_RANGE/2)) {
      swingPosition = 90 - SWING_RANGE/2;
      swingDirection = 1;
    }
    
    // 设置舵机位置
    setServoPosition(swingPosition);
  }
  
  // 检查手动按钮
  void checkManualButton() {
    static bool lastButtonState = true;
    static uint32_t buttonPressTime = 0;
    
    bool buttonState = digitalRead(BUTTON_PIN);
    
    // 检测按钮按下
    if (!buttonState && lastButtonState) {
      buttonPressTime = millis();
    }
    
    // 检测按钮释放
    if (buttonState && !lastButtonState) {
      uint32_t pressDuration = millis() - buttonPressTime;
      
      if (pressDuration < 1000) {
        // 短按：切换开关状态
        toggleFanPower();
      } else {
        // 长按：切换摆动模式
        toggleSwingMode();
      }
    }
    
    lastButtonState = buttonState;
  }
  
  // 检查旋转编码器
  void checkRotaryEncoder() {
    static bool lastCLK = true;
    static bool lastDT = true;
    
    bool currentCLK = digitalRead(ENCODER_CLK_PIN);
    bool currentDT = digitalRead(ENCODER_DT_PIN);
    
    // 检测编码器旋转
    if (currentCLK != lastCLK && !currentCLK) {  // CLK下降沿
      if (currentDT != currentCLK) {
        // 顺时针旋转 - 增加速度
        adjustFanSpeed(2);
      } else {
        // 逆时针旋转 - 减少速度
        adjustFanSpeed(-2);
      }
    }
    
    lastCLK = currentCLK;
    lastDT = currentDT;
  }
  
  // 切换风扇电源
  void toggleFanPower() {
    isActive = !isActive;
    active.setVal(isActive ? 1 : 0);
    updateFanHardware();
    updateFanState();
    
    Serial.printf("🔘 手动切换: 风扇%s\n", isActive ? "开启" : "关闭");
  }
  
  // 切换摆动模式
  void toggleSwingMode() {
    isSwinging = !isSwinging;
    swingMode.setVal(isSwinging ? 1 : 0);
    
    if (!isSwinging) {
      // 停止摆动时回到中间位置
      swingPosition = 90;
      setServoPosition(swingPosition);
    }
    
    Serial.printf("↔️  手动切换: 摆动%s\n", isSwinging ? "开启" : "关闭");
  }
  
  // 调整风扇速度
  void adjustFanSpeed(int delta) {
    int newSpeed = currentSpeed + delta;
    newSpeed = constrain(newSpeed, 0, 100);
    
    if (newSpeed != currentSpeed) {
      currentSpeed = newSpeed;
      targetSpeedValue = newSpeed;
      rotationSpeed.setVal(currentSpeed);
      speedChanges++;
      
      // 自动开关风扇逻辑
      if (newSpeed == 0 && isActive) {
        // 速度降到0%时自动关闭风扇
        isActive = false;
        active.setVal(0);
        Serial.printf("🔘 自动关闭: 速度降至0%%\n");
      } else if (newSpeed > 0 && !isActive) {
        // 速度大于0%且风扇关闭时自动开启风扇
        isActive = true;
        active.setVal(1);
        Serial.printf("🔘 自动开启: 速度调至%d%%\n", newSpeed);
      }
      
      updateFanHardware();
      updateFanState();
      
      Serial.printf("🎚️  手动调速: %d%%\n", currentSpeed);
    }
  }
  
  // 更新LED状态指示
  void updateStatusLED() {
    uint32_t currentTime = millis();
    
    if (!isActive) {
      // 风扇关闭 - LED熄灭
      digitalWrite(LED_PIN, LOW);
    } else if (currentSpeed == 0) {
      // 风扇开启但速度为0 - LED慢闪
      digitalWrite(LED_PIN, (currentTime / 1000) % 2);
    } else if (isSwinging) {
      // 摆动模式 - LED快闪
      digitalWrite(LED_PIN, (currentTime / 200) % 2);
    } else {
      // 正常运行 - LED常亮
      digitalWrite(LED_PIN, HIGH);
    }
  }
  
  // 发送心跳信号
  void sendHeartbeat() {
    uint32_t uptimeSeconds = totalRunTime / 1000;
    
    Serial.printf("💓 风扇心跳 - 状态: %s | 速度: %d%% | 模式: %s | 运行: %ds | 调速: %d次\n",
                 isActive ? "运行" : "停止",
                 currentSpeed,
                 isManualMode ? "手动" : "自动",
                 uptimeSeconds,
                 speedChanges);
    
    // 报告详细状态
    if (isActive) {
      Serial.printf("   🌀 方向: %s | 摆动: %s | 锁定: %s\n",
                   isClockwise ? "顺时针" : "逆时针",
                   isSwinging ? "开启" : "关闭",
                   isPhysicalLocked ? "是" : "否");
    }
  }
  
  // 获取风扇状态描述
  String getFanStatusDescription() {
    if (!isActive) {
      return "风扇已关闭";
    } else if (currentSpeed == 0) {
      return "风扇已开启但速度为0";
    } else if (currentSpeed < MIN_SPEED_THRESHOLD) {
      return "风扇转速过低，待机中";
    } else {
      return String("风扇运行中 - ") + currentSpeed + "% 速度";
    }
  }
  
  // 执行自检程序
  void performSelfTest() {
    Serial.println("🔧 开始风扇自检...");
    
    // 测试正向旋转
    Serial.println("   测试正向旋转...");
    digitalWrite(FAN_DIR_PIN, HIGH);
    ledcWrite(FAN_PWM_PIN, 128);
    delay(2000);
    
    // 测试反向旋转
    Serial.println("   测试反向旋转...");
    digitalWrite(FAN_DIR_PIN, LOW);
    delay(2000);
    
    // 测试摆动功能
    Serial.println("   测试摆动功能...");
    for (int i = 0; i <= 180; i += 10) {
      setServoPosition(i);
      delay(100);
    }
    
    // 回到初始状态
    ledcWrite(FAN_PWM_PIN, 0);
    setServoPosition(90);
    digitalWrite(FAN_DIR_PIN, HIGH);
    
    Serial.println("✅ 风扇自检完成");
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("\n🌀 HomeKit 智能风扇启动中...");
  
  // 初始化HomeSpan
  homeSpan.begin(Category::Fans, "HomeKit智能风扇");
  homeSpan.enableAutoStartAP();
  
  // 创建配件
  new SpanAccessory();
  
  // 添加必需的配件信息服务
  new Service::AccessoryInformation();
  new Characteristic::Name("智能风扇");
  new Characteristic::Manufacturer("XcuiTech Inc.");
  new Characteristic::SerialNumber("FAN-001");
  new Characteristic::Model("SmartFan-Pro");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();
  
  // 添加智能风扇服务
  new DEV_SmartFan();
  
}

void loop() {
  homeSpan.poll();
}