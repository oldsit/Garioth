#ifndef BCRYPT_UTILS_H
#define BCRYPT_UTILS_H

#include <string>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/err.h>

#define SALT_SIZE 16
#define HASH_SIZE 64

class BcryptUtils {
public:
    static std::string generateSalt() {
        unsigned char salt[SALT_SIZE];
        if (!RAND_bytes(salt, sizeof(salt))) {
            throw std::runtime_error("Error generating salt: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        }
        return std::string(reinterpret_cast<char*>(salt), sizeof(salt));
    }

    static std::string generateHash(const std::string& password, const std::string& salt) {
        unsigned char hash[HASH_SIZE];
        if (!PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), password.length(), reinterpret_cast<const unsigned char*>(salt.c_str()), salt.length(), 10000, HASH_SIZE, hash)) {
            throw std::runtime_error("Error hashing password: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        }
        return std::string(reinterpret_cast<char*>(hash), sizeof(hash));
    }

    static bool validatePassword(const std::string& password, const std::string& salt, const std::string& hash) {
        std::string new_hash = generateHash(password, salt);
        return new_hash == hash;
    }
};

#endif // BCRYPT_UTILS_H
