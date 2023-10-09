#ifndef PM_H
#define PM_H

#include "FITboardsCommon.h"
const QHash<QString, Parameter> PMparameters = {
    //name                  address width shift interval
    {"OR_GATE"              , {0x00,  8,  0}      },
    {"PairedChannelsMode"   , {0x00,  1,  8}      },
    {"TIME_ALIGN"           , {0x01, 12,  0,    1}},
    {"noTriggerMode"        , {0x01,  1, 12,    1}},
    {"ADC0_RANGE"           , {0x25, 32,  0,    2}},
    {"ADC1_RANGE"           , {0x26, 32,  0,    2}},
    {"TRGchargeLevelHi"     , {0x3D, 12,  0}    }, //formerly known as CFD_SATR
    {"TRGchargeLevelLo"     , {0x3D,  4, 12}    },
    {"CH_MASK_DATA"         ,  0x7C               },
    {"TRG_CNT_MODE"         , {0x7F,  1, 10}      },
    {"CFD_THRESHOLD"        , {0x80, 32,  0,    4}},
    {"CFD_ZERO"             , {0x81, 32,  0,    4}},
    {"ADC_ZERO"             , {0x82, 32,  0,    4}},
    {"ADC_DELAY"            , {0x83, 32,  0,    4}},
    {"THRESHOLD_CALIBR"     , {0xB0, 32,  0,    1}}
};

struct TypePM {
    struct ActualValues{
        quint32 OR_GATE            : 8, //┐
                PairedChannelsMode : 1, //│00
                                   :23; //⌡
        struct TimeAlignment {          //┐
            qint32                      //│
                value              :12, //│
                blockTriggers      : 1, //│01-0C
                                   :19; //│
        }      timeAlignment[12]      ; //⌡
        quint32 ADC_BASELINE[12][2]   , //]0D-24 //[Ch][0] for ADC0, [Ch][1] for ADC1
                ADC_RANGE   [12][2]   , //]25-3C
                TRGchargeLevelHi   :12, //┐
                TRGchargeLevelLo   : 4, //│3D
                                   :16; //⌡
        qint32  TDC1tuning         : 8, //┐
                TDC2tuning         : 8, //│3E
                                   :16, //⌡
                TDC3tuning         : 8, //┐
                                   :24; //┘3F
        quint8  RAW_TDC_DATA[12][4]   ; //]40-4B //[Ch][0] for val1, [Ch][1] for val2
        quint32 DISPERSION  [12][2]   ; //]4C-63
        qint16  MEANAMPL    [12][2][2]; //]64-7B //[Ch][0][0] for ADC0, [Ch][1][0] for ADC1
        quint32 CH_MASK_DATA          , //]7C
                CH_BASELINES_NOK      , //]7D
                _reservedSpace0       , //]7E
                mainPLLlocked      : 1, //┐
                TDC1PLLlocked      : 1, //│
                TDC2PLLlocked      : 1, //│
                TDC3PLLlocked      : 1, //│
                GBTRxReady         : 1, //│
                GBTRxError         : 1, //│
                TDC1syncError      : 1, //│
                TDC2syncError      : 1, //│
                TDC3syncError      : 1, //│7F
                RESET_COUNTERS     : 1, //│
                TRGcountMode       : 1, //│
                restartDetected    : 1, //│
                GBTRxPhaseError    : 1, //│
                BCIDsyncLost       : 1, //│
                droppingHits       : 1, //│
                                   :17; //⌡
        struct ChannelSettings {        //┐
            quint32 CFD_THRESHOLD  :16, //│
                                   :16; //│
            qint32  CFD_ZERO       :16, //│
                                   :16, //│
                    ADC_ZERO       :16, //│80-AF
                                   :16; //│
            quint32 ADC_DELAY      :16, //│
                                   :16; //│
        } Ch[12];                       //⌡
        quint32 THRESHOLD_CALIBR[12]  , //]B0-BB
                boardTemperature   :16, //┐
                                   :16; //┘BC
        quint32 boardType          : 2, //┐
                                   : 6, //│
                SERIAL_NUM         : 8, //│BD
                                   :16, //⌡
                restartReasonCode  : 2, //┐
                                   :30, //┘BE
                _reservedSpace1[0xD8 - 0xBE - 1];
        GBTunit GBT;                    //]D8-F1
        quint32 _reservedSpace2[0xF7 - 0xF1 - 1];
        Timestamp FW_TIME_MCU;      //]F7
        quint32 _reservedSpace3[0xFC - 0xF7 - 1];
        quint32 FPGAtemperature,        //]FC
                voltage1,               //]FD
                voltage1_8;             //]FE
        Timestamp FW_TIME_FPGA;         //]FF
        quint32 *registers = (quint32 *)this;
        static const inline QVector<regblock> regblocks {{0x00, 0x7D}, //block0     , 126 registers
                                                         {0x7F, 0xBE}, //block1     ,  64 registers
                                                         {0xD8, 0xE4}, //GBTcontrol ,  13 registers
                                                         {0xE8, 0xF1}, //GBTstatus  ,  10 registers
                                                         {0xF7, 0xF7}, //FW_TIME_MCU
                                                         {0xFC, 0xFF}};//block2     ,   4 registers
        float //calculable values
            TEMP_BOARD = 20.0F,
            TEMP_FPGA  = 20.0F,
            VOLTAGE_1V   = 1.0F,
            VOLTAGE_1_8V = 1.8F,
            RMS_Ch[12][2]; //[Ch][0] for ADC0, [Ch][1] for ADC1
        qint16
            TIME_ALIGN[12]  ,
            TRG_CNT_MODE    ;
        quint32 CH_MASK_TRG;
        char BOARD_TYPE[4] = {0};
        void calculateValues() {
            TEMP_BOARD   = boardTemperature / 10.;
            TEMP_FPGA    = FPGAtemperature  * 503.975 / 65536 - 273.15;
            VOLTAGE_1V   = voltage1         *   3.    / 65536;
            VOLTAGE_1_8V = voltage1_8       *   3.    / 65536;
            CH_MASK_TRG = 0;
            for (quint8 iCh=0; iCh<12; ++iCh) {
                RMS_Ch[iCh][0] = sqrt(DISPERSION[iCh][0]);
                RMS_Ch[iCh][1] = sqrt(DISPERSION[iCh][1]);
                TIME_ALIGN[iCh] = timeAlignment[iCh].value;
                CH_MASK_TRG |= !timeAlignment[iCh].blockTriggers << iCh;
                TRG_CNT_MODE = TRGcountMode;
            }
            memcpy(BOARD_TYPE, FIT[boardType].name, 4);
        }
    } act;

    struct Settings {
        quint32 OR_GATE            : 8, //┐
                PairedChannelsMode : 1, //│00
                                   :23; //⌡
        struct TimeAlignment {          //┐
            qint32                      //│
                value              :12, //│
                blockTriggers      : 1, //│01-0C
                                   :19; //│
        } TIME_ALIGN           [12]   ;  //⌡
        quint32 _reservedSpace0[0x25 - 0x0C - 1],
                ADC_RANGE      [12][2], //]25-3C
                TRGchargeLevelHi   :12, //┐
                TRGchargeLevelLo   : 4, //│3D
                                   :16, //⌡
                _reservedSpace1[0x7C - 0x3D - 1],
                CH_MASK_DATA          , //]7C
                _reservedSpace2[0x80 - 0x7C - 1];
        struct ChannelSettings {        //┐
            quint32 CFD_THRESHOLD  :16, //│
                                   :16; //│
            qint32  CFD_ZERO       :16, //│
                                   :16, //│80-AF
                    ADC_ZERO       :16, //│
                                   :16; //│
            quint32 ADC_DELAY      :16, //│
                                   :16; //│
        } Ch[12];                       //⌡
        quint32  THRESHOLD_CALIBR[12] , //]B0-BB
                _reservedSpace3[0xD8 - 0xBB - 1];
        GBTunit::ControlData GBT;       //]D8-E7

        quint32 *registers = (quint32 *)this;
        static const inline QVector<regblock> regblocks {{0x00, 0x0C}, //block0     , 13 registers
                                                         {0x25, 0x3D}, //block1     , 25 registers
                                                         {0x7C, 0x7C}, //CH_MASK_DATA
                                                         {0x80, 0xBB}, //block2     , 60 registers
                                                         {0xD8, 0xE4}};//GBT control, 13 registers
    } set;

    struct Counters {
        static const quint16
            addressFIFO     = 0x100,
            addressFIFOload = 0x101;
        static const quint8
            number = 24,
            addressDirect   =  0xC0;
        quint32 FIFOload;
        QDateTime newTime, oldTime = QDateTime::currentDateTime();
        union {
            quint32 New[number] = {0};
            struct {
                quint32 CFD,
                        TRG;
            } Ch[12];
        };
        quint32 Old[number] = {0};
        union {
            float rate[number] = {-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1};
            struct {
                float CFD,
                      TRG;
            } rateCh[12];
        };
        GBTcounters GBT;
    } counters;

    QList<DimService *> services;
    QList<DimCommand *> commands;
    quint16 FEEid;
    const quint16 baseAddress;
    const char *name;
    QString fullName() const { return "PM" + QString(name); }
    TRGsyncStatus &TRGsync;
    struct { quint32
        BCsyncLostInRun : 1 = 0,
        earlyHeader     : 1 = 0;
    } errorsLogged;

    bool isOK() { return
         act.mainPLLlocked &&
         act.TDC1PLLlocked &&
         act.TDC2PLLlocked &&
         act.TDC3PLLlocked &&
        !act.TDC1syncError &&
        !act.TDC2syncError &&
        !act.TDC3syncError &&
         TRGsync.linkOK    &&
        !TRGsync.syncError &&
         act.restartReasonCode != 2 ; //not by PLL relock
    }

    bool GBTisOK() const { return
        act.GBT.isOK() &&
        act.GBTRxReady;
    }

    bool setParameter(QString parameterName, quint32 value, quint8 iCh = 0) {
        if (!PMparameters.contains(parameterName)) return false;
        Parameter par = PMparameters[parameterName];
        quint32 &reg = set.registers[par.address + iCh * par.interval];
        reg = changeNbits(reg, par.bitwidth, par.bitshift, value);
        return true;
    }

    TypePM(quint16 addr, const char *PMname, TRGsyncStatus &TRGsyncRef) : baseAddress(addr), name(PMname), TRGsync(TRGsyncRef) {}
};

#endif // PM_H
