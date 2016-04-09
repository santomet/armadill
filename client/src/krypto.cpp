#include "krypto.h"
#include <QDateTime>

inline const unsigned char * toUChar(const QByteArray & a) { return reinterpret_cast<const unsigned char *>(a.operator const char *()); };


Krypto::Krypto()
{

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

	unsigned char iv[16], tag[TAG_LENGTH];
	mbedtls_ctr_drbg_random(&random, iv, 16);

	QByteArray ret(1, keyid);
	ret.resize(17 + TAG_LENGTH + message.length()); // 1 for keyid + 16 for IV + tag + message
	ret.replace(1, 16, reinterpret_cast<const char *>(iv), 16);

	mbedtls_gcm_setkey(&gcmc, MBEDTLS_CIPHER_ID_AES, toUChar(currentkey), 256);
	mbedtls_gcm_crypt_and_tag(&gcmc, MBEDTLS_GCM_ENCRYPT, message.length(), iv, 16, toUChar(data), data.length(), toUChar(message), reinterpret_cast<uchar *>(ret.data() + 17 + TAG_LENGTH), TAG_LENGTH, tag);
	
	ret.replace(17, TAG_LENGTH, reinterpret_cast<const char *>(tag), TAG_LENGTH);
	return ret;
}

QByteArray SessionKey::unprotect(const QByteArray & message, const QByteArray & data) {
	//message = keyId(1)/iv(16)/tag(TAG_LENGTH)/encData
	size_t dataLength = message.length() - 16 - TAG_LENGTH - 1; // iv + tag + keyId
	unsigned char messageKeyId = message[0];
	unsigned char * output = new unsigned char[dataLength];
	
	if (messageKeyId == keyid) {
		mbedtls_gcm_setkey(&gcmc, MBEDTLS_CIPHER_ID_AES, toUChar(currentkey), 256);
		++key_dec_uses;
	}
	else if (messageKeyId == keyid - 1) {
		mbedtls_gcm_setkey(&gcmc, MBEDTLS_CIPHER_ID_AES, toUChar(oldkey), 256);
	}
	else {
		throw KryptoException("key is too old");
	}

	int ret_success;
	if ((ret_success = mbedtls_gcm_auth_decrypt(&gcmc, dataLength, toUChar(message)+1, 16, toUChar(data), data.length(), toUChar(message)+17, TAG_LENGTH, toUChar(message)+17+TAG_LENGTH, output))) {
		if(ret_success == MBEDTLS_ERR_GCM_AUTH_FAILED)		throw KryptoException("Authentication of message failed");
		else if(ret_success == MBEDTLS_ERR_GCM_BAD_INPUT)	throw KryptoException("Bad message data for unprotect");
		else												throw KryptoException("Unknown Error");
	}

	QByteArray ret(reinterpret_cast<const char *> (output), dataLength);
	delete[] output;
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
	key_enc_uses = key_dec_uses = 0;
	return true;
}