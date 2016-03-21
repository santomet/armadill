


#include "crypto.h"


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
	}


	unsigned char ipass[64];
	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);

	mbedtls_pkcs5_pbkdf2_hmac(&ctx, reinterpret_cast<const unsigned char *>(password), strlen(password), salt, 16, 1000, 64, ipass);
	return memcmp(ipass, pass, 64) == 0;
}
