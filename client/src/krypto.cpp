#include "krypto.h"
#include <QDateTime>
#include <QDebug>




Krypto::Krypto()
{

}

void Krypto::createCert(QByteArray &priv, QByteArray &request, const QString common) {
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_pk_context pk; //rsa key pair
	mbedtls_x509write_csr req;
	unsigned char output[4096];
	mbedtls_pk_init(&pk);
	mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));

	//generate RSA pair
	const char *pers = "rsa_genkey";
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,(const unsigned char *)pers,strlen(pers))
		!= 0)
	{
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		throw new KryptoException("Generating entropy for RSA pair failed.");
	}

	
	if (mbedtls_rsa_gen_key(mbedtls_pk_rsa(pk), mbedtls_ctr_drbg_random, &ctr_drbg, RSA_SIZE, RSA_EXPONENT)
		!= 0)
	{
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
		throw new KryptoException("Generating RSA pair failed.");
	}

	//generate cert request

	mbedtls_x509write_csr_init(&req);
	mbedtls_x509write_csr_set_md_alg(&req, MBEDTLS_MD_SHA256);
	
	mbedtls_x509write_csr_set_subject_name(&req, common.toStdString().c_str());
	mbedtls_x509write_csr_set_key(&req, &pk);

	//exporting key and cert req

	memset(output, 0, 4096);
	mbedtls_x509write_csr_pem(&req, output, 4096, mbedtls_ctr_drbg_random, &ctr_drbg);
	request = QByteArray(reinterpret_cast<const char *>(output), strlen(reinterpret_cast<const char *>(output)));
	memset(output, 0, 4096);
	mbedtls_pk_write_key_pem(&pk, output, 4096);
	priv = QByteArray(reinterpret_cast<const char *>(output), strlen(reinterpret_cast<const char *>(output)));

	//clean
	mbedtls_pk_free(&pk);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
}

SessionKey::HelperMpi SessionKey::helper;

void SessionKey::setDH(QByteArray dh) {
    std::lock_guard<std::mutex> dhmLock(dhmUse);
    if (mbedtls_dhm_read_public(&dhmc, toUChar(dh), dh.length())) throw KryptoException("setDH: can't read DH");
    other = true;
}

QByteArray SessionKey::getDH() {
    QByteArray ret;

    std::lock_guard<std::mutex> dhmLock(dhmUse);
    ret.resize(dhmc.len);
    if (mbedtls_dhm_make_public(&dhmc, DH_SIZE, reinterpret_cast<uchar *>(ret.data()), dhmc.len, mbedtls_ctr_drbg_random, &random)) throw KryptoException("getDH: can't generate public dh.");
    my = true;
    return ret;
}

QByteArray SessionKey::conditionalGetDH() {
    QByteArray ret;

    std::lock_guard<std::mutex> dhmLock(dhmUse);
    if (my) return ret;
    ret.resize(dhmc.len);
    if (mbedtls_dhm_make_public(&dhmc, DH_SIZE, reinterpret_cast<uchar *>(ret.data()), dhmc.len, mbedtls_ctr_drbg_random, &random)) throw KryptoException("getDH: can't generate public dh.");
    my = true;
    return ret;
}

QByteArray SessionKey::protect(const QByteArray & message, const QByteArray & data) {
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

	std::pair<int, std::unique_ptr<QByteArray>> key;
    {
        std::lock_guard<std::mutex> keyLock(keyUse);
		key = getSenderKey();
		qDebug() << "s" << (unsigned int)key.first << ": " << key.second->toBase64() << "=" << message.toBase64() << data.toBase64();
        mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, toUChar(key.second->data()), ENCRYPTION_KEY_BITS);
    }

    unsigned char iv[IV_LENGTH], tag[TAG_LENGTH];
    mbedtls_ctr_drbg_random(&random, iv, IV_LENGTH);

    QByteArray ret(1, key.first);
    ret.resize(1 + IV_LENGTH + TAG_LENGTH + message.length()); // 1 for keyid + IV + tag + message
    ret.replace(1, IV_LENGTH, reinterpret_cast<const char *>(iv), IV_LENGTH);

	int ret_success;
	if ((ret_success = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, message.length(), iv, IV_LENGTH, toUChar(data), data.length(), toUChar(message), reinterpret_cast<uchar *>(ret.data() + 1 + IV_LENGTH + TAG_LENGTH), TAG_LENGTH, tag))) {
		qDebug() << ret_success;
		mbedtls_gcm_free(&ctx);
		throw KryptoException("Failed to encrypt.");
	}

    ret.replace(1 + IV_LENGTH, TAG_LENGTH, reinterpret_cast<const char *>(tag), TAG_LENGTH);
    mbedtls_gcm_free(&ctx);
	qDebug() << ret.toBase64();
    return ret;
}

QByteArray SessionKey::unprotect(const QByteArray & message, const QByteArray & data) {
    //message = keyId(1)/iv(16)/tag(TAG_LENGTH)/encData

    unsigned char messageKeyId = message[0];
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

	std::unique_ptr<QByteArray> key;
    {
        std::lock_guard<std::mutex> keyLock(keyUse);
		key = getReceiverKey(messageKeyId);
		qDebug() << "r" << (unsigned int)messageKeyId << ": " << key->toBase64() << "=" << message.toBase64() << data.toBase64();
        mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, toUChar(key->data()), ENCRYPTION_KEY_BITS);
    }

    size_t dataLength = message.length() - IV_LENGTH - TAG_LENGTH - 1; // iv + tag + keyId
    unsigned char * output = new unsigned char[dataLength];

    int ret_success;
    if ((ret_success = mbedtls_gcm_auth_decrypt(&ctx, dataLength, toUChar(message) + 1, IV_LENGTH, toUChar(data), data.length(), toUChar(message) + 1 + IV_LENGTH, TAG_LENGTH, toUChar(message) + 1 + IV_LENGTH +TAG_LENGTH, output))) {
		if (ret_success == MBEDTLS_ERR_GCM_AUTH_FAILED)	{
			mbedtls_gcm_free(&ctx);
			throw KryptoException("Authentication of message failed");
		}
		else if (ret_success == MBEDTLS_ERR_GCM_BAD_INPUT) {
			mbedtls_gcm_free(&ctx);
			throw KryptoException("Bad message data for unprotect");
		}
		else {
			mbedtls_gcm_free(&ctx);
			throw KryptoException("Unknown Error");
		}
    }

    QByteArray ret(reinterpret_cast<const char *> (output), dataLength);
    delete[] output;
    mbedtls_gcm_free(&ctx);
    return ret;
}

template <class T>
class ScopedLock {
	T & a;
	T & b;
public:
	ScopedLock(T & a, T & b) : a(a), b(b) {
		std::lock(a, b);
	}
	ScopedLock(const ScopedLock &) = delete;
	ScopedLock(ScopedLock &&) = delete;
	~ScopedLock() {
		a.unlock();
		b.unlock();
	}
};

bool SessionKey::generateKey() {
	ScopedLock<std::mutex> lock(dhmUse, keyUse);
    if (!my || !other) return false;

	QByteArray newKeys(DH_SIZE, '0');
    size_t olen;
    if (mbedtls_dhm_calc_secret(&dhmc, reinterpret_cast<unsigned char *>(newKeys.data()), DH_SIZE, &olen, mbedtls_ctr_drbg_random, &random)) throw KryptoException("generateKey: Can't calculate secret.");
    my = other = false;
    key_enc_uses = 0;
	keysReady = true;

	QByteArray receiverKeyBase; // (newKeys.data(), 32);
	if (initiator) {
		receiverKeyBase = newKeys.left(32);
		senderKey = newKeys.mid(32, 32);
	}
	else {
		senderKey = newKeys.left(32);
		receiverKeyBase = newKeys.mid(32, 32);
	}

	SenderKeyID = ReceiverKeyID;
	for(int i=0;i<MAX_MESSAGES_WITH_ONE_KEY; ++i) {
		QByteArray * output = new QByteArray(32, '0');

		QByteArray input;
		input.append("A", 1);
		input.append(receiverKeyBase);
		mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), toUChar(input), input.length(), reinterpret_cast<unsigned char*>(output->data()));
		receiverKeys.at(ReceiverKeyID++).swap(std::unique_ptr<QByteArray>(output));

		input[0] = 'B';
		mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), toUChar(input), input.length(), reinterpret_cast<unsigned char*>(receiverKeyBase.data()));
	}
	qDebug() << "Key regenerated with DH.";
    return true;
}
