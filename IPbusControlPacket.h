#ifndef IPBUSCONTROLPACKET_H
#define IPBUSCONTROLPACKET_H
#include "IPbusHeaders.h"
#include <QObject>
#include <QDateTime>

const quint16 maxPacket = 368; //368 words, limit from ethernet MTU of 1500 bytes
enum errorType {networkError = 0, IPbusError = 1, logicError = 2};
static const char *errorTypeName[3] = {"Network error" , "IPbus error", "Logic error"};

class IPbusControlPacket : public QObject {
    Q_OBJECT
public:
    QList<Transaction> transactionsList;
    quint16 requestSize = 1, responseSize = 1; //values are measured in words
    quint32 request[maxPacket], response[maxPacket];
    quint32 dt[2]; //temporary data

    IPbusControlPacket() {
        request[0] = PacketHeader(control, 0);
        connect(this, &IPbusControlPacket::error, this, &IPbusControlPacket::debugPrint);
    }
    ~IPbusControlPacket() { this->disconnect(); }

    void debugPrint(QString st) {
        qDebug(qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + st));
        qDebug("request:");
        for (quint16 i=0; i<requestSize; ++i)  qDebug("%08X", request[i]);
        qDebug("        response:");
        for (quint16 i=0; i<responseSize; ++i) qDebug("        %08X", response[i]);
    }

    quint32 *masks(quint32 mask0, quint32 mask1) { //for convinient adding RMWbit transaction
        dt[0] = mask0; //for writing 0's: AND term
        dt[1] = mask1; //for writing 1's: OR term
        return dt;
    }

    void addTransaction(TransactionType type, quint32 address, quint32 *data, quint8 nWords = 1) {
        Transaction currentTransaction;
        request[requestSize] = TransactionHeader(type, nWords, transactionsList.size());
        currentTransaction.requestHeader = (TransactionHeader *)(request + requestSize++);
        request[requestSize] = address;
        currentTransaction.address = request + requestSize++;
        currentTransaction.responseHeader = (TransactionHeader *)(response + responseSize++);
        switch (type) {
            case                read:
            case nonIncrementingRead:
            case   configurationRead:
                currentTransaction.data = data;
                responseSize += nWords;
                break;
            case                write:
            case nonIncrementingWrite:
            case   configurationWrite:
                currentTransaction.data = request + requestSize;
                for (quint8 i=0; i<nWords; ++i) request[requestSize++] = data[i];
                break;
            case RMWbits:
                request[requestSize++] = data[0]; //AND term
                request[requestSize++] = data[1]; // OR term
                currentTransaction.data = response + responseSize++;
                break;
            case RMWsum:
                request[requestSize++] = *data; //addend
                currentTransaction.data = response + responseSize++;
                break;
            default:
                emit error("unknown transaction type", IPbusError);
        }
        if (requestSize > maxPacket || responseSize > maxPacket) {
            emit error("packet size exceeded", IPbusError);
            return;
        } else transactionsList.append(currentTransaction);
    }

    void addWordToWrite(quint32 address, quint32 value) { addTransaction(write, address, &value, 1); }

    void addNBitsToChange(quint32 address, quint32 data, quint8 nbits = 16, quint8 shift = 0) {
        quint32 mask = (1 << nbits) - 1; //e.g. 0x00000FFF for nbits==12
        addTransaction(RMWbits, address, masks( ~quint32(mask << shift), quint32((data & mask) << shift) ));
    }

    bool processResponse() { //check transactions successfulness and copy read data to destination
        for (quint16 i=0; i<transactionsList.size(); ++i) {
            TransactionHeader *th = transactionsList.at(i).responseHeader;
            if (th->ProtocolVersion != 2 || th->TransactionID != i || th->TypeID != transactionsList.at(i).requestHeader->TypeID) {
                emit error(QString::asprintf("unexpected transaction header: %08X, expected: %08X", *th, *transactionsList.at(i).requestHeader & 0xFFFFFFF0), IPbusError);
                return false;
            }
            if (th->Words > 0) switch (th->TypeID) {
                case                read:
                case nonIncrementingRead:
                case   configurationRead: {
                    quint32 wordsAhead = response + responseSize - (quint32 *)th - 1;
                    if (th->Words > wordsAhead) { //response too short to contain nWords values
                        if (transactionsList.at(i).data != nullptr) memcpy(transactionsList.at(i).data, (quint32 *)th + 1, wordsAhead * wordSize);
                        emit successfulRead(wordsAhead);
                        if (th->InfoCode == 0) emit error(QString::asprintf("read transaction from %08X truncated: %d/%d words received", *transactionsList.at(i).address, wordsAhead, th->Words), IPbusError);
                        return false;
                    } else {
                        if (transactionsList.at(i).data != nullptr) memcpy(transactionsList.at(i).data, (quint32 *)th + 1, th->Words * wordSize);
                        emit successfulRead(th->Words);
                    }
                    break;
                }
                case RMWbits:
                case RMWsum :
                    if (th->Words != 1) {
                        emit error("wrong RMW transaction", IPbusError);
                        return false;
                    }
                    emit successfulRead(1);
                    /* fall through */ //[[fallthrough]];
                case                write:
                case nonIncrementingWrite:
                case   configurationWrite:
                    emit successfulWrite(th->Words);
                    break;
                default:
                    emit error("unknown transaction type", IPbusError);
                    return false;
            }
            if (th->InfoCode != 0) {
				emit error(th->infoCodeString() + QString::asprintf(", address: %08X", *transactionsList.at(i).address + th->Words), IPbusError);
                return false;
            }
        }
        return true;
    }
    void reset() {
        transactionsList.clear();
        requestSize = 1;
        responseSize = 1;
    }

signals:
    void error(QString, errorType);
    void successfulRead(quint8 nWords);
    void successfulWrite(quint8 nWords);
};


#endif // IPBUSCONTROLPACKET_H
