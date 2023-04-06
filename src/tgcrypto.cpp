#include <stdexcept>
#include <vector>

#include <emscripten/bind.h>
#include <emscripten/val.h>

extern "C" {
#include <aes256.h>
#include <cbc256.h>
#include <ctr256.h>
#include <ige256.h>
}

using namespace emscripten;

val ige(std::string data, std::string key, std::string iv, bool encrypt) {
    if (data.length() == 0) {
        throw std::length_error("data must not be empty");
    }
    if (data.length() % 16 != 0) {
        throw std::length_error("data size must be divisible by 16");
    }
    if (key.length() != 32) {
        throw std::length_error("key must be 32 bytes");
    }
    if (iv.length() != 32) {
        throw std::length_error("iv must be 32 bytes");
    }

    uint8_t* out =
        ige256(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(),
               reinterpret_cast<const uint8_t*>(key.c_str()),
               reinterpret_cast<const uint8_t*>(iv.c_str()), encrypt);

    return val(typed_memory_view(data.length(), out));
}

val ige256_encrypt(std::string data, std::string key, std::string iv) {
    return ige(data, key, iv, true);
}

val ige256_decrypt(std::string data, std::string key, std::string iv) {
    return ige(data, key, iv, false);
}

val ctr256_encrypt(std::string data,
                   std::string key,
                   std::string iv,
                   std::string state) {
    if (data.length() == 0) {
        throw std::length_error("data must not be empty");
    }
    if (key.length() != 32) {
        throw std::length_error("key must be 32 bytes");
    }
    if (iv.length() != 16) {
        throw std::length_error("iv must be 16 bytes");
    }
    if (state.length() != 1) {
        throw std::length_error("state must be 1 byte");
    }
    if (state[0] > 15) {
        throw std::range_error("state must be in the range 0..15");
    }

    char* ivp = (char*)malloc(16);
    iv.copy(ivp, 16);

    char* statep = (char*)malloc(1);
    state.copy(statep, 1);

    uint8_t* out = ctr256(
        reinterpret_cast<const uint8_t*>(data.c_str()), data.length(),
        reinterpret_cast<const uint8_t*>(key.c_str()),
        reinterpret_cast<uint8_t*>(ivp), reinterpret_cast<uint8_t*>(statep));
    std::vector<memory_view<uint8_t>> result;

    result.push_back(typed_memory_view(data.length(), out));
    result.push_back(typed_memory_view(16, reinterpret_cast<uint8_t*>(ivp)));
    result.push_back(typed_memory_view(1, reinterpret_cast<uint8_t*>(statep)));

    return val::array(result);
}

val cbc(std::string data, std::string key, std::string iv, bool encrypt) {
    if (data.length() == 0) {
        throw std::length_error("data must not be empty");
    }
    if (data.length() % 16 != 0) {
        throw std::length_error("data size must be divisible by 16");
    }
    if (key.length() != 32) {
        throw std::length_error("key must be 32 bytes");
    }
    if (iv.length() != 16) {
        throw std::length_error("iv must be 16 bytes");
    }

    uint8_t* out =
        cbc256(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(),
               reinterpret_cast<const uint8_t*>(key.c_str()),
               (uint8_t*)(iv.c_str()), encrypt);

    return val(typed_memory_view(data.length(), out));
}

val cbc256_encrypt(std::string data, std::string key, std::string iv) {
    return cbc(data, key, iv, true);
}

val cbc256_decrypt(std::string data, std::string key, std::string iv) {
    return cbc(data, key, iv, false);
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("ige256Encrypt", &ige256_encrypt);
    function("ige256Decrypt", &ige256_decrypt);
    function("ctr256Encrypt", &ctr256_encrypt);
    function("ctr256Decrypt", &ctr256_encrypt);
    function("cbc256Encrypt", &cbc256_encrypt);
    function("cbc256Decrypt", &cbc256_decrypt);
}
