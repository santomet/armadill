#ifndef COMMON
#define COMMON
//----------peers
/*!
     * \brief The peer struct
     */
struct peer
{
    QString name;
    QString address;
    int listeningPort;
    QString cert;
    bool trustedByServer = false;
    bool active = false;
};

#endif // COMMON

