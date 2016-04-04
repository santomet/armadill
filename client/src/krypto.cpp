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

QByteArray SessionKey::encrypt(const QByteArray & message, const QByteArray & data) {
	if (key_enc_uses >= MAX_MESSAGES_WITH_ONE_KEY) throw KryptoOveruseException("Key was already used for 10 encryptions.");
	++key_enc_uses;

	unsigned char iv[16], tag[TAG_LENGTH];
	unsigned char * output = new unsigned char[message.length()];
	mbedtls_ctr_drbg_random(&random, iv, 16);

	mbedtls_gcm_setkey(&gcmc, MBEDTLS_CIPHER_ID_AES, toUChar(currentkey), 256);
	mbedtls_gcm_crypt_and_tag(&gcmc, MBEDTLS_GCM_ENCRYPT, message.length(), iv, 16, toUChar(data), data.length(), toUChar(message), output, TAG_LENGTH, tag);
	QByteArray ret(1, keyid);
	ret.append(reinterpret_cast<const char *>(iv), 16);
	ret.append(reinterpret_cast<const char *>(tag), TAG_LENGTH);
	ret.append(reinterpret_cast<const char *>(output), message.length());
	delete[] output;
	return ret;
}

QByteArray SessionKey::decrypt(const QByteArray & message, const QByteArray & data) {
	size_t dataLength = message.length() - 16 - TAG_LENGTH - 1; // iv + tag + keyId
	unsigned char messageKeyId;
	unsigned char iv[16], tag[TAG_LENGTH];
	unsigned char * input = new unsigned char[dataLength];
	unsigned char * output = new unsigned char[dataLength];
	messageKeyId = message[0];
	qstrncpy(reinterpret_cast<char * >(iv), message.data() + 1, 16);
	qstrncpy(reinterpret_cast<char * >(tag), message.data() + 16 + 1, TAG_LENGTH);
	qstrncpy(reinterpret_cast<char * >(input), message.data() + 16 + TAG_LENGTH + 1, dataLength);

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

	if (mbedtls_gcm_auth_decrypt(&gcmc, dataLength, iv, 16, toUChar(data), data.length(), tag, TAG_LENGTH, input, output) == MBEDTLS_ERR_GCM_AUTH_FAILED) {
		throw KryptoException("Authentication of message failed");
	}

	QByteArray ret(reinterpret_cast<const char *> (output), dataLength);

	return ret;
}




















bool SessionKey::generateKey() {
	if (!my || !other) return false;
	oldkey = currentkey;
	++keyid;
	
	unsigned char key[ENCRYPTION_KEY_SIZE];
	size_t olen;
	if (mbedtls_dhm_calc_secret(&dhmc, key, ENCRYPTION_KEY_SIZE, &olen, mbedtls_ctr_drbg_random, &random)) throw KryptoException("generateKey: Can't calculate secret.");
	currentkey.setRawData(reinterpret_cast<const char *>(key), olen);
	my = other = false;
	key_enc_uses = key_dec_uses = 0;
	return true;
}