


#include "crypto.h"
#include <iostream>

inline const unsigned char * toUChar(const QByteArray & a) { return reinterpret_cast<const unsigned char *>(a.operator const char *()); };


bool CertificateManager::createCert(QString userName, QByteArray req, QByteArray& cert) {
	int ret;
	mbedtls_pk_context subject_key;
	mbedtls_x509_csr subject_request;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_x509write_cert crt;

	mbedtls_x509write_crt_init(&crt);
	mbedtls_x509write_crt_set_md_alg(&crt, MBEDTLS_MD_SHA256);
	mbedtls_pk_init(&subject_key);
	mbedtls_ctr_drbg_init(&ctr_drbg);


	const char *pers = "crt creation";
	mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers));

    req.append('\0'); //must be null terminated string

	std::cout << req.toStdString() << std::endl;
	if(ret = mbedtls_x509_csr_parse(&subject_request, toUChar(req), req.length())) throw CryptoException("Can't read csr.", ret);

	mbedtls_x509write_crt_set_subject_name(&crt, (const char *)&subject_request.subject.val.p);
	mbedtls_x509write_crt_set_subject_key(&crt, &subject_request.pk);

	//check issuer key and certificate
	if (!mbedtls_pk_can_do(&server_crt.pk, MBEDTLS_PK_RSA) ||
		mbedtls_mpi_cmp_mpi(&mbedtls_pk_rsa(server_crt.pk)->N,
		&mbedtls_pk_rsa(server_key)->N) != 0 ||
		mbedtls_mpi_cmp_mpi(&mbedtls_pk_rsa(server_crt.pk)->E,
		&mbedtls_pk_rsa(server_key)->E) != 0)
	{
		throw new CryptoException("Server certificate and key do not match.");
	}

	mbedtls_x509write_crt_set_serial(&crt, &serial);
	mbedtls_x509write_crt_set_issuer_key(&crt, &server_key);
	mbedtls_x509write_crt_set_issuer_name(&crt, (const char *)&server_crt.issuer.val.p);

	if (mbedtls_x509write_crt_set_validity(&crt,
		QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmss").toStdString().c_str(),
		QDateTime::currentDateTimeUtc().addDays(1).toString("yyyyMMddhhmmss").toStdString().c_str()))
		throw CryptoException("Certificate validity time error.");

	mbedtls_x509write_crt_set_basic_constraints(&crt, 0, 10);

#if defined(MBEDTLS_SHA1_C)
	mbedtls_x509write_crt_set_subject_key_identifier(&crt);
	ret = mbedtls_x509write_crt_set_authority_key_identifier(&crt);
#endif

	//export certificate
	unsigned char output[4096];
	memset(output, 0, 4096);
	if(ret = mbedtls_x509write_crt_pem(&crt, output, 4096, mbedtls_ctr_drbg_random, &ctr_drbg)) throw CryptoException("Certificate write error.", ret);
	cert = QByteArray(reinterpret_cast<const char *>(output), strlen(reinterpret_cast<const char *>(output)));

	return true;
}

std::string PasswordManager::hash(const char *password) {
	unsigned char pass[64], salt[16];

	mbedtls_ctr_drbg_context ctr_drbg;
	const char *personalization = "]76kXV-$P?0qdQtfpkTPUSvWcq&(dyub";
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)personalization, strlen(personalization));

	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);

	mbedtls_ctr_drbg_random(&ctr_drbg, salt, 16);
	mbedtls_pkcs5_pbkdf2_hmac(&ctx, reinterpret_cast<const unsigned char *>(password), strlen(password), salt, 16, 1000, 64, pass);

	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_md_free(&ctx);

	size_t len;
	char enc[128];
	mbedtls_base64_encode(reinterpret_cast<unsigned char *>(enc), 128, &len, salt, 16);
	std::string str = "1#";
	str += enc;
	str += "#";

	mbedtls_base64_encode(reinterpret_cast<unsigned char *>(enc), 128, &len, pass, 64);
	str += enc;
	return str;
}

bool PasswordManager::verify(const char *password, const char *hash) const {
	unsigned char salt[16];
	unsigned char pass[64];

	if (hash[0] == '1' && hash[1] == '#') {
		size_t len = strlen(hash);
		size_t olen;

		for (size_t i = 2; i < len; ++i) {
			if (hash[i] == '#') {
				mbedtls_base64_decode(salt, 16, &olen, reinterpret_cast<const unsigned char *>(hash) + 2, i - 2);
				mbedtls_base64_decode(pass, 64, &olen, reinterpret_cast<const unsigned char *>(hash) + i + 1, len - i - 1);
				break;
			}
		}

		unsigned char ipass[64];
		mbedtls_md_context_t ctx;
		mbedtls_md_init(&ctx);
		mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);

		mbedtls_pkcs5_pbkdf2_hmac(&ctx, reinterpret_cast<const unsigned char *>(password), strlen(password), salt, 16, 1000, 64, ipass);
		mbedtls_md_free(&ctx);

		return memcmp(ipass, pass, 64) == 0;
	}
	return false;
}





