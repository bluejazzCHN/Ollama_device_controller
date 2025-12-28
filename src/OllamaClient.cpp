#include "OllamaClient.h"
#include <curl/curl.h>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <sstream>

// libcurl回调函数，用于接收数据
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

OllamaClient::OllamaClient(const std::string &host, int port)
    : m_host(host), m_port(port), m_timeout(30000) {}

bool OllamaClient::testConnection() {
  CURL *curl = curl_easy_init();
  if (!curl)
    return false;

  std::string url =
      "http://" + m_host + ":" + std::to_string(m_port) + "/api/tags";
  std::string response;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_timeout);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  return res == CURLE_OK;
}

std::string OllamaClient::httpPost(const std::string &endpoint,
                                   const std::string &jsonData) {
  CURL *curl = curl_easy_init();
  if (!curl)
    return "{}";

  std::string url =
      "http://" + m_host + ":" + std::to_string(m_port) + endpoint;
  std::string response;

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_timeout);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    std::cerr << "HTTP请求失败: " << curl_easy_strerror(res) << std::endl;
    response = "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  return response;
}

std::string OllamaClient::generateCommandSchema() {
  // 定义智能家居命令的JSON Schema
  return R"({
        "type": "object",
        "properties": {
            "command": {
                "type": "string",
                "enum": ["turn_on", "turn_off", "set_brightness", "set_temperature", "set_color"]
            },
            "device": {
                "type": "string",
                "enum": ["living_room_light", "bedroom_light", "kitchen_light", 
                        "air_conditioner", "heater", "fan", "all_lights"]
            },
            "value": {
                "type": "integer",
                "minimum": 0,
                "maximum": 100
            },
            "color": {
                "type": "string",
                "pattern": "^#[0-9A-Fa-f]{6}$"
            },
            "temperature": {
                "type": "integer",
                "minimum": 16,
                "maximum": 30
            }
        },
        "required": ["command", "device"]
    })";
}

std::map<std::string, std::string>
OllamaClient::getStructuredResponse(const std::string &model,
                                    const std::string &userMessage,
                                    const std::string &customSchema) {
  // 使用默认schema或自定义schema
  std::string schema =
      customSchema.empty() ? generateCommandSchema() : customSchema;

  // 构建请求JSON
  Json::Value request;
  request["model"] = model;
  request["stream"] = false;

  // 解析schema字符串为JSON对象
  Json::CharReaderBuilder schemaReader;
  std::unique_ptr<Json::CharReader> reader(schemaReader.newCharReader());
  Json::Value schemaJson;
  std::string schemaErrors;

  bool parsingSuccessful =
      reader->parse(schema.c_str(), schema.c_str() + schema.length(),
                    &schemaJson, &schemaErrors);

  if (parsingSuccessful) {
    request["format"] = schemaJson;
  }

  // 设置系统提示词
  Json::Value messages(Json::arrayValue);

  Json::Value systemMsg;
  systemMsg["role"] = "system";
  systemMsg["content"] =
      R"(你是一个智能家居控制系统。请根据用户指令，严格输出指定格式的JSON对象。
输出要求：
1. 只输出JSON，不要有任何额外文本、解释或标记
2. 确保所有值都符合schema定义
3. 如果用户指令模糊，使用合理的默认值)";
  messages.append(systemMsg);

  Json::Value userMsg;
  userMsg["role"] = "user";
  userMsg["content"] = userMessage;
  messages.append(userMsg);

  request["messages"] = messages;

  // 设置模型选项
  Json::Value options;
  options["temperature"] = 0.1; // 低温确保输出稳定
  options["num_predict"] = 100;
  request["options"] = options;

  // 转换为JSON字符串
  Json::StreamWriterBuilder writer;
  std::string requestJson = Json::writeString(writer, request);

  // 发送请求
  std::string response = httpPost("/api/chat", requestJson);

  return parseResponse(response);
}

std::map<std::string, std::string>
OllamaClient::parseResponse(const std::string &jsonResponse) {
  std::map<std::string, std::string> result;

  Json::CharReaderBuilder readerBuilder;
  std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
  Json::Value responseJson;
  std::string errors;

  if (!reader->parse(jsonResponse.c_str(),
                     jsonResponse.c_str() + jsonResponse.length(),
                     &responseJson, &errors)) {
    std::cerr << "JSON解析失败: " << errors << std::endl;
    result["error"] = "解析失败";
    return result;
  }

  // 检查是否有错误
  if (responseJson.isMember("error")) {
    result["error"] = responseJson["error"].asString();
    return result;
  }

  // 提取模型返回的content（这是一个JSON字符串）
  if (responseJson.isMember("message") &&
      responseJson["message"].isMember("content")) {

    std::string content = responseJson["message"]["content"].asString();

    // 解析content中的JSON
    Json::Value contentJson;
    if (reader->parse(content.c_str(), content.c_str() + content.length(),
                      &contentJson, &errors)) {

      // 将JSON对象转换为map
      for (const auto &key : contentJson.getMemberNames()) {
        if (contentJson[key].isString()) {
          result[key] = contentJson[key].asString();
        } else if (contentJson[key].isInt()) {
          result[key] = std::to_string(contentJson[key].asInt());
        } else if (contentJson[key].isDouble()) {
          result[key] = std::to_string(contentJson[key].asDouble());
        } else if (contentJson[key].isBool()) {
          result[key] = contentJson[key].asBool() ? "true" : "false";
        }
      }
    } else {
      result["error"] = "内容解析失败: " + errors;
    }
  } else {
    result["error"] = "响应格式不正确";
  }

  return result;
}
