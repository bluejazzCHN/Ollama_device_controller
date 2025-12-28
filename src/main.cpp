#include "DeviceController.h"
#include "OllamaClient.h"
#include <atomic>
#include <iostream>
#include <signal.h>
#include <string>
#include <thread>

std::atomic<bool> running(true);

void signalHandler(int signum) {
  std::cout << "\n收到中断信号，正在关闭..." << std::endl;
  running = false;
}

void printBanner() {
  std::cout << "\n======================================" << std::endl;
  std::cout << "   树莓派5B 智能家居控制系统" << std::endl;
  std::cout << "   基于Ollama结构化输出" << std::endl;
  std::cout << "======================================\n" << std::endl;
}

void printHelp() {
  std::cout << "\n可用命令:" << std::endl;
  std::cout << "  help              - 显示帮助信息" << std::endl;
  std::cout << "  status            - 查看设备状态" << std::endl;
  std::cout << "  devices           - 列出所有设备" << std::endl;
  std::cout << "  exit              - 退出程序" << std::endl;
  std::cout << "  <自然语言指令>     - 控制设备" << std::endl;
  std::cout << "\n示例指令:" << std::endl;
  std::cout << "  \"打开客厅的灯\"" << std::endl;
  std::cout << "  \"把卧室灯亮度调到50%\"" << std::endl;
  std::cout << "  \"打开空调并设置到25度\"" << std::endl;
  std::cout << "  \"关闭所有灯光\"" << std::endl;
}

int main(int argc, char *argv[]) {
  // 设置信号处理
  signal(SIGINT, signalHandler);

  printBanner();

  // 初始化Ollama客户端
  OllamaClient ollamaClient("localhost", 11434);
  ollamaClient.setTimeout(15000); // 15秒超时

  // 测试连接
  std::cout << "正在连接Ollama服务..." << std::endl;
  if (!ollamaClient.testConnection()) {
    std::cerr << "错误: 无法连接到Ollama服务" << std::endl;
    std::cerr << "请确保Ollama已安装并运行: sudo systemctl start ollama"
              << std::endl;
    return 1;
  }
  std::cout << "✓ Ollama连接成功" << std::endl;

  // 初始化设备控制器
  DeviceController deviceController;

  // 列出可用设备
  std::cout << "\n已注册设备:" << std::endl;
  for (const auto &device : deviceController.listDevices()) {
    std::cout << "  - " << device << std::endl;
  }

  printHelp();

  // 主循环
  while (running) {
    std::cout << "\n>>> ";
    std::string userInput;
    std::getline(std::cin, userInput);

    // 处理内置命令
    if (userInput == "exit" || userInput == "quit") {
      break;
    } else if (userInput == "help") {
      printHelp();
      continue;
    } else if (userInput == "status") {
      std::cout << "\n设备状态:" << std::endl;
      // 显示设备状态...
      continue;
    } else if (userInput == "devices") {
      std::cout << "\n已注册设备:" << std::endl;
      for (const auto &device : deviceController.listDevices()) {
        std::cout << "  - " << device << std::endl;
      }
      continue;
    } else if (userInput.empty()) {
      continue;
    }

    // 处理自然语言指令
    std::cout << "正在处理指令: \"" << userInput << "\"" << std::endl;

    try {
      // 获取结构化响应
      std::cout << "调用AI模型解析指令..." << std::endl;
      auto command =
          ollamaClient.getStructuredResponse("qwen2.5:1.5b", // 使用你下载的模型
                                             userInput);

      // 检查错误
      if (command.find("error") != command.end()) {
        std::cerr << "AI解析失败: " << command["error"] << std::endl;
        continue;
      }

      // 显示解析结果
      std::cout << "\n解析结果:" << std::endl;
      for (const auto &[key, value] : command) {
        std::cout << "  " << key << ": " << value << std::endl;
      }

      // 执行命令
      std::cout << "\n执行控制命令..." << std::endl;
      if (deviceController.executeCommand(command)) {
        std::cout << "✓ 命令执行成功" << std::endl;
      } else {
        std::cerr << "✗ 命令执行失败" << std::endl;
      }

    } catch (const std::exception &e) {
      std::cerr << "发生异常: " << e.what() << std::endl;
    }
  }

  std::cout << "\n感谢使用智能家居控制系统！" << std::endl;
  return 0;
}
