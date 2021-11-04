#ifndef FITELECTRONICS_H
#define FITELECTRONICS_H

#include "IPbusInterface.h"
#include "TCM.h"
#include "PM.h"
#include <cmath>

extern double systemClock_MHz; //40
extern double TDCunit_ps; // 13
extern double halfBC_ns; // 12.5
extern double phaseStepLaser_ns, phaseStep_ns;

class FITelectronics: public IPbusTarget, public DimCommandHandler {
    Q_OBJECT
public:
    TypeFITsubdetector subdetector;
    const quint16 TCMid;
    DimServer DIMserver;
    QHash<DimCommand *, std::function<void(DimCommand *)>> allCommands;
    TypeTCM TCM;
    TypePM allPMs[20] = { //PMs by link №
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

    QMap<quint16, TypePM *> PM;
    QList<TypePM *> PMsA, PMsC;
	QTimer *countersTimer = new QTimer();
    //QTimer *fullSyncTimer = new QTimer();
    QFile logFile;
    QTextStream logStream;
    bool PMsReady = false;
//*
    QMetaObject::Connection adjustConnection;
    bool adjEven = false;
    quint16 thHi[12], thLo[12] = {0};
    double targetRate_Hz = 15.;
    TypePM *targetPM;
//*/
    FITelectronics(TypeFITsubdetector sd): IPbusTarget(50006), subdetector(sd), TCMid(FIT[sd].TCMid) {
        logFile.setFileName(QCoreApplication::applicationName() + ".log");
        logFile.open(QFile::WriteOnly | QIODevice::Append | QFile::Text);
        logStream.setDevice(&logFile);
        for (quint8 i=0; i<10; ++i) {
            allPMs[i     ].FEEid = FIT[sd].PMA0id + i;
            allPMs[i + 10].FEEid = FIT[sd].PMC0id + i;
        }
        TCM.set.T1_SIGN = FIT[sd].triggers[0].signature;
        TCM.set.T2_SIGN = FIT[sd].triggers[1].signature;
        TCM.set.T3_SIGN = FIT[sd].triggers[2].signature;
        TCM.set.T4_SIGN = FIT[sd].triggers[3].signature;
        TCM.set.T5_SIGN = FIT[sd].triggers[4].signature;
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
        connect(this, &IPbusTarget::IPbusStatusOK, this, [=]() { //init some parameters
            if (subdetector == FV0) writeNbits(0x3, 0xE, 2, 8); //apply FV0 trigger mode
            addWordToWrite(TCMparameters["T1_SIGN"].address, prepareSignature(FIT[sd].triggers[0].signature));
            addWordToWrite(TCMparameters["T2_SIGN"].address, prepareSignature(FIT[sd].triggers[1].signature));
            addWordToWrite(TCMparameters["T3_SIGN"].address, prepareSignature(FIT[sd].triggers[2].signature));
            addWordToWrite(TCMparameters["T4_SIGN"].address, prepareSignature(FIT[sd].triggers[3].signature));
            addWordToWrite(TCMparameters["T5_SIGN"].address, prepareSignature(FIT[sd].triggers[4].signature));
            //addWordToWrite(0x6A, 0x6DB6); //switch all trigger outputs to signature mode
            addTransaction(read, TCMparameters["COUNTERS_UPD_RATE"].address, &TCM.act.COUNTERS_UPD_RATE);
            if (!transceive()) return;
            defaultGBT(true);
            clearFIFOs();
            if (TCM.act.COUNTERS_UPD_RATE != 0) countersTimer->start(countersUpdatePeriod_ms[TCM.act.COUNTERS_UPD_RATE] / 2);
            PMsReady = false;
            sync();
            if (TCM.services.isEmpty()) createTCMservices();
        });

        DIMserver.setDnsNode("localhost");
        DIMserver.start(qPrintable(QString(FIT[sd].name) + "_DIM_SERVER"));
    }

    ~FITelectronics() {
        for (quint8 i=0; i<20; ++i) deletePMservices(allPMs + i);
        deleteTCMservices();
        DIMserver.stop();
        logFile.close();
    }

    void log(QString st) { logStream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + st; }

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
		pm->servicesNew.insert("OR_GATE" , new DimService(qPrintable(prefix+"control/OR_GATE" "/new"), "S", &pm->set.OR_GATE     , 2));

        addCommand(pm->commands, prefix+"control/TRG_CNT_MODE""/apply", "S", [=](DimCommand *c) { apply_TRG_CNT_MODE(pm->FEEid, c->getShort()); });
        addCommand(pm->commands, prefix+"control/CFD_SATR"    "/apply", "S", [=](DimCommand * ) { apply_CFD_SATR(pm->FEEid); });
        addCommand(pm->commands, prefix+"control/OR_GATE"     "/apply", "S", [=](DimCommand * ) { apply_OR_GATE_PM(pm->FEEid); });
		addCommand(pm->commands, prefix+"control/RESET_COUNTERS"	  , "S", [=](DimCommand * ) { apply_RESET_COUNTERS(pm->FEEid); });
        addCommand(pm->commands, prefix+"control/switchTRGsync"		  , "S", [=](DimCommand *c) { switchTRGsyncPM(pm-allPMs, c->getShort()); });
        addCommand(pm->commands, prefix+"control/CH_MASK_DATA""/apply", "I", [=](DimCommand *c) { pm->set.CH_MASK_DATA = c->getShort(); apply_CH_MASK_DATA(pm->FEEid); });
        addCommand(pm->commands, prefix+"control/CH_MASK_TRG" "/apply", "I", [=](DimCommand *c) {
            quint16 mask = c->getShort();
            for (quint8 i=0; i<12; ++i) pm->set.TIME_ALIGN[i].blockTriggers = !(1 << i & mask);
            apply_CH_MASK_TRG(pm->FEEid);
        });


		addCommand(pm->commands, prefix+"control/CFD_SATR""/set", "S", [=](DimCommand *c) { pm->set.CFD_SATR = c->getShort(); pm->servicesNew["CFD_SATR"]->updateService(); });
		addCommand(pm->commands, prefix+"control/OR_GATE" "/set", "S", [=](DimCommand *c) { pm->set.OR_GATE  = c->getShort(); pm->servicesNew["OR_GATE" ]->updateService(); });

        for (quint8 iCh=0; iCh<12; ++iCh) {
			QString ch = QString::asprintf("Ch%02d/", iCh + 1);
            pm->services.append(new DimService(qPrintable(prefix+ch+"status/ADC0_BASELINE"), "S", &pm->act.ADC_BASELINE[iCh][0]   , 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"status/ADC1_BASELINE"), "S", &pm->act.ADC_BASELINE[iCh][1]   , 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"status/ADC0_MEANAMPL"), "S", &pm->act.MEANAMPL    [iCh][0][0], 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"status/ADC1_MEANAMPL"), "S", &pm->act.MEANAMPL    [iCh][1][0], 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"status/ADC0_RMS"     ), "D", &pm->act.RMS_Ch      [iCh][0]   , 8));
            pm->services.append(new DimService(qPrintable(prefix+ch+"status/ADC1_RMS"     ), "D", &pm->act.RMS_Ch      [iCh][1]   , 8));
            pm->counters.services.append(new DimService(qPrintable(prefix+ch+"status/CNT_CFD"), "I", &pm->counters.Ch[iCh].CFD, 4));
            pm->counters.services.append(new DimService(qPrintable(prefix+ch+"status/CNT_TRG"), "I", &pm->counters.Ch[iCh].TRG, 4));
            pm->counters.services.append(new DimService(qPrintable(prefix+ch+"status/CNT_RATE_CFD"), "D", &pm->counters.rateCh[iCh].CFD, 8));
            pm->counters.services.append(new DimService(qPrintable(prefix+ch+"status/CNT_RATE_TRG"), "D", &pm->counters.rateCh[iCh].TRG, 8));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/TIME_ALIGN"    "/actual"), "S", pm->act.TIME_ALIGN + iCh, 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/ADC0_RANGE"    "/actual"), "S", &pm->act.ADC_RANGE   [iCh][0] , 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/ADC1_RANGE"    "/actual"), "S", &pm->act.ADC_RANGE   [iCh][1] , 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/CFD_THRESHOLD" "/actual"), "S", (qint16 *)&pm->act.Ch[iCh]    , 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/CFD_ZERO"      "/actual"), "S", (qint16 *)&pm->act.Ch[iCh] + 2, 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/ADC_ZERO"      "/actual"), "S", (qint16 *)&pm->act.Ch[iCh] + 4, 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/ADC_DELAY"     "/actual"), "S", (qint16 *)&pm->act.Ch[iCh] + 6, 2));
            pm->services.append(new DimService(qPrintable(prefix+ch+"control/THRESHOLD_CALIBR/actual"), "S", (qint16 *)&pm->act.THRESHOLD_CALIBR[iCh], 2));

			pm->servicesNew.insert(ch+"TIME_ALIGN"      , new DimService(qPrintable(prefix+ch+"control/TIME_ALIGN"    "/new"), "S", pm->set.TIME_ALIGN +  iCh	  , 2));
			pm->servicesNew.insert(ch+"ADC0_RANGE"      , new DimService(qPrintable(prefix+ch+"control/ADC0_RANGE"    "/new"), "S", &pm->set.ADC_RANGE   [iCh][0] , 2));
			pm->servicesNew.insert(ch+"ADC1_RANGE"      , new DimService(qPrintable(prefix+ch+"control/ADC1_RANGE"    "/new"), "S", &pm->set.ADC_RANGE   [iCh][1] , 2));
			pm->servicesNew.insert(ch+"CFD_THRESHOLD"   , new DimService(qPrintable(prefix+ch+"control/CFD_THRESHOLD" "/new"), "S", (qint16 *)&pm->set.Ch[iCh]    , 2));
			pm->servicesNew.insert(ch+"CFD_ZERO"        , new DimService(qPrintable(prefix+ch+"control/CFD_ZERO"      "/new"), "S", (qint16 *)&pm->set.Ch[iCh] + 2, 2));
			pm->servicesNew.insert(ch+"ADC_ZERO"        , new DimService(qPrintable(prefix+ch+"control/ADC_ZERO"      "/new"), "S", (qint16 *)&pm->set.Ch[iCh] + 4, 2));
			pm->servicesNew.insert(ch+"ADC_DELAY"       , new DimService(qPrintable(prefix+ch+"control/ADC_DELAY"     "/new"), "S", (qint16 *)&pm->set.Ch[iCh] + 6, 2));
			pm->servicesNew.insert(ch+"THRESHOLD_CALIBR", new DimService(qPrintable(prefix+ch+"control/THRESHOLD_CALIBR/new"), "S", (qint16 *)&pm->set.THRESHOLD_CALIBR[iCh], 2));

			addCommand(pm->commands, prefix+ch+"control/TIME_ALIGN"    "/set", "S", [=](DimCommand *c) { pm->set.TIME_ALIGN[iCh].value = c->getShort(); pm->servicesNew[ch+"TIME_ALIGN"      ]->updateService(); });
			addCommand(pm->commands, prefix+ch+"control/ADC0_RANGE"    "/set", "S", [=](DimCommand *c) { pm->set.ADC_RANGE[iCh][0]     = c->getShort(); pm->servicesNew[ch+"ADC0_RANGE"      ]->updateService(); });
			addCommand(pm->commands, prefix+ch+"control/ADC1_RANGE"    "/set", "S", [=](DimCommand *c) { pm->set.ADC_RANGE[iCh][1]     = c->getShort(); pm->servicesNew[ch+"ADC1_RANGE"      ]->updateService(); });
			addCommand(pm->commands, prefix+ch+"control/CFD_THRESHOLD" "/set", "S", [=](DimCommand *c) { pm->set.Ch[iCh].CFD_THRESHOLD = c->getShort(); pm->servicesNew[ch+"CFD_THRESHOLD"   ]->updateService(); });
			addCommand(pm->commands, prefix+ch+"control/CFD_ZERO"      "/set", "S", [=](DimCommand *c) { pm->set.Ch[iCh].CFD_ZERO      = c->getShort(); pm->servicesNew[ch+"CFD_ZERO"        ]->updateService(); });
			addCommand(pm->commands, prefix+ch+"control/ADC_ZERO"      "/set", "S", [=](DimCommand *c) { pm->set.Ch[iCh].ADC_ZERO      = c->getShort(); pm->servicesNew[ch+"ADC_ZERO"        ]->updateService(); });
			addCommand(pm->commands, prefix+ch+"control/ADC_DELAY"     "/set", "S", [=](DimCommand *c) { pm->set.Ch[iCh].ADC_DELAY     = c->getShort(); pm->servicesNew[ch+"ADC_DELAY"       ]->updateService(); });
			addCommand(pm->commands, prefix+ch+"control/THRESHOLD_CALIBR/set", "S", [=](DimCommand *c) { pm->set.THRESHOLD_CALIBR[iCh] = c->getShort(); pm->servicesNew[ch+"THRESHOLD_CALIBR"]->updateService(); });

			addCommand(pm->commands, prefix+ch+"control/TIME_ALIGN"    "/apply", "S", [=](DimCommand *) { apply_TIME_ALIGN      (pm->FEEid, iCh+1); });
			addCommand(pm->commands, prefix+ch+"control/ADC0_RANGE"    "/apply", "S", [=](DimCommand *) { apply_ADC0_RANGE      (pm->FEEid, iCh+1); });
			addCommand(pm->commands, prefix+ch+"control/ADC1_RANGE"    "/apply", "S", [=](DimCommand *) { apply_ADC1_RANGE      (pm->FEEid, iCh+1); });
			addCommand(pm->commands, prefix+ch+"control/CFD_THRESHOLD" "/apply", "S", [=](DimCommand *) { apply_CFD_THRESHOLD   (pm->FEEid, iCh+1); });
			addCommand(pm->commands, prefix+ch+"control/CFD_ZERO"      "/apply", "S", [=](DimCommand *) { apply_CFD_ZERO        (pm->FEEid, iCh+1); });
			addCommand(pm->commands, prefix+ch+"control/ADC_ZERO"      "/apply", "S", [=](DimCommand *) { apply_ADC_ZERO        (pm->FEEid, iCh+1); });
			addCommand(pm->commands, prefix+ch+"control/ADC_DELAY"     "/apply", "S", [=](DimCommand *) { apply_ADC_DELAY       (pm->FEEid, iCh+1); });
			addCommand(pm->commands, prefix+ch+"control/THRESHOLD_CALIBR/apply", "S", [=](DimCommand *) { apply_THRESHOLD_CALIBR(pm->FEEid, iCh+1); });
            addCommand(pm->commands, prefix+ch+"control/switch"				   , "S", [=](DimCommand *c) { switchPMchannel(pm-allPMs, iCh+1, c->getShort()); });
            addCommand(pm->commands, prefix+ch+"control/noTriggerMode"		   , "S", [=](DimCommand *c) { apply_PMchannelNoTRG(pm-allPMs, iCh+1, c->getShort()); });
        }
    }

    void createTCMservices() {
		QString prefix = QString::asprintf("%s/TCM/", FIT[subdetector].name);
		TCM.services.append(new DimService(qPrintable(prefix+"status/TEMP_BOARD"  ), "D", &TCM.act.TEMP_BOARD                   , 8));
		TCM.services.append(new DimService(qPrintable(prefix+"status/TEMP_FPGA"   ), "D", &TCM.act.TEMP_FPGA                    , 8));
		TCM.services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1V"  ), "D", &TCM.act.VOLTAGE_1V                   , 8));
		TCM.services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1_8V"), "D", &TCM.act.VOLTAGE_1_8V                 , 8));
		TCM.services.append(new DimService(qPrintable(prefix+"status/SERIAL_NUM"  ), "S",  TCM.act.pointer0 + 0x7 * wordSize + 1, 2));
		TCM.services.append(new DimService(qPrintable(prefix+"status/BOARD_TYPE"  ), "C:4",TCM.act.BOARD_TYPE                   , 4));
		TCM.services.append(new DimService(qPrintable(prefix+"status/FW_TIME_MCU" ), "I", &TCM.act.FW_TIME_MCU                  , 4));
		TCM.services.append(new DimService(qPrintable(prefix+"status/FW_TIME_FPGA"), "I", &TCM.act.FW_TIME_FPGA                 , 4));
		TCM.services.append(new DimService(qPrintable(prefix+"status/PM_MASK_SPI" ), "I", &TCM.act.PM_MASK_SPI                  , 4));
        for (quint8 iPM=0; iPM<10; ++iPM) {
			TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/TRG_SYNC/%s", FIT[subdetector].name, allPMs[iPM   ].name)), "I", TCM.act.TRG_SYNC_A + iPM, 4));
			TCM.services.append(new DimService(qPrintable(QString::asprintf("%s/TCM/status/TRG_SYNC/%s", FIT[subdetector].name, allPMs[iPM+10].name)), "I", TCM.act.TRG_SYNC_C + iPM, 4));
        }

        for (quint8 i=0; i<5; ++i) {
            TCM.staticServices.append(new DimService( qPrintable(prefix+"Trigger"+QString::number(i+1)+"/NAME"     ),  const_cast<char   *>( FIT[subdetector].triggers[i].name     ) ));
            TCM.staticServices.append(new DimService( qPrintable(prefix+"Trigger"+QString::number(i+1)+"/SIGNATURE"), *const_cast<qint16 *>(&FIT[subdetector].triggers[i].signature) ));
        }
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd0/NAME"), const_cast<char *>("NoiseA          ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd1/NAME"), const_cast<char *>("NoiseC          ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd2/NAME"), const_cast<char *>("Total noise     ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd3/NAME"), const_cast<char *>("True OrA        ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd4/NAME"), const_cast<char *>("True OrC        ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd5/NAME"), const_cast<char *>("Interaction     ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd6/NAME"), const_cast<char *>("True Interaction") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd7/NAME"), const_cast<char *>("True Vertex     ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd8/NAME"), const_cast<char *>("Beam-gas A      ") ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd9/NAME"), const_cast<char *>("Beam-gas C      ") ));

        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger1/CNT"), "I", &TCM.counters.CNT_T1, 4));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger2/CNT"), "I", &TCM.counters.CNT_T2, 4));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger3/CNT"), "I", &TCM.counters.CNT_T3, 4));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger4/CNT"), "I", &TCM.counters.CNT_T4, 4));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger5/CNT"), "I", &TCM.counters.CNT_T5, 4));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger1/CNT_RATE"), "D", TCM.counters.rate + 3, 8));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger2/CNT_RATE"), "D", TCM.counters.rate + 2, 8));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger3/CNT_RATE"), "D", TCM.counters.rate + 4, 8));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger4/CNT_RATE"), "D", TCM.counters.rate + 1, 8));
        TCM.counters.services.append(new DimService(qPrintable(prefix+"Trigger5/CNT_RATE"), "D", TCM.counters.rate    , 8));
        for (quint8 i=5; i<15; ++i) {
            TCM.counters.services.append(new DimService(qPrintable(prefix+"Bkgrnd"+QString::number(i-5)+"/CNT"), "I", TCM.counters.New + i, 4));
            TCM.counters.services.append(new DimService(qPrintable(prefix+"Bkgrnd"+QString::number(i-5)+"/CNT_RATE"), "D", TCM.counters.rate + i, 8));
        }

        //TCM.services.append(new DimService(qPrintable(prefix+"Trigger1/OUTPUT_ENABLED"), "S", TCM.counters.rate    , 8));
        addCommand(TCM.commands, prefix+"control/ORBIT_FILL_MASK/set", "I:223", [=](DimCommand *c) { int s = c->getSize(); memcpy(TCM.ORBIT_FILL_MASK, c->getData(), s); });
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
        foreach (DimService *s, TCM.services + TCM.counters.services + TCM.staticServices) delete s;
        TCM.services.clear();
        TCM.counters.services.clear();
        TCM.staticServices.clear();
//        foreach (DimService *s, TCM.servicesNew) delete s;
//        TCM.servicesNew.clear();
        foreach (DimCommand *c, TCM.commands) { allCommands.remove(c); delete c; }
        TCM.commands.clear();
    }

    void commandHandler() {
        DimCommand *cmd = getCommand();
        allCommands[cmd](cmd);
    }

signals:
    void linksStatusReady();
    void valuesReady();
    void countersReady();

public slots:

    void clearFIFOs() {
		quint32 load = readRegister(TypeTCM::Counters::addressFIFOload);
        if (load == 0xFFFFFFFF) return;
        while (load) {
			addTransaction(nonIncrementingRead, TypeTCM::Counters::addressFIFO, nullptr, load > 255 ? 255 : load);
			addTransaction(read, TypeTCM::Counters::addressFIFOload, &load);
			transceive();
		}
		foreach (TypePM *pm, PM) {
			load = readRegister(pm->baseAddress + TypePM::Counters::addressFIFOload);
            if (load == 0xFFFFFFFF) return;
			while (load) {
				addTransaction(nonIncrementingRead, pm->baseAddress + TypePM::Counters::addressFIFO, nullptr, load > 255 ? 255 : load);
				addTransaction(read, pm->baseAddress + TypePM::Counters::addressFIFOload, &load);
				transceive();
			}
		}
    }

    void apply_COUNTERS_UPD_RATE(quint8 val) {
        countersTimer->stop();
        writeRegister(TCMparameters["COUNTERS_UPD_RATE"].address, 0, false);
        clearFIFOs();
        if (val <= 7) {
            TCM.set.COUNTERS_UPD_RATE = val;
            if (val > 0) {
                writeParameter("COUNTERS_UPD_RATE", val, TCMid);
                countersTimer->start(countersUpdatePeriod_ms[val] / 2);
            }
        } else emit error("Wrong COUNTERS_UPD_RATE value: " + QString::number(val), logicError);
    }

    void defaultGBT(bool TCMonly = false) {
        quint16 BCIDdelay = TCM.set.GBT.BCID_DELAY;
        for (quint8 j=0; j<GBTunit::controlSize; ++j) TCM.set.GBT.registers[j] = GBTunit::defaults[j];
        TCM.set.GBT.RDH_FEE_ID = TCMid;
        TCM.set.GBT.RDH_SYS_ID = FIT[subdetector].systemID;
        TCM.set.GBT.BCID_DELAY = BCIDdelay;
        addTransaction(write, GBTunit::controlAddress, TCM.set.GBT.registers, GBTunit::controlSize);
        if (!TCMonly) foreach (TypePM *pm, PM) {
            BCIDdelay = pm->set.GBT.BCID_DELAY;
            for (quint8 j=0; j<GBTunit::controlSize; ++j) pm->set.GBT.registers[j] = GBTunit::defaults[j];
            pm->set.GBT.RDH_FEE_ID = pm->FEEid;
            pm->set.GBT.RDH_SYS_ID = FIT[subdetector].systemID;
            pm->set.GBT.BCID_DELAY = BCIDdelay;
            addTransaction(write, pm->baseAddress + GBTunit::controlAddress, pm->set.GBT.registers, GBTunit::controlSize);
        }
        transceive();
    }

    void checkPMlinks() {
        addTransaction(read, TCMparameters["PM_MASK_SPI"].address, &TCM.act.PM_MASK_SPI);
        addTransaction(read, TCMparameters["CH_MASK_A"].address, dt);
        addTransaction(read, TCMparameters["CH_MASK_C"].address, dt + 1);
        if (transceive()) {
            TCM.set.CH_MASK_A = dt[0];
            TCM.set.CH_MASK_C = dt[1];
            TCM.set.PM_MASK_SPI = TCM.act.PM_MASK_SPI;
        } else return;
        PM.clear();
        PMsA.clear();
        PMsC.clear();
        for (quint8 i=0; i<20; ++i) {
            if (!(TCM.set.PM_MASK_SPI >> i & 1)) setBit(i, TCMparameters["PM_MASK_SPI"].address, false);
            if (readRegister(allPMs[i].baseAddress + 0xFE) == 0xFFFFFFFF) { //SPI or IPbus error
                clearBit(i, TCMparameters["PM_MASK_SPI"].address, false);
				deletePMservices(allPMs + i);
            } else {
                TCM.set.PM_MASK_SPI |= 1 << i;
				PM.insert(allPMs[i].FEEid, allPMs + i);
                (i < 10 ? PMsA : PMsC).append(allPMs + i);
				if (allPMs[i].services.isEmpty()) createPMservices(allPMs + i);
                if (i > 9) TCM.set.CH_MASK_C |= 1 << (i - 10);
                else       TCM.set.CH_MASK_A |= 1 << i;
            }
        }
        addWordToWrite(TCMparameters["CH_MASK_A"].address, TCM.set.CH_MASK_A);
        addWordToWrite(TCMparameters["CH_MASK_C"].address, TCM.set.CH_MASK_C);
        if (transceive()) emit linksStatusReady();
    }



    void writeParameter(QString name, quint64 val, quint16 FEEid, quint8 iCh = 0) {
        if ( !GBTparameters.contains(name) && (FEEid == TCMid ? !TCMparameters.contains(name) : !PMparameters.contains(name)) ) {
            emit error("'" + name + "' - no such parameter", logicError);
            return;
        }
        Parameter p(GBTparameters.contains(name) ? GBTparameters[name] : (FEEid == TCMid ? TCMparameters[name] : PMparameters[name]));
        quint16 address = p.address + (FEEid == TCMid ? 0 : PM[FEEid]->baseAddress + iCh * p.interval);
        if (p.bitwidth == 32)
            writeRegister(val, address);
        else if (p.bitwidth == 1)
            val != 0 ? setBit(p.bitshift, address) : clearBit(p.bitshift, address);
        else if (p.bitwidth == 64) {
            quint32 w[2] = {quint32(val), quint32(val >> 32)};
			addTransaction(write, address, w, 2);
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
            foreach (DimService *s, TCM.counters.services) s->updateService();
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
            if (time_ms < 100) return;
            for (quint8 i=0; i<TypeTCM::Counters::number; ++i) {
                TCM.counters.rate[i] = TCM.counters.New[i] == TCM.counters.Old[i] ? 0 : (TCM.counters.New[i] - TCM.counters.Old[i]) * 1000. / time_ms;
                TCM.counters.Old[i] = TCM.counters.New[i];
            }
            TCM.counters.oldTime = TCM.counters.newTime;
            foreach (DimService *s, TCM.counters.services) s->updateService();
        }
        if (PMsReady) foreach (TypePM *pm, PM) {
            addTransaction(read, pm->baseAddress + TypePM::Counters::addressDirect, pm->counters.New, TypePM::Counters::number);
            if (!transceive() || !PM.contains(pm->FEEid)) return;
            pm->counters.newTime = QDateTime::currentDateTime();
            quint32 time_ms = pm->counters.oldTime.msecsTo(pm->counters.newTime);
            for (quint8 i=0; i<TypePM::Counters::number; ++i) {
                pm->counters.rate[i] = pm->counters.New[i] == pm->counters.Old[i] ? 0. : (pm->counters.New[i] - pm->counters.Old[i]) * 1000. / time_ms;
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
        addTransaction(read, pm->baseAddress + TypePM::ActualValues::block2addr, pm->act.registers2, TypePM::ActualValues::block2size - 1); //voltage1_8 value is already read
    }

	bool read1PM(TypePM *pm) {
        pm->act.voltage1_8 = readRegister(pm->baseAddress + 0xFE);
        if (pm->act.voltage1_8 == 0xFFFFFFFF) { //SPI error
            clearBit(pm - allPMs, 0x1E, false);
            deletePMservices(pm);
            TCM.set.PM_MASK_SPI &= ~(1 << (pm - allPMs));
            PM.remove(pm->FEEid);
            (pm - allPMs < 10 ? PMsA : PMsC).removeOne(pm);
            log(QString(pm->name) + " not available by SPI");
            emit linksStatusReady();
        } else {
            addPMvaluesToRead(pm);
            if (!transceive()) return false;
            pm->act.calculateValues();
            pm->counters.GBT.calculateRate(pm->act.GBT.Status.wordsCount, pm->act.GBT.Status.eventsCount);
            foreach (DimService *s, pm->services) s->updateService();
        }
        return true;
    }

        void sync() { //read actual values
            addSystemValuesToRead();
            addTCMvaluesToRead();
            if (!transceive()) return;
            TCM.act.calculateValues();
            TCM.counters.GBT.calculateRate(TCM.act.GBT.Status.wordsCount, TCM.act.GBT.Status.eventsCount);
            foreach (DimService *s, TCM.services) s->updateService();
            if (TCM.act.resetSystem) {
                PMsReady = false;
                PM.clear();
            } else if (PMsReady == false) { // reset completed
                checkPMlinks();
                defaultGBT();
                //apply_RESET_ERRORS(false);
                PMsReady = true;
            }
            if (TCM.act.PM_MASK_SPI != TCM.set.PM_MASK_SPI) checkPMlinks();
            if (PMsReady) { foreach (TypePM *pm, PM) if (!read1PM(pm)) return; }
            emit valuesReady();
            if (TCM.act.COUNTERS_UPD_RATE == 0) readCountersDirectly();
        }

        void adjustThresholds(TypePM *pm, double rate_Hz) { //experimental function! lab use only!
            if (adjustConnection) return;
            targetPM = pm;
            targetRate_Hz = rate_Hz;
            for (quint8 iCh = 0; iCh<12; ++iCh) {
                thLo[iCh] = 1500;
                thHi[iCh] = 2500;
            }
            adjEven = false;
            adjustConnection = connect(this, &FITelectronics::countersReady, this, [&]() {
                adjEven = !adjEven;
                if (adjEven) return;
                quint8 c = 0;
                for (quint8 iCh = 0; iCh<12; ++iCh) {
                    if (targetPM->counters.rateCh[iCh].CFD < targetRate_Hz + sqrt(targetRate_Hz)) { thHi[iCh] = targetPM->act.THRESHOLD_CALIBR[iCh]; }
                    if (targetPM->counters.rateCh[iCh].CFD > targetRate_Hz - sqrt(targetRate_Hz)) { thLo[iCh] = targetPM->act.THRESHOLD_CALIBR[iCh]; }
                    if (thLo[iCh] != thHi[iCh]) {
                        addWordToWrite(targetPM->baseAddress + PMparameters["THRESHOLD_CALIBR"].address + iCh, (thHi[iCh] + thLo[iCh]) / 2);
                        ++c;
                    }
                }
                if (c == 0) disconnect(adjustConnection); else transceive();
            });
        }

    void reset(quint16 FEEid, quint8 RB_position, bool syncOnSuccess = true) {
        quint32 address = (FEEid == TCMid ? 0x0 : PM[FEEid]->baseAddress) + GBTunit::controlAddress;
        addTransaction(RMWbits, address, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        addTransaction(RMWbits, address, masks(0xFFFFFFFF, 1 << RB_position)); //set specific bit, e.g. 0x00000800 for RS_GBTerrors
        addTransaction(RMWbits, address, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        if (transceive() && syncOnSuccess) sync();
    }

    void apply_RESET_ORBIT_SYNC           (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_orbitSync            , syncOnSuccess); }
    void apply_RESET_DATA_COUNTER         (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_dataCounter          , syncOnSuccess); }
    void apply_RESET_GEN_BUNCH_OFFSET     (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_generatorsBunchOffset, syncOnSuccess); }
    void apply_RESET_GBT_ERRORS           (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_GBTRxError           , syncOnSuccess); }
    void apply_RESET_GBT                  (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_GBT                  , syncOnSuccess); }
    void apply_RESET_RX_PHASE_ERROR       (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_RXphaseError         , syncOnSuccess); }
    void apply_RESET_FSM                  (quint16 FEEid, bool syncOnSuccess = true) { reset(FEEid, GBTunit::RB_readoutFSM           , syncOnSuccess); }

    void apply_TG_CTP_EMUL_MODE(quint16 FEEid, quint8 RO_mode) { writeParameter("TG_CTP_EMUL_MODE", RO_mode, FEEid); }

    void apply_DG_MODE(quint16 FEEid, quint8 DG_mode) {
        (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_MODE = DG_mode;
        writeParameter("DG_MODE", DG_mode, FEEid);
    }

    void apply_TG_MODE(quint16 FEEid, quint8 TG_mode) {
        (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_MODE = TG_mode;
        writeParameter("TG_MODE", TG_mode, FEEid);
    }

    void apply_TG_PATTERN           (quint16 FEEid) { writeParameter("TG_PATTERN"           , *(quint64 *)&(FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_PATTERN_LSB, FEEid); }
    void apply_TG_CONT_VALUE        (quint16 FEEid) { writeParameter("TG_CONT_VALUE"        , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_CONT_VALUE        , FEEid); }
    void apply_TG_BUNCH_FREQ        (quint16 FEEid) { writeParameter("TG_BUNCH_FREQ"        , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_BUNCH_FREQ        , FEEid); }
    void apply_TG_FREQ_OFFSET       (quint16 FEEid) { writeParameter("TG_FREQ_OFFSET"       , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_FREQ_OFFSET       , FEEid); }
    void apply_TG_HBr_RATE          (quint16 FEEid) { writeParameter("TG_HBr_RATE"          , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).TG_HBr_RATE          , FEEid); }
    void apply_DG_TRG_RESPOND_MASK  (quint16 FEEid) { writeParameter("DG_TRG_RESPOND_MASK"  , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_TRG_RESPOND_MASK  , FEEid); }
    void apply_DG_BUNCH_PATTERN     (quint16 FEEid) { writeParameter("DG_BUNCH_PATTERN"     , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_BUNCH_PATTERN     , FEEid); }
    void apply_DG_BUNCH_FREQ        (quint16 FEEid) { writeParameter("DG_BUNCH_FREQ"        , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_BUNCH_FREQ        , FEEid); }
    void apply_DG_FREQ_OFFSET       (quint16 FEEid) { writeParameter("DG_FREQ_OFFSET"       , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DG_FREQ_OFFSET       , FEEid); }
    void apply_BCID_DELAY           (quint16 FEEid) { writeParameter("BCID_DELAY"           , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).BCID_DELAY           , FEEid); }
    void apply_DATA_SEL_TRG_MASK    (quint16 FEEid) { writeParameter("DATA_SEL_TRG_MASK"    , (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).DATA_SEL_TRG_MASK    , FEEid); }

    void apply_HB_RESPONSE (quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).HB_RESPONSE  = on; writeParameter("HB_RESPONSE" , on, FEEid); }
    void apply_READOUT_LOCK(quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).READOUT_LOCK = on; writeParameter("READOUT_LOCK", on, FEEid); }
    void apply_BYPASS_MODE (quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).BYPASS_MODE  = on; writeParameter("BYPASS_MODE" , on, FEEid); }
    void apply_HB_REJECT   (quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).BYPASS_MODE  = on; writeParameter("HB_REJECT"   , on, FEEid); }

    void switchTRGsyncPM(quint8 iPM, bool on) {
        if (iPM >= 20) {
            emit error("Incorrect PM index: " + QString::number(iPM), logicError);
            return;
        }
        if (on) {
            if (iPM < 10) {
                TCM.set.CH_MASK_A |= 1 << iPM % 10;
                setBit  (iPM % 10, TCMparameters["CH_MASK_A"].address);
            } else {
                TCM.set.CH_MASK_C |= 1 << iPM % 10;
                setBit  (iPM % 10, TCMparameters["CH_MASK_C"].address);
            }
        } else {
            if (iPM < 10) {
                TCM.set.CH_MASK_A &= ~(1 << iPM % 10);
                clearBit(iPM % 10, TCMparameters["CH_MASK_A"].address);
            } else {
                TCM.set.CH_MASK_C &= ~(1 << iPM % 10);
                clearBit(iPM % 10, TCMparameters["CH_MASK_C"].address);
            }
        }
    }

    void switchPMchannel(quint8 iPM, quint8 Ch, bool on) {
        if (iPM >= 20) {
            emit error("Incorrect PM index: " + QString::number(iPM), logicError);
            return;
        }
        quint8 iCh = Ch - 1;
        if (iCh < 12) {
            if (on) {
                allPMs[iPM].set.CH_MASK_DATA |= 1 << iCh;
                setBit  (iCh, allPMs[iPM].baseAddress + PMparameters["CH_MASK_DATA"].address);
            } else {
                allPMs[iPM].set.CH_MASK_DATA &= ~(1 << iCh);
                clearBit(iCh, allPMs[iPM].baseAddress + PMparameters["CH_MASK_DATA"].address);
            }
        } else emit error("invalid channel: " + QString::number(Ch), logicError);
    }

    void apply_PMchannelNoTRG(quint8 iPM, quint8 Ch, bool noTRG) {
        if (iPM >= 20) {
            emit error("Incorrect PM index: " + QString::number(iPM), logicError);
            return;
        }
        quint8 iCh = Ch - 1;
        if (iCh < 12) {
            allPMs[iPM].set.TIME_ALIGN[iCh].blockTriggers = noTRG;
            writeParameter("noTriggerMode", noTRG, allPMs[iPM].FEEid, iCh);
        } else emit error("invalid channel: " + QString::number(Ch + 1), logicError);
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
    void apply_RESET_SYSTEM(bool forceLocalClock = false) { PMsReady = false; writeRegister(forceLocalClock ? 0xC00 : 0x800, 0xF); }
    void apply_RESET_ERRORS(bool syncOnSuccess = true) {
        if (!TCM.act.GBT.isOK()) {
            addTransaction(RMWbits, GBTunit::controlAddress, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
            addTransaction(RMWbits, GBTunit::controlAddress, masks(0xFFFFFFFF, 1 << GBTunit::RB_readoutFSM | 1 << GBTunit::RB_GBTRxError));
            addTransaction(RMWbits, GBTunit::controlAddress, masks(0xFFBF00FF, 0x00000000)); //clear all reset bits and unlock
        }
        foreach (TypePM *pm, PM) if (!pm->act.GBT.isOK()) {
            quint32 address = pm->baseAddress + GBTunit::controlAddress;
            addTransaction(RMWbits, address, masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
            addTransaction(RMWbits, address, masks(0xFFFFFFFF, 1 << GBTunit::RB_readoutFSM | 1 << GBTunit::RB_GBTRxError));
            addTransaction(RMWbits, address, masks(0xFFBF00FF, 0x00000000)); //clear all reset bits and unlock
        }
        writeRegister(0x4, 0xF, syncOnSuccess);
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
    void apply_LASER_PATTERN() {
        addTransaction(write, TCMparameters["LASER_PATTERN"].address, &TCM.set.laserPatternMSB, 2);
        if (transceive()) sync();
    }
	void apply_SwLaserPatternBit(quint8 bit, bool on) {
		quint32 address = TCMparameters["LASER_PATTERN"].address + (bit < 32 ? 1 : 0);
		on ? setBit(bit % 32, address) : clearBit(bit % 32, address);
    }
	void apply_attenSteps() { writeParameter("attenSteps", TCM.set.attenSteps, TCMid); }
    void apply_LASER_ENABLED(bool on) { writeParameter("LASER_ENABLED", on, TCMid); }
    void apply_LASER_DELAY() { writeParameter("LASER_DELAY", TCM.set.LASER_DELAY, TCMid); }
    void apply_LASER_TRG_SUPPR_DUR  () { writeParameter("LASER_TRG_SUPPR_DUR", TCM.set.lsrTrgSupprDur, TCMid);}
    void apply_LASER_TRG_SUPPR_DELAY() { writeParameter("LASER_TRG_SUPPR_DELAY", TCM.set.lsrTrgSupprDelay, TCMid);}
    void apply_DELAY_A() { writeParameter("DELAY_A", TCM.set.DELAY_A, TCMid); }
    void apply_DELAY_C() { writeParameter("DELAY_C", TCM.set.DELAY_C, TCMid); }
    void apply_CH_MASK_A() { writeParameter("CH_MASK_A", TCM.set.CH_MASK_A, TCMid); }
    void apply_CH_MASK_C() { writeParameter("CH_MASK_C", TCM.set.CH_MASK_C, TCMid); }

    void apply_T1_ENABLED(bool on) { TCM.set.T1_ENABLED = on; writeParameter("T1_ENABLED", on, TCMid); }
    void apply_T2_ENABLED(bool on) { TCM.set.T2_ENABLED = on; writeParameter("T2_ENABLED", on, TCMid); }
    void apply_T3_ENABLED(bool on) { TCM.set.T3_ENABLED = on; writeParameter("T3_ENABLED", on, TCMid); }
    void apply_T4_ENABLED(bool on) { TCM.set.T4_ENABLED = on; writeParameter("T4_ENABLED", on, TCMid); }
    void apply_T5_ENABLED(bool on) { TCM.set.T5_ENABLED = on; writeParameter("T5_ENABLED", on, TCMid); }
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
    void apply_T1_MODE(quint8 mode) { TCM.set.T1_MODE = mode; writeParameter("T1_MODE", mode, TCMid); }
    void apply_T2_MODE(quint8 mode) { TCM.set.T2_MODE = mode; writeParameter("T2_MODE", mode, TCMid); }
    void apply_T3_MODE(quint8 mode) { TCM.set.T3_MODE = mode; writeParameter("T3_MODE", mode, TCMid); }
    void apply_T4_MODE(quint8 mode) { TCM.set.T4_MODE = mode; writeParameter("T4_MODE", mode, TCMid); }
    void apply_T5_MODE(quint8 mode) { TCM.set.T5_MODE = mode; writeParameter("T5_MODE", mode, TCMid); }
    void apply_T1_RATE() { writeParameter("T1_RATE", TCM.set.T1_RATE, TCMid); }
    void apply_T2_RATE() { writeParameter("T2_RATE", TCM.set.T2_RATE, TCMid); }
    void apply_T3_RATE() { writeParameter("T3_RATE", TCM.set.T3_RATE, TCMid); }
    void apply_T4_RATE() { writeParameter("T4_RATE", TCM.set.T4_RATE, TCMid); }
    void apply_T5_RATE() { writeParameter("T5_RATE", TCM.set.T5_RATE, TCMid); }
    void apply_T1_LEVEL_A() { writeParameter("T1_LEVEL_A", TCM.set.T1_LEVEL_A, TCMid); }
    void apply_T2_LEVEL_A() { writeParameter("T2_LEVEL_A", TCM.set.T2_LEVEL_A, TCMid); }
    void apply_T1_LEVEL_C() { writeParameter("T1_LEVEL_C", TCM.set.T1_LEVEL_C, TCMid); }
    void apply_T2_LEVEL_C() { writeParameter("T2_LEVEL_C", TCM.set.T2_LEVEL_C, TCMid); }
    void apply_VTIME_LOW () { writeParameter("VTIME_LOW" , TCM.set.VTIME_LOW , TCMid); }
    void apply_VTIME_HIGH() { writeParameter("VTIME_HIGH", TCM.set.VTIME_HIGH, TCMid); }

    void apply_OR_GATE_PM (quint16 FEEid) { writeParameter("OR_GATE" , PM[FEEid]->set.OR_GATE , FEEid); }
    void apply_OR_GATE_sideA(quint16 val) {
        for (quint8 iPM =  0; iPM < 10; ++iPM) if (TCM.act.PM_MASK_SPI & 1 << iPM) addWordToWrite(allPMs[iPM].baseAddress + PMparameters["OR_GATE"].address, val);
        if (transceive()) sync();
    }
    void apply_OR_GATE_sideC(quint16 val) {
        for (quint8 iPM = 10; iPM < 20; ++iPM) if (TCM.act.PM_MASK_SPI & 1 << iPM) addWordToWrite(allPMs[iPM].baseAddress + PMparameters["OR_GATE"].address, val);
        if (transceive()) sync();
    }
    void apply_CFD_SATR(quint16 FEEid) { writeParameter("CFD_SATR", PM[FEEid]->set.CFD_SATR, FEEid); }
    void apply_TRG_CNT_MODE(quint16 FEEid, bool CFDinGate) { writeParameter("TRG_CNT_MODE", CFDinGate, FEEid); }
    void apply_CH_MASK_DATA (quint16 FEEid) { writeParameter("CH_MASK_DATA" , PM[FEEid]->set.CH_MASK_DATA , FEEid); }
    void apply_CH_MASK_TRG  (quint16 FEEid) {
        for (quint8 i=0; i<12; ++i) {
            bool b = PM[FEEid]->set.TIME_ALIGN[i].blockTriggers;
            if (bool(PM[FEEid]->act.timeAlignment[i].blockTriggers) != b) {
				if (b) addTransaction(RMWbits, PM[FEEid]->baseAddress + PMparameters["noTriggerMode"].address + i, masks(0xFFFFFFFF, 1 << PMparameters["noTriggerMode"].bitshift));
				else   addTransaction(RMWbits, PM[FEEid]->baseAddress + PMparameters["noTriggerMode"].address + i, masks(~(1 << PMparameters["noTriggerMode"].bitshift), 0));
            }
        }
        if (transceive()) sync();
    }

    void apply_SC_EVAL_MODE(bool Nchan) {
        TCM.set.SC_EVAL_MODE = Nchan;
        Nchan ? setBit(8, 0xE) : clearBit(8, 0xE);
    }

    void copyActualToSettingsPM(TypePM *pm) {
        memcpy(pm->set.registers0, pm->act.registers0 + TypePM::Settings::block0addr, TypePM::Settings::block0size * wordSize);
        memcpy(pm->set.registers1, pm->act.registers0 + TypePM::Settings::block1addr, TypePM::Settings::block1size * wordSize);
        pm->set.CH_MASK_DATA = pm->act.CH_MASK_DATA;
        memcpy(pm->set.registers2, pm->act.registers0 + TypePM::Settings::block2addr, TypePM::Settings::block2size * wordSize);
        memcpy(pm->set.GBT.registers, pm->act.GBT.Control.registers, GBTunit::controlSize * wordSize);
    }

    void applySettingsPM(TypePM *pm) {
        addTransaction(write, pm->baseAddress + TypePM::Settings::block0addr, pm->set.registers0, TypePM::Settings::block0size);
        addTransaction(write, pm->baseAddress + TypePM::Settings::block1addr, pm->set.registers1, TypePM::Settings::block1size);
        addWordToWrite(pm->baseAddress + PMparameters["CH_MASK_DATA"].address, pm->set.CH_MASK_DATA);
        addTransaction(write, pm->baseAddress + TypePM::Settings::block2addr, pm->set.registers2, TypePM::Settings::block2size);
        addTransaction(write, pm->baseAddress + GBTunit::controlAddress, pm->set.GBT.registers, GBTunit::controlSize);
        transceive();
    }

    void copyActualToSettingsTCM() {
        memcpy(TCM.set.registers0, TCM.act.registers0 + TypeTCM::Settings::block0addr, TypeTCM::Settings::block0size * wordSize);
        memcpy(TCM.set.registers1, TCM.act.registers0 + TypeTCM::Settings::block1addr, TypeTCM::Settings::block1size * wordSize);
        memcpy(TCM.set.registers2, TCM.act.registers0 + TypeTCM::Settings::block2addr, TypeTCM::Settings::block2size * wordSize);
        TCM.set.CH_MASK_C = TCM.act.CH_MASK_C;
        TCM.set.COUNTERS_UPD_RATE = TCM.act.COUNTERS_UPD_RATE;
        memcpy(TCM.set.registers3, TCM.act.registers0 + TypeTCM::Settings::block3addr, TypeTCM::Settings::block3size * wordSize);
        memcpy(TCM.set.GBT.registers, TCM.act.GBT.Control.registers, GBTunit::controlSize * wordSize);
    }

    void applySettingsTCM() {
        addTransaction(write, TypeTCM::Settings::block0addr, TCM.set.registers0, TypeTCM::Settings::block0size);
        addTransaction(write, TypeTCM::Settings::block1addr, TCM.set.registers1, TypeTCM::Settings::block1size);
        addTransaction(write, TypeTCM::Settings::block2addr, TCM.set.registers2, TypeTCM::Settings::block2size - 2); //0x1E should be skipped
        addTransaction(write, TypeTCM::Settings::block2addr + TypeTCM::Settings::block2size - 1, TCM.set.registers2 + TypeTCM::Settings::block2size - 1, 1);
        addWordToWrite(TCMparameters["CH_MASK_C"].address, TCM.set.CH_MASK_C);
        addTransaction(write, TypeTCM::Settings::block3addr, TCM.set.registers3, TypeTCM::Settings::block3size);
        addTransaction(write, GBTunit::controlAddress, TCM.set.GBT.registers, GBTunit::controlSize);

        apply_COUNTERS_UPD_RATE(TCM.set.COUNTERS_UPD_RATE);
    }

    void copyActualToSettingsAll() {
        foreach(TypePM *pm, PM) copyActualToSettingsPM(pm);
        copyActualToSettingsTCM();
    }

    void applySettingsAll() {
        foreach(TypePM *pm, PM) applySettingsPM(pm);
        applySettingsTCM();
    }

    void apply_ORBIT_FILL_MASK() {
        addTransaction(write, 0x2A00, TCM.ORBIT_FILL_MASK, 223);
        transceive();
    }
};

#endif // FITELECTRONICS_H
