#ifndef KRYPTO_H
#define KRYPTO_H

#include <QObject>

#include "../include/mbedtls/aes.h"
#include "../include/mbedtls/dhm.h"
#include "../include/mbedtls/x509.h"
#include "../include/mbedtls/x509_csr.h"
#include "../include/mbedtls/sha512.h"
#include "../include/mbedtls/gcm.h"
#include "../include/mbedtls/asn1.h"
#include "../include/mbedtls/entropy.h"
#include "../include/mbedtls/ctr_drbg.h"


#define ENCRYPTION_KEY_SIZE 256
#define TAG_LENGTH	128
#define MAX_MESSAGES_WITH_ONE_KEY 10


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
    //QString saltHash(QString pass);

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



class KryptoException : public std::runtime_error {
public:
	KryptoException(const char * msg) : std::runtime_error(msg) {};
};

class KryptoOveruseException : public KryptoException {
public:
	KryptoOveruseException(const char * msg) : KryptoException(msg) {};
};

class SessionKey {
	mbedtls_entropy_context* entropy;
	mbedtls_ctr_drbg_context random;
	mbedtls_dhm_context dhmc;
	mbedtls_gcm_context gcmc;

	unsigned char keyid = 0;
	QByteArray oldkey;
	QByteArray currentkey;

	bool my;
	bool other;

	size_t key_enc_uses = 0;
	size_t key_dec_uses = 0;

public:
	SessionKey(mbedtls_entropy_context * ectx) : entropy(ectx), my(false), other(false) {
		mbedtls_gcm_init(&gcmc);
		mbedtls_dhm_init(&dhmc);

		const char *personalization = "]76kXV-$P?0qdQtfpkTPUSvWcq&(dyub";
		mbedtls_ctr_drbg_init(&random);
		mbedtls_ctr_drbg_seed(&random, mbedtls_entropy_func, &entropy, (const unsigned char *)personalization, strlen(personalization));
	};
	SessionKey(const SessionKey &) = delete;
	~SessionKey() {
		mbedtls_gcm_free(&gcmc);
		mbedtls_dhm_free(&dhmc);
		mbedtls_ctr_drbg_free(&random);
	};

	SessionKey & operator=(const SessionKey &) = delete;

	/*!
	* \brief setDH
	* \param dh						   Diffie Hellman retrieved from partner
	* \return                          void
	*/
	void setDH(QByteArray dh);

	/*!
	* \brief getDH
	* \return                          New generated diffie-hellman
	*/
	QByteArray getDH();

	/*!
	* \brief encrypt
	* \param message					Message, that should be encrypted and signed
	* \param data						Data, that should be signed but not encrypted
	* \return							Payload, to be sent to partner
	*/
	QByteArray encrypt(const QByteArray & message, const QByteArray & data);

	/*!
	* \brief encrypt
	* \param message					Message, that should be decrypted and verified
	* \param data						Data, that should be verified but not decrypted
	* \return							Decrypted message
	*/
	QByteArray decrypt(const QByteArray & message, const QByteArray & data);

	/*!
	* \brief getKeyEncUses
	* \return							Number of times the current key was used to encrypt a message
	*/
	size_t getKeyEncUses() const { return key_enc_uses; };

	/*!
	* \brief getKeyDecUses
	* \return							Number of times the current key was used to decrypt a message
	*/
	size_t getKeyDecUses() const { return key_dec_uses; };

	/*!
	* \brief isMyDHCreated
	* \return                          true, if client diffie-hellman was already created for the next key, false otherwise
	*/
	bool isMyDHCreated() const { return my; };

	/*!
	* \brief isMyDHCreated
	* \return                          true, if partner diffie-hellman was already recieved for the next key, false otherwise
	*/
	bool isOtherDHRecieved() const { return other; };

	/*!
	* \brief generateKey				Generates new key
	* \return							true if successfull, false if missing diffie-helman part
	*/
	bool generateKey();
};

#endif // KRYPTO_H
