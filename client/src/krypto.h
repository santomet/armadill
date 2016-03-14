#ifndef KRYPTO_H
#define KRYPTO_H

#include <QObject>

#include "../include/mbedtls/aes.h"
#include "../include/mbedtls/dhm.h"
#include "../include/mbedtls/x509.h"
#include "../include/mbedtls/x509_csr.h"
#include "../include/mbedtls/sha512.h"

class Krypto    : public QObject
{
    Q_OBJECT
    friend class UTest;
public:

    Krypto();

//----------------------CERTs-------------------------------------------------------
    /*!
     * \brief createCert        Creates a short-term private key and certificate request
     *                          for server to sign
     *
     * \param priv              QString in wich base64 private key will be written to
     *                          (QString will be overwritten)
     *
     * \param req               QString in which base64 request will be written to
     *                          (QString will be overwritten)
     *
     * \param rand              Seed for random number generator
     *                          (should be based on user input like in Kleopatra)
     *
     * \param common            Common name for certificate
     */
    bool createCert(QString &priv, QString &req, const int rand, const QString common);

    /*!
     * \brief verifyCert        Verifies signature
     * \param pubToVerify       base64 certificate to verify
     * \param pubCA             base64 certificate which should have signed pubToVerify
     * \return                  True if pubToVerify is properly signed by pubCA
     */
    bool verifyCert(const QString pubToVerify, const QString pubCA);
//---------------------------------------------------------------------

//-----------------------------DIFFIE-HELLMAN---------------------------
    /*!
     * \brief generateDHPublic
     *                          Generates new private and exports public for DH value
     *                          NOTE: The DH context will not be freed after this,
     *                          because we need the secret to compute shared secret afterwards
     *
     * \param output            public DH value will be exported here
     * \return                  true if successful
     */
    bool generateDHPublic(QByteArray &output);

    /*!
     * \brief computeDHSharedSecret
     *                          Computes shared secret Key which can be used for encrypting messages
     *                          NOTE: We need to have initialized DH context with generated secret!
     *
     * \param key               the key will be written here
     * \param theirPub          Public value we get from peer
     * \return                  true if successful
     */
    bool computeDHSharedSecret(const QByteArray &key, const QByteArray theirPub);
//------------------------------------------------------------------------

//---------------------HMAC-------------------------------------------------
    /*!
     * \brief computeHMAC       Computes HMAC (SHA512) for given message and key
     * \param hash              result will be stored here
     * \param key               Key for HMAC
     * \param message           Message to authenticate
     * \return                  true if succesful
     */
    bool computeHMAC(QByteArray &result, const QByteArray &key, const QByteArray &message);

//------------------------------------------------------------------------------

//------------------------------AES---------------------------------------------

    /*!
     * \brief encryptMessage    Encrypts message with AES256
     * \param toEncrypt         Message/Data which are going to be encrypted
     * \param encrypted         This will contain (Initialization Vector || Encrypted data || size of original data)
     * \param key               256b Key which will be used for encryption
     * \param originalsize      Original size of data (You can use size method from QByteArray)
     */
    void encryptMessage(const QByteArray &toEncrypt, QByteArray &encrypted, const unsigned char *key, size_t originalsize);

    /*!
     * \brief decryptMessage    Decrypts message with AES256
     * \param toDecrypt         Data to decrypt (skould be (Initialization Vector || Encrypted data || size of original data))
     * \param decrypted         Pure decrypted data with origianl size
     * \param key               256b Key for decryption
     */
    void decryptMessage(const QByteArray &toDecrypt, QByteArray &decrypted, const unsigned char *key);
//----------------------------------------------------------------------------------------
//-----------------------PASSWORDS--------------------------------------------------------
    /*!
     * \brief saltHash          salt hashes the password
     * \param pass              password
     * \return                  salted hash
     */
    QString saltHash(QString pass);

//--------------------------------PRIVATE--------------------------------------------------
private:
    /*!
     * \brief hash              Calculates SHA512 of any given data
     * \param chunks            data
     * \return                  hash in base64 format
     */
    QString hash(QByteArray &chunks);


//--------------------------------------------------------------------------------------

    mbedtls_dhm_context mDHMContext;

};

#endif // KRYPTO_H
