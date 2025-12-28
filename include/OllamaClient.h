#ifndef OLLAMA_CLIENT_H
#define OLLAMA_CLIENT_H

#include <map>
#include <memory>
#include <string>

class OllamaClient {
public:
  OllamaClient(const std::string &host = "localhost", int port = 11434);

  // 发送消息并获取结构化响应
  std::map<std::string, std::string>
  getStructuredResponse(const std::string &model,
                        const std::string &userMessage,
                        const std::string &jsonSchema = "");

  // 测试连接
  bool testConnection();

  // 设置超时（毫秒）
  void setTimeout(int timeout_ms) { m_timeout = timeout_ms; }

private:
  std::string m_host;
  int m_port;
  int m_timeout;

  // 执行HTTP POST请求
  std::string httpPost(const std::string &endpoint,
                       const std::string &jsonData);

  // 解析JSON响应
  std::map<std::string, std::string>
  parseResponse(const std::string &jsonResponse);

  // 生成智能家居命令的JSON Schema
  std::string generateCommandSchema();
};

#endif
