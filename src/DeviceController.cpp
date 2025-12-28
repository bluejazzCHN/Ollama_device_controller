#include "DeviceController.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

DeviceController::DeviceController() {
  initializeDefaultDevices();
  initializeCommandHandlers();
}

void DeviceController::initializeDefaultDevices() {
  // 默认设备配置
  m_devices = {{"living_room_light", {"客厅灯", false, 50, 0, "#FFFFFF"}},
               {"bedroom_light", {"卧室灯", false, 30, 0, "#FFE4C4"}},
               {"kitchen_light", {"厨房灯", false, 70, 0, "#FFFFFF"}},
               {"air_conditioner", {"空调", false, 0, 24, ""}},
               {"heater", {"暖气", false, 0, 0, ""}},
               {"fan", {"风扇", false, 0, 0, ""}}};
}

void DeviceController::initializeCommandHandlers() {
  m_commandHandlers["turn_on"] = [this](const auto &p) {
    return handleTurnOn(p);
  };
  m_commandHandlers["turn_off"] = [this](const auto &p) {
    return handleTurnOff(p);
  };
  m_commandHandlers["set_brightness"] = [this](const auto &p) {
    return handleSetBrightness(p);
  };
  m_commandHandlers["set_temperature"] = [this](const auto &p) {
    return handleSetTemperature(p);
  };
  m_commandHandlers["set_color"] = [this](const auto &p) {
    return handleSetColor(p);
  };
}

void DeviceController::logAction(const std::string &deviceId,
                                 const std::string &action,
                                 const std::string &value) {
  std::time_t now = std::time(nullptr);
  std::tm *localTime = std::localtime(&now);

  std::ofstream logFile("home_automation.log", std::ios::app);
  if (logFile.is_open()) {
    logFile << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << " | "
            << "设备: " << deviceId << " | "
            << "动作: " << action;

    if (!value.empty()) {
      logFile << " | 值: " << value;
    }

    logFile << std::endl;
    logFile.close();
  }
}

bool DeviceController::executeCommand(
    const std::map<std::string, std::string> &command) {
  if (command.find("command") == command.end()) {
    std::cerr << "命令格式错误: 缺少'command'字段" << std::endl;
    return false;
  }

  std::string cmd = command.at("command");

  if (m_commandHandlers.find(cmd) == m_commandHandlers.end()) {
    std::cerr << "未知命令: " << cmd << std::endl;
    return false;
  }

  return m_commandHandlers[cmd](command);
}

bool DeviceController::handleTurnOn(
    const std::map<std::string, std::string> &params) {
  if (params.find("device") == params.end())
    return false;

  std::string deviceId = params.at("device");

  // 检查是否控制所有灯
  if (deviceId == "all_lights") {
    for (auto &[id, status] : m_devices) {
      if (id.find("light") != std::string::npos) {
        status.is_on = true;
        simulateGPIOControl(id, "turn_on", 0);
        logAction(id, "turn_on");
      }
    }
    std::cout << "已打开所有灯光" << std::endl;
    return true;
  }

  if (m_devices.find(deviceId) == m_devices.end()) {
    std::cerr << "设备不存在: " << deviceId << std::endl;
    return false;
  }

  m_devices[deviceId].is_on = true;
  simulateGPIOControl(deviceId, "turn_on", 0);
  logAction(deviceId, "turn_on");

  std::cout << "已打开设备: " << m_devices[deviceId].name << std::endl;
  return true;
}

bool DeviceController::handleTurnOff(
    const std::map<std::string, std::string> &params) {
  // 实现类似handleTurnOn...
  return true;
}

bool DeviceController::handleSetBrightness(
    const std::map<std::string, std::string> &params) {
  if (params.find("device") == params.end() ||
      params.find("value") == params.end()) {
    return false;
  }

  std::string deviceId = params.at("device");
  int brightness = std::stoi(params.at("value"));

  if (brightness < 0 || brightness > 100) {
    std::cerr << "亮度值超出范围 (0-100): " << brightness << std::endl;
    return false;
  }

  if (m_devices.find(deviceId) == m_devices.end()) {
    std::cerr << "设备不存在: " << deviceId << std::endl;
    return false;
  }

  m_devices[deviceId].brightness = brightness;
  simulateGPIOControl(deviceId, "set_brightness", brightness);
  logAction(deviceId, "set_brightness", std::to_string(brightness));

  std::cout << m_devices[deviceId].name << " 亮度设置为: " << brightness << "%"
            << std::endl;
  return true;
}

bool DeviceController::handleSetTemperature(
    const std::map<std::string, std::string> &params) {
  // 实现温度设置逻辑...
  return true;
}

bool DeviceController::handleSetColor(
    const std::map<std::string, std::string> &params) {
  // 实现颜色设置逻辑...
  return true;
}

bool DeviceController::simulateGPIOControl(const std::string &deviceId,
                                           const std::string &action,
                                           int value) {
  // 这里模拟GPIO控制
  // 实际使用时替换为真实GPIO库，如wiringPi或libgpiod

  std::cout << "[GPIO模拟] 设备: " << deviceId << ", 动作: " << action
            << ", 值: " << value << std::endl;

  // 示例：使用wiringPi的实际代码
  /*
  #include <wiringPi.h>

  if (action == "turn_on") {
      digitalWrite(PIN_NUM, HIGH);
  } else if (action == "turn_off") {
      digitalWrite(PIN_NUM, LOW);
  } else if (action == "set_brightness") {
      pwmWrite(PIN_NUM, value * 10.23); // 将0-100映射到0-1023
  }
  */

  return true;
}

DeviceStatus
DeviceController::getDeviceStatus(const std::string &deviceId) const {
  auto it = m_devices.find(deviceId);
  if (it != m_devices.end()) {
    return it->second;
  }
  return {"", false, 0, 0, ""};
}

std::vector<std::string> DeviceController::listDevices() const {
  std::vector<std::string> devices;
  for (const auto &[id, status] : m_devices) {
    devices.push_back(id + ": " + status.name);
  }
  return devices;
}
