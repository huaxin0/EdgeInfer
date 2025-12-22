#pragma once
#include <string>
#include <vector>

class Base64 {
public:
    /**
     * @brief 解码base64字符串为字节数组
     * @param encoded base64编码的字符串
     * @return 解码后的字节数组
     * @throws std::runtime_error 如果解码失败
     */
    static std::vector<uint8_t> decode(const std::string& encoded);
    
private:
    static const std::string base64_chars;
    static inline bool is_base64(unsigned char c);
};