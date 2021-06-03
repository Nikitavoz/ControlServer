#ifndef IPBUSINTERFACE_H
#define IPBUSINTERFACE_H

#include <QtNetwork>
#include "IPbusHeaders.h"

const quint16 maxPacket = 368; //368 words, limit from ethernet MTU of 1500 bytes
enum errorType {networkError = 0, IPbusError = 1, logicError = 2};
static const char *errorTypeName[3] = {"Network error" , "IPbus error", "Logic error"};

class IPbusTarget: public QObject {
    Q_OBJECT
    quint16 localport;
    QUdpSocket *qsocket = new QUdpSocket(this);
    StatusPacket statusPacket;
    QList<Transaction> transactionsList;

protected:
    quint16 requestSize = 0, responseSize = 0; //values are measured in words
    quint32 request[maxPacket], response[maxPacket];
    char *pRequest = reinterpret_cast<char *>(request);
    char *pResponse = reinterpret_cast<char *>(response);
    quint32 dt[2]; //temporary data

public:
    QString IPaddress = "172.20.75.180";
//    QString IPaddress = "127.0.0.1";
    bool isOnline = false, isInitializing = true;
    QTimer *updateTimer = new QTimer(this);
    QTimer *testTimer = new QTimer(this);
    quint16 updatePeriod_ms = 1000;

    IPbusTarget(quint16 lport = 0) : localport(lport) {
        updateTimer->setTimerType(Qt::PreciseTimer);
        connect(updateTimer, &QTimer::timeout, this, [=]() { if (isOnline) sync(); else requestStatus(); });
        connect(this, &IPbusTarget::error, this, [=]() {
            updateTimer->stop();
            resetTransactions();
        });
        qsocket->bind(QHostAddress::AnyIPv4, localport);
        updateTimer->start(updatePeriod_ms);
    }

    ~IPbusTarget() {
        qsocket->disconnectFromHost();
    }

    quint32 readRegister(quint32 address) {
        quint32 data = 0xFFFFFFFF;
        addTransaction(read, address, &data, 1);
        return transceive(true) ? data : 0xFFFFFFFF;
    }

    void log(QString st) {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") << st;
    }

signals:
    void error(QString, errorType);
    void noResponse();
    void IPbusStatusOK();
    void initComplete();
    void successfulRead(quint8 nWords);
    void successfulWrite(quint8 nWords);

protected:
    quint32 *masks(quint32 mask0, quint32 mask1) { //for convinient adding RMWbit transaction
        dt[0] = mask0; //for writing 0's: AND term
        dt[1] = mask1; //for writing 1's: OR term
        return dt;
    }

    void debugPrint() {
        printf("request:\n");
        for (quint16 i=0; i<requestSize; ++i)  printf("%08X\n", request[i]);
        printf("        response:\n");
        for (quint16 i=0; i<responseSize; ++i) printf("        %08X\n", response[i]);
        printf("\n");
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
                currentTransaction.data = response + responseSize;
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

	bool transceive(bool analyze = true) { //send request, wait for response, receive it and check correctness
        if (requestSize <= 1) {
            emit error("Empty request", logicError);
            return false;
        }
//        if (qsocket->state() != QAbstractSocket::ConnectedState) {
//            qsocket->connectToHost(IPaddress, 50001, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
//            qDebug() << "was disconnected before write";
//        }
        //qsocket->hasPendingDatagrams()
//        if (qsocket->bytesAvailable() > 0) {
//            qsocket->readAll();
//            qDebug() << "extra bytes before write";
//        }
        qint32 n = qint32(qsocket->write(pRequest, requestSize * wordSize));
		if (n < 0) {
            emit error("Socket write error: " + qsocket->errorString(), networkError);
			return false;
		} else if (n != requestSize * wordSize) {
            emit error("Sending packet failed", networkError);
			return false;
        } else if (!qsocket->waitForReadyRead(100)) {
            isOnline = false;
            emit noResponse();
			return false;
		} else {
//            if (qsocket->state() != QAbstractSocket::ConnectedState) {
//                qsocket->connectToHost(IPaddress, 50001, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
//                qDebug() << "was disconnected before read";
//            }
//			n = qint32(qsocket->read(pResponse, responseSize * wordSize));
//			qint32 m = qint32(qsocket->bytesAvailable());
//            if (m > 0) {
//                qDebug() << "extra bytes after read";
//                qsocket->readAll();
//                //emit error("Extra bytes: " + QString::number(m), networkError);
//            }

//            if (n < 0) {
//                if (qsocket->state() != QAbstractSocket::ConnectedState) {
//                    qsocket->connectToHost(IPaddress, 50001, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
//                    qDebug() << "was disconnected on read";
//                } else {
//                    emit error("Socket read error: " + qsocket->errorString(), networkError);
//                    return false;
//                }
            while (qsocket->hasPendingDatagrams() && qsocket->pendingDatagramSize() == 64) {
                qsocket->readDatagram(pResponse, 64);
                if () {}
            }
            while (qsocket->hasPendingDatagrams()) {
                n = qsocket->pendingDatagramSize();
                qsocket->readDatagram(pResponse, n);
                if (n > responseSize || n % wordSize > 0) {
                    emit error(QString::asprintf("incorrect response (%d bytes)", n ), networkError);
                    return false;
                }
                if (n == 64) {}
            }
            //} else
            if (n == 0) {
                emit error("empty response, no IPbus", networkError);
				return false;
			} else if (response[0] != request[0] || n % wordSize > 0) {
                emit error(QString::asprintf("incorrect response (%d bytes)", n + m), networkError);
				return false;
			} else {
				responseSize = quint16(n / wordSize);    
                bool result = analyze ? analyzeResponse() : true;
                resetTransactions();
                return result;
			}
		}
	}

    bool analyzeResponse() { //check transactions successfulness and copy read data to destination
        for (quint16 i=0; i<transactionsList.size(); ++i) {
            TransactionHeader *th = transactionsList.at(i).responseHeader;
            if (th->ProtocolVersion != 2 || th->TransactionID != i || th->TypeID != transactionsList.at(i).requestHeader->TypeID) {
                emit error(QString::asprintf("unexpected transaction header: %08X, expected: %08X", *th, *transactionsList.at(i).requestHeader & 0xFFFFFFF0), IPbusError);
                return false;
            }
            if (th->Words == 0) continue;
            switch (th->TypeID) {
                case                read:
                case nonIncrementingRead:
                case   configurationRead:
                    if (transactionsList.at(i).data != nullptr) {
                        quint32 *src = (quint32 *)th + 1, *dst = transactionsList.at(i).data;
                        while (src <= (quint32 *)th + th->Words && src < response + responseSize) *dst++ = *src++;
                    }
                    if ((quint32 *)th + th->Words >= response + responseSize || ) { //response too short to contain nWords values
                        emit error("read transaction truncated", IPbusError);
                        return false;

                    } else {

                    }

                    emit successfulRead(nWords);
                    break;
                case RMWbits:
                case RMWsum :
                    if (th.Words != 1) {
                        emit error("wrong RMW transaction", IPbusError);
                        return false;
                    }
                    ++j;     //skipping received value
                    emit successfulRead(1);
                    /* fall through */ //[[fallthrough]];
                case                write:
                case nonIncrementingWrite:
                case   configurationWrite:
                    emit successfulWrite(nWords);
                    break;
                default:
                    emit error("unknown transaction type", IPbusError);
                    return false;
            }
            if (th.InfoCode != 0) {
            emit error(th.infoCodeString(), IPbusError);
            return false;
            }
        }
        for (quint16 j = 1, n = 0; !transactionsList.isEmpty() && j < responseSize; ++j, ++n) {
            TransactionHeader th = response[j];
            if (th.ProtocolVersion != 2 || th.TransactionID != n) {
                emit error(QString::asprintf("unexpected transaction header: %08X, expected: 2%03X??F0", th, n), IPbusError);
                return false;
            }
            quint8 nWords = th.Words;
            quint32 *data = transactionsList.takeFirst();
            bool ok = true;
            if (nWords > 0) {
                switch (th.TypeID) {
                    case                read:
                    case nonIncrementingRead:
                    case   configurationRead:
                        if (j + nWords >= responseSize) { //response too short to contain nWords values
                            ok = false;
                            nWords = quint8(responseSize - j - 1);
                        }
                        if (data != nullptr)
                            for (quint8 i=0; i<nWords; ++i) data[i] = response[++j];
                        else
                            j += nWords;
                        emit successfulRead(nWords);
                        if (!ok) {
                            emit error("read transaction truncated", IPbusError);
                            return false;
                        }
                        break;
                    case RMWbits:
                    case RMWsum :
                        if (th.Words != 1) {
                            emit error("wrong RMW transaction", IPbusError);
                            return false;
                        }
                        ++j;     //skipping received value
                        emit successfulRead(1);
                        /* fall through */ //[[fallthrough]];
                    case                write:
                    case nonIncrementingWrite:
                    case   configurationWrite:
                        emit successfulWrite(nWords);
                        break;
                    default:
                        emit error("unknown transaction type", IPbusError);
                        return false;
                }
            }
            if (th.InfoCode != 0) {
            emit error(th.infoCodeString(), IPbusError);
            return false;
            }
        }
        return true;
    }

protected slots:
    void resetTransactions() { //return to initial (default) state
        request[0] = quint32(PacketHeader(control, 0));
        requestSize = 1;
        responseSize = 1;
        transactionsList.clear();
    }

public slots:
    void reconnect() {
//        if (qsocket->state() == QAbstractSocket::ConnectedState) qsocket->disconnectFromHost();
//        qsocket->connectToHost(IPaddress, 50001, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
        requestStatus();
        if (!updateTimer->isActive()) updateTimer->start(updatePeriod_ms);
    }

    void requestStatus() {
        requestSize = sizeof(statusPacket) / wordSize; //16 words
        memcpy(pRequest, &statusPacket, sizeof(statusPacket));
        responseSize = requestSize;
        if (transceive(false)) { //no transactions to process
            if (!isOnline) {
                isInitializing = true;
                QTimer::singleShot(10000, this, [=]() {
                    isInitializing = false;
                    qDebug() << "init complete (?)";
                    emit initComplete();
                });
            }
            emit IPbusStatusOK();
            isOnline = true;
            qDebug() << "online";
        }
    }

    virtual void sync() =0;

	void writeRegister(quint32 data, quint32 address, bool syncOnSuccess = true) {
        addTransaction(write, address, &data, 1);
		if (transceive() && syncOnSuccess) sync();
    }

	void setBit(quint8 n, quint32 address, bool syncOnSuccess = true) {
		addTransaction(RMWbits, address, masks(0xFFFFFFFF, 1 << n));
		if (transceive() && syncOnSuccess) sync();
	}

	void clearBit(quint8 n, quint32 address, bool syncOnSuccess = true) {
		addTransaction(RMWbits, address, masks(~(1 << n), 0x00000000));
		if (transceive() && syncOnSuccess) sync();
	}

	void writeNbits(quint32 data, quint32 address, quint8 nbits = 16, quint8 shift = 0, bool syncOnSuccess = true) {
        quint32 mask = (1 << nbits) - 1; //e.g. 0x00000FFF for nbits==12
        addTransaction(RMWbits, address, masks( ~quint32(mask << shift), quint32((data & mask) << shift) ));
		if (transceive() && syncOnSuccess) sync();
    }
};

#endif // IPBUSINTERFACE_H
