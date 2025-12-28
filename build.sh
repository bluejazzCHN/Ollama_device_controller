#!/bin/bash

# 智能家居控制系统编译脚本
echo "编译智能家居控制系统..."

# 创建构建目录
mkdir -p build
cd build

# 运行CMake
cmake ..

# 编译
make -j4

if [ $? -eq 0 ]; then
  echo "编译成功！"
  echo "可执行文件: ./build/smart_home"
  echo ""
  echo "使用说明:"
  echo "1. 确保Ollama服务已启动: sudo systemctl start ollama"
  echo "2. 确保已下载模型: ollama pull qwen2.5:1.5b"
  echo "3. 运行程序: ./build/smart_home"
else
  echo "编译失败！"
  exit 1
fi
