#ifndef MESSAGES_H
#define MESSAGES_H

#include <QObject>
#include "krypto.h"
#include "peerconnection.h"
#include "common.h"
#include <QFile>
#include <QQueue>
#include <QVector>
#include <QList>

#define MAX_MESSAGES_WITH_ONE_KEY 10

class Messages : public QObject
{
    Q_OBJECT
public:
    const char armaSeparator = '#';

    explicit Messages(QObject *parent = 0);


//-----------------------------Structures and types-------------------------------------
    /*!
     * \brief ArmaMessage               ArmaMessage should looks like this:
     *                                  Sender's nick||Receiver's nick||Timestamp||TYPE||Used Key identificator||DH||HMAC||message size||Message
     *                                  where || concat is non-base64 char defined in armaSeparator
     *                                  Message is binary, HMAC and DH are base64
     */
    typedef QByteArray ArmaMessage;

    /*!
     * \brief FileChunk                 NOT EXACTLY DEFINED YET
     */
    typedef QByteArray FileChunkEncrypted;
    typedef QByteArray FileChunkDecrypted;


    enum MsgType
    {
        RegularMessage = 0,
        RegularMessageDH = 1,
        FileMessage = 2,
        FileMessageDH = 3,
        PureDH = 4
    };

    struct FileContext
    {
        unsigned char id[16]; //shortened hash, maybe? TODO
        bool sending; //true if file is being sent, false if received
        QFile *file = nullptr;
        size_t wholeSize = 0;
        size_t processedSize = 0;
        int allParts = 0;
        int processedParts = 0;
    };
//-----------------------Public Methods--------------------------------------------------

    Messages(peer *peerToConnect, QObject *parent = 0);
    ~Messages();

    /*!
     * \brief parseMessage              Parses message and makes proper actions
     *                                  Distinguishes between different types and sends the message
     *                                  part of ArmaMessage to another method
     * \param message                   message
     * \return                          true if everything goes allright
     */
    bool parseMessage(const ArmaMessage &message);

    /*!
     * \brief createRegularMessage      Creates regular ArmaMessage that will be ready to send
     * \param message                   message
     * \return                          ArmaMessage which can be sent to peer
     */
    ArmaMessage* createRegularMessage(const QString message);

    /*!
     * \brief createFileSendingContext  Prepares File for sending to peger
     *                                  - creates a context which getFileMessage() will be working with
     * \param path                      path to file
     * \param receiver                  intended receiver of a file
     * \return                          true if everything goes well
     */
    bool createFileSendingContext(QString path, QString receiver);

    /*!
     * \brief getFileMessage
     * \return                          Returns an ArmaMessage*
     */
    ArmaMessage* getFileMessage();


//---------------------------Others---------------------------------------------------
    Krypto mKrypto;
    peer *mPeer;    //actual peer we are communicating with

private:
    /*!
     * \brief parseFileMessage          Parses File data block (creates file for first one)
     * \param encryptedData
     * \return
     */
    bool parseFileMessage(const FileChunkEncrypted &fileChunk);

    bool parseRegularMessage(const QByteArray &encryptedData);

    ArmaMessage* createFileMessage(FileContext *context, QString eceiver);

    QList<FileContext*>  mFileContexts;
    QList<FileChunkDecrypted*> mUnorderedFileBlocks;

    //---------DH
    QByteArray mDHSharedPrivateMy; //result of DH - one part used as AES key, second as HMAC key
    QByteArray mDHSharedPrivatePeer; //Shared Private my peer is using
    int mMyMessagesCounter; //max 10
    int mPeerMessagesCounter;   //max 10

};

#endif // MESSAGES_H
