#pragma once


#include "../../include/mbedtls/md.h"
#include "../../include/mbedtls/pkcs5.h"
#include "../../include/mbedtls/base64.h"
#include "../../include/mbedtls/entropy.h"
#include "../../include/mbedtls/ctr_drbg.h"

#include <cstring>
#include <string>










class PasswordManager {
	mbedtls_entropy_context entropy;
public:
	PasswordManager() {
		mbedtls_entropy_init(&entropy);
		mbedtls_entropy_gather(&entropy);
	};

	~PasswordManager() {
		mbedtls_entropy_free(&entropy);
	};

	std::string hash(const char *password);
	bool verify(const char *password, const char *hash) const;
};