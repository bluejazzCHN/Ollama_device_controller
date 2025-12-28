#ifndef DEVICE_CONTROLLER_H
#define DEVICE_CONTROLLER_H

#include <functional>
#include <map>
#include <string>
#include <vector>

// 设备状态结构体
struct DeviceStatus {
  std::string name;
  bool is_on;
  int brightness;    // 0-100
  int temperature;   // 16-30°C
  std::string color; // HEX颜色
};

class DeviceController {
public:
  DeviceController();

  // 执行命令
  bool executeCommand(const std::map<std::string, std::string> &command);

  // 注册设备
  void registerDevice(const std::string &deviceId, const std::string &name,
                      const std::string &type);

  // 获取设备状态
  DeviceStatus getDeviceStatus(const std::string &deviceId) const;

  // 列出所有设备
  std::vector<std::string> listDevices() const;

  // 模拟GPIO控制（实际使用时替换为真实GPIO库）
  bool simulateGPIOControl(const std::string &deviceId,
                           const std::string &action, int value = 0);

private:
  // 设备映射表
  std::map<std::string, DeviceStatus> m_devices;

  // 命令处理器映射
  std::map<std::string,
           std::function<bool(const std::map<std::string, std::string> &)>>
      m_commandHandlers;

  // 初始化默认设备
  void initializeDefaultDevices();

  // 初始化命令处理器
  void initializeCommandHandlers();

  // 具体命令处理方法
  bool handleTurnOn(const std::map<std::string, std::string> &params);
  bool handleTurnOff(const std::map<std::string, std::string> &params);
  bool handleSetBrightness(const std::map<std::string, std::string> &params);
  bool handleSetTemperature(const std::map<std::string, std::string> &params);
  bool handleSetColor(const std::map<std::string, std::string> &params);

  // 日志记录
  void logAction(const std::string &deviceId, const std::string &action,
                 const std::string &value = "");
};
#endif
