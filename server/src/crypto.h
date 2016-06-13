#pragma once

#define MBEDTLS_PEM_PARSE_C

#include "../../include/mbedtls/md.h"
#include "../../include/mbedtls/pkcs5.h"
#include "../../include/mbedtls/base64.h"
#include "../../include/mbedtls/entropy.h"
#include "../../include/mbedtls/ctr_drbg.h"
#include "../../include/mbedtls/x509_crt.h"
#include "../../include/mbedtls/x509_csr.h"


#include <cstring>
#include <stdexcept>
#include <string>
#include <QString>
#include <QDateTime>
#include <QDebug>

#define SERVER_CERT_FILE "test/ARMADILL.crt"
#define SERVER_KEY_FILE "test/ARMADILL.key"

class CryptoException : public std::runtime_error {
	int err;
public:
	CryptoException(const char * msg, int err = 0) : std::runtime_error(msg), err(err) {};
	int error() const { return err; };
};

class CertificateManager {
private:
	mbedtls_entropy_context entropy;
	mbedtls_pk_context server_key;
	mbedtls_x509_crt server_crt;
	mbedtls_mpi serial;
public:
	CertificateManager() {
		mbedtls_entropy_init(&entropy);
		mbedtls_entropy_gather(&entropy);
		//TODO: load server key
        if (mbedtls_pk_parse_keyfile(&server_key, SERVER_KEY_FILE, nullptr) != 0)
		{
			throw CryptoException("Unable to load server key.");
        }
		//TODO: load server crt

        if (mbedtls_x509_crt_parse_file(&server_crt, SERVER_CERT_FILE) != 0)
		{
			throw CryptoException("Unable to load server certificate.");
		}
		mbedtls_mpi_init(&serial);
	}
	~CertificateManager() {
		mbedtls_entropy_free(&entropy);
		mbedtls_x509_crt_free(&server_crt);
		mbedtls_pk_free(&server_key);
	}
	/*!
	* \brief createCert        Creates a certificate from request
	*
	* \param userName          QString with name of user
	*
	* \param req               QByteArray with PEM encoded request
	*
	* \param cert              QByteArray where PEM encoded certificate will be written
	*							(QByteArray will be overwritten)
	*/
	bool createCert(QString userName, QByteArray req, QByteArray& cert);
private:
	void freeContexts(mbedtls_pk_context *subject_key, mbedtls_x509write_cert *crt, mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_x509_csr *subject_request) {
		mbedtls_pk_free(subject_key);
		mbedtls_x509write_crt_free(crt);
		mbedtls_ctr_drbg_free(ctr_drbg);
		mbedtls_x509_csr_free(subject_request);
	}
};

class PasswordManager {
	mbedtls_entropy_context entropy;
public:
	PasswordManager() {
		mbedtls_entropy_init(&entropy);
		mbedtls_entropy_gather(&entropy);
    }

	~PasswordManager() {
		mbedtls_entropy_free(&entropy);
    }

    std::string hash(const char *password);
	bool verify(const char *password, const char *hash) const;
};
