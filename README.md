# HomeSpan HomeKit 智能设备项目集合

[![HomeSpan](https://img.shields.io/badge/HomeSpan-v1.8.0-blue.svg)](https://github.com/HomeSpan/HomeSpan)
[![ESP32](https://img.shields.io/badge/ESP32-Compatible-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![HomeKit](https://img.shields.io/badge/HomeKit-Compatible-orange.svg)](https://developer.apple.com/homekit/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

基于 **HomeSpan** 库的 ESP32 HomeKit 智能设备项目集合，包含多种传感器和执行器的完整实现。每个项目都是独立的 HomeKit 配件，可直接与 iPhone/iPad 家庭App集成。

## 🏠 项目概述

本项目集合提供了多种常用的 HomeKit 智能设备实现，所有设备基于 ESP32 开发板，使用 HomeSpan 库实现 HomeKit 协议。每个设备都经过完整测试，可直接部署使用。

### 🔧 核心技术栈
- **开发板**: ESP32 (Wi-Fi + 蓝牙)
- **HomeKit库**: HomeSpan v1.8.0+
- **开发环境**: Arduino IDE
- **编程语言**: C++
- **协议**: HomeKit Accessory Protocol (HAP)

## 📦 包含的项目

### 1. 🚶 运动传感器 (HomeKit-MotionSensor)
- **功能**: PIR红外运动检测
- **特性**: 运动状态检测、电池电量显示
- **应用**: 走廊自动灯光、安防监控

### 2. 👥 占用传感器 (HomeKit-OccupancySensor)
- **功能**: 房间占用状态检测
- **特性**: 占用检测、高级传感器算法
- **应用**: 智能空调控制、节能管理

### 3. 🌬️ 空气质量传感器 (HomeKit-AirQualitySensor)
- **功能**: 多参数空气质量检测
- **特性**: PM2.5、温湿度、空气质量等级
- **应用**: 空气净化器联动、健康监控

### 4. 💨 二氧化碳传感器 (HomeKit-CarbonDioxideSensor)
- **功能**: CO2浓度检测
- **特性**: 实时CO2监测、报警提醒
- **应用**: 通风系统控制、室内空气管理

### 5. 🚪 接触传感器 (HomeKit-ContactSensor)
- **功能**: 门窗开关状态检测
- **特性**: 磁性接触检测、低功耗设计
- **应用**: 门窗监控、安防系统联动

### 6. 💨 智能风扇 (HomeKit-Fan)
- **功能**: 全功能智能风扇控制
- **特性**: 速度调节、方向控制、摆动功能、物理控制
- **应用**: 客厅吊扇、台式风扇、工业通风

### 7. 🔒 安全系统 (HomeKit-SecuritySystem)
- **功能**: 完整的家庭安防系统
- **特性**: 布防/撤防、夜间模式、入侵检测、警报通知
- **应用**: 家庭安防、入侵报警、智能联动

### 8. 🔌 智能插座 (HomeKit-Outlet)
- **功能**: 4路智能插座控制
- **特性**: 独立开关控制、全开/全关按钮、状态指示、继电器控制
- **应用**: 智能电源管理、多设备控制、定时开关

### 9. 💡 智能灯 (HomeKit-Light)
- **功能**: 基础智能灯光控制
- **特性**: 开关控制、物理按钮、LED状态指示、低电平触发
- **应用**: 家庭照明、远程控制、自动化场景

## 🚀 快速开始

### 环境准备
1. **安装 Arduino IDE**
   ```bash
   # 下载并安装 Arduino IDE 2.0+
   # 添加 ESP32 开发板支持
   ```

2. **安装 HomeSpan 库**
   ```bash
   # 在 Arduino IDE 中:
   # 工具 → 管理库 → 搜索 "HomeSpan" → 安装
   ```

3. **硬件准备**
   - ESP32 开发板 (推荐 ESP32-WROOM-32)
   - 面包板和跳线
   - 各项目对应的传感器/执行器

### 使用步骤
1. **选择项目** - 从上述项目中选择需要的设备
2. **硬件连接** - 根据各项目的README进行接线
3. **代码烧录** - 使用Arduino IDE烧录对应的.ino文件
4. **HomeKit配对** - 使用iPhone家庭App扫码添加设备
5. **功能测试** - 验证设备功能和HomeKit集成


## ⚙️ 系统要求

### 硬件要求
- **ESP32开发板** (推荐型号: ESP32-WROOM-32)
- **电源供应**: 5V/1A (USB供电或外部电源)
- **传感器/执行器**: 根据具体项目需求

### 软件要求
- **Arduino IDE**: 2.0.0 或更高版本
- **ESP32 Board Package**: 2.0.0+
- **HomeSpan库**: 1.8.0+
- **依赖库**: 各项目特定的传感器库

### 移动设备要求
- **iOS**: 13.0+ (支持HomeKit)
- **设备**: iPhone、iPad、Apple TV、HomePod (作为家庭中枢)

## 🔧 高级功能

### 🏠 HomeKit 集成特性
- **Siri语音控制**: "嘿Siri，打开客厅风扇"
- **自动化场景**: 基于时间、位置、传感器状态的智能联动
- **远程控制**: 通过家庭中枢实现远程访问
- **多用户共享**: 支持家庭成员共同控制

### 🔄 设备联动示例
```
运动传感器检测到人 → 自动开启智能灯 → 启动空气净化器
CO2浓度过高 → 自动开窗 → 启动新风系统
安全系统报警 → 开启所有灯光 → 发送推送通知
离家场景触发 → 智能插座全部关闭 → 关闭智能灯 → 安全系统自动布防
夜间模式触发 → 智能灯调至夜灯模式 → 关闭娱乐设备插座
```

### 📊 监控和调试
- **串口监控**: 实时查看设备状态和调试信息
- **HomeKit状态**: 通过家庭App查看所有设备状态
- **日志记录**: 详细的运行日志和错误信息

## 🛠️ 故障排查

### 常见问题

**Q: 设备无法被HomeKit发现？**
A: 检查Wi-Fi连接，确保ESP32和iPhone在同一网络，重启设备后重新扫描。

**Q: 传感器读数异常？**  
A: 检查硬件连接，确认传感器型号匹配，查看串口调试信息。

**Q: HomeKit控制无响应？**
A: 检查家庭中枢设备状态，确认网络连接稳定，尝试重启家庭App。

**Q: 设备频繁掉线？**
A: 检查Wi-Fi信号强度，确认电源供电稳定，更新ESP32固件。

### 调试技巧
1. **使用串口监控器** - 查看详细的运行日志
2. **检查HomeSpan状态** - 观察配对和连接过程
3. **逐步测试** - 先测试基础功能，再测试HomeKit集成
4. **网络诊断** - 确保路由器支持mDNS和组播

## 🤝 贡献指南

欢迎为本项目贡献代码！请遵循以下步骤：

1. **Fork 本仓库**
2. **创建功能分支**: `git checkout -b feature/new-sensor`
3. **提交更改**: `git commit -am 'Add new sensor support'`
4. **推送分支**: `git push origin feature/new-sensor`
5. **提交Pull Request**

### 代码规范
- 使用清晰的变量和函数命名
- 添加详细的注释说明
- 遵循Arduino编程规范
- 提供完整的使用文档

## 📜 许可证

本项目采用 [MIT许可证](LICENSE)，您可以自由使用、修改和分发。

## 🔗 相关链接

- [HomeSpan官方文档](https://github.com/HomeSpan/HomeSpan)
- [ESP32官方文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [HomeKit开发者文档](https://developer.apple.com/homekit/)
- [Arduino ESP32指南](https://github.com/espressif/arduino-esp32)

## 📞 联系方式

- **作者**: XcuiTech Inc.
- **邮箱**: cuiyao07@gmail.com
- **项目主页**: [GitHub](https://github.com/CuiYao631/HomeSpan-project)
- **问题反馈**: [Issues](https://github.com/CuiYao631/HomeSpan-project/issues)

---

⭐ 如果这个项目对您有帮助，请给我们一个Star！

🔔 欢迎Watch本项目以获取最新更新！