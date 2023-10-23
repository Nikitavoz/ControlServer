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
    QHash<DimCommand *, std::function<void(void *)>> allCommands;
    QList<CustomDIMservice *> services, countServices;
    QList<DimCommand *> commands;
    struct TypeServerStatus {
        char string[1024] = "offline";
        DimService *service;
        void update(QString st) {
            qstrcpy(string, qPrintable(st));
            service->updateService();
        }
    } serverStatus;
    TypeTCM TCM;
    TypePM allPMs[20] = { //PMs by link №
		TypePM(0x0200, "A0", TCM.act.TRG_SYNC_A[0]),
		TypePM(0x0400, "A1", TCM.act.TRG_SYNC_A[1]),
		TypePM(0x0600, "A2", TCM.act.TRG_SYNC_A[2]),
		TypePM(0x0800, "A3", TCM.act.TRG_SYNC_A[3]),
		TypePM(0x0A00, "A4", TCM.act.TRG_SYNC_A[4]),
		TypePM(0x0C00, "A5", TCM.act.TRG_SYNC_A[5]),
		TypePM(0x0E00, "A6", TCM.act.TRG_SYNC_A[6]),
		TypePM(0x1000, "A7", TCM.act.TRG_SYNC_A[7]),
		TypePM(0x1200, "A8", TCM.act.TRG_SYNC_A[8]),
		TypePM(0x1400, "A9", TCM.act.TRG_SYNC_A[9]),
		TypePM(0x1600, "C0", TCM.act.TRG_SYNC_C[0]),
		TypePM(0x1800, "C1", TCM.act.TRG_SYNC_C[1]),
		TypePM(0x1A00, "C2", TCM.act.TRG_SYNC_C[2]),
		TypePM(0x1C00, "C3", TCM.act.TRG_SYNC_C[3]),
		TypePM(0x1E00, "C4", TCM.act.TRG_SYNC_C[4]),
		TypePM(0x2000, "C5", TCM.act.TRG_SYNC_C[5]),
		TypePM(0x2200, "C6", TCM.act.TRG_SYNC_C[6]),
		TypePM(0x2400, "C7", TCM.act.TRG_SYNC_C[7]),
		TypePM(0x2600, "C8", TCM.act.TRG_SYNC_C[8]),
		TypePM(0x2800, "C9", TCM.act.TRG_SYNC_C[9]),
    };
    QMap<quint16, TypePM *> PM;
    QList<TypePM *> PMsA, PMsC;
    QFile logFile;
    QTextStream logStream;
    bool PMsReady = false;
	quint8 noResponseCounter = 0;

//debug functions variables
    QMetaObject::Connection adjustConnection;
    bool adjEven = false;
    quint16 thHi[12], thLo[12] = {0};
    double targetRate_Hz = 15.;
    TypePM *targetPM;

    QTimer *countersTimer = new QTimer();
    QTimer *shuttleTimer = new QTimer();
    qint16 shuttleStartPhase = -1024;
//system variables
    quint32 BOARDS_OK = 0xFFFFFFFF;

    FITelectronics(TypeFITsubdetector sd): IPbusTarget(50006), subdetector(sd), TCMid(FIT[sd].TCMid) {
        logFile.setFileName(QCoreApplication::applicationName() + ".log");
        logFile.open(QFile::WriteOnly | QIODevice::Append | QFile::Text);
        logStream.setDevice(&logFile);
        log(qApp->applicationName() + " v" + qApp->applicationVersion() + " started");
        for (quint8 i=0; i<10; ++i) {
            allPMs[i     ].FEEid = FIT[sd].PMA0id + i;
            allPMs[i + 10].FEEid = FIT[sd].PMC0id + i;
        }
        TCM.set.T1_SIGN = FIT[sd].triggers[0].signature;
        TCM.set.T2_SIGN = FIT[sd].triggers[1].signature;
        TCM.set.T3_SIGN = FIT[sd].triggers[2].signature;
        TCM.set.T4_SIGN = FIT[sd].triggers[3].signature;
        TCM.set.T5_SIGN = FIT[sd].triggers[4].signature;
        countersTimer->setTimerType(Qt::PreciseTimer);
        connect(countersTimer, &QTimer::timeout, this, [=](){
            IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
            p.addTransaction(read, 0x0F, &TCM.act.registers[0x0F]); //status register
            if (!transceive(p)) return;
            if (TCM.act.resetSystem) {
                PMsReady = false;
                PM.clear();
                PMsA.clear();
                PMsC.clear();
            }
            readCountersFIFO();
        });
        connect(shuttleTimer, &QTimer::timeout, this, &FITelectronics::inverseLaserPhase);
        connect(this, &IPbusTarget::error     , this, [=](QString message) {
			countersTimer->stop();
			setRatesUnknown();
            serverStatus.update("Error: " + message);
			log("Error: " + message);
        });
        connect(this, &IPbusTarget::noResponse, this, [=](QString message) {
			countersTimer->stop();
			if (noResponseCounter < 3) log(QString::asprintf("No response %d", noResponseCounter++));
			else if (noResponseCounter == 3) {
				++noResponseCounter;
				log("FEE is OFF");
				setRatesUnknown();
				serverStatus.update(message);
			}
        });
        connect(this, &IPbusTarget::IPbusStatusOK, this, [=]() {
			noResponseCounter = 0;
            serverStatus.update("OK");
            if (subdetector == FV0) writeNbits(0xE, 0x3, 2, 8, false); //apply FV0 trigger mode
            IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
            p.addWordToWrite(TCMparameters["T1_SIGN"].address, prepareSignature(FIT[sd].triggers[0].signature));
            p.addWordToWrite(TCMparameters["T2_SIGN"].address, prepareSignature(FIT[sd].triggers[1].signature));
            p.addWordToWrite(TCMparameters["T3_SIGN"].address, prepareSignature(FIT[sd].triggers[2].signature));
            p.addWordToWrite(TCMparameters["T4_SIGN"].address, prepareSignature(FIT[sd].triggers[3].signature));
            p.addWordToWrite(TCMparameters["T5_SIGN"].address, prepareSignature(FIT[sd].triggers[4].signature));
            p.addTransaction(read, TCMparameters["PM_MASK_SPI"].address, &TCM.act.PM_MASK_SPI);

            if (!transceive(p)) return;
            PMsReady = false;
            sync();
			if (TCM.services.isEmpty()) createTCMservices();
        });
        connect(this, &FITelectronics::resetFinished, this, [=]() {
            checkPMlinks();
			initGBT();
			readCountersDirectly();
            apply_COUNTERS_UPD_RATE(TCM.set.COUNTERS_UPD_RATE);
        });

        qRegisterMetaType<DimCommand *>("DIMcommandPointer");
        connect(this, &FITelectronics::DIMcommandReceived, this, &FITelectronics::executeDIMcommand);
        serverStatus.service = new DimService(qPrintable(QString(FIT[sd].name) + "/SERVER_STATUS"), serverStatus.string);
        allCommands.insert(new DimCommand(qPrintable(QString(FIT[sd].name) + "/STOP_SERVER"), "C:1", this), [=](void * ) { qApp->exit(); });
        DIMserver.setDnsNode("localhost");
        DIMserver.start(qPrintable(QString(FIT[sd].name) + "_DIM_SERVER"));
    }

    ~FITelectronics() {
        for (quint8 i=0; i<20; ++i) deletePMservices(allPMs + i);
        deleteTCMservices();
        setRatesUnknown();
        serverStatus.update("offline");
        DIMserver.stop();
        log(qApp->applicationName() + " v" + qApp->applicationVersion() + " stopped");
        logFile.close();
    }

	void log(QString st) { logStream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + st << Qt::endl; logStream.flush(); }

    void addCommand(QList<DimCommand *> &list, QString name, const char* format, std::function<void(void *)> function) {
        DimCommand *command = new DimCommand(qPrintable(name), format, this);
        list.append(command);
        allCommands.insert(command, function);
    }
    void addArrayCommand(QString parameter) { //PM parameters only for now
        if (!PMparameters.contains(parameter)) {
            emit error("No such PM parameter: " + parameter, logicError);
            return;
        }
        const Parameter par = PMparameters[parameter];
        if (par.interval == 0) emit error("Not an array parameter!", logicError);
        addCommand(commands, QString(FIT[subdetector].name) + "/" + parameter + "/apply", "I", [=](void *d) {
            qint32 &id = ((qint32 *)d)[0], *V = (qint32 *)d + 1;
            if (id == -1) {//all channels at once
                for(quint8 iPM=0; iPM<20; ++iPM) for (quint8 iCh=0; iCh<12; ++iCh) allPMs[iPM].setParameter(parameter, V[20*iCh + iPM], iCh);
                IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
                foreach(auto side, QList({PMsA, PMsC})) {
                    foreach(TypePM *pm, side) for (quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh) {
						quint32 address = par.address + iCh * par.interval, value = changeNbits(pm->act.registers[address], par.bitwidth, par.bitshift, V[20*iCh + iPM]);
						p.addWordToWrite(pm->baseAddress + address, value);
                    }
                    if (!p.transactionsList.isEmpty()) transceive(p);
                }
                sync();
            } else if (id < 240) {
                quint8 iPM = id % 20, iCh = id / 20;
                if ((1 << iPM & TCM.act.PM_MASK_SPI) == 0) return;
                TypePM *pm = allPMs + iPM;
                pm->setParameter(parameter, *V, iCh);
                writeParameter(parameter, *V, pm->FEEid, iCh);
            }
        });
    }

    void createPMservices(TypePM *pm) {
        QString prefix = QString::asprintf("%s/PM%s/", FIT[subdetector].name, pm->name);
        pm->services.append(new DimService(qPrintable(prefix+"status/TEMP_BOARD"      ), "F", &pm->act.TEMP_BOARD      , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/TEMP_FPGA"       ), "F", &pm->act.TEMP_FPGA       , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1V"      ), "F", &pm->act.VOLTAGE_1V      , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1_8V"    ), "F", &pm->act.VOLTAGE_1_8V    , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/BOARD_TYPE"      ), "C:4",pm->act.BOARD_TYPE      , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/FW_TIME_MCU"     ), "I", &pm->act.FW_TIME_MCU     , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/FW_TIME_FPGA"    ), "I", &pm->act.FW_TIME_FPGA    , 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/CH_BASELINES_NOK"), "I", &pm->act.CH_BASELINES_NOK, 4));
        pm->services.append(new DimService(qPrintable(prefix+"status/SERIAL_NUM"      ), "S", (char *)&pm->act.registers[0xBD]+ 1, 2));
        pm->services.append(new DimService(qPrintable(prefix+"control/CH_MASK_DATA""/actual"), "I", &pm->act.CH_MASK_DATA, 4));
        pm->services.append(new DimService(qPrintable(prefix+"control/CH_MASK_TRG" "/actual"), "I", &pm->act.CH_MASK_TRG , 4));
    }

    void createTCMservices() { //+ system services
		QString prefix = QString::asprintf("%s/TCM/", FIT[subdetector].name);
        TCM.services.append(new DimService(qPrintable(prefix+"status/TEMP_BOARD"  ), "F", &TCM.act.TEMP_BOARD                   , 4));
        TCM.services.append(new DimService(qPrintable(prefix+"status/TEMP_FPGA"   ), "F", &TCM.act.TEMP_FPGA                    , 4));
        TCM.services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1V"  ), "F", &TCM.act.VOLTAGE_1V                   , 4));
        TCM.services.append(new DimService(qPrintable(prefix+"status/VOLTAGE_1_8V"), "F", &TCM.act.VOLTAGE_1_8V                 , 4));
        TCM.services.append(new DimService(qPrintable(prefix+"status/SERIAL_NUM"  ), "S", (char *)&TCM.act.registers[0x7] + 1, 2));
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
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd0/NAME"), const_cast<char *>("NoiseA: A-side out-of-gate hits AND NOT OrA"			) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd1/NAME"), const_cast<char *>("NoiseC: C-side out-of-gate hits AND NOT OrC"			) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd2/NAME"), const_cast<char *>("Total noise: NoiseA OR NoiseC"							) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd3/NAME"), const_cast<char *>("True OrA: bunch in both beams AND OrA"					) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd4/NAME"), const_cast<char *>("True OrC: bunch in both beams AND OrC"					) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd5/NAME"), const_cast<char *>("Interaction: both sides Or (OrA AND OrC)"				) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd6/NAME"), const_cast<char *>("True Interaction: bunch in both beams AND Interaction"	) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd7/NAME"), const_cast<char *>("True Vertex: bunch in both beams AND Vertex"			) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd8/NAME"), const_cast<char *>("Background A: bunch ONLY in beam1 AND OrC"				) ));
        TCM.staticServices.append(new DimService( qPrintable(prefix+"Bkgrnd9/NAME"), const_cast<char *>("Background C: bunch ONLY in beam2 AND OrA"				) ));

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

		addCommand(TCM.commands, prefix+"control/ORBIT_FILL_MASK/set"  , "I:223", [=](void *d) { memcpy(TCM.ORBIT_FILL_MASK, d, 223*wordSize); apply_ORBIT_FILL_MASK(); }); //223 = ceil( 0xDEC * 2 / 32 )
		addCommand(TCM.commands, prefix+"control/ORBIT_FILL_MASK/apply", "I:223", [=](void *d) { memcpy(TCM.ORBIT_FILL_MASK, d, 223*wordSize); apply_ORBIT_FILL_MASK(); });

        prefix = QString(FIT[subdetector].name) + "/"; //for system services
        services.append(new CustomDIMservice(qPrintable(prefix+"BOARDS_OK"), "I", 4, {}, &BOARDS_OK));
        services.append(new CustomDIMservice(qPrintable(prefix+"LASER_ENABLED"      "/actual"), "S", 2, [=](void *d) { *(quint16 *)d = TCM.act.LASER_ENABLED        ; }));
        services.append(new CustomDIMservice(qPrintable(prefix+"LASER_SOURCE"       "/actual"), "S", 2, [=](void *d) { *(quint16 *)d = TCM.act.LASER_SOURCE         ; }));
        services.append(new CustomDIMservice(qPrintable(prefix+"LASER_DIVIDER"      "/actual"), "I", 4, [=](void *d) { *(quint32 *)d = TCM.act.LASER_DIVIDER        ; }));
        services.append(new CustomDIMservice(qPrintable(prefix+"LASER_FREQUENCY_Hz" "/actual"), "F", 4,              { },             &TCM.act.laserFrequency_Hz       ));
        services.append(new CustomDIMservice(qPrintable(prefix+"LASER_DELAY_ns"     "/actual"), "F", 4,              { },             &TCM.act.delayLaser_ns           ));
        services.append(new CustomDIMservice(qPrintable(prefix+"LASER_PATTERN"      "/actual"), "X", 8,              { },             &TCM.act.LASER_PATTERN           ));
        services.append(new CustomDIMservice(qPrintable(prefix+"LSR_TRG_SUPPR_DUR"  "/actual"), "I", 4, [=](void *d) { *(quint32 *)d = TCM.act.lsrTrgSupprDur       ; }));
        services.append(new CustomDIMservice(qPrintable(prefix+"LSR_TRG_SUPPR_DELAY""/actual"), "I", 4, [=](void *d) { *(quint32 *)d = TCM.act.lsrTrgSupprDelay     ; }));
        services.append(new CustomDIMservice(qPrintable(prefix+"ATTEN_STEPS"        "/actual"), "I", 4, [=](void *d) { *(quint32 *)d = TCM.act.attenSteps           ; }));
        services.append(new CustomDIMservice(qPrintable(prefix+"ATTEN_STATUS"                ), "I", 4, [=](void *d) { *(quint32 *)d = TCM.act.registers[0x3] >> 14 ; }));

        addCommand(commands, prefix+"LOAD_CONFIG"   , "C"  , [=](void *d) { QString name((char *)d); fileRead(name, true); });
        addCommand(commands, prefix+"CLEAR_ERRORS"  , "C:1", [=](void * ) { apply_RESET_ERRORS(); });
        addCommand(commands, prefix+"RECONNECT"     , "C:1", [=](void * ) { reconnect(); });
        addCommand(commands, prefix+"RESTART_SYSTEM", "C:1", [=](void * ) { apply_RESET_SYSTEM(false); });

        addCommand(commands, prefix+"LASER_ENABLED"      "/apply", "S", [=](void *d) { apply_LASER_ENABLED            (*(bool    *)d); });
        addCommand(commands, prefix+"LASER_SOURCE"       "/apply", "S", [=](void *d) { apply_LASER_SOURCE             (*(bool    *)d); });
        addCommand(commands, prefix+"LASER_FREQUENCY_Hz" "/apply", "F", [=](void *d) { TCM.set.calculate_LASER_DIVIDER(*(float   *)d); apply_LASER_DIVIDER      (); });
        addCommand(commands, prefix+"LASER_DELAY_ns"     "/apply", "F", [=](void *d) { TCM.set.calculate_LASER_DELAY  (*(float   *)d); apply_LASER_DELAY        (); });
		addCommand(commands, prefix+"LASER_PATTERN"      "/apply", "X", [=](void *d) { TCM.set.LASER_PATTERN		 = *(quint64 *)d ; apply_LASER_PATTERN      (); });
        addCommand(commands, prefix+"LSR_TRG_SUPPR_DUR"  "/apply", "I", [=](void *d) { TCM.set.lsrTrgSupprDur        = *(quint32 *)d ; apply_LSR_TRG_SUPPR_DUR  (); });
        addCommand(commands, prefix+"LSR_TRG_SUPPR_DELAY""/apply", "I", [=](void *d) { TCM.set.lsrTrgSupprDelay      = *(quint32 *)d ; apply_LSR_TRG_SUPPR_DELAY(); });
        addCommand(commands, prefix+"LASER_DIVIDER"      "/apply", "I", [=](void *d) { TCM.set.LASER_DIVIDER         = *(quint32 *)d ; apply_LASER_DIVIDER      (); });
        addCommand(commands, prefix+"ATTEN_STEPS"        "/apply", "I", [=](void *d) { TCM.set.attenSteps            = *(quint32 *)d ; apply_attenSteps         (); });

        services.append(new CustomDIMservice(qPrintable(prefix+"ADC_RMS"                ), "F:480", 4*2*12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh) {
            ((float   *)d)[  0 + 20*iCh + iPM] = pm->act.RMS_Ch[iCh][0];
            ((float   *)d)[240 + 20*iCh + iPM] = pm->act.RMS_Ch[iCh][1];                                                    } }));
        services.append(new CustomDIMservice(qPrintable(prefix+"ADC_MEANAMPL"           ), "I:480", 4*2*12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh) {
            (( qint32 *)d)[  0 + 20*iCh + iPM] = pm->act.MEANAMPL[iCh][0][0];
            (( qint32 *)d)[240 + 20*iCh + iPM] = pm->act.MEANAMPL[iCh][1][0];                                               } }));
        services.append(new CustomDIMservice(qPrintable(prefix+"ADC_BASELINE"           ), "I:480", 4*2*12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh) {
            ((quint32 *)d)[  0 + 20*iCh + iPM] = pm->act.ADC_BASELINE[iCh][0];
            ((quint32 *)d)[240 + 20*iCh + iPM] = pm->act.ADC_BASELINE[iCh][1];                                              } }));
        services.append(new CustomDIMservice(qPrintable(prefix+"ADC_ZERO"      "/actual"), "I:240", 4  *12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh)
            (( qint32 *)d)[  0 + 20*iCh + iPM] = pm->act.Ch[iCh].ADC_ZERO;                                                  }));
        services.append(new CustomDIMservice(qPrintable(prefix+"ADC_DELAY"     "/actual"), "I:240", 4  *12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh)
            ((quint32 *)d)[  0 + 20*iCh + iPM] = pm->act.Ch[iCh].ADC_DELAY;                                                 }));
        services.append(new CustomDIMservice(qPrintable(prefix+"ADC_RANGE"     "/actual"), "I:480", 4*2*12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh) {
            ((quint32 *)d)[  0 + 20*iCh + iPM] = pm->act.ADC_RANGE[iCh][0];
            ((quint32 *)d)[240 + 20*iCh + iPM] = pm->act.ADC_RANGE[iCh][1];                                                 } }));
        services.append(new CustomDIMservice(qPrintable(prefix+"TIME_ALIGN"    "/actual"), "I:240", 4  *12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh)
            (( qint32 *)d)[  0 + 20*iCh + iPM] = pm->act.timeAlignment[iCh].value;                                          }));
        services.append(new CustomDIMservice(qPrintable(prefix+"CFD_ZERO"      "/actual"), "I:240", 4  *12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh)
            (( qint32 *)d)[  0 + 20*iCh + iPM] = pm->act.Ch[iCh].CFD_ZERO;                                                  }));
        services.append(new CustomDIMservice(qPrintable(prefix+"CFD_THRESHOLD" "/actual"), "I:240", 4  *12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh)
            ((quint32 *)d)[  0 + 20*iCh + iPM] = pm->act.Ch[iCh].CFD_THRESHOLD;                                             }));
        services.append(new CustomDIMservice(qPrintable(prefix+"THRESHOLD_CALIBR/actual"), "I:240", 4  *12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh)
            ((quint32 *)d)[  0 + 20*iCh + iPM] = pm->act.THRESHOLD_CALIBR[iCh];                                             }));

        countServices.append(new CustomDIMservice(qPrintable(prefix+"CNT_RATE_CH"       ), "F:480", 4*2*12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh) {
            ((float   *)d)[  0 + 20*iCh + iPM] = pm->counters.rateCh[iCh].CFD;
            ((float   *)d)[240 + 20*iCh + iPM] = pm->counters.rateCh[iCh].TRG;                                              } }));
        countServices.append(new CustomDIMservice(qPrintable(prefix+"CNT_CH"            ), "I:480", 4*2*12*20, [=](void *d) { foreach(TypePM *pm, PM) for(quint8 iCh=0, iPM=pm-allPMs; iCh<12; ++iCh) {
            ((quint32 *)d)[  0 + 20*iCh + iPM] = pm->counters.Ch[iCh].CFD;
            ((quint32 *)d)[240 + 20*iCh + iPM] = pm->counters.Ch[iCh].TRG;                                                  } }));

        services.append(new CustomDIMservice(qPrintable(prefix+"CH_MASK_DATA"  "/actual"), "I:20" , 4     *20, [=](void *d) { foreach(TypePM *pm, PM) ((quint32 *)d)[pm-allPMs] = pm->act.CH_MASK_DATA; }));
        services.append(new CustomDIMservice(qPrintable(prefix+"CH_MASK_TRG"   "/actual"), "I:20" , 4     *20, [=](void *d) { foreach(TypePM *pm, PM) ((quint32 *)d)[pm-allPMs] = pm->act.CH_MASK_TRG ; }));

        services.append(new CustomDIMservice(qPrintable(prefix+"GBT/RX_PHASE"           ), "I:21" , 4     *21, [=](void *d) {
            for(TypePM *pm=allPMs, *e=pm+20; pm<e; ++pm) ((quint32 *)d)[pm-allPMs] = (PM.contains(pm->FEEid) ? pm->act.GBT.Status.RX_PHASE : -1);
                                                         ((quint32 *)d)[20]        =                           TCM.act.GBT.Status.RX_PHASE;
        }));

        addArrayCommand("THRESHOLD_CALIBR");
        addArrayCommand("ADC_ZERO"        );
        addArrayCommand("ADC_DELAY"       );
        addArrayCommand("TIME_ALIGN"      );
        addArrayCommand("CFD_ZERO"        );
        addArrayCommand("CFD_THRESHOLD"   );
        addCommand(commands, prefix+"ADC_RANGE/apply", "I", [=](void *d) {
            qint32 &id = ((qint32 *)d)[0], *V = (qint32 *)d + 1;
            if (id == -1) {
                for(quint8 iPM=0; iPM<20; ++iPM) for (quint8 iCh=0; iCh<12; ++iCh) { allPMs[iPM].set.ADC_RANGE[iCh][0] = V[20*iCh + iPM]; allPMs[iPM].set.ADC_RANGE[iCh][1] = V[240 + 20*iCh + iPM]; }
                IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
                foreach(auto side, QList({PMsA, PMsC})) {
                    foreach(TypePM *pm, side) p.addTransaction(write, pm->baseAddress + PMparameters["ADC0_RANGE"].address, pm->set.ADC_RANGE[0], 24);
                    if (!p.transactionsList.isEmpty()) transceive(p);
                }
                sync();
            } else if (id < 480) {
                quint8 iPM = id % 20, iCh = id / 20 % 12, iADC = id / 240;
                if ((1 << iPM & TCM.act.PM_MASK_SPI) == 0) return;
                TypePM *pm = allPMs + iPM;
                pm->set.ADC_RANGE[iCh][iADC] = *V;
                writeRegister(pm->baseAddress + PMparameters["ADC0_RANGE"].address + 2*iCh + iADC, *V);
            }
        });
        addCommand(commands, prefix+"CH_MASK_DATA/apply", "I", [=](void *d) {
            qint32 &id = ((qint32 *)d)[0], *V = (qint32 *)d + 1;
            if (id == -1) {
                for(quint8 iPM=0; iPM<20; ++iPM) { allPMs[iPM].set.CH_MASK_DATA = V[iPM]; }
                IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
                foreach(TypePM *pm, PM) p.addWordToWrite(pm->baseAddress + PMparameters["CH_MASK_DATA"].address, pm->set.CH_MASK_DATA);
                if (!p.transactionsList.isEmpty() && transceive(p)) sync();
            } else if (id < 240) {
                quint8 iPM = id % 20, iCh = id / 20;
                if ((1 << iPM & TCM.act.PM_MASK_SPI) == 0) return;
                TypePM *pm = allPMs + iPM;
                pm->set.CH_MASK_DATA = changeNbits(pm->set.CH_MASK_DATA, 1, iCh, *V);
                *V ? setBit(iCh, pm->baseAddress + PMparameters["CH_MASK_DATA"].address) : clearBit(iCh, pm->baseAddress + PMparameters["CH_MASK_DATA"].address);
            }
        });
        addCommand(commands, prefix+"CH_MASK_TRG/apply", "I", [=](void *d) {
            qint32 &id = ((qint32 *)d)[0], *V = (qint32 *)d + 1;
            if (id == -1) {
                for(quint8 iPM=0; iPM<20; ++iPM) for (quint8 iCh=0; iCh<12; ++iCh) {
                    bool enableTrigger = V[iPM] & 1<<iCh;
                    allPMs[iPM].setParameter("noTriggerMode", !enableTrigger, iCh);
                    allPMs[iPM].act.timeAlignment[iCh].blockTriggers = !enableTrigger;
                }
                IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
                foreach(TypePM *pm, PM) p.addTransaction(write, pm->baseAddress + PMparameters["noTriggerMode"].address, (quint32 *)&pm->act.timeAlignment, 12);
                if (!p.transactionsList.isEmpty() && transceive(p)) sync();
            } else if (id < 240) {
                quint8 iPM = id % 20, iCh = id / 20;
                bool enableTrigger = *V;
                if ((1 << iPM & TCM.act.PM_MASK_SPI) == 0) return;
                TypePM *pm = allPMs + iPM;
                pm->setParameter("noTriggerMode", !enableTrigger, iCh);
                writeParameter("noTriggerMode", !enableTrigger, pm->FEEid, iCh);
            }
        });

		addCommand(commands, prefix+"RESET_COUNTS", "I", [=](void *d) {
			qint32 id = *(qint32 *)d;
			if (id > 20 || id < -2) return;
			resetCounts(id >= 0 ? (id == 20 ? TCMid : allPMs[id].FEEid) : id);
		});

        addCommand(commands, prefix+"GBT/SUPPRESS_ERROR_REPORTS", "I", [=](void *d) { switchGBTerrorReports(!*(qint32 *)d); });
    }

    void deletePMservices(TypePM *pm) {
        foreach (DimService *s, pm->services) delete s;
        pm->services.clear();
//        pm->counters.services.clear();
		foreach (DimCommand *c, pm->commands) { allCommands.remove(c); delete c; }
        pm->commands.clear();
    }

    void deleteTCMservices() {
        foreach (DimService *s, TCM.services + TCM.counters.services + TCM.staticServices) delete s;
        TCM.services.clear();
        TCM.counters.services.clear();
        TCM.staticServices.clear();
        foreach (CustomDIMservice *s, services) delete s;
        services.clear();
        foreach (DimCommand *c, TCM.commands) { allCommands.remove(c); delete c; }
        TCM.commands.clear();
    }

    void commandHandler() { DimCommand *c = getCommand(); emit DIMcommandReceived(c, c->getData()); }

signals:
    void linksStatusReady();
    void valuesReady();
    void countersReady(quint16 FEEid);
    void resetFinished();
    void DIMcommandReceived(DimCommand *, void *);

public slots:
    void executeDIMcommand(DimCommand *cmd, void *data) { allCommands[cmd](data); }

    void fileWrite(QString fileName) {
        QSettings newset(fileName, QSettings::IniFormat);
        newset.remove("TCM");
        newset.beginGroup("TCM");
        foreach (regblock b, TCM.set.regblocksToRead) for (quint8 i=b.addr; i<=b.endAddr; ++i) newset.setValue(QString::asprintf("reg%02X", i), QString::asprintf("%08X", TCM.set.registers[i]));
        newset.endGroup();
        foreach (TypePM *pm, PM) {
            newset.remove(QString("PM") + pm->name);
            if (!TRGsyncEnabledForPM(pm-allPMs)) continue;
            newset.beginGroup(QString("PM") + pm->name);
            foreach(regblock b, pm->set.regblocks) for (quint8 i=b.addr; i<=b.endAddr; ++i) newset.setValue(QString::asprintf("reg%02X", i), QString::asprintf("%08X", pm->set.registers[i]));
            newset.endGroup();
        }
        newset.sync();
    }

    void fileRead(QString fileName, bool doApply = false) {
        if (!QFileInfo::exists(fileName)) {
            fileName.prepend("./configuration/");
            if (!QFileInfo::exists(fileName)) fileName = "./configuration/default.cfg";
        }
        QSettings newset(fileName, QSettings::IniFormat);
        if (newset.contains("TCM")) { //old settings format
            quint32 *r = (quint32 *)newset.value("TCM").toByteArray().remove(64,4).data(); //PM_MASK_SPI is not a setting value starting from v1.e, so 4 bytes are removed
            foreach (regblock b, TCM.set.regblocksToRead) for (quint8 i=b.addr; i<=b.endAddr; ++i) TCM.set.registers[i] = *r++;
            if (doApply) applySettingsTCM();
        } else { //new settings format
            newset.beginGroup("TCM");
            if (!newset.childKeys().isEmpty()) {
                QMap<quint8, quint32> M;
                foreach (QString reg, newset.childKeys()) {
                    bool addressOK, valueOK;
                    quint16 address = reg.rightRef(2).toUShort(&addressOK, 16);
                    quint32 value = newset.value(reg).toString().toUInt(&valueOK, 16);
                    if (valueOK && addressOK && address < 256) M[address] = value;
                    TCM.set.registers[address] = value;
                }
                if (doApply) {
                    IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
                    quint8 a = TCMparameters["DELAY_A"].address; if (M.contains(a)) p.addWordToWrite(a, M[a]);
                           a = TCMparameters["DELAY_C"].address; if (M.contains(a)) p.addWordToWrite(a, M[a]);
                    if (!p.transactionsList.isEmpty()) { //phase is to be changed
                        qint32 delay_ms = qMax(qAbs(TCM.set.DELAY_A - TCM.act.DELAY_A), qAbs(TCM.set.DELAY_C - TCM.act.DELAY_C)) + 10; //phase needs time to move
                        if (!transceive(p)) return;
                        if (delay_ms > 1) QThread::msleep(delay_ms); //waiting for phases shift to finish
                    }
                    M.remove(TCMparameters["COUNTERS_UPD_RATE"].address); //will be applied afterwards
                    if (!M.isEmpty()) {
						if (M.contains(0xE)) {//reg0E contains TCM histogram settings (bits 4..7 and 10), they should not change
							quint32 mask = 0x0000030F; //only bits 0..3 and 8..9 will be written
							p.addTransaction(RMWbits, 0xE, p.masks(~mask, M[0xE] & mask));
							M.remove(0xE);
						}
						foreach(quint8 a, M.keys()) p.addWordToWrite(a, M[a]);
                        if (!transceive(p)) return;
                    }
                    if (M.contains(TCMparameters["CH_MASK_A"].address) || M.contains(TCMparameters["CH_MASK_C"].address)) {
                        QThread::msleep(10);
                    }
					apply_RESET_ERRORS();
                    if (newset.childKeys().contains("reg50") && TCM.act.COUNTERS_UPD_RATE != TCM.set.COUNTERS_UPD_RATE) apply_COUNTERS_UPD_RATE(TCM.set.COUNTERS_UPD_RATE);
                }
            }
            newset.endGroup();
        }
        TypePM *pm = allPMs;
        for (quint8 i=0; i<20; ++i, ++pm) {
            QString name = QString("PM") + pm->name;
            if (newset.contains(name)) { //old settings format
                quint32 *r = (quint32 *)newset.value(name).toByteArray().data();
                foreach (regblock b, pm->set.regblocks) for (quint8 i=b.addr; i<=b.endAddr; ++i) pm->set.registers[i] = *r++;
                if (doApply && TCM.act.PM_MASK_SPI & 1 <<i) applySettingsPM(pm);
            } else { //new settings format
                newset.beginGroup(name);
                if (!newset.childKeys().isEmpty()) {
                    QMap<quint8, quint32> M;
                    foreach (QString reg, newset.childKeys()) {
                        bool addressOK, valueOK;
                        quint16 address = reg.rightRef(2).toUShort(&addressOK, 16);
                        quint32 value = newset.value(reg).toString().toUInt(&valueOK, 16);
                        if (valueOK && addressOK && address < 256) M[address] = value;
                        pm->set.registers[address] = value;
                    }
                    if (doApply && TCM.act.PM_MASK_SPI & 1 <<i) {
                        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
                        foreach(quint8 a, M.keys()) p.addWordToWrite(pm->baseAddress + a, M[a]);
                        if (!transceive(p)) return;
                    }
                }
                newset.endGroup();
            }
        }
		log("Settings loaded" + QString(doApply ? " and applied" : "") + " from file " + fileName);
    }

    void clearFIFOs() {
		quint32 load = readRegister(TypeTCM::Counters::addressFIFOload);
        if (load == 0xFFFFFFFF) return;
        while (load) {
            IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
            p.addTransaction(nonIncrementingRead, TypeTCM::Counters::addressFIFO, nullptr, load > 255 ? 255 : load);
            p.addTransaction(read, TypeTCM::Counters::addressFIFOload, &load);
            if (!transceive(p)) return;
		}
        foreach (TypePM *pm, PM) {
			load = readRegister(pm->baseAddress + TypePM::Counters::addressFIFOload);
            if (load == 0xFFFFFFFF) continue;
			while (load) {
                IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
                p.addTransaction(nonIncrementingRead, pm->baseAddress + TypePM::Counters::addressFIFO, nullptr, load > 255 ? 255 : load);
                p.addTransaction(read, pm->baseAddress + TypePM::Counters::addressFIFOload, &load);
				if (!transceive(p)) return;
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
//				countersTimer->start(countersUpdatePeriod_ms[val]/2);
				countersTimer->start(50);
            }
        } else emit error("Wrong COUNTERS_UPD_RATE value: " + QString::number(val), logicError);
    }

	void initGBT() {        
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        if (quint16(readRegister(GBTparameters["RDH_FEE_ID"].address)) != TCMid) {
            for (quint8 j=0; j<GBTunit::controlSize; ++j) if (j != GBTparameters["BCID_DELAY"].address - GBTunit::controlAddress) TCM.set.GBT.registers[j] = GBTunit::defaults[j];
            TCM.set.GBT.RDH_FEE_ID = TCMid;
            TCM.set.GBT.RDH_SYS_ID = FIT[subdetector].systemID;
            p.addTransaction(write, GBTunit::controlAddress, TCM.set.GBT.registers, GBTunit::controlSize);
        }
        foreach (TypePM *pm, PM) if (quint16(readRegister(pm->baseAddress + GBTparameters["RDH_FEE_ID"].address)) != pm->FEEid) {
            for (quint8 j=0; j<GBTunit::controlSize; ++j) if (j != GBTparameters["BCID_DELAY"].address - GBTunit::controlAddress) pm->set.GBT.registers[j] = GBTunit::defaults[j];
            pm->set.GBT.RDH_FEE_ID = pm->FEEid;
            pm->set.GBT.RDH_SYS_ID = FIT[subdetector].systemID;
            p.addTransaction(write, pm->baseAddress + GBTunit::controlAddress, pm->set.GBT.registers, GBTunit::controlSize);
        }
        if (!p.transactionsList.isEmpty()) transceive(p);
    }

    void checkPMlinks() {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(read, TCMparameters["PM_MASK_SPI"].address, &TCM.act.PM_MASK_SPI);
        p.addTransaction(read, TCMparameters["CH_MASK_A"].address, p.dt);
        p.addTransaction(read, TCMparameters["CH_MASK_C"].address, p.dt + 1);
        if (transceive(p)) {
            TCM.act.CH_MASK_A = p.dt[0];
            TCM.act.CH_MASK_C = p.dt[1];
        } else return;

        PM.clear();
        PMsA.clear();
        PMsC.clear();
        for (quint8 i=0; i<20; ++i) {
            if (!(TCM.act.PM_MASK_SPI >> i & 1)) setBit(i, TCMparameters["PM_MASK_SPI"].address, false);
            if (readRegister(allPMs[i].baseAddress + 0xFE) == 0xFFFFFFFF) { //SPI or IPbus error
                clearBit(i, TCMparameters["PM_MASK_SPI"].address, false);
				deletePMservices(allPMs + i);
            } else {
                TCM.act.PM_MASK_SPI |= 1 << i;
				PM.insert(allPMs[i].FEEid, allPMs + i);
                (i < 10 ? PMsA : PMsC).append(allPMs + i);
				if (allPMs[i].services.isEmpty()) createPMservices(allPMs + i);
//                if (i > 9) TCM.set.CH_MASK_C |= 1 << (i - 10);
//                else       TCM.set.CH_MASK_A |= 1 << i;
            }
        }
        if (!TCM.act.CH_MASK_A && TCM.set.CH_MASK_A) p.addWordToWrite(TCMparameters["CH_MASK_A"].address, TCM.set.CH_MASK_A);
        if (!TCM.act.CH_MASK_C && TCM.set.CH_MASK_C) p.addWordToWrite(TCMparameters["CH_MASK_C"].address, TCM.set.CH_MASK_C);
        if (p.transactionsList.isEmpty() || transceive(p)) emit linksStatusReady();
    }

    void writeParameter(QString name, quint64 val, quint16 FEEid, quint8 iCh = 0) {
        if ( !GBTparameters.contains(name) && (FEEid == TCMid ? !TCMparameters.contains(name) : !PMparameters.contains(name)) ) {
            emit error("'" + name + "' - no such parameter", logicError);
            return;
        }
        Parameter p(GBTparameters.contains(name) ? GBTparameters[name] : (FEEid == TCMid ? TCMparameters[name] : PMparameters[name]));
        quint16 address = p.address + (FEEid == TCMid ? 0 : PM[FEEid]->baseAddress + iCh * p.interval);
        if (p.bitwidth == 32)
            writeRegister(address, val);
        else if (p.bitwidth == 1)
            val != 0 ? setBit(p.bitshift, address) : clearBit(p.bitshift, address);
        else if (p.bitwidth == 64) {
            quint32 w[2] = {quint32(val), quint32(val >> 32)};
            IPbusControlPacket packet; connect(&packet, &IPbusControlPacket::error, this, &IPbusTarget::error);
            packet.addTransaction(write, address, w, 2);
            if (transceive(packet)) sync();
        } else
            writeNbits(address, val, p.bitwidth, p.bitshift);
    }

    void readCountersFIFO() {
        quint16 time_ms = countersUpdatePeriod_ms[TCM.act.COUNTERS_UPD_RATE];
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(read, TypeTCM::Counters::addressFIFOload, &TCM.counters.FIFOload);
        foreach (TypePM *pm, PM) p.addTransaction(read, pm->baseAddress + TypePM::Counters::addressFIFOload, &pm->counters.FIFOload);
        if (!transceive(p)) return;
        if (TCM.counters.FIFOload) p.addTransaction(nonIncrementingRead, TypeTCM::Counters::addressFIFO, TCM.counters.New, TypeTCM::Counters::number);
        foreach (TypePM *pm, PM) if (pm->counters.FIFOload) {
            if (maxPacket - p.responseSize <= TypePM::Counters::number) if (!transceive(p)) return;
            p.addTransaction(nonIncrementingRead, pm->baseAddress + TypePM::Counters::addressFIFO, pm->counters.New, TypePM::Counters::number);
        }
		if (p.requestSize > 1) if (!transceive(p)) return;
        if (TCM.counters.FIFOload) {
            TCM.counters.oldTime = QDateTime::currentDateTime();
            for (quint8 i=0; i<TypeTCM::Counters::number; ++i) {
                TCM.counters.rate[i] = (TCM.counters.New[i] - TCM.counters.Old[i]) * 1000. / time_ms;
                TCM.counters.Old[i] = TCM.counters.New[i];
            }
            foreach (DimService *s, TCM.counters.services) s->updateService();
            emit countersReady(TCMid);
        }
        foreach (TypePM *pm, PM) if (pm->counters.FIFOload) {
            pm->counters.oldTime = TCM.counters.oldTime;
            for (quint8 i=0; i<TypePM::Counters::number; ++i) {
                pm->counters.rate[i] = (pm->counters.New[i] - pm->counters.Old[i]) * 1000. / time_ms;
                pm->counters.Old[i] = pm->counters.New[i];
            }
            emit countersReady(pm->FEEid);
        }
        foreach (CustomDIMservice *s, countServices) s->updateService();
    }

    void readCountersDirectly() {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(read, TypeTCM::Counters::addressDirect, TCM.counters.New, TypeTCM::Counters::number);
        if (transceive(p)) {
            TCM.counters.newTime = QDateTime::currentDateTime();
            quint32 time_ms = TCM.counters.oldTime.msecsTo(TCM.counters.newTime);
            if (time_ms < 100) return;
            for (quint8 i=0; i<TypeTCM::Counters::number; ++i) {
				TCM.counters.rate[i] = (TCM.counters.New[i] - TCM.counters.Old[i]) * 1000. / time_ms;
                TCM.counters.Old[i] = TCM.counters.New[i];
            }
            TCM.counters.oldTime = TCM.counters.newTime;
            foreach (DimService *s, TCM.counters.services) s->updateService();
            emit countersReady(TCMid);
        } else return;
        foreach (TypePM *pm, PM) {
            p.addTransaction(read, pm->baseAddress + TypePM::Counters::addressDirect, pm->counters.New, TypePM::Counters::number);
            if (!transceive(p) || !PM.contains(pm->FEEid)) continue;
            pm->counters.newTime = QDateTime::currentDateTime();
            quint32 time_ms = pm->counters.oldTime.msecsTo(pm->counters.newTime);
            for (quint8 i=0; i<TypePM::Counters::number; ++i) {
                pm->counters.rate[i] = (pm->counters.New[i] - pm->counters.Old[i]) * 1000. / time_ms;
                pm->counters.Old[i] = pm->counters.New[i];
            }
			pm->counters.oldTime = pm->counters.newTime;
            emit countersReady(pm->FEEid);
        }
        foreach (CustomDIMservice *s, countServices) s->updateService();
    }

	void resetCounts(qint32 FEEid) {
		IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
		if (TCM.act.COUNTERS_UPD_RATE) { //HW timer
			readCountersFIFO();
			if (FEEid == -1 || FEEid == TCMid) {
				p.addTransaction(read, TypeTCM::Counters::addressDirect, TCM.counters.New, TypeTCM::Counters::number);
				p.addTransaction(RMWbits, 0xF, p.masks(0xFFFFFFFF, 1 << 9)); //set counters reset bit
                if (transceive(p)) for (quint8 i=0; i<TypeTCM::Counters::number; ++i) TCM.counters.Old[i] -= TCM.counters.New[i]; else return;
			}
			if (FEEid == -1 || FEEid == -2) {
				foreach(TypePM *pm, PMsA) {
					p.addTransaction(read, pm->baseAddress + TypePM::Counters::addressDirect, pm->counters.New, TypePM::Counters::number);
					p.addTransaction(RMWbits, 0x7F + pm->baseAddress, p.masks(0xFFFFFFFF, 1 << 9));
				}
				if (transceive(p)) { foreach(TypePM *pm, PMsA) for (quint8 i=0; i<TypePM::Counters::number; ++i) pm->counters.Old[i] -= pm->counters.New[i]; }
				foreach(TypePM *pm, PMsC) {
					p.addTransaction(read, pm->baseAddress + TypePM::Counters::addressDirect, pm->counters.New, TypePM::Counters::number);
					p.addTransaction(RMWbits, 0x7F + pm->baseAddress, p.masks(0xFFFFFFFF, 1 << 9));
				}
				if (transceive(p)) { foreach(TypePM *pm, PMsC) for (quint8 i=0; i<TypePM::Counters::number; ++i) pm->counters.Old[i] -= pm->counters.New[i]; }
			} else if (PM.contains(FEEid)) {
				p.addTransaction(read, PM[FEEid]->baseAddress + TypePM::Counters::addressDirect, PM[FEEid]->counters.New, TypePM::Counters::number);
				p.addTransaction(RMWbits, 0x7F + PM[FEEid]->baseAddress, p.masks(0xFFFFFFFF, 1 << 9));
				if (transceive(p)) for (quint8 i=0; i<TypePM::Counters::number; ++i) PM[FEEid]->counters.Old[i] -= PM[FEEid]->counters.New[i];
			}
		} else { //SW timer
			readCountersDirectly();
			if (FEEid == -1 || FEEid == TCMid) {
				p.addTransaction(RMWbits, 0xF, p.masks(0xFFFFFFFF, 1 << 9));
				for (quint8 i=0; i<TypeTCM::Counters::number; ++i) TCM.counters.Old[i] = 0;
			}
			if (FEEid == -1 || FEEid == -2) foreach (TypePM *pm, PM) {
				for (quint8 i=0; i<TypePM::Counters::number; ++i) pm->counters.Old[i] = 0;
				p.addTransaction(RMWbits, 0x7F + pm->baseAddress, p.masks(0xFFFFFFFF, 1 << 9));
			} else if (PM.contains(FEEid)) {
				for (quint8 i=0; i<TypePM::Counters::number; ++i) PM[FEEid]->counters.Old[i] = 0;
				p.addTransaction(RMWbits, 0x7F + PM[FEEid]->baseAddress, p.masks(0xFFFFFFFF, 1 << 9));
			}
			if (transceive(p)) sync();
		}
	}

	void setRatesUnknown() {
		for (quint8 iPM=0; iPM<20; ++iPM) for(quint8 i=0; i<TypePM::Counters::number; ++i) allPMs[iPM].counters.rate[i] = 0;
		for(quint8 i=0; i<TypeTCM::Counters::number; ++i) TCM.counters.rate[i] = 0;
		foreach (CustomDIMservice *s, countServices) s->updateService();
	}

	bool read1PM(TypePM *pm) {
        pm->act.voltage1_8 = readRegister(pm->baseAddress + 0xFE);
        if (pm->act.voltage1_8 == 0xFFFFFFFF) { //SPI error
            clearBit(pm - allPMs, 0x1E, false);
            deletePMservices(pm);
            TCM.set.PM_MASK_SPI &= ~(1 << (pm - allPMs));
            PM.remove(pm->FEEid);
            (pm - allPMs < 10 ? PMsA : PMsC).removeOne(pm);
            log(pm->fullName() + " is not available by SPI");
            emit linksStatusReady();
        } else {
            IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
            foreach(regblock b, pm->act.regblocks) p.addTransaction(read, pm->baseAddress + b.addr, pm->act.registers + b.addr, b.size()); // reading PM registers with actual values
            if (!transceive(p)) return false;
            pm->act.calculateValues();
            pm->counters.GBT.calculateRate(pm->act.GBT.Status.wordsCount, pm->act.GBT.Status.eventsCount);
            foreach (DimService *s, pm->services) s->updateService();
            if (pm->act.FW_TIME_FPGA.printCode1() >= "28T.CI" && !pm->act.GBT.Status.FIFOempty_errorReport) {
                GBTerrorReport errorReport;
                p.addTransaction(nonIncrementingRead, pm->baseAddress + GBTerrorReport::address, errorReport.data, GBTerrorReport::reportSize);
                if (!transceive(p)) return false;
                log(pm->fullName() + " " + errorReport.print());
            }
        }
        return true;
    }     

    void sync() { //read actual values
        if (!isOnline) return;
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        foreach(regblock b, TCM.act.regblocks) p.addTransaction(read, b.addr, TCM.act.registers + b.addr, b.size()); //reading TCM registers with actual values
        if (!transceive(p)) return;
        TCM.act.calculateValues();
        TCM.counters.GBT.calculateRate(TCM.act.GBT.Status.wordsCount, TCM.act.GBT.Status.eventsCount);
        foreach (DimService *s, TCM.services) s->updateService();
        if (TCM.act.resetSystem) {
            PMsReady = false;
            PM.clear();
            PMsA.clear();
            PMsC.clear();
        } else if (PMsReady == false) {
            PMsReady = true;
            emit resetFinished();
            return;
        }
        if (TCM.act.FW_TIME_FPGA.printCode1() >= "28T.CI" && !TCM.act.GBT.Status.FIFOempty_errorReport) {
            GBTerrorReport errorReport;
            p.addTransaction(nonIncrementingRead, GBTerrorReport::address, errorReport.data, GBTerrorReport::reportSize);
            if (!transceive(p)) return;
            log("TCM " + errorReport.print());
        }
        if (PMsReady) {
            foreach (TypePM *pm, PM) if (!read1PM(pm)) return;
            calculateSystemValues();
        }
        foreach (CustomDIMservice *s, services) s->updateService();
        emit valuesReady();
        if (TCM.act.COUNTERS_UPD_RATE == 0) readCountersDirectly();
    }

    void calculateSystemValues() {
//        BOARDS_OK = 0;
//        for(quint8 iPM= 0; iPM<10; ++iPM) BOARDS_OK |= (TCM.act.TRG_SYNC_A[iPM   ].linkOK && allPMs[iPM].act.GBT.isOK() && allPMs[iPM].isOK()) << iPM;
//        for(quint8 iPM=10; iPM<20; ++iPM) BOARDS_OK |= (TCM.act.TRG_SYNC_C[iPM-10].linkOK && allPMs[iPM].act.GBT.isOK() && allPMs[iPM].isOK()) << iPM;
//        BOARDS_OK |= (TCM.isOK() && TCM.act.GBT.isOK()) << 20;
        BOARDS_OK = (TCM.isOK() && TCM.act.GBT.isOK());
        for(qint8 iPM=19; iPM>=0; --iPM) { BOARDS_OK <<= 1; if (allPMs[iPM].isOK() && allPMs[iPM].act.GBT.isOK()) ++BOARDS_OK; }
    }

    void adjustThresholds(TypePM *pm, double rate_Hz) { //debug function! lab use only!
        if (adjustConnection) return;
        targetPM = pm;
        targetRate_Hz = rate_Hz;
        for (quint8 iCh = 0; iCh<12; ++iCh) {
            thLo[iCh] = 1024;
            thHi[iCh] = 3072;
        }
        adjEven = false;
        adjustConnection = connect(this, &FITelectronics::countersReady, this, [&](quint16 FEEid) {
            if (FEEid != targetPM->FEEid) return;
            adjEven = !adjEven;
            if (adjEven) return;
            IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
            quint8 c = 0;
            for (quint8 iCh = 0; iCh<12; ++iCh) {
                if (targetPM->counters.rateCh[iCh].CFD < targetRate_Hz + sqrt(targetRate_Hz)) { thHi[iCh] = targetPM->act.THRESHOLD_CALIBR[iCh]; }
                if (targetPM->counters.rateCh[iCh].CFD > targetRate_Hz - sqrt(targetRate_Hz)) { thLo[iCh] = targetPM->act.THRESHOLD_CALIBR[iCh]; }
                if (thLo[iCh] != thHi[iCh]) {
                    p.addWordToWrite(targetPM->baseAddress + PMparameters["THRESHOLD_CALIBR"].address + iCh, (thHi[iCh] + thLo[iCh]) / 2);
                    ++c;
                }
            }
            if (c) transceive(p); else disconnect(adjustConnection);
        });
    }

    void inverseLaserPhase() { writeRegister(0x2, shuttleStartPhase = -shuttleStartPhase, false); }

    void reset(quint16 FEEid, quint8 RB_position, bool syncOnSuccess = true) {
        quint32 address = (FEEid == TCMid ? 0x0 : PM[FEEid]->baseAddress) + GBTunit::controlAddress;
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(RMWbits, address, p.masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        p.addTransaction(RMWbits, address, p.masks(0xFFFFFFFF, 1 << RB_position)); //set specific bit, e.g. 0x00000800 for RS_GBTerrors
        p.addTransaction(RMWbits, address, p.masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        if (transceive(p) && syncOnSuccess) sync();
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
	void apply_HB_REJECT   (quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).HB_REJECT    = on; writeParameter("HB_REJECT"   , on, FEEid); }
	void apply_shiftRxPhase(quint16 FEEid, bool on) { (FEEid == TCMid ? TCM.set.GBT : PM[FEEid]->set.GBT).shiftRxPhase = on; writeParameter("shiftRxPhase", on, FEEid); }

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

	void apply_RESET_SYSTEM(bool forceLocalClock = false) {
		PMsReady = false;
        switchGBTerrorReports(false);
		writeRegister(0xF, forceLocalClock ? 0xC00 : 0x800);
        QTimer::singleShot(2000, this, [=](){ writeRegister(0xF, 0x4, false); switchGBTerrorReports(true); }); //clearing 'system restarted' and 'readiness changed' flags after restart
	}
    void apply_RESET_ERRORS(bool syncOnSuccess = true) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
		p.addTransaction(RMWbits, GBTunit::controlAddress, p.masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
        p.addTransaction(RMWbits, GBTunit::controlAddress, p.masks(0xFFFFFFFF, 1 << GBTunit::RB_readoutFSM | 1 << GBTunit::RB_GBTRxError | 1 << GBTunit::RB_errorReport));
		p.addTransaction(RMWbits, GBTunit::controlAddress, p.masks(0xFFBF00FF, 0x00000000)); //clear all reset bits and unlock
		foreach (TypePM *pm, PM) {
            quint32 address = pm->baseAddress + GBTunit::controlAddress;
            p.addTransaction(RMWbits, address, p.masks(0xFFFF00FF, 0x00000000)); //clear all reset bits
            p.addTransaction(RMWbits, address, p.masks(0xFFFFFFFF, 1 << GBTunit::RB_readoutFSM | 1 << GBTunit::RB_GBTRxError | 1 << GBTunit::RB_errorReport));
            p.addTransaction(RMWbits, address, p.masks(0xFFBF00FF, 0x00000000)); //clear all reset bits and unlock
        }
        p.addWordToWrite(0xF, 0x4);
        if (transceive(p) && syncOnSuccess) sync();
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
    void apply_LASER_SOURCE(bool isGenerator) { TCM.set.LASER_SOURCE = isGenerator; writeParameter("LASER_SOURCE", isGenerator, TCMid); }
    void apply_LASER_PATTERN() {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
		p.addTransaction(write, TCMparameters["LASER_PATTERN"].address, (quint32 *)&TCM.set.LASER_PATTERN, 2);
        if (transceive(p)) sync();
    }
	void apply_SwLaserPatternBit(quint8 bit, bool on) {
		if (bit >= 64) return;
		quint32 address = TCMparameters["LASER_PATTERN"].address + bit/32;
		on ? TCM.set.LASER_PATTERN |= 1ULL << bit : TCM.set.LASER_PATTERN &= ~(1ULL << bit);
		on ? setBit(bit % 32, address) : clearBit(bit % 32, address);
    }
	void apply_attenSteps() { writeParameter("attenSteps", TCM.set.attenSteps, TCMid); }
    void apply_LASER_ENABLED(bool on) { TCM.set.LASER_ENABLED = on; writeParameter("LASER_ENABLED", on, TCMid); }
    void apply_LASER_DELAY() { writeParameter("LASER_DELAY", TCM.set.LASER_DELAY, TCMid); }
    void apply_LSR_TRG_SUPPR_DUR  () { writeParameter("LASER_TRG_SUPPR_DUR", TCM.set.lsrTrgSupprDur, TCMid);}
    void apply_LSR_TRG_SUPPR_DELAY() { writeParameter("LASER_TRG_SUPPR_DELAY", TCM.set.lsrTrgSupprDelay, TCMid);}
	void apply_DELAY_A() { qint32 delay_ms = qAbs(TCM.set.DELAY_A - TCM.act.DELAY_A); writeParameter("DELAY_A", TCM.set.DELAY_A, TCMid); if (delay_ms > 1) QThread::msleep(delay_ms); apply_RESET_ERRORS(); }
	void apply_DELAY_C() { qint32 delay_ms = qAbs(TCM.set.DELAY_C - TCM.act.DELAY_C); writeParameter("DELAY_C", TCM.set.DELAY_C, TCMid); if (delay_ms > 1) QThread::msleep(delay_ms); apply_RESET_ERRORS(); }
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
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        for (quint8 iPM =  0; iPM < 10; ++iPM) if (TCM.act.PM_MASK_SPI & 1 << iPM) p.addWordToWrite(allPMs[iPM].baseAddress + PMparameters["OR_GATE"].address, val);
        if (transceive(p)) sync();
    }
    void apply_OR_GATE_sideC(quint16 val) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        for (quint8 iPM = 10; iPM < 20; ++iPM) if (TCM.act.PM_MASK_SPI & 1 << iPM) p.addWordToWrite(allPMs[iPM].baseAddress + PMparameters["OR_GATE"].address, val);
        if (transceive(p)) sync();
    }
    void apply_TRGchargeLevelHi(qint8 iBd, quint16 val) {
        if (val > 4095) val = 4095;
        QString parameterName = "TRGchargeLevelHi";
        Parameter par = PMparameters[parameterName];
        if (iBd == -1) {
            IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
            foreach (TypePM *pm, PM) {
                pm->setParameter(parameterName, val);
                p.addNBitsToChange(pm->baseAddress + par.address, val, par.bitwidth, par.bitshift);
            }
            transceive(p);
        } else if (iBd >= 0 && iBd < 20 && TCM.act.PM_MASK_SPI & 1 << iBd) {
            allPMs[iBd].setParameter(parameterName, val);
            writeParameter(parameterName, val, allPMs[iBd].FEEid);
        }
    }
    void apply_TRGchargeLevelLo(qint8 iBd, quint16 val) {
        if (val > 15) val = 15;
        QString parameterName = "TRGchargeLevelLo";
        Parameter par = PMparameters[parameterName];
        if (iBd == -1) {
            IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
            foreach (TypePM *pm, PM) {
                pm->setParameter(parameterName, val);
                p.addNBitsToChange(pm->baseAddress + par.address, val, par.bitwidth, par.bitshift);
            }
            transceive(p);
        } else if (iBd >= 0 && iBd < 20 && TCM.act.PM_MASK_SPI & 1 << iBd) {
            allPMs[iBd].setParameter(parameterName, val);
            writeParameter(parameterName, val, allPMs[iBd].FEEid);
        }
    }
//    void apply_TRGchargeLevelHi(quint16 FEEid) { writeParameter("TRGchargeLevelHi", PM[FEEid]->set.TRGchargeLevelHi, FEEid); }
	void apply_TRG_CNT_MODE(quint16 FEEid, bool CFDinGate) { writeParameter("TRG_CNT_MODE", CFDinGate, FEEid); }
    void apply_CH_MASK_DATA (quint16 FEEid) { writeParameter("CH_MASK_DATA" , PM[FEEid]->set.CH_MASK_DATA , FEEid); }
    void apply_CH_MASK_TRG  (quint16 FEEid) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        for (quint8 i=0; i<12; ++i) {
            bool b = PM[FEEid]->set.TIME_ALIGN[i].blockTriggers;
            if (bool(PM[FEEid]->act.timeAlignment[i].blockTriggers) != b) {
                if (b) p.addTransaction(RMWbits, PM[FEEid]->baseAddress + PMparameters["noTriggerMode"].address + i, p.masks(0xFFFFFFFF, 1 << PMparameters["noTriggerMode"].bitshift));
                else   p.addTransaction(RMWbits, PM[FEEid]->baseAddress + PMparameters["noTriggerMode"].address + i, p.masks(~(1 << PMparameters["noTriggerMode"].bitshift), 0));
            }
        }
        if (transceive(p)) sync();
    }

    void apply_SC_EVAL_MODE(bool Nchan) {
        TCM.set.SC_EVAL_MODE = Nchan;
        Nchan ? setBit(8, 0xE) : clearBit(8, 0xE);
    }

    void copyActualToSettingsPM(TypePM *pm) { foreach(regblock b, pm->set.regblocks) memcpy(pm->set.registers + b.addr, pm->act.registers + b.addr, b.size() * wordSize); }

    void applySettingsPM(TypePM *pm) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        foreach(regblock b, pm->set.regblocks) p.addTransaction(write, pm->baseAddress + b.addr, pm->set.registers + b.addr, b.size());
        transceive(p);
    }

    void copyActualToSettingsTCM() { foreach(regblock b, TCM.set.regblocksToRead) memcpy(TCM.set.registers + b.addr, TCM.act.registers + b.addr, b.size() * wordSize); }

    void applySettingsTCM() {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        foreach(regblock b, TCM.set.regblocksToApply) p.addTransaction(write, b.addr, TCM.set.registers + b.addr, b.size());
        qint32 delay_ms = qMax(qAbs(TCM.set.DELAY_A - TCM.act.DELAY_A), qAbs(TCM.set.DELAY_C - TCM.act.DELAY_C)) + 10; //phase needs time to move
        if (!transceive(p)) return;
        if (delay_ms > 1) QThread::msleep(delay_ms); //waiting for phases shift to complete
        p.addWordToWrite(TCMparameters["CH_MASK_A"].address, TCM.set.CH_MASK_A);
        p.addWordToWrite(TCMparameters["CH_MASK_C"].address, TCM.set.CH_MASK_C);
        if (!transceive(p)) return;
        QThread::msleep(10); //to finish all PMs resync with TCM
		apply_RESET_ERRORS();
        writeRegister(0xF, 0x4, false); //clear "Readiness changed" flags
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
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
        p.addTransaction(write, 0x2A00, TCM.ORBIT_FILL_MASK, 223);
        transceive(p);
    }

    void switchGBTerrorReports(bool on) {
        IPbusControlPacket p; connect(&p, &IPbusControlPacket::error, this, &IPbusTarget::error);
//        p.addTransaction(RMWbits, GBTunit::controlAddress,);
        p.addNBitsToChange(GBTunit::controlAddress, !on, 1, GBTunit::RB_errorReport);
        foreach (TypePM *pm, PM) p.addNBitsToChange(pm->baseAddress + GBTunit::controlAddress, !on, 1, GBTunit::RB_errorReport);
        transceive(p);
    }

    bool TRGsyncEnabledForPM(quint8 iPM) { return 1 << iPM & TCM.act.PM_MASK_TRG(); }
};

#endif // FITELECTRONICS_H
