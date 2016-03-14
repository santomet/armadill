QT += core testlib
QT -= gui

CONFIG += console
CONFIG -= app_bundle

QMAKE_CXX = g++-4.8
QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = app

TARGET = armadill

HEADERS += src/clientconsole.h \
    ../include/aes.h \
    ../include/aesni.h \
    ../include/arc4.h \
    ../include/asn1.h \
    ../include/asn1write.h \
    ../include/base64.h \
    ../include/bignum.h \
    ../include/blowfish.h \
    ../include/bn_mul.h \
    ../include/camellia.h \
    ../include/ccm.h \
    ../include/certs.h \
    ../include/cipher.h \
    ../include/cipher_internal.h \
    ../include/compat-1.3.h \
    ../include/config.h \
    ../include/ctr_drbg.h \
    ../include/debug.h \
    ../include/des.h \
    ../include/dhm.h \
    ../include/ecdh.h \
    ../include/ecdsa.h \
    ../include/ecjpake.h \
    ../include/ecp.h \
    ../include/entropy.h \
    ../include/entropy_poll.h \
    ../include/error.h \
    ../include/gcm.h \
    ../include/havege.h \
    ../include/hmac_drbg.h \
    ../include/check_config.h \
    ../include/md.h \
    ../include/md2.h \
    ../include/md4.h \
    ../include/md5.h \
    ../include/md_internal.h \
    ../include/memory_buffer_alloc.h \
    ../include/net.h \
    ../include/oid.h \
    ../include/padlock.h \
    ../include/pem.h \
    ../include/pk.h \
    ../include/pkcs5.h \
    ../include/pkcs11.h \
    ../include/pkcs12.h \
    ../include/pk_internal.h \
    ../include/platform.h \
    ../include/ripemd160.h \
    ../include/rsa.h \
    ../include/sha1.h \
    ../include/sha256.h \
    ../include/sha512.h \
    ../include/ssl.h \
    ../include/ssl_cache.h \
    ../include/ssl_ciphersuites.h \
    ../include/ssl_cookie.h \
    ../include/ssl_internal.h \
    ../include/ssl_ticket.h \
    ../include/threading.h \
    ../include/timing.h \
    ../include/version.h \
    ../include/x509.h \
    ../include/x509_crl.h \
    ../include/x509_crt.h \
    ../include/x509_csr.h \
    ../include/xtea.h \
    test/utest.h \
    src/krypto.h \
    src/messages.h \
    src/serverconnection.h \
    src/peerconnection.h \
    src/common.h

SOURCES += \
    src/client.cpp \
    src/clientconsole.cpp \
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
    test/utest.cpp \
    src/krypto.cpp \
    src/messages.cpp \
    src/serverconnection.cpp \
    src/peerconnection.cpp

DISTFILES += \
    ../include/cmake_install.cmake \
    ../include/CMakeLists.txt

