#ifndef IPBUSINTERFACE_H
#define IPBUSINTERFACE_H

#include <QtNetwork>
#include <QMutex>
#include "IPbusControlPacket.h"

class IPbusTarget: public QObject {
    Q_OBJECT
    const quint16 localport;
    QUdpSocket *qsocket = new QUdpSocket(this);
    const StatusPacket statusRequest;
    StatusPacket statusResponse;
    QMutex mutex;
    bool isLocked = false;

public:
    QString IPaddress = "172.20.75.180";
    bool isOnline = false;
    QTimer *updateTimer = new QTimer(this);
    quint16 updatePeriod_ms = 1000;

    IPbusTarget(quint16 lport = 0) : localport(lport) {
        qRegisterMetaType<errorType>("errorType");
        qRegisterMetaType<QAbstractSocket::SocketError>("socketError");
        updateTimer->setTimerType(Qt::PreciseTimer);
        connect(updateTimer, &QTimer::timeout, this, [=]() { if (isOnline) sync(); else checkStatus(); });
        connect(this, &IPbusTarget::error, this, [=]() {
            qDebug()<<(isLocked?"Locked":"not locked");
            updateTimer->stop();
        });
        if (!qsocket->bind(QHostAddress::AnyIPv4, localport)) qsocket->bind(QHostAddress::AnyIPv4);
        updateTimer->start(updatePeriod_ms);
    }

    quint32 readRegister(quint32 address) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(read, address, nullptr, 1);
        TransactionHeader *th = p.transactionsList.last().responseHeader;
        return transceive(p, false) && th->InfoCode == 0 ? quint32(*++th) : 0xFFFFFFFF;
    }

signals:
    void error(QString, errorType);
    void noResponse(QString message = "no response");
    void IPbusStatusOK();

protected:
    bool transceive(IPbusControlPacket &p, bool shouldResponseBeProcessed = true) { //send request, wait for response, receive it and check correctness
        if (!isOnline) return false;
        if (p.requestSize <= 1) {
            emit error("Empty request", logicError);
            return false;
        }
        QMutexLocker ml(&mutex);
        qint32 n = qint32(qsocket->write((char *)p.request, p.requestSize * wordSize));
		if (n < 0) {
            emit error("Socket write error: " + qsocket->errorString(), networkError);
			return false;
        } else if (n != p.requestSize * wordSize) {
            emit error("Sending packet failed", networkError);
			return false;
        } else if (!qsocket->waitForReadyRead(100) && !qsocket->hasPendingDatagrams()) {
            isOnline = false;
            emit noResponse();
			return false;
        }
        n = qsocket->readDatagram((char *)p.response, qsocket->pendingDatagramSize());
        if (n == 64 && p.response[0] == statusRequest.header) {
            if (!qsocket->hasPendingDatagrams() && !qsocket->waitForReadyRead(100) && !qsocket->hasPendingDatagrams()) {
                isOnline = false;
                emit noResponse();
                return false;
            }
            n = qsocket->readDatagram((char *)p.response, qsocket->pendingDatagramSize());
        }
        if (n == 0) {
            emit error("empty response, no IPbus on " + IPaddress, networkError);
            return false;
        } else if (n / wordSize > p.responseSize || p.response[0] != p.request[0] || n % wordSize > 0) {
            emit error(QString::asprintf("incorrect response (%d bytes)", n), networkError);
            return false;
        } else {
            p.responseSize = quint16(n / wordSize); //response can be shorter then expected if a transaction wasn't successful
            bool result = shouldResponseBeProcessed ? p.processResponse() : true;
            p.reset();
            return result;
        }
	}

public slots:
    void reconnect() {
        if (qsocket->state() == QAbstractSocket::ConnectedState) {
            qsocket->disconnectFromHost();
        }
        qsocket->connectToHost(IPaddress, 50001, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
        if (!qsocket->waitForConnected() && qsocket->state() != QAbstractSocket::ConnectedState) {
            isOnline = false;
            emit noResponse();
            return;
        }
        if (!updateTimer->isActive()) updateTimer->start(updatePeriod_ms);
        checkStatus();
    }

    void checkStatus() {
        qsocket->write((char *)&statusRequest, sizeof(statusRequest));
        if (!qsocket->waitForReadyRead(100) && !qsocket->hasPendingDatagrams()) {
            isOnline = false;
            emit noResponse();
        } else {
            qint32 n = qsocket->read((char *)&statusResponse, qsocket->pendingDatagramSize());
            if (n != sizeof (statusResponse) || statusResponse.header != statusRequest.header) {
                isOnline = false;
                emit noResponse(QString::asprintf("incorrect response (%d bytes). No IPbus?", n));
            } else {
                isOnline = true;
                emit IPbusStatusOK();
            }
        }
    }

    virtual void sync() =0;

    void writeRegister(quint32 address, quint32 data, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(write, address, &data, 1);
        if (transceive(p) && syncOnSuccess) sync();
    }

	void setBit(quint8 n, quint32 address, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(RMWbits, address, p.masks(0xFFFFFFFF, 1 << n));
        if (transceive(p) && syncOnSuccess) sync();
	}

	void clearBit(quint8 n, quint32 address, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(RMWbits, address, p.masks(~(1 << n), 0x00000000));
        if (transceive(p) && syncOnSuccess) sync();
	}

    void writeNbits(quint32 address, quint32 data, quint8 nbits = 16, quint8 shift = 0, bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        quint32 mask = (1 << nbits) - 1; //e.g. 0x00000FFF for nbits==12
        p.addTransaction(RMWbits, address, p.masks( ~quint32(mask << shift), quint32((data & mask) << shift) ));
        if (transceive(p) && syncOnSuccess) sync();
    }
};

#endif // IPBUSINTERFACE_H
