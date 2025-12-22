#include "utils/base64.h"
#include <stdexcept>

const std::string Base64::base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";   //构建字符映射表

inline bool Base64::is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}   //判断结束标志

//核心思想， 4位字符为一组，按位置找到对应的数字，转化为6bit, 然后8位一组一共
//三组转化为asci字符 ， ret 是最后解码的结果

std::vector<uint8_t> Base64::decode(const std::string& encoded_string) {
    size_t in_len = encoded_string.size();
    size_t i = 0;
    size_t in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<uint8_t> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; 
        in_++;
        //筹够4位
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
                ret.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }
    //最后不够4位补零操作
    if (i) {
        size_t j;
        for (j = 0; j < i; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; j++) {
            ret.push_back(char_array_3[j]);
        }
    }

    return ret;
}