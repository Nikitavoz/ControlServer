#ifndef TCM_H
#define TCM_H

#include "FITboardsCommon.h"

const double LHCclock_MHz = 40.0789658; //reference value
const quint16 countersUpdatePeriod_ms[8] = {0, 100, 200, 500, 1000, 2000, 5000, 10000};
extern double systemClock_MHz; //40
extern double TDCunit_ps; // 13
extern double halfBC_ns; // 25
extern double phaseStepLaser_ns, phaseStep_ns;

struct TypeTCM {
    struct ActualValues {
        qint32  DELAY_A         :16,  //┐
                                :16,  //┘00
                DELAY_C         :16,  //┐
                                :16,  //┘01
                LASER_DELAY     :16,  //┐
                                :16;  //┘02
        quint32 attenSteps      :14,  //┐
                attenBusy       : 1,  //│
                attenNotFound   : 1,  //│03
                                :16,  //┘
                EXT_SW          : 4,  //┐
                                :28;  //┘04
        qint32  boardTemperature:16,  //┐
                                :16,  //┘05
                                :32;  //]06
        quint32 boardType       : 2,  //┐
                                : 6,  //│
                SERIAL_NUM      : 8,  //│07
                                :16;  //┘
        qint32  VTIME_LOW       :10,  //┐
                                :22,  //┘08
                VTIME_HIGH      :10,  //┐
                                :22;  //┘09
        quint32 T2_LEVEL_A      :16,  //┐
                                :16,  //┘0A
                T2_LEVEL_C      :16,  //┐
                                :16,  //┘0B
                T1_LEVEL_A      :16,  //┐
                                :16,  //┘0C
                T1_LEVEL_C      :16,  //┐
                                :16,  //┘0D
                ADD_C_DELAY     : 1,  //┐
                sidesCombMode   : 2,  //│
                EXTENDED_READOUT: 1,  //│
                corrCNTselect   : 4,  //│0E
                SC_EVAL_MODE    : 1,  //│
                FV0_MODE        : 1,  //│
                                :22,  //┘
                PLLlockC        : 1,  //┐
                PLLlockA        : 1,  //│
                systemRestarted : 1,  //│
                externalClock   : 1,  //│
                GBTRxReady      : 1,  //│
                GBTRxError      : 1,  //│
                GBTRxPhaseError : 1,  //│0F
                BCIDsyncLost    : 1,  //│
                droppingHits    : 1,  //│
                resetCounters   : 1,  //│
                forceLocalClock : 1,  //│
                resetSystem     : 1,  //│
                PMstatusChanged :20;  //┘
        TRGsyncStatus TRG_SYNC_A[10]; //]10-19
        quint32 CH_MASK_A       :10,  //┐
                                : 7,  //│
                syncErrorInLinkA:10,  //│
                masterLinkErrorA: 1,  //│
                sideAenabled    : 1,  //│1A
                delayRangeErrorA: 1,  //│
                readinessChangeA: 1,  //│
                sideAready      : 1,  //┘
                LASER_DIVIDER   :24,  //┐
                                : 6,  //│
                LASER_ENABLED   : 1,  //│1B
				LASER_SOURCE    : 1;  //┘
		quint64 LASER_PATTERN	   ;  //]1C-1D
        quint32 PM_MASK_SPI =0x1FFFFF,//]1E
				lsrTrgSupprDelay: 6,  //┐
                lsrTrgSupprDur  : 2,  //│1F
                                :24;  //┘
        qint16  averageTimeA       ,  //┐
                averageTimeC       ;  //┘20
        quint32 _reservedSpace0[0x30 - 0x20 - 1];
        TRGsyncStatus TRG_SYNC_C[10]; //]30-39
        quint32 CH_MASK_C       :10,  //┐
                                : 7,  //│
                syncErrorInLinkC:10,  //│
                masterLinkErrorC: 1,  //│
                sideCenabled    : 1,  //│3A
                delayRangeErrorC: 1,  //│
                readinessChangeC: 1,  //│
                sideCready      : 1,  //┘
                _reservedSpace1[0x50 - 0x3A - 1],
                COUNTERS_UPD_RATE  ,  //]50
                _reservedSpace2[0x60 - 0x50 - 1],
                                : 7,  //┐
                T5_SIGN         : 7,  //│60
                                :18,  //┘
                T5_RATE         :31,  //┐
                                : 1,  //┘61
                                : 7,  //┐
                T4_SIGN         : 7,  //│62
                                :18,  //┘
                T4_RATE         :31,  //┐
                                : 1,  //┘63
                                : 7,  //┐
                T2_SIGN         : 7,  //│64
                                :18,  //┘
                T2_RATE         :31,  //┐
                                : 1,  //┘65
                                : 7,  //┐
                T1_SIGN         : 7,  //│66
                                :18,  //┘
                T1_RATE         :31,  //┐
                                : 1,  //┘67
                                : 7,  //┐
                T3_SIGN         : 7,  //│68
                                :18,  //┘
                T3_RATE         :31,  //┐
                                : 1,  //┘69
                T5_MODE         : 2,  //┐
                T5_ENABLED      : 1,  //│
                T4_MODE         : 2,  //│
                T4_ENABLED      : 1,  //│
                T2_MODE         : 2,  //│
                T2_ENABLED      : 1,  //│6A
                T1_MODE         : 2,  //│
                T1_ENABLED      : 1,  //│
                T3_MODE         : 2,  //│
                T3_ENABLED      : 1,  //│
                                :17,  //┘
                _reservedSpace3[0xD8 - 0x6A - 1];
        GBTunit GBT;                  //]D8-F1
        quint32 _reservedSpace4[0xF7 - 0xF1 - 1];
        Timestamp FW_TIME_MCU;        //]F7
        quint32 _reservedSpace5[0xFC - 0xF7 - 1],
                FPGAtemperature,      //]FC
                voltage1,             //]FD
                voltage1_8;           //]FE
        Timestamp FW_TIME_FPGA;       //]FF

        quint32 *registers = (quint32 *)this;
        static const inline QVector<regblock> regblocks {{0x00, 0x20}, //block0     , 33 registers
                                                         {0x30, 0x3A}, //block1     , 11 registers
                                                         {0x50, 0x50}, //COUNTERS_UPD_RATE
                                                         {0x60, 0x6A}, //block2     , 11 registers
                                                         {0xD8, 0xE4}, //GBTcontrol , 13 registers
                                                         {0xE8, 0xF1}, //GBTstatus  , 10 registers
                                                         {0xF7, 0xF7}, //FW_TIME_MCU
                                                         {0xFC, 0xFF}};//block3     ,  4 registers
        float //calculable values
            TEMP_BOARD = 20.0F,
            TEMP_FPGA  = 20.0F,
            VOLTAGE_1V   = 1.0F,
            VOLTAGE_1_8V = 1.8F,
            delayLaser_ns,
            delayAside_ns,
            delayCside_ns,
            averageTimeA_ns,
            averageTimeC_ns,
            laserFrequency_Hz,
            attenuation_dB;
        char BOARD_TYPE[4] = {0};
        void calculateValues() {
            TEMP_BOARD   = boardTemperature / 10.;
            TEMP_FPGA    = FPGAtemperature  * 503.975 / 65536 - 273.15;
            VOLTAGE_1V   = voltage1         * 3.      / 65536;
            VOLTAGE_1_8V = voltage1_8       * 3.      / 65536;
            memcpy(BOARD_TYPE, FIT[boardType].name, 4);
            systemClock_MHz = externalClock ? LHCclock_MHz : 40.;
            TDCunit_ps = 1e6 / 30 / 64 / systemClock_MHz;
            halfBC_ns = 500. / systemClock_MHz;
            phaseStepLaser_ns = halfBC_ns / 1024;
            phaseStep_ns = halfBC_ns / (SERIAL_NUM ? 1024 : 1280); //TCM prototype has SERIAL_NUM == 0 and sides delay regvalue range of [-1280 .. 1280] instead of [-1024 .. 1024] in production TCMs
            delayLaser_ns = LASER_DELAY * phaseStepLaser_ns;
            delayAside_ns = DELAY_A     * phaseStep_ns;
            delayCside_ns = DELAY_C     * phaseStep_ns;
            averageTimeA_ns = averageTimeA * TDCunit_ps / 1000;
            averageTimeC_ns = averageTimeC * TDCunit_ps / 1000;
            laserFrequency_Hz = systemClock_MHz * 1e6 / (LASER_DIVIDER == 0 ? 1 << 24 : LASER_DIVIDER);
            for (quint8 i=0; i<10; ++i) {
                TRG_SYNC_A[i].syncError = syncErrorInLinkA & 1 << i;
                TRG_SYNC_C[i].syncError = syncErrorInLinkC & 1 << i;
            }
        }
		quint32 PM_MASK_TRG() { return CH_MASK_C << 10 | CH_MASK_A; }
    } act;

    struct Settings {
        qint32  DELAY_A         :16,  //┐
                                :16,  //┘00
                DELAY_C         :16,  //┐
                                :16,  //┘01
                LASER_DELAY     :16,  //┐
                                :16;  //┘02
        quint32 attenSteps      :14,  //┐
                                :18,  //┘03
                EXT_SW          : 4,  //┐
                                :28,  //┘04
                _reservedSpace0[0x08 - 0x04 - 1];
        qint32  VTIME_LOW       :10,  //┐
                                :22,  //┘08
                VTIME_HIGH      :10,  //┐
                                :22;  //┘09
        quint32 T2_LEVEL_A      :16,  //┐
                                :16,  //┘0A
                T2_LEVEL_C      :16,  //┐
                                :16,  //┘0B
                T1_LEVEL_A      :16,  //┐
                                :16,  //┘0C
                T1_LEVEL_C      :16,  //┐
                                :16,  //┘0D
                ADD_C_DELAY     : 1,  //┐
                sidesCombMode   : 2,  //│
                EXTENDED_READOUT: 1,  //│
                corrCNTselect   : 4,  //│0E
                SC_EVAL_MODE    : 1,  //│
                FV0_MODE        : 1,  //│
                                :22,  //┘
                _reservedSpace1[0x1A - 0x0E - 1],
                CH_MASK_A       :10,  //┐
                                :22,  //┘1A
                LASER_DIVIDER   :24,  //┐
                                : 6,  //│
                LASER_ENABLED   : 1,  //│1B
                LASER_SOURCE    : 1;  //┘
        quint64 LASER_PATTERN      ;  //]1C-1D
        quint32 PM_MASK_SPI        ,  //]1E
                lsrTrgSupprDelay: 6,  //┐
                lsrTrgSupprDur  : 2,  //│1F
                                :24,  //┘
                _reservedSpace2[0x3A - 0x1F - 1],
                CH_MASK_C       :10,  //┐
                                :22,  //┘3A
                _reservedSpace3[0x50 - 0x3A - 1],
                COUNTERS_UPD_RATE,    //]50
                _reservedSpace4[0x60 - 0x50 - 1],
                                : 7,  //┐
                T5_SIGN         : 7,  //│60
                                :18,  //┘
                T5_RATE         :31,  //┐
                                : 1,  //┘61
                                : 7,  //┐
                T4_SIGN         : 7,  //│62
                                :18,  //┘
                T4_RATE         :31,  //┐
                                : 1,  //┘63
                                : 7,  //┐
                T2_SIGN         : 7,  //│64
                                :18,  //┘
                T2_RATE         :31,  //┐
                                : 1,  //┘65
                                : 7,  //┐
                T1_SIGN         : 7,  //│66
                                :18,  //┘
                T1_RATE         :31,  //┐
                                : 1,  //┘67
                                : 7,  //┐
                T3_SIGN         : 7,  //│68
                                :18,  //┘
                T3_RATE         :31,  //┐
                                : 1,  //┘69
                T5_MODE         : 2,  //┐
                T5_ENABLED      : 1,  //│
                T4_MODE         : 2,  //│
                T4_ENABLED      : 1,  //│
                T2_MODE         : 2,  //│
                T2_ENABLED      : 1,  //│6A
                T1_MODE         : 2,  //│
                T1_ENABLED      : 1,  //│
                T3_MODE         : 2,  //│
                T3_ENABLED      : 1,  //│
                                :17,  //┘
                _reservedSpace5[0xD8 - 0x6A - 1];
        GBTunit::ControlData GBT;     //]D8-E7

        quint32 *registers = (quint32 *)this;
        static const inline QVector<regblock> regblocksToRead {{0x00, 0x04}, //block0     ,  5 registers
                                                               {0x08, 0x0E}, //block1     ,  7 registers
                                                               {0x1A, 0x1D}, //block2     ,  4 registers
                                                               {0x1F, 0x1F}, //trigger suppression
                                                               {0x3A, 0x3A}, //CH_MASK_C
                                                               {0x50, 0x50}, //COUNTERS_UPD_RATE
                                                               {0x60, 0x6A}, //block3     , 11 registers
                                                               {0xD8, 0xE4}},//GBT control, 13 registers

                                             regblocksToApply {{0x00, 0x04}, //block0     ,  5 registers
                                                               {0x08, 0x0E}, //block1     ,  7 registers
                                                               {0x1B, 0x1D}, //block2     ,  3 registers
                                                               {0x1F, 0x1F}, //trigger suppression
                                                               {0x60, 0x6A}, //block3     , 11 registers
                                                               {0xD8, 0xE4}};//GBT control, 13 registers
        float //calculable values
            delayLaser_ns,
            laserFrequency_Hz,
            attenuation_dB;
        void calculate_LASER_DIVIDER(float frequency_Hz) { quint32 div = lround(systemClock_MHz * 1e6 / frequency_Hz); LASER_DIVIDER = div ? (div >= 1<<24 ? 0 : div) : 1; }
        void calculate_LASER_DELAY(float delay_ns) { LASER_DELAY = lround(delay_ns / phaseStepLaser_ns); }
    } set;

    struct Counters {
        static const quint16
            addressFIFO     = 0x100,
            addressFIFOload = 0x101;
        static const quint8
            number = 15,
            addressDirect   =  0x70;
        quint32 FIFOload;
        QDateTime newTime, oldTime = QDateTime::currentDateTime();
        union {
            quint32 New[number] = {0};
            struct {
                quint32 CNT_T5             , //]70
                        CNT_T4             , //]71
                        CNT_T2             , //]72
                        CNT_T1             , //]73
                        CNT_T3             , //]74
                        CNT_noiseA         , //]75
                        CNT_noiseC         , //]76
                        CNT_noiseAll       , //]77
                        CNT_TrueOrA        , //]78
                        CNT_TrueOrC        , //]79
                        CNT_Interaction    , //]7A
                        CNT_TrueInteraction, //]7B
                        CNT_TrueVertex     , //]7C
                        CNT_BeamGasA       , //]7D
                        CNT_BeamGasC       ; //]7E
            };
        };
        quint32 Old[number] = {0};
        double rate[number] = {-1,-1,-1,-1,-1, -1,-1,-1,-1,-1, -1,-1,-1,-1,-1};
        GBTcounters GBT;
        QList<DimService *> services;
    } counters;

    bool isOK() {return
         act.PLLlockA &&
         act.PLLlockC &&
        !act.delayRangeErrorA &&
        !act.delayRangeErrorC &&
        !act.masterLinkErrorA &&
        !act.masterLinkErrorC &&
        !act.readinessChangeA &&
        !act.readinessChangeC ;
    }

    bool GBTisOK() {return
        act.GBT.isOK() &&
        act.GBTRxReady;
    }

    QList<DimService *> services, staticServices;
    QList<DimCommand *> commands;
    quint32 ORBIT_FILL_MASK[223];
    struct { quint32
        BCsyncLostInRun : 1 = 0;
    } errorsLogged;
};

const QHash<QString, Parameter> TCMparameters = {
    //name                  address width shift
    {"DELAY_A"              ,  0x00         },
    {"DELAY_C"              ,  0x01         },
    {"LASER_DELAY"          ,  0x02         },
    {"attenSteps"           ,  0x03         },
    {"EXT_SW"               ,  0x04         },
    {"VTIME_LOW"            ,  0x08         },
    {"VTIME_HIGH"           ,  0x09         },
    {"T2_LEVEL_A"           ,  0x0A         },
    {"T2_LEVEL_C"           ,  0x0B         },
    {"T1_LEVEL_A"           ,  0x0C         },
    {"T1_LEVEL_C"           ,  0x0D         },
    {"ADD_C_DELAY"          , {0x0E,  1,  0}},
    {"sidesCombMode"        , {0x0E,  2,  1}},
    {"EXTENDED_READOUT"     , {0x0E,  1,  3}},
    {"SC_EVAL_MODE"         , {0x0E,  1,  8}},
    {"FV0_MODE"             , {0x0E,  1,  9}},
    {"CH_MASK_A"            ,  0x1A         },
    {"LASER_DIVIDER"        , {0x1B, 24,  0}},
    {"LASER_ENABLED"        , {0x1B,  1, 30}},
    {"LASER_SOURCE"         , {0x1B,  1, 31}},
    {"LASER_PATTERN"        , {0x1C, 64,  0}},
    {"PM_MASK_SPI"          ,  0x1E         },
    {"LASER_TRG_SUPPR_DELAY", {0x1F,  6,  0}},
    {"LASER_TRG_SUPPR_DUR"  , {0x1F,  2,  6}},
    {"CH_MASK_C"            ,  0x3A         },
    {"COUNTERS_UPD_RATE"    ,  0x50         },
    {"T5_SIGN"              ,  0x60         },
    {"T5_RATE"              ,  0x61         },
    {"T4_SIGN"              ,  0x62         },
    {"T4_RATE"              ,  0x63         },
    {"T2_SIGN"              ,  0x64         },
    {"T2_RATE"              ,  0x65         },
    {"T1_SIGN"              ,  0x66         },
    {"T1_RATE"              ,  0x67         },
    {"T3_SIGN"              ,  0x68         },
    {"T3_RATE"              ,  0x69         },
    {"T5_MODE"              , {0x6A,  2,  0}},
    {"T5_ENABLED"           , {0x6A,  1,  2}},
    {"T4_MODE"              , {0x6A,  2,  3}},
    {"T4_ENABLED"           , {0x6A,  1,  5}},
    {"T2_MODE"              , {0x6A,  2,  6}},
    {"T2_ENABLED"           , {0x6A,  1,  8}},
    {"T1_MODE"              , {0x6A,  2,  9}},
    {"T1_ENABLED"           , {0x6A,  1, 11}},
    {"T3_MODE"              , {0x6A,  2, 12}},
    {"T3_ENABLED"           , {0x6A,  1, 14}}
};

inline quint32 prepareSignature(quint32 sign) { return sign << 7 | (~sign & 0x7F); }

#endif // TCM_H
