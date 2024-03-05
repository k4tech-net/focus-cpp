#include "crypto.hpp"

std::string Crypto::encrypt(std::string plaintext, std::string key, std::string iv) {
	AES_KEY enc_key;

    std::string decodedkey = base64_decode(key);
    std::string decodediv = base64_decode(iv);

    std::vector<unsigned char> keyByteArray(decodedkey.begin(), decodedkey.end());
    std::vector<unsigned char> ivByteArray(decodediv.begin(), decodediv.end());

    const unsigned char* keyCharPtr = keyByteArray.data();
    unsigned char* ivCharPtr = ivByteArray.data();

    AES_set_encrypt_key(keyCharPtr, 256, &enc_key);

    // Determine padding length
    int paddingLength = AES_BLOCK_SIZE - (plaintext.length() % AES_BLOCK_SIZE);

    // If the plaintext length is already a multiple of the block size,
    // add a full block of padding
    if (paddingLength == 0) {
        paddingLength = AES_BLOCK_SIZE;
    }

    // Append padding bytes
    std::string paddedText = plaintext;
    for (int i = 0; i < paddingLength; ++i) {
        paddedText.push_back((char)paddingLength);
    }

    // Encrypt padded plaintext
    std::string cipherText;
    cipherText.resize(paddedText.length());
    AES_cbc_encrypt((const unsigned char*)paddedText.c_str(), (unsigned char*)&cipherText[0], paddedText.length(), &enc_key, ivCharPtr, AES_ENCRYPT);

    return cipherText;
}

std::string Crypto::decrypt(std::string ciphertext, std::string key, std::string iv) {
    AES_KEY dec_key;

    std::string decodedkey = base64_decode(key);
    std::string decodediv = base64_decode(iv);

    std::vector<unsigned char> keyByteArray(decodedkey.begin(), decodedkey.end());
    std::vector<unsigned char> ivByteArray(decodediv.begin(), decodediv.end());

    const unsigned char* keyCharPtr = keyByteArray.data();
    unsigned char* ivCharPtr = ivByteArray.data();

    AES_set_decrypt_key(keyCharPtr, 256, &dec_key);

    // The ciphertext should be a multiple of the block size for CBC mode
    if (ciphertext.length() % AES_BLOCK_SIZE != 0) {
        throw std::runtime_error(xorstr_("Ciphertext length is not a multiple of the block size"));
    }

    std::string decryptedText;
    decryptedText.resize(ciphertext.length());

    // Decrypt ciphertext
    AES_cbc_encrypt((const unsigned char*)ciphertext.c_str(), (unsigned char*)&decryptedText[0], ciphertext.length(), &dec_key, ivCharPtr, AES_DECRYPT);

    // Remove padding
    unsigned char lastChar = decryptedText[decryptedText.length() - 1];
    if (lastChar > 0 && lastChar <= AES_BLOCK_SIZE) { // Simple PKCS#7 padding validation
        bool paddingValid = true;
        for (size_t i = 0; i < lastChar; i++) {
            if (decryptedText[decryptedText.length() - 1 - i] != lastChar) {
                paddingValid = false;
                break;
            }
        }
        if (paddingValid) {
            decryptedText.resize(decryptedText.length() - lastChar); // Remove padding bytes
        }
        else {
            throw std::runtime_error(xorstr_("Invalid padding on decrypted text"));
        }
    }
    else {
        throw std::runtime_error(xorstr_("Invalid padding on decrypted text"));
    }

    return decryptedText;
}

std::string Crypto::generateIV() {
    std::vector<unsigned char> iv(AES_BLOCK_SIZE);

    // Generate random bytes for the IV
    if (!RAND_bytes(iv.data(), AES_BLOCK_SIZE)) {
        throw std::runtime_error(xorstr_("Failed to generate random IV"));
    }

    // Convert the IV to a std::string
    std::string ivString(iv.begin(), iv.end());

    // Optionally, encode the IV in base64 if you need to transmit it as a string
    return base64_encode(ivString);
}

std::string Crypto::encryptEncode(std::string plaintext, std::string key, std::string iv) {
    return base64_encode(encrypt(plaintext, key, iv));
}

std::string Crypto::decryptDecode(std::string ciphertext, std::string key, std::string iv) {
    return decrypt(base64_decode(ciphertext), key, iv);
}

// Watchdog function
void Crypto::watchdog() {
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto lastAuth = lastAuthTime.load();

        // Check if more than 10 seconds have passed since last authentication
        if (now - lastAuth > std::chrono::seconds(3)) {
            // Authentication timeout - perform cleanup and exit
            exit(0);
            break; // Exit the thread loop
        }

        // Debugger in here too

        // Sleep for a short time before checking again
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}