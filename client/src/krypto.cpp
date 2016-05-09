#include "krypto.h"
#include <QDateTime>

inline const unsigned char * toUChar(const QByteArray & a) { return reinterpret_cast<const unsigned char *>(a.operator const char *()); };


Krypto::Krypto()
{

}

bool Krypto::createCert(QByteArray &priv, QByteArray &request, const QString common) {
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
		throw new KryptoException("Generating entropy for RSA pair failed.");
	}

	
	if (mbedtls_rsa_gen_key(mbedtls_pk_rsa(pk), mbedtls_ctr_drbg_random, &ctr_drbg, RSA_SIZE, RSA_EXPONENT)
		!= 0)
	{
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
	request = QByteArray(reinterpret_cast<const char *>(output), sizeof(output));
	memset(output, 0, 4096);
	mbedtls_pk_write_key_pem(&pk, output, 4096);
	priv = QByteArray(reinterpret_cast<const char *>(output), sizeof(output));

	//clean
	mbedtls_pk_free(&pk);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

	return true;
}

bool Krypto::verifyCert(const QByteArray pubToVerify, const QByteArray pubCA) {
	//TODO
	return true;
}

void SessionKey::setDH(QByteArray dh) {
	if (mbedtls_dhm_read_public(&dhmc, toUChar(dh), dh.length())) throw KryptoException("setDH: can't read DH");
	other = true;
}

QByteArray SessionKey::getDH() {
	size_t len = dhmc.len;
	unsigned char * dhm = new unsigned char[len];
	if (mbedtls_dhm_make_public(&dhmc, ENCRYPTION_KEY_SIZE, dhm, len, mbedtls_ctr_drbg_random, &random)) throw KryptoException("getDH: can't generate public dh.");
	QByteArray ret = QByteArray(reinterpret_cast<const char *>(dhm), len);
	delete[] dhm;
	my = true;
	return ret;
}

QByteArray SessionKey::protect(const QByteArray & message, const QByteArray & data) {
	if (key_enc_uses >= MAX_MESSAGES_WITH_ONE_KEY) throw KryptoOveruseException("Key was already used for 10 encryptions.");
	++key_enc_uses;

	unsigned char iv[IV_LENGTH], tag[TAG_LENGTH];
	mbedtls_ctr_drbg_random(&random, iv, IV_LENGTH);

	QByteArray ret(1, keyid);
	ret.resize(1 + IV_LENGTH + TAG_LENGTH + message.length()); // 1 for keyid + IV + tag + message
	ret.replace(1, IV_LENGTH, reinterpret_cast<const char *>(iv), IV_LENGTH);

	mbedtls_gcm_context ctx;
	mbedtls_gcm_init(&ctx);
	mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, toUChar(currentkey), 256);
	mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, message.length(), iv, IV_LENGTH, toUChar(data), data.length(), toUChar(message), reinterpret_cast<uchar *>(ret.data() + 1 + IV_LENGTH + TAG_LENGTH), TAG_LENGTH, tag);
	
	ret.replace(1 + IV_LENGTH, TAG_LENGTH, reinterpret_cast<const char *>(tag), TAG_LENGTH);
	mbedtls_gcm_free(&ctx);
	return ret;
}

QByteArray SessionKey::unprotect(const QByteArray & message, const QByteArray & data) {
	//message = keyId(1)/iv(16)/tag(TAG_LENGTH)/encData
	size_t dataLength = message.length() - IV_LENGTH - TAG_LENGTH - 1; // iv + tag + keyId
	unsigned char messageKeyId = message[0];
	unsigned char * output = new unsigned char[dataLength];
	
	mbedtls_gcm_context ctx;
	mbedtls_gcm_init(&ctx);

	if (messageKeyId == keyid) {
		if (key_dec_uses >= MAX_MESSAGES_WITH_ONE_KEY) throw KryptoOveruseException("Key was already used for 10 decryptions.");
		mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, toUChar(currentkey), 256);
		++key_dec_uses;
	}
	else if (messageKeyId == keyid - 1) {
		if (key_old_dec_uses >= MAX_MESSAGES_WITH_ONE_KEY) throw KryptoOveruseException("Key was already used for 10 decryptions.");
		mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, toUChar(oldkey), 256);
	}
	else {
		throw KryptoException("key is too old");
	}

	int ret_success;
	if ((ret_success = mbedtls_gcm_auth_decrypt(&ctx, dataLength, toUChar(message) + 1, IV_LENGTH, toUChar(data), data.length(), toUChar(message) + 1 + IV_LENGTH, TAG_LENGTH, toUChar(message) + 1 + IV_LENGTH +TAG_LENGTH, output))) {
		if(ret_success == MBEDTLS_ERR_GCM_AUTH_FAILED)		throw KryptoException("Authentication of message failed");
		else if(ret_success == MBEDTLS_ERR_GCM_BAD_INPUT)	throw KryptoException("Bad message data for unprotect");
		else												throw KryptoException("Unknown Error");
	}

	QByteArray ret(reinterpret_cast<const char *> (output), dataLength);
	delete[] output;
	mbedtls_gcm_free(&ctx);
	return ret;
}




















bool SessionKey::generateKey() {
	if (!my || !other) return false;
	oldkey = currentkey;
	++keyid;
	
	size_t olen;
	currentkey.resize(ENCRYPTION_KEY_SIZE);
	if (mbedtls_dhm_calc_secret(&dhmc, reinterpret_cast<unsigned char *>(currentkey.data()), ENCRYPTION_KEY_SIZE, &olen, mbedtls_ctr_drbg_random, &random)) throw KryptoException("generateKey: Can't calculate secret.");
	my = other = false;
	key_old_dec_uses = key_dec_uses;
	key_enc_uses = key_dec_uses = 0;
	return true;
}