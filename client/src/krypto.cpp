#include "krypto.h"
#include <QDateTime>

Krypto::Krypto()
{

}

void Krypto::encryptMessage(const QByteArray &toEncrypt, QByteArray &encrypted, const unsigned char *key, size_t originalsize)
{
    unsigned char iv[16];
    qsrand(QDateTime::currentDateTime().toTime_t());
    for(int i = 0; i<16; ++i)
        iv[i] = qrand();
    encrypted.append((const char*)iv, 16);
    unsigned char* out;
    out = (unsigned char*)malloc(toEncrypt.size());
    memset(out, 0, toEncrypt.size());

    mbedtls_aes_context aescon;
    mbedtls_aes_setkey_enc(&aescon, key, 256);
    mbedtls_aes_crypt_cbc(&aescon, MBEDTLS_AES_ENCRYPT, toEncrypt.size(), iv, (const unsigned char*)toEncrypt.constData(), out);
    encrypted.append((const char*)out, toEncrypt.size());
    mbedtls_aes_free(&aescon);
    long unsigned int writesize = originalsize;
    encrypted.append((const char*)&writesize, sizeof(long unsigned int));

    free(out);
}

void Krypto::decryptMessage(const QByteArray &toDecrypt, QByteArray &decrypted, const unsigned char *key)
{
    unsigned char *out;

    long unsigned int *originalsize = (long unsigned int*)(toDecrypt.data()+(toDecrypt.size()-sizeof(long unsigned int)));

    unsigned char *iv;
    iv = (unsigned char*)malloc(16);
    memcpy(iv, (toDecrypt.data()), 16);

    out = (unsigned char*)malloc(toDecrypt.size()-16-sizeof(long unsigned int));
    memset(out, 0, *originalsize);

    mbedtls_aes_context aescon;
    mbedtls_aes_setkey_dec(&aescon, key, 256);
    mbedtls_aes_crypt_cbc(&aescon, MBEDTLS_AES_DECRYPT, (toDecrypt.size()-16-sizeof(long unsigned int)), iv, (const unsigned char*)toDecrypt.data()+16, out);
    mbedtls_aes_free(&aescon);
    decrypted.append((char*)out, *originalsize);
    free(iv);
    free(out);
}

QString Krypto::hash(QByteArray &chunks)
{
    unsigned char digest[64];
    char out[128];
    memset(digest, 0, 64);
    memset(out, 0, 128);
    mbedtls_sha512((const unsigned char*)chunks.constData(),chunks.size(), digest, 0);
    for(int i = 0; i<64; ++i)
    {
        sprintf(out+i*2, "%02x", digest[i]);
    }
    QString qOut(out);
    return qOut;
}

