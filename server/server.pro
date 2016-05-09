QT += core network concurrent
QT -= gui

CONFIG += console
CONFIG -= app_bundle
CONFIG -= flat

unix {
	QMAKE_CXX = g++-4.8
	QMAKE_CXXFLAGS += -std=c++11
	
	LIBS += -ldl
}

TEMPLATE = app

TARGET = armadill-server

HEADERS += src/serverconsole.h \
    ../include/mbedtls/aes.h \
    ../include/mbedtls/aesni.h \
    ../include/mbedtls/arc4.h \
    ../include/mbedtls/asn1.h \
    ../include/mbedtls/asn1write.h \
    ../include/mbedtls/base64.h \
    ../include/mbedtls/bignum.h \
    ../include/mbedtls/blowfish.h \
    ../include/mbedtls/bn_mul.h \
    ../include/mbedtls/camellia.h \
    ../include/mbedtls/ccm.h \
    ../include/mbedtls/certs.h \
    ../include/mbedtls/cipher.h \
    ../include/mbedtls/cipher_internal.h \
    ../include/mbedtls/compat-1.3.h \
    ../include/mbedtls/config.h \
    ../include/mbedtls/ctr_drbg.h \
    ../include/mbedtls/debug.h \
    ../include/mbedtls/des.h \
    ../include/mbedtls/dhm.h \
    ../include/mbedtls/ecdh.h \
    ../include/mbedtls/ecdsa.h \
    ../include/mbedtls/ecjpake.h \
    ../include/mbedtls/ecp.h \
    ../include/mbedtls/entropy.h \
    ../include/mbedtls/entropy_poll.h \
    ../include/mbedtls/error.h \
    ../include/mbedtls/gcm.h \
    ../include/mbedtls/havege.h \
    ../include/mbedtls/hmac_drbg.h \
    ../include/mbedtls/check_config.h \
    ../include/mbedtls/md.h \
    ../include/mbedtls/md2.h \
    ../include/mbedtls/md4.h \
    ../include/mbedtls/md5.h \
    ../include/mbedtls/md_internal.h \
    ../include/mbedtls/memory_buffer_alloc.h \
    ../include/mbedtls/net.h \
    ../include/mbedtls/oid.h \
    ../include/mbedtls/padlock.h \
    ../include/mbedtls/pem.h \
    ../include/mbedtls/pk.h \
    ../include/mbedtls/pkcs5.h \
    ../include/mbedtls/pkcs11.h \
    ../include/mbedtls/pkcs12.h \
    ../include/mbedtls/pk_internal.h \
    ../include/mbedtls/platform.h \
    ../include/mbedtls/ripemd160.h \
    ../include/mbedtls/rsa.h \
    ../include/mbedtls/sha1.h \
    ../include/mbedtls/sha256.h \
    ../include/mbedtls/sha512.h \
    ../include/mbedtls/ssl.h \
    ../include/mbedtls/ssl_cache.h \
    ../include/mbedtls/ssl_ciphersuites.h \
    ../include/mbedtls/ssl_cookie.h \
    ../include/mbedtls/ssl_internal.h \
    ../include/mbedtls/ssl_ticket.h \
    ../include/mbedtls/threading.h \
    ../include/mbedtls/timing.h \
    ../include/mbedtls/version.h \
    ../include/mbedtls/x509.h \
    ../include/mbedtls/x509_crl.h \
    ../include/mbedtls/x509_crt.h \
    ../include/mbedtls/x509_csr.h \
    ../include/mbedtls/xtea.h \
	../include/sqlite3.h \
	../include/catch.hpp \
    test/utest.h \
	src/crypto.h \
    src/clientdb.h \
	src/servermanager.h \
    src/clientconnection.h

SOURCES += \
    src/server.cpp \
    src/serverconsole.cpp \
    ../include/aes.c \
    ../include/aesni.c \
    ../include/arc4.c \
    ../include/asn1parse.c \
    ../include/asn1write.c \
    ../include/base64.c \
    ../include/bignum.c \
    ../include/blowfish.c \
    ../include/camellia.c \
    ../include/ccm.c \
    ../include/certs.c \
    ../include/cipher.c \
    ../include/cipher_wrap.c \
    ../include/ctr_drbg.c \
    ../include/debug.c \
    ../include/des.c \
    ../include/dhm.c \
    ../include/ecdh.c \
    ../include/ecdsa.c \
    ../include/ecjpake.c \
    ../include/ecp.c \
    ../include/ecp_curves.c \
    ../include/entropy.c \
    ../include/entropy_poll.c \
    ../include/error.c \
    ../include/gcm.c \
    ../include/havege.c \
    ../include/hmac_drbg.c \
    ../include/md.c \
    ../include/md2.c \
    ../include/md4.c \
    ../include/md5.c \
    ../include/md_wrap.c \
    ../include/memory_buffer_alloc.c \
    ../include/net.c \
    ../include/oid.c \
    ../include/padlock.c \
    ../include/pem.c \
    ../include/pk.c \
    ../include/pkcs5.c \
    ../include/pkcs11.c \
    ../include/pkcs12.c \
    ../include/pkparse.c \
    ../include/pk_wrap.c \
    ../include/pkwrite.c \
    ../include/platform.c \
    ../include/ripemd160.c \
    ../include/rsa.c \
    ../include/sha1.c \
    ../include/sha256.c \
    ../include/sha512.c \
    ../include/ssl_cache.c \
    ../include/ssl_ciphersuites.c \
    ../include/ssl_cli.c \
    ../include/ssl_cookie.c \
    ../include/ssl_srv.c \
    ../include/ssl_ticket.c \
    ../include/ssl_tls.c \
    ../include/threading.c \
    ../include/timing.c \
    ../include/version.c \
    ../include/version_features.c \
    ../include/x509.c \
    ../include/x509_create.c \
    ../include/x509_crl.c \
    ../include/x509_crt.c \
    ../include/x509_csr.c \
    ../include/x509write_crt.c \
    ../include/x509write_csr.c \
    ../include/xtea.c \
	../include/sqlite3.c \
    test/utest.cpp \
	src/crypto.cpp \
    src/clientdb.cpp \
	src/servermanager.cpp \
    src/clientconnection.cpp

DISTFILES += \
    ../include/cmake_install.cmake \
    ../include/CMakeLists.txt \
    ../include/Makefile


