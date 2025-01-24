#ifndef BCRYPT_UTILS_H
#define BCRYPT_UTILS_H

#include <string>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/buffer.h>

#define SALT_SIZE 16
#define HASH_SIZE 64

class BcryptUtils {
public:
    static std::string generateSalt() {
        unsigned char salt[SALT_SIZE];
        if (!RAND_bytes(salt, sizeof(salt))) {
            throw std::runtime_error("Error generating salt: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        }
        return base64Encode(salt, sizeof(salt));
    }

    static std::string generateHash(const std::string& password, const std::string& salt) {
        unsigned char hash[HASH_SIZE];
        std::string decodedSalt = base64Decode(salt);
        if (!PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), password.length(), reinterpret_cast<const unsigned char*>(decodedSalt.c_str()), decodedSalt.length(), 10000, HASH_SIZE, hash)) {
            throw std::runtime_error("Error hashing password: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        }
        return base64Encode(hash, sizeof(hash));
    }

    static bool validatePassword(const std::string& password, const std::string& salt, const std::string& hash) {
        std::string new_hash = generateHash(password, salt);
        return new_hash == hash;
    }

private:
    static std::string base64Encode(const unsigned char* buffer, size_t length) {
        BIO* bio = BIO_new(BIO_s_mem());
        BIO* b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer, length);
        BIO_flush(bio);
        BUF_MEM* bufferPtr;
        BIO_get_mem_ptr(bio, &bufferPtr);
        std::string base64String(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        return base64String;
    }

    static std::string base64Decode(const std::string& encoded) {
        BIO* bio = BIO_new_mem_buf(encoded.data(), static_cast<int>(encoded.size()));
        BIO* b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        std::vector<unsigned char> buffer(encoded.size());
        int length = BIO_read(bio, buffer.data(), static_cast<int>(buffer.size()));
        BIO_free_all(bio);
        return std::string(buffer.begin(), buffer.begin() + length);
    }
};

#endif // BCRYPT_UTILS_H
