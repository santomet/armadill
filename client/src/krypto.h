#ifndef KRYPTO_H
#define KRYPTO_H

#include <atomic>
#include <mutex>
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
#define TAG_LENGTH	16
#define IV_LENGTH	12
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

	unsigned char keyid = 0;
	QByteArray oldkey;
	QByteArray currentkey;

	std::atomic<bool> my;
	std::atomic<bool> other;

	std::atomic<size_t> key_enc_uses		= 0;
	std::atomic<size_t> key_dec_uses		= 0;
	std::atomic<size_t> key_old_dec_uses	= 0;

	std::mutex keyUse;
	std::mutex dhmUse;

public:
	SessionKey(mbedtls_entropy_context * ectx) : entropy(ectx), my(false), other(false) {
		mbedtls_dhm_init(&dhmc);
        //TODO set dhm_context here
        uchar out[2048];
        size_t len;

		const char *personalization = "]76kXV-$P?0qdQtfpkTPUSvWcq&(dyub";
		mbedtls_ctr_drbg_init(&random);
		mbedtls_ctr_drbg_seed(&random, mbedtls_entropy_func, entropy, (const unsigned char *)personalization, strlen(personalization));


		mbedtls_mpi_read_string(&dhmc.P, 16, "C5577A1DD79D0ADFE26D012F976778B069180AD0D704C61BBF23435838EE899D700B11A15713B8C09639DABF8DC1EC36F34AF3C2ECEB45BF5E0DA2C1E9DC04CAF0612F5128AA21B0306EDFE311D42999E17B54C7BA0FBDC3B46316D5CA592ED22AB934D21234D2E475F225D1F91AE0C32DDBFA0D468F43399E19E557D6F62B8A1EC66BE73C1F889422576B9CE34F3F02D2A4809D0AE48C46A2B92DBB47C97DC20085EB34BBD20F803E30689CBE4A83D9D6D215DE645CD984763C5B3FE098CEF3914CF0987C17D20749B6E996BA3DFB66340681B3B4369AB746F29E76277DDB93D17F0BC8F409369F9B370C71FDCE2ADCB8B6433A3DED4FAF13B91683D04005B3");
		mbedtls_mpi_read_string(&dhmc.G, 16, "04");

        mbedtls_dhm_make_params(&dhmc, 256, out, &len, mbedtls_ctr_drbg_random, &random);
    };
	SessionKey(const SessionKey &) = delete;
	~SessionKey() {
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
	* \brief conditionalGetDH
	* \return                          If my DH was not generated, returns newly generated diffie-hellman, otherwise returns empty QByteArray
	*/
	QByteArray conditionalGetDH();

	/*!
	* \brief protect
	* \param message					Message, that should be encrypted and signed
	* \param data						Data, that should be signed but not encrypted
	* \return							Payload, to be sent to partner
	*/
	QByteArray protect(const QByteArray & message, const QByteArray & data);

	/*!
	* \brief unprotect
	* \param message					Message, that should be decrypted and verified
	* \param data						Data, that should be verified but not decrypted
	* \return							Decrypted message
	*/
	QByteArray unprotect(const QByteArray & message, const QByteArray & data);

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



	const QByteArray & getSharedKey() const { return currentkey; };
};

#endif // KRYPTO_H
