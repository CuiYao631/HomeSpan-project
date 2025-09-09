# HomeKit 安全系统

基于 HomeSpan 的 ESP32 HomeKit 安全系统，支持布防、撤防、报警、传感器检测等功能。

## 功能特性
- HomeKit 安全系统服务（SecuritySystem）
- 支持在家布防、外出布防、撤防、报警
- 门磁/红外传感器触发报警
- LED 状态指示
- 故障与防拆状态支持

## 硬件接线
| 功能         | 引脚      |
| ------------ | --------- |
| LED指示灯     | GPIO2     |
| 门磁/红外传感器 | GPIO0     |

> 可根据实际硬件修改引脚定义

## 使用说明
1. 烧录代码到 ESP32
2. 通过 HomeKit 添加配件，名称为"安全系统"
3. 在 HomeKit App 中切换布防/撤防状态
4. 传感器触发时自动报警（LED指示，状态更新）
5. 撤防后自动解除报警

## HomeKit 状态说明
- 撤防（Disarmed）: 当前状态=3
- 布防在家（Armed Stay）: 当前状态=0
- 布防外出（Armed Away）: 当前状态=1
- 报警（Alarm Triggered）: 当前状态=4

## 故障排查
- 无法报警：检查传感器接线和状态
- 状态不同步：检查 HomeSpan 日志输出
- 防拆/故障：可扩展相关特性

## 参考
- [HomeSpan SecuritySystem 文档](https://github.com/HomeSpan/HomeSpan/blob/master/docs/ServiceList.md#securitysystem-7e)
- [HomeKit 安全系统服务定义](https://developer.apple.com/documentation/homekit/hmservice/securitysystem)

---

如需扩展更多传感器或报警类型，可在 `DEV_SecuritySystem` 类中添加逻辑。