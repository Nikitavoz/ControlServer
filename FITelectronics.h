#ifndef FITELECTRONICS_H
#define FITELECTRONICS_H

#include "IPbusInterface.h"
#include "TCM.h"
#include "PM.h"

extern double systemClock_MHz; //40
extern double TDCunit_ps; // 13
extern double halfBC_ns; // 12.5
extern double phaseStepLaser_ns, phaseStep_ns;

class FITelectronics: public IPbusTarget, public DimCommandHandler {
    Q_OBJECT
public:
    TypeFITsubdetector subdetector;
    const quint16 TCMid;
    quint16 selectedBoard;
    bool isTCM = true; //is TCM selected
    DimServer DIMserver;
    QHash<DimCommand *, std::function<void(DimCommand *)>> allCommands;
    TypeTCM TCM;
    TypePM *curPM, allPMs[20] = { //PMs by link №
        TypePM(0x0200, "A0"),
        TypePM(0x0400, "A1"),
        TypePM(0x0600, "A2"),
        TypePM(0x0800, "A3"),
        TypePM(0x0A00, "A4"),
        TypePM(0x0C00, "A5"),
        TypePM(0x0E00, "A6"),
        TypePM(0x1000, "A7"),
        TypePM(0x1200, "A8"),
        TypePM(0x1400, "A9"),
        TypePM(0x1600, "C0"),
        TypePM(0x1800, "C1"),
        TypePM(0x1A00, "C2"),
        TypePM(0x1C00, "C3"),
        TypePM(0x1E00, "C4"),
        TypePM(0x2000, "C5"),
        TypePM(0x2200, "C6"),
        TypePM(0x2400, "C7"),
        TypePM(0x2600, "C8"),
        TypePM(0x2800, "C9"),
    };
    GBTunit *curGBTact = &TCM.act.GBT;
    GBTunit::ControlData *curGBTset = &TCM.set.GBT;
    QMap<quint16, TypePM *> PM;
	QTimer *countersTimer = new QTimer();
    //QTimer *fullSyncTimer = new QTimer();

    FITelectronics(TypeFITsubdetector sd): IPbusTarget(50006), subdetector(sd), TCMid(FIT[sd].TCMid) {
        TCM.set.GBT.RDH_FEE_ID = TCMid;
        for (quint8 i=0; i<10; ++i) {
            allPMs[i     ].FEEid = TCMid + 0xA0 + i;
            allPMs[i + 10].FEEid = TCMid + 0xC0 + i;
        }
        selectedBoard = TCMid;
        connect(countersTimer, &QTimer::timeout, this, &FITelectronics::readCountersFIFO);
        //connect(fullSyncTimer, &QTimer::timeout, this, &FITelectronics::fullSync);
        connect(this, &IPbusTarget::error, this, [=]() {
            if (countersTimer->isActive()) countersTimer->stop();
            //if (fullSyncTimer->isActive()) fullSyncTimer->stop();
        });
        connect(this, &IPbusTarget::noResponse, this, [=]() {
            if (countersTimer->isActive()) countersTimer->stop();
            //if (fullSyncTimer->isActive()) fullSyncTimer->stop();
        });
        connect(this, &IPbusTarget::IPbusStatusOK, this, [=]() { if (TCM.services.isEmpty()) createTCMservices(); });
        connect(this, &FITelectronics::linksStatusReady, this, [=]() {
            for (quint8 i=0; i<20; ++i) {
                if (TCM.act.PM_MASK_SPI >> i & 1) {
                    if (allPMs[i].services.isEmpty()) createPMservices(allPMs + i);
                } else {
                    deletePMservices(allPMs + i);
                }
            }
        });
        DIMserver.start(qPrintable(QString(FIT[sd].name) + "_DIM_SERVER"));
    }

    ~FITelectronics() {
        deleteTCMservices();
        for (quint8 i=0; i<20; ++i) deletePMservices(allPMs + i);
        DIMserver.stop();
    }

    void addCommand(QList<DimCommand *> &list, QString name, const char* format, std::function<void(DimCommand *)> function) {
        DimCommand *command = new DimCommand(qPrintable(name), format, this);
        list.append(command);
        allCommands.insert(command, function);
    }

    void createPMservices(TypePM *pm) {
        QString prefix = QString::asprintf("%s/PM%s/", FIT[subdetector].name, pm->name);
        pm->services.append(new DimService(qPrintable(prefix+"status/TEMP_BOARD"      ), "D", &pm->act.TEMP_BOARD      , 8));
        pm->services.append(new DimService(qPrintable(prefix+"status/TEMP_FPGA"       ), "D", &pm->act.TEMP_FPGA       , 8));
        pm->services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1V"      ), "D", &pm->act.VOLTAGE_1V      , 8));
        pm->services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1_8V"    ), "D", &pm->act.VOLTAGE_1_8V    , 8));
        pm->services.append(new DimService(qPrintable(prefix+"status/BOARD_TYPE"      ), "C:4",pm->act.BOARD_TYPE      , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/FW_TIME_MCU"     ), "I", &pm->act.FW_TIME_MCU     , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/FW_TIME_FPGA"    ), "I", &pm->act.FW_TIME_FPGA    , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/CH_BASELINES_NOK"), "I", &pm->act.CH_BASELINES_NOK, 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/SERIAL_NUM"      ), "S",  pm->act.pointer1 + (0xBD-0x7F) * wordSize + 1, 2));
        pm->services.append(new DimService(qPrintable(prefix+"control/CH_MASK_DATA""/actual"), "I", &pm->act.CH_MASK_DATA, 4));
        pm->services.append(new DimService(qPrintable(prefix+"control/CH_MASK_TRG" "/actual"), "I", &pm->act.CH_MASK_TRG , 4));
        pm->services.append(new DimService(qPrintable(prefix+"control/TRG_CNT_MODE""/actual"), "S", &pm->act.TRG_CNT_MODE, 2));
        pm->services.append(new DimService(qPrintable(prefix+"control/CFD_SATR"    "/actual"), "S", &pm->act.CFD_SATR    , 2));
        pm->services.append(new DimService(qPrintable(prefix+"control/OR_GATE"     "/actual"), "S", &pm->act.OR_GATE     , 2));

        pm->services.append(new DimService(qPrintable(prefix+"GBT/status/BITS"        ), "I", pm->act.GBT.Status.pointer, 4));

        pm->servicesNew.insert("CFD_SATR", new DimService(qPrintable(prefix+"control/CFD_SATR""/new"), "S", &pm->set.CFD_SATR    , 2));
        pm->servicesNew.insert("OR_GATE" , new DimService(qPrintable(prefix+"control/OR_GATE ""/new"), "S", &pm->set.OR_GATE     , 2));

        addCommand(pm->commands, prefix+"control/TRG_CNT_MODE""/apply", "S", [=](DimCommand *c) { apply_TRG_CNT_MODE(pm->FEEid, c->getShort()); });
        addCommand(pm->commands, prefix+"control/CFD_SATR"    "/apply", "S", [=](DimCommand * ) { apply_CFD_SATR(pm->FEEid); });
        addCommand(pm->commands, prefix+"control/CH_MASK_DATA""/apply", "I", [=](DimCommand *c) { pm->set.CH_MASK_DATA = c->getShort(); apply_CH_MASK_DATA(pm->FEEid); });
        addCommand(pm->commands, prefix+"control/CH_MASK_TRG" "/apply", "I", [=](DimCommand *c) {
            quint16 mask = c->getShort();
            for (quint8 i=0; i<12; ++i) pm->set.TIME_ALIGN[i].blockTriggers = !(1 << i & mask);
            apply_CH_MASK_TRG(pm->FEEid);
        });

        addCommand(pm->commands, prefix+"control/CFD_SATR"    "/set", "S", [=](DimCommand *c) { pm->set.CFD_SATR = c->getShort(); pm->servicesNew["CFD_SATR"]->updateService(); });
        addCommand(pm->commands, prefix+"control/OR_GATE"     "/set", "S", [=](DimCommand *c) { pm->set.OR_GATE  = c->getShort(); pm->servicesNew["OR_GATE" ]->updateService(); });


        for (quint8 iCh=0; iCh<12; ++iCh) {
            QString prefix = QString::asprintf("%s/PM%s/Ch%02d/", FIT[subdetector].name, pm->name, iCh + 1);
            char ch[6] = {0}; sprintf(ch, "Ch%02d", iCh + 1);
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC0_BASELINE"), "S", &pm->act.ADC_BASELINE[iCh][0]   , 2));
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC1_BASELINE"), "S", &pm->act.ADC_BASELINE[iCh][1]   , 2));
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC0_MEANAMPL"), "S", &pm->act.MEANAMPL    [iCh][0][0], 2));
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC1_MEANAMPL"), "S", &pm->act.MEANAMPL    [iCh][1][0], 2));
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC0_RMS"     ), "D", &pm->act.RMS_Ch      [iCh][0]   , 8));
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC1_RMS"     ), "D", &pm->act.RMS_Ch      [iCh][1]   , 8));
            pm->counters.services.append(new DimService(qPrintable(prefix+"status/CNT_CFD"), "I", &pm->counters.Ch[iCh].CFD, 4));
            pm->counters.services.append(new DimService(qPrintable(prefix+"status/CNT_TRG"), "I", &pm->counters.Ch[iCh].TRG, 4));
            pm->counters.services.append(new DimService(qPrintable(prefix+"status/CNT_RATE_CFD"), "D", &pm->counters.rateCh[iCh].CFD, 8));
            pm->counters.services.append(new DimService(qPrintable(prefix+"status/CNT_RATE_TRG"), "D", &pm->counters.rateCh[iCh].TRG, 8));
            pm->services.append(new DimService(qPrintable(prefix+"control/TIME_ALIGN"    "/actual"), "S", pm->act.TIME_ALIGN + iCh, 2));
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC0_RANGE"     "/actual"), "S", &pm->act.ADC_RANGE   [iCh][0] , 2));
            pm->services.append(new DimService(qPrintable(prefix+"status/ADC1_RANGE"     "/actual"), "S", &pm->act.ADC_RANGE   [iCh][1] , 2));
            pm->services.append(new DimService(qPrintable(prefix+"control/CFD_THRESHOLD" "/actual"), "S", (qint16 *)&pm->act.Ch[iCh]    , 2));
            pm->services.append(new DimService(qPrintable(prefix+"control/CFD_ZERO"      "/actual"), "S", (qint16 *)&pm->act.Ch[iCh] + 2, 2));
            pm->services.append(new DimService(qPrintable(prefix+"control/ADC_ZERO"      "/actual"), "S", (qint16 *)&pm->act.Ch[iCh] + 4, 2));
            pm->services.append(new DimService(qPrintable(prefix+"control/ADC_DELAY"     "/actual"), "S", (qint16 *)&pm->act.Ch[iCh] + 6, 2));
            pm->services.append(new DimService(qPrintable(prefix+"control/THRESHOLD_CALIBR/actual"), "S", (qint16 *)&pm->act.THRESHOLD_CALIBR[iCh], 2));

            pm->servicesNew.insert(QString(ch)+"/TIME_ALIGN"      , new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/TIME_ALIGN"    "/new", FIT[subdetector].name, pm->name, ch)), "S", pm->set.TIME_ALIGN + iCh, 2));
            pm->servicesNew.insert(QString(ch)+"/ADC0_RANGE"      , new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/ADC0_RANGE"    "/new", FIT[subdetector].name, pm->name, ch)), "S", &pm->set.ADC_RANGE   [iCh][0] , 2));
            pm->servicesNew.insert(QString(ch)+"/ADC1_RANGE"      , new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/ADC1_RANGE"    "/new", FIT[subdetector].name, pm->name, ch)), "S", &pm->set.ADC_RANGE   [iCh][1] , 2));
            pm->servicesNew.insert(QString(ch)+"/CFD_THRESHOLD"   , new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/CFD_THRESHOLD" "/new", FIT[subdetector].name, pm->name, ch)), "S", (qint16 *)&pm->set.Ch[iCh]    , 2));
            pm->servicesNew.insert(QString(ch)+"/CFD_ZERO"        , new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/CFD_ZERO"      "/new", FIT[subdetector].name, pm->name, ch)), "S", (qint16 *)&pm->set.Ch[iCh] + 2, 2));
            pm->servicesNew.insert(QString(ch)+"/ADC_ZERO"        , new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/ADC_ZERO"      "/new", FIT[subdetector].name, pm->name, ch)), "S", (qint16 *)&pm->set.Ch[iCh] + 4, 2));
            pm->servicesNew.insert(QString(ch)+"/ADC_DELAY"       , new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/ADC_DELAY"     "/new", FIT[subdetector].name, pm->name, ch)), "S", (qint16 *)&pm->set.Ch[iCh] + 6, 2));
            pm->servicesNew.insert(QString(ch)+"/THRESHOLD_CALIBR", new DimService(qPrintable(QString::asprintf("%s/PM%s/%s/control/THRESHOLD_CALIBR/new", FIT[subdetector].name, pm->name, ch)), "S", (qint16 *)&pm->set.THRESHOLD_CALIBR[iCh], 2));

            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/TIME_ALIGN"    "/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.TIME_ALIGN[iCh].value = c->getShort(); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC0_RANGE"    "/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.ADC_RANGE[iCh][0]     = c->getShort(); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC1_RANGE"    "/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.ADC_RANGE[iCh][1]     = c->getShort(); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/CFD_THRESHOLD" "/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.Ch[iCh].CFD_THRESHOLD = c->getShort(); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/CFD_ZERO"      "/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.Ch[iCh].CFD_ZERO      = c->getShort(); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC_ZERO"      "/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.Ch[iCh].ADC_ZERO      = c->getShort(); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC_DELAY"     "/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.Ch[iCh].ADC_DELAY     = c->getShort(); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/THRESHOLD_CALIBR/set", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *c) { pm->set.THRESHOLD_CALIBR[iCh] = c->getShort(); });

            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/TIME_ALIGN"    "/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_TIME_ALIGN      (pm->FEEid, iCh+1); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC0_RANGE"    "/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_ADC0_RANGE      (pm->FEEid, iCh+1); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC1_RANGE"    "/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_ADC1_RANGE      (pm->FEEid, iCh+1); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/CFD_THRESHOLD" "/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_CFD_THRESHOLD   (pm->FEEid, iCh+1); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/CFD_ZERO"      "/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_CFD_ZERO        (pm->FEEid, iCh+1); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC_ZERO"      "/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_ADC_ZERO        (pm->FEEid, iCh+1); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/ADC_DELAY"     "/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_ADC_DELAY       (pm->FEEid, iCh+1); });
            addCommand(pm->commands, QString::asprintf("%s/PM%s/%s/control/THRESHOLD_CALIBR/apply", FIT[subdetector].name, pm->name, ch), "S", [=](DimCommand *) { apply_THRESHOLD_CALIBR(pm->FEEid, iCh+1); });
        }
    }

    void createTCMservices() {
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/TEMP_BOARD"  , FIT[subdetector].name)), "D", &TCM.act.TEMP_BOARD                   , 8));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/TEMP_FPGA"   , FIT[subdetector].name)), "D", &TCM.act.TEMP_FPGA                    , 8));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/VOLTAGE_1V"  , FIT[subdetector].name)), "D", &TCM.act.VOLTAGE_1V                   , 8));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/VOLTAGE_1_8V", FIT[subdetector].name)), "D", &TCM.act.VOLTAGE_1_8V                 , 8));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/SERIAL_NUM"  , FIT[subdetector].name)), "S",  TCM.act.pointer0 + 0x7 * wordSize + 1, 2));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/BOARD_TYPE"  , FIT[subdetector].name)), "C:4",TCM.act.BOARD_TYPE                   , 4));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/FW_TIME_MCU" , FIT[subdetector].name)), "I", &TCM.act.FW_TIME_MCU                  , 4));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/FW_TIME_FPGA", FIT[subdetector].name)), "I", &TCM.act.FW_TIME_FPGA                 , 4));
        TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/PM_MASK_SPI" , FIT[subdetector].name)), "I", &TCM.act.PM_MASK_SPI                  , 4));
        for (quint8 iPM=0; iPM<10; ++iPM) {
            TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/PM%s/status/TRG_SYNC", FIT[subdetector].name, allPMs[iPM   ].name)), "I", TCM.act.TRG_SYNC_A + iPM, 4));
            TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/PM%s/status/TRG_SYNC", FIT[subdetector].name, allPMs[iPM+10].name)), "I", TCM.act.TRG_SYNC_C + iPM, 4));
        }
    }

    void deletePMservices(TypePM *pm) {
        foreach (DimService *s, pm->services + pm->counters.services) delete s;
        pm->services.clear();
        pm->counters.services.clear();
        foreach (DimService *s, pm->servicesNew) delete s;
        pm->servicesNew.clear();
        foreach (DimCommand *c, pm->commands) { allCommands.remove(c); delete c; }
        pm->commands.clear();
    }

    void deleteTCMservices() {
        foreach (DimService *s, TCM.services + TCM.counters.services) delete s;
        TCM.services.clear();
        TCM.counters.services.clear();
//        foreach (DimService *s, TCM.servicesNew) delete s;
//        TCM.servicesNew.clear();
//        foreach (DimCommand *c, TCM.commands) { allCommands.remove(c); delete c; }
//        TCM.commands.clear();
    }

    void commandHandler() {
        DimCommand *cmd = getCommand();
        allCommands[cmd](cmd);

        //(void (*)(DimCommand *)) (allCommands[cmd]) (cmd);
//        QStringList tab = QString(cmd->getName()).split('/');
//        if (tab.last() == "apply") {
//            qint64 val = 0;
//            memcpy(&val, cmd->getData(), cmd->getSize());
//            quint16 FEEid = TCMid;
//            if (tab[1].startsWith("PM")) FEEid += tab[1].rightRef(2).toUShort(nullptr, 16);
//            qDebug(qPrintable("Setting " + tab[tab.size()-2] + " = %d for %04X"), val, FEEid);
//            //qDebug() << "Setting" << tab[tab.size()-2] << "=" << val << "for" << FEEid;
//            writeParameter(tab[tab.size()-2], val, FEEid, tab[2].startsWith("Ch") ? tab[2].rightRef(2).toUShort() : 0);
//        }
    }

signals:
    void linksStatusReady();
    void valuesReady();
    void countersReady();

public slots:

    void clearFIFOs() {
		quint32 load = readRegister(TypeTCM::Counters::addressFIFOload);
		while (load) {
			addTransaction(nonIncrementingRead, TypeTCM::Counters::addressFIFO, nullptr, load > 255 ? 255 : load);
			addTransaction(read, TypeTCM::Counters::addressFIFOload, &load);
			transceive();
		}
		foreach (TypePM *pm, PM) {
			load = readRegister(pm->baseAddress + TypePM::Counters::addressFIFOload);
			while (load) {
				addTransaction(nonIncrementingRead, pm->baseAddress + TypePM::Counters::addressFIFO, nullptr, load > 255 ? 255 : load);
				addTransaction(read, pm->baseAddress + TypePM::Counters::addressFIFOload, &load);
				transceive();
			}
		}
    }

    void apply_COUNTERS_UPD_RATE(quint8 val) {
        countersTimer->stop();
        writeParameter("COUNTERS_UPD_RATE", 0, TCMid);
        clearFIFOs();
        if (val <= 7) {
            TCM.act.COUNTERS_UPD_RATE = val;
            if (val > 0) {
                writeParameter("COUNTERS_UPD_RATE", val, TCMid);
                countersTimer->start(countersUpdatePeriod_ms[val] / 2);
            }
        } else emit error("Wrong COUNTERS_UPD_RATE value: " + QString::number(val), logicError);
    }

    void checkPMlinks() {
        addTransaction(read, TCMparameters["PM_MASK_SPI"].address, &TCM.act.PM_MASK_SPI);
        addTransaction(read, TCMparameters["CH_MASK_A"].address, dt);
        addTransaction(read, TCMparameters["CH_MASK_C"].address, dt + 1);
        if (transceive()) {
            TCM.set.CH_MASK_A = dt[0];
            TCM.set.CH_MASK_C = dt[1];
        } else return;
        PM.clear();
        for (quint8 i=0; i<20; ++i) {
            if (!(TCM.act.PM_MASK_SPI >> i & 1)) setBit(i, TCMparameters["PM_MASK_SPI"].address, false);
            addTransaction(read, allPMs[i].baseAddress + 0xFD, &allPMs[i].act.voltage1);
            addTransaction(read, allPMs[i].baseAddress + 0x7F, allPMs[i].act.registers1); //board status register
            if (!transceive()) return;
            if (allPMs[i].act.voltage1 == 0xFFFFFFFF) { //SPI error
                clearBit(i, TCMparameters["PM_MASK_SPI"].address, false);
            } else {
                TCM.act.PM_MASK_SPI |= 1 << i;
                PM.insert(allPMs[i].set.GBT.RDH_FEE_ID, allPMs + i);
                if (i > 9) TCM.set.CH_MASK_C |= 1 << (i - 10);
                else       TCM.set.CH_MASK_A |= 1 << i;
            }
        }
        quint16 FEEid = TCM.set.GBT.RDH_FEE_ID;
        for (quint8 j=0; j<GBTunit::controlSize; ++j) TCM.set.GBT.registers[j] = GBTunit::defaults[j];
        TCM.set.GBT.RDH_FEE_ID = FEEid;
        addTransaction(write, GBTunit::controlAddress, TCM.set.GBT.registers, GBTunit::controlSize);
        foreach (TypePM *pm, PM) {
            FEEid = pm->set.GBT.RDH_FEE_ID;
            for (quint8 j=0; j<GBTunit::controlSize; ++j) pm->set.GBT.registers[j] = GBTunit::defaults[j];
            pm->set.GBT.RDH_FEE_ID = FEEid;
            addTransaction(write, pm->baseAddress + GBTunit::controlAddress, pm->set.GBT.registers, GBTunit::controlSize);
        }
        addWordToWrite(TCMparameters["CH_MASK_A"].address, TCM.set.CH_MASK_A);
        addWordToWrite(TCMparameters["CH_MASK_C"].address, TCM.set.CH_MASK_C);
        if (transceive()) emit linksStatusReady();
    }

    void defaultGBT() {

    }

    void writeParameter(QString name, qint64 val, quint16 FEEid, quint8 Ch = 0) {
        if ( !GBTparameters.contains(name) && (FEEid == TCMid ? !TCMparameters.contains(name) : !PMparameters.contains(name)) ) {
            emit error("'" + name + "' - no such parameter", logicError);
            return;
        }
        Parameter p(GBTparameters.contains(name) ? GBTparameters[name] : (FEEid == TCMid ? TCMparameters[name] : PMparameters[name]));
        quint16 address = p.address + (FEEid == TCMid ? 0 : PM[FEEid]->baseAddress + Ch * p.interval);
        if (p.bitwidth == 32)
            writeRegister(val, address);
        else if (p.bitwidth == 1)
            val != 0 ? setBit(p.bitshift, address) : clearBit(p.bitshift, address);
        else if (p.bitwidth == 64) {
            addTransaction(write, address, (quint32 *)&val, 2);
            if (transceive()) sync();
        } else
            writeNbits(val, address, p.bitwidth, p.bitshift);
    }

    void readCountersFIFO() {
		if (readRegister(TypeTCM::Counters::addressFIFOload) >= TypeTCM::Counters::number) {
			addTransaction(nonIncrementingRead, TypeTCM::Counters::addressFIFO, TCM.counters.New, TypeTCM::Counters::number);
			foreach (TypePM *pm, PM) {
				if (maxPacket - responseSize <= TypePM::Counters::number) transceive();
				addTransaction(nonIncrementingRead, pm->baseAddress + TypePM::Counters::addressFIFO, pm->counters.New, TypePM::Counters::number);
			}
			transceive();
			quint16 time_ms = countersUpdatePeriod_ms[TCM.act.COUNTERS_UPD_RATE];
            TCM.counters.oldTime = QDateTime::currentDateTime();
			for (quint8 i=0; i<TypeTCM::Counters::number; ++i) {
				TCM.counters.rate[i] = (TCM.counters.New[i] - TCM.counters.Old[i]) * 1000. / time_ms;
				TCM.counters.Old[i] = TCM.counters.New[i];
			}
			foreach (TypePM *pm, PM) {
                pm->counters.oldTime = QDateTime::currentDateTime();
				for (quint8 i=0; i<TypePM::Counters::number; ++i) {
					pm->counters.rate[i] = (pm->counters.New[i] - pm->counters.Old[i]) * 1000. / time_ms;
					pm->counters.Old[i] = pm->counters.New[i];
				}
                foreach (DimService *s, pm->counters.services) s->updateService();
			}
			emit countersReady();
		}
    }

    void readCountersDirectly() {
        addTransaction(read, TypeTCM::Counters::addressDirect, TCM.counters.New, TypeTCM::Counters::number);
        if (transceive()) {
            TCM.counters.newTime = QDateTime::currentDateTime();
            quint32 time_ms = TCM.counters.oldTime.msecsTo(TCM.counters.newTime);
            for (quint8 i=0; i<TypeTCM::Counters::number; ++i) {
				TCM.counters.rate[i] = (TCM.counters.New[i] > TCM.counters.Old[i]) ? (TCM.counters.New[i] - TCM.counters.Old[i]) * 1000. / time_ms : 0;
                TCM.counters.Old[i] = TCM.counters.New[i];
            }
            TCM.counters.oldTime = TCM.counters.newTime;
        }
        foreach (TypePM *pm, PM) {
            addTransaction(read, pm->baseAddress + TypePM::Counters::addressDirect, pm->counters.New, TypePM::Counters::number);
            if (!transceive()) return;
            pm->counters.newTime = QDateTime::currentDateTime();
            quint32 time_ms = pm->counters.oldTime.msecsTo(pm->counters.newTime);
            for (quint8 i=0; i<TypePM::Counters::number; ++i) {
				pm->counters.rate[i] = (pm->counters.New[i] > pm->counters.Old[i]) ? (pm->counters.New[i] - pm->counters.Old[i]) * 1000. / time_ms : 0;
                pm->counters.Old[i] = pm->counters.New[i];
            }
			pm->counters.oldTime = pm->counters.newTime;
            foreach (DimService *s, pm->counters.services) s->updateService();
        }
        emit countersReady();
    }



    void addSystemValuesToRead() {
        addTransaction(read, TypeTCM::ActualValues::block0addr, TCM.act.registers0, TypeTCM::ActualValues::block0size);
        addTransaction(read, TypeTCM::ActualValues::block1addr, TCM.act.registers1, TypeTCM::ActualValues::block1size);
        addTransaction(read, TCMparameters["COUNTERS_UPD_RATE"].address, &TCM.act.COUNTERS_UPD_RATE);
    }

    void addTCMvaluesToRead() {
        addTransaction(read, GBTunit::controlAddress, TCM.act.GBT.Control.registers, GBTunit::controlSize);
        addTransaction(read, GBTunit:: statusAddress, TCM.act.GBT.Status .registers, GBTunit:: statusSize);
        addTransaction(read, 0xF7, (quint32 *)&TCM.act.FW_TIME_MCU );
        addTransaction(read, 0xFF, (quint32 *)&TCM.act.FW_TIME_FPGA);
        addTransaction(read, TypeTCM::ActualValues::block2addr, TCM.act.registers2, TypeTCM::ActualValues::block2size);
        addTransaction(read, TypeTCM::ActualValues::block3addr, TCM.act.registers3, TypeTCM::ActualValues::block3size);
    }

    void addPMvaluesToRead(TypePM *pm) {
        addTransaction(read, pm->baseAddress + GBTunit::controlAddress, pm->act.GBT.Control.registers, GBTunit::controlSize);
        addTransaction(read, pm->baseAddress + GBTunit:: statusAddress, pm->act.GBT.Status .registers, GBTunit:: statusSize);
        addTransaction(read, pm->baseAddress + 0xF7, (quint32 *)&pm->act.FW_TIME_MCU );
        addTransaction(read, pm->baseAddress + 0xFF, (quint32 *)&pm->act.FW_TIME_FPGA);
        addTransaction(read, pm->baseAddress + TypePM::ActualValues::block0addr, pm->act.registers0, TypePM::ActualValues::block0size);
        addTransaction(read, pm->baseAddress + TypePM::ActualValues::block1addr, pm->act.registers1, TypePM::ActualValues::block1size);
        addTransaction(read, pm->baseAddress + TypePM::ActualValues::block2addr, pm->act.registers2, TypePM::ActualValues::block2size);
    }

    bool read1PM(TypePM *pm) {
        addPMvaluesToRead(pm);
        if (!transceive()) return false;
        if (pm->act.voltage1 == 0xFFFFFFFF && !TCM.act.systemRestarted) {
            clearBit(pm->baseAddress / 0x200 - 1, 0x1E, false);
            PM.remove(pm->set.GBT.RDH_FEE_ID);
            emit linksStatusReady();
        } else {
            pm->act.calculateValues();
            foreach (DimService *s, pm->services) s->updateService();
        }
        return true;
    }

    void fullSync() { //read all available values
        if (!isTCM) {
            addTCMvaluesToRead();
            if (!transceive()) return;
        }
        foreach (TypePM *pm, PM) if (isTCM || pm != curPM) if (!read1PM(pm)) break;
    }

//    void sync() { //read actual values
//        addSystemValuesToRead();
//        if (isTCM) {
//            addTCMvaluesToRead();
//            if (!transceive()) return;
//        } else if (!read1PM(curPM)) return;
//        TCM.act.COUNTERS_UPD_RATE &= 0b111;
//        TCM.act.calculateValues();
//        foreach (DimService *s, TCM.services) s->updateService();
//        emit valuesReady();
//        if (TCM.act.COUNTERS_UPD_RATE == 0) readCountersDirectly();
//    }

        void sync() { //read actual values
            addSystemValuesToRead();
            addTCMvaluesToRead();
            if (!transceive()) return;
            //TCM.act.COUNTERS_UPD_RATE &= 0b111;
            TCM.act.calculateValues();
            foreach (DimService *s, TCM.services) s->updateService();
            foreach (TypePM *pm, PM) if (!read1PM(pm)) return;
            emit valuesReady();
            if (TCM.act.COUNTERS_UPD_RATE == 0) readCountersDirectly();
        }

    void apply_TG_SEND_SINGLE(quint16 FEEid, quint32 value) {
        quint32 address = (FEEid == TCMid ? 0 : PM[FEEid]->baseAddress) + GBTunit::controlAddress;
        quint32 data[3] = {0x0, value, 0x0};
        addTransaction(nonIncrementingWrite, address, data, 3);
        if (transceive()) sync();
    }

    void reset(quint16 FEEid, quint8 RB_position, bool syncOnSuccess = true) {
        quint32 address = (FEEid == TCMid ? 0x0 : PM[FEEid]->baseAddress) + GBTunit::controlAddress;
        addTransaction(RMWbits, address, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        addTransaction(RMWbits, address, masks(0xFFFFFFFF, 1 << RB_position)); //set specific bit, e.g. 0x00000800 for RS_GBTerrors
        addTransaction(RMWbits, address, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        if (transceive() && syncOnSuccess) sync();
    }

    void apply_RESET_ORBIT_SYNC           (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_orbitSync            , syncOnSuccess); }
    void apply_RESET_DROPPING_HIT_COUNTERS(quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_droppingHitCounters  , syncOnSuccess); }
    void apply_RESET_GEN_BUNCH_OFFSET     (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_generatorsBunchOffset, syncOnSuccess); }
    void apply_RESET_GBT_ERRORS           (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_GBTerrors            , syncOnSuccess); }
    void apply_RESET_GBT                  (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_GBT                  , syncOnSuccess); }
    void apply_RESET_RX_PHASE_ERROR       (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_RXphaseError         , syncOnSuccess); }

//    void apply_SEND_READOUT_COMMAND(quint16 FEEid, quint8 RO_cmd) {
//        quint32 address = (FEEid == TCMid ? 0 : PM[FEEid]->baseAddress) + GBTunit::controlAddress;
//        addTransaction(RMWbits, address, masks(0xFFF0FFFF, 0x00000000)); //clear all readout command bits
//        addTransaction(RMWbits, address, masks(0xFFFFFFFF, quint32(RO_cmd << 16))); //set specific bit, e.g. 0x00040000 for RC_EOT
//        addTransaction(RMWbits, address, masks(0xFFF0FFFF, 0x00000000)); //clear all readout command bits
//        if (transceive()) sync();
//    }

    void apply_TG_CTP_EMUL_MODE(quint16 FEEid, quint8 RO_mode) { writeParameter("TG_CTP_EMUL_MODE", RO_mode, FEEid); }

    void apply_DG_MODE(quint16 FEEid, quint8 DG_mode) {
        (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_MODE = DG_mode;
        writeParameter("DG_MODE", DG_mode, FEEid);
    }

    void apply_TG_MODE(quint16 FEEid, quint8 TG_mode) {
        (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_MODE = TG_mode;
        writeParameter("TG_MODE", TG_mode, FEEid);
    }

    void apply_TG_PATTERN(quint16 FEEid) { //TG_PATTERN_0 and TG_PATTERN_1
        if (FEEid == TCMid)
            addTransaction(write, GBTparameters["TG_PATTERN_1"].address, &TCM.set.GBT.TG_PATTERN_1, 2);
        else
            addTransaction(write, PM[FEEid]->baseAddress + GBTparameters["TG_PATTERN_1"].address, &PM[FEEid]->set.GBT.TG_PATTERN_1, 2);
        if (transceive()) sync();
    }

    void apply_TG_PATTERN_0         (quint16 FEEid) { writeParameter("TG_PATTERN_0"         , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_PATTERN_0         , FEEid); }
    void apply_TG_PATTERN_1         (quint16 FEEid) { writeParameter("TG_PATTERN_1"         , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_PATTERN_1         , FEEid); }
    void apply_TG_CONT_VALUE        (quint16 FEEid) { writeParameter("TG_CONT_VALUE"        , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_CONT_VALUE        , FEEid); }
    void apply_TG_BUNCH_FREQ        (quint16 FEEid) { writeParameter("TG_BUNCH_FREQ"        , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_BUNCH_FREQ        , FEEid); }
    void apply_TG_FREQ_OFFSET       (quint16 FEEid) { writeParameter("TG_FREQ_OFFSET"       , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_FREQ_OFFSET       , FEEid); }
    void apply_DG_TRG_RESPOND_MASK  (quint16 FEEid) { writeParameter("DG_TRG_RESPOND_MASK"  , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_TRG_RESPOND_MASK  , FEEid); }
    void apply_DG_BUNCH_PATTERN     (quint16 FEEid) { writeParameter("DG_BUNCH_PATTERN"     , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_BUNCH_PATTERN     , FEEid); }
    void apply_DG_BUNCH_FREQ        (quint16 FEEid) { writeParameter("DG_BUNCH_FREQ"        , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_BUNCH_FREQ        , FEEid); }
    void apply_DG_FREQ_OFFSET       (quint16 FEEid) { writeParameter("DG_FREQ_OFFSET"       , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_FREQ_OFFSET       , FEEid); }
    void apply_RDH_FEE_ID           (quint16 FEEid) { writeParameter("RDH_FEE_ID"           , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).RDH_FEE_ID           , FEEid); }
    void apply_RDH_PAR              (quint16 FEEid) { writeParameter("RDH_PAR"              , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).RDH_PAR              , FEEid); }
    void apply_RDH_MAX_PAYLOAD      (quint16 FEEid) { writeParameter("RDH_MAX_PAYLOAD"      , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).RDH_MAX_PAYLOAD      , FEEid); }
    void apply_RDH_DET_FIELD        (quint16 FEEid) { writeParameter("RDH_DET_FIELD"        , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).RDH_DET_FIELD        , FEEid); }
    void apply_CRU_TRG_COMPARE_DELAY(quint16 FEEid) { writeParameter("CRU_TRG_COMPARE_DELAY", (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).CRU_TRG_COMPARE_DELAY, FEEid); }
    void apply_BCID_DELAY           (quint16 FEEid) { writeParameter("BCID_DELAY"           , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).BCID_DELAY           , FEEid); }
    void apply_DATA_SEL_TRG_MASK    (quint16 FEEid) { writeParameter("DATA_SEL_TRG_MASK"    , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DATA_SEL_TRG_MASK    , FEEid); }

    void apply_HB_RESPONSE (quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).HB_RESPONSE  = on; writeParameter("HB_RESPONSE" , on, FEEid); }
    void apply_READOUT_LOCK(quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).READOUT_LOCK = on; writeParameter("READOUT_LOCK", on, FEEid); }
    void apply_BYPASS_MODE (quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).BYPASS_MODE  = on; writeParameter("BYPASS_MODE" , on, FEEid); }

    void apply_SwChOn(quint16 FEEid, quint8 Ch) {
        if (FEEid == TCMid) {
            if (Ch >= 0xC0 && Ch <= 0xC9) {
                TCM.set.CH_MASK_C |= 1 << (Ch - 0xC0);
                setBit(Ch - 0xC0, TCMparameters["CH_MASK_C"].address);
            } else if (Ch >= 0xA0 && Ch <= 0xA9) {
                TCM.set.CH_MASK_A |= 1 << (Ch - 0xA0);
                setBit(Ch - 0xA0, TCMparameters["CH_MASK_A"].address);
            } else
                emit error("invalid channel: " + QString::asprintf("%02X", Ch), logicError);
        } else { //PM
            --Ch;
            if (Ch < 12) {
                PM[FEEid]->set.CH_MASK_DATA |= 1 << Ch;
                setBit(Ch, PM[FEEid]->baseAddress + PMparameters["CH_MASK_DATA"].address);
            } else
                emit error("invalid channel: " + QString::number(Ch + 1), logicError);
        }
    }

    void apply_SwChOff(quint16 FEEid, quint8 Ch) {
        if (FEEid == TCMid) {
            if (Ch >= 0xC0 && Ch <= 0xC9) {
                TCM.set.CH_MASK_C &= 1 << ~(Ch - 0xC0);
                clearBit(Ch - 0xC0, TCMparameters["CH_MASK_C"].address);
            } else if (Ch >= 0xA0 && Ch <= 0xA9) {
                TCM.set.CH_MASK_A &= 1 << ~(Ch - 0xA0);
                clearBit(Ch - 0xA0, TCMparameters["CH_MASK_A"].address);
            } else
                emit error("invalid channel: " + QString::asprintf("%02X", Ch), logicError);
        } else { //PM
            --Ch;
            if (Ch < 12) {
                PM[FEEid]->set.CH_MASK_DATA &= ~(1 << Ch);
                clearBit(Ch, PM[FEEid]->baseAddress + PMparameters["CH_MASK_DATA"].address);
            } else
                emit error("invalid channel: " + QString::number(Ch + 1), logicError);
        }
    }

    void apply_RESET_COUNTERS(quint16 FEEid) {
        if (FEEid == TCMid) {
            TCM.counters.oldTime = QDateTime::currentDateTime();
            for (quint8 i=0; i<TypeTCM::Counters::number; ++i) TCM.counters.Old[i] = 0;
            setBit(9, 0xF);
        } else { //PM
            PM[FEEid]->counters.oldTime = QDateTime::currentDateTime();
            for (quint8 i=0; i<TypePM::Counters::number; ++i) PM[FEEid]->counters.Old[i] = 0;
            setBit(9, 0x7F + PM[FEEid]->baseAddress);
        }
    }
    void apply_RESET_SYSTEM(bool forceLocalClock = false) { writeRegister(forceLocalClock ? 0xC00 : 0x800, 0xF); }
    void apply_RESET_ERRORS() {
        addTransaction(RMWbits, GBTunit::controlAddress, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        addTransaction(RMWbits, GBTunit::controlAddress, masks(0xFFFFFFFF, 1 << GBTunit::RB_GBTerrors | 1 << GBTunit::RB_RXphaseError));
        addTransaction(RMWbits, GBTunit::controlAddress, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        foreach (TypePM *pm, PM) {
            quint32 address = pm->baseAddress + GBTunit::controlAddress;
            addTransaction(RMWbits, address, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
            addTransaction(RMWbits, address, masks(0xFFFFFFFF, 1 << GBTunit::RB_GBTerrors | 1 << GBTunit::RB_RXphaseError));
            addTransaction(RMWbits, address, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        }
        writeRegister(0x4, 0xF, true);
    }

    void apply_ADC0_RANGE      (quint16 FEEid, quint8 Ch) { writeParameter("ADC0_RANGE"      , PM[FEEid]->set.ADC_RANGE[Ch-1][0]    , FEEid, Ch-1); }
    void apply_ADC1_RANGE      (quint16 FEEid, quint8 Ch) { writeParameter("ADC1_RANGE"      , PM[FEEid]->set.ADC_RANGE[Ch-1][1]    , FEEid, Ch-1); }
    void apply_ADC_ZERO        (quint16 FEEid, quint8 Ch) { writeParameter("ADC_ZERO"        , PM[FEEid]->set.Ch[Ch-1].ADC_ZERO     , FEEid, Ch-1); }
    void apply_CFD_ZERO        (quint16 FEEid, quint8 Ch) { writeParameter("CFD_ZERO"        , PM[FEEid]->set.Ch[Ch-1].CFD_ZERO     , FEEid, Ch-1); }
    void apply_ADC_DELAY       (quint16 FEEid, quint8 Ch) { writeParameter("ADC_DELAY"       , PM[FEEid]->set.Ch[Ch-1].ADC_DELAY    , FEEid, Ch-1); }
    void apply_CFD_THRESHOLD   (quint16 FEEid, quint8 Ch) { writeParameter("CFD_THRESHOLD"   , PM[FEEid]->set.Ch[Ch-1].CFD_THRESHOLD, FEEid, Ch-1); }
    void apply_TIME_ALIGN      (quint16 FEEid, quint8 Ch) { writeParameter("TIME_ALIGN"      , PM[FEEid]->set.TIME_ALIGN[Ch-1].value, FEEid, Ch-1); }
    void apply_THRESHOLD_CALIBR(quint16 FEEid, quint8 Ch) { writeParameter("THRESHOLD_CALIBR", PM[FEEid]->set.THRESHOLD_CALIBR[Ch-1], FEEid, Ch-1); }

    void apply_LASER_DIVIDER() { writeParameter("LASER_DIVIDER", TCM.set.LASER_DIVIDER, TCMid); }
    void apply_LASER_SOURCE(bool on) { TCM.set.LASER_SOURCE = on; writeParameter("LASER_SOURCE", on, TCMid); }
    void apply_LaserPattern() {
        addTransaction(write, TCMparameters["LASER_PATTERN_1"].address, &TCM.set.LASER_PATTERN_1, 2);
        transceive();
    }
    void apply_LASER_PATTERN_0() { writeParameter("LASER_PATTERN_0", TCM.set.LASER_PATTERN_0, TCMid); }
    void apply_LASER_PATTERN_1() { writeParameter("LASER_PATTERN_1", TCM.set.LASER_PATTERN_1, TCMid); }
    void apply_SwLaserPatternBit(quint8 bit, bool on) {
        if (on) setBit(bit % 32, TCMparameters[bit < 32 ? "LASER_PATTERN_0" : "LASER_PATTERN_1"].address);
        else  clearBit(bit % 32, TCMparameters[bit < 32 ? "LASER_PATTERN_0" : "LASER_PATTERN_1"].address);
    }
	void apply_attenSteps() { writeParameter("attenSteps", TCM.set.attenSteps, TCMid); }
    void apply_LASER_DELAY() { writeParameter("LASER_DELAY", TCM.set.LASER_DELAY, TCMid); }
    void apply_DELAY_A() { writeParameter("DELAY_A", TCM.set.DELAY_A, TCMid); }
    void apply_DELAY_C() { writeParameter("DELAY_C", TCM.set.DELAY_C, TCMid); }
    void apply_CH_MASK_A() { writeParameter("CH_MASK_A", TCM.set.CH_MASK_A, TCMid); }
    void apply_CH_MASK_C() { writeParameter("CH_MASK_C", TCM.set.CH_MASK_C, TCMid); }

    void apply_ORA_ENABLED(bool on) { TCM.set.ORA_ENABLED = on; writeParameter("ORA_ENABLED", on, TCMid); }
    void apply_ORC_ENABLED(bool on) { TCM.set.ORC_ENABLED = on; writeParameter("ORC_ENABLED", on, TCMid); }
    void apply_SC_ENABLED (bool on) { TCM.set. SC_ENABLED = on; writeParameter( "SC_ENABLED", on, TCMid); }
    void apply_C_ENABLED  (bool on) { TCM.set.  C_ENABLED = on; writeParameter(  "C_ENABLED", on, TCMid); }
    void apply_V_ENABLED  (bool on) { TCM.set.  V_ENABLED = on; writeParameter(  "V_ENABLED", on, TCMid); }
    void apply_EXTENDED_READOUT(bool on) { TCM.set.EXTENDED_READOUT = on; writeParameter("EXTENDED_READOUT", on, TCMid); }
    void apply_ADD_C_DELAY(bool on) { TCM.set.ADD_C_DELAY = on; writeParameter("ADD_C_DELAY", on, TCMid); }
    void apply_C_SC_TRG_MODE(quint8 mode) { TCM.set.C_SC_TRG_MODE = mode; writeParameter("C_SC_TRG_MODE", mode, TCMid); }
    void apply_SW_EXT(quint8 sw, bool on) {
        if (on) {
            TCM.set.EXT_SW |= 1 << (sw - 1);
            setBit(sw - 1, TCMparameters["EXT_SW"].address);
        } else {
            TCM.set.EXT_SW &= ~(1 << (sw - 1));
            clearBit(sw - 1, TCMparameters["EXT_SW"].address);
        }
    }
    void apply_ORA_MODE(quint8 mode) { TCM.set.ORA_MODE = mode; writeParameter("ORA_MODE", mode, TCMid); }
    void apply_ORC_MODE(quint8 mode) { TCM.set.ORC_MODE = mode; writeParameter("ORC_MODE", mode, TCMid); }
    void apply_SC_MODE (quint8 mode) { TCM.set. SC_MODE = mode; writeParameter( "SC_MODE", mode, TCMid); }
    void apply_C_MODE  (quint8 mode) { TCM.set.  C_MODE = mode; writeParameter(  "C_MODE", mode, TCMid); }
    void apply_V_MODE  (quint8 mode) { TCM.set.  V_MODE = mode; writeParameter(  "V_MODE", mode, TCMid); }
    void apply_ORA_RATE() { writeParameter("ORA_RATE", TCM.set.ORA_RATE, TCMid); }
    void apply_ORC_RATE() { writeParameter("ORC_RATE", TCM.set.ORC_RATE, TCMid); }
    void apply_SC_RATE () { writeParameter( "SC_RATE", TCM.set. SC_RATE, TCMid); }
    void apply_C_RATE  () { writeParameter(  "C_RATE", TCM.set.  C_RATE, TCMid); }
    void apply_V_RATE  () { writeParameter(  "V_RATE", TCM.set.  V_RATE, TCMid); }
	void apply_ORA_SIGN() { quint16 s = TCM.set.ORA_SIGN & 0x7F; TCM.set.ORA_SIGN = s += (~s << 7); writeParameter("ORA_SIGN", s, TCMid); }
	void apply_ORC_SIGN() { quint16 s = TCM.set.ORC_SIGN & 0x7F; TCM.set.ORC_SIGN = s += (~s << 7); writeParameter("ORC_SIGN", s, TCMid); }
	void apply_SC_SIGN () { quint16 s = TCM.set. SC_SIGN & 0x7F; TCM.set. SC_SIGN = s += (~s << 7); writeParameter( "SC_SIGN", s, TCMid); }
	void apply_C_SIGN  () { quint16 s = TCM.set.  C_SIGN & 0x7F; TCM.set.  C_SIGN = s += (~s << 7); writeParameter(  "C_SIGN", s, TCMid); }
	void apply_V_SIGN  () { quint16 s = TCM.set.  V_SIGN & 0x7F; TCM.set.  V_SIGN = s += (~s << 7); writeParameter(  "V_SIGN", s, TCMid); }
    void apply_SC_LEVEL_A() { writeParameter("SC_LEVEL_A", TCM.set.SC_LEVEL_A, TCMid); }
    void apply_C_LEVEL_A () { writeParameter( "C_LEVEL_A", TCM.set. C_LEVEL_A, TCMid); }
    void apply_SC_LEVEL_C() { writeParameter("SC_LEVEL_C", TCM.set.SC_LEVEL_C, TCMid); }
    void apply_C_LEVEL_C () { writeParameter( "C_LEVEL_C", TCM.set. C_LEVEL_C, TCMid); }
    void apply_VTIME_LOW () { writeParameter("VTIME_LOW" , TCM.set.VTIME_LOW , TCMid); }
    void apply_VTIME_HIGH() { writeParameter("VTIME_HIGH", TCM.set.VTIME_HIGH, TCMid); }

    void apply_OR_GATE (quint16 FEEid) { writeParameter("OR_GATE" , PM[FEEid]->set.OR_GATE , FEEid); }
    void apply_CFD_SATR(quint16 FEEid) { writeParameter("CFD_SATR", PM[FEEid]->set.CFD_SATR, FEEid); }
    void apply_TRG_CNT_MODE(quint16 FEEid, bool CFDinGate) { writeParameter("TRG_CNT_MODE", CFDinGate, FEEid); }
    void apply_CH_MASK_DATA (quint16 FEEid) { writeParameter("CH_MASK_DATA" , PM[FEEid]->set.CH_MASK_DATA , FEEid); }
    void apply_CH_MASK_TRG  (quint16 FEEid) {
        for (quint8 i=0; i<12; ++i) {
            bool b = PM[FEEid]->set.TIME_ALIGN[i].blockTriggers;
            if (bool(PM[FEEid]->act.timeAlignment[i].blockTriggers) != b) {
                if (b) addTransaction(RMWbits, PM[FEEid]->baseAddress + PMparameters["CH_TRG_BLOCK"].address + i, masks(0xFFFFFFFF, 1 << PMparameters["CH_TRG_BLOCK"].bitshift));
                else addTransaction(RMWbits, PM[FEEid]->baseAddress + PMparameters["CH_TRG_BLOCK"].address + i, masks(~(1 << PMparameters["CH_TRG_BLOCK"].bitshift), 0));
            }
        }
        if (transceive()) sync();
    }

};

#endif // FITELECTRONICS_H
