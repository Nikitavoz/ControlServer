#ifndef PM_H
#define PM_H

#include "FITboardsCommon.h"

struct TypePM {
    struct ActualValues{
        static const quint8// size = end_address + 1 - start_address
            block0addr = 0x00, block0size = 0x7D + 1 - block0addr, //126
            block1addr = 0x7F, block1size = 0xBE + 1 - block1addr, // 64
			block2addr = 0xFC, block2size = 0xFE + 1 - block2addr; //  3

        union { //block0
            quint32 registers0[block0size] = {0};
            char pointer0[block0size * sizeof(quint32)];
            struct {
                quint32 OR_GATE               ; //]00
                struct TimeAlignment {          //┐
                    qint32                      //│
                        value              :12, //│
                        blockTriggers      : 1, //│01-0C
                                           :19; //│
                }      timeAlignment[12]      ; //┘
                quint32 ADC_BASELINE[12][2]   , //]0D-24 //[Ch][0] for ADC0, [Ch][1] for ADC1
                        ADC_RANGE   [12][2]   , //]25-3C
                        CFD_SATR              ; //]3D
                qint32  TDC1tuning         : 8, //┐
                        TDC2tuning         : 8, //│3E
                                           :16, //┘
                        TDC3tuning         : 8, //┐
                                           :24; //┘3F
                quint8  RAW_TDC_DATA[12][4]   ; //]40-4B //[Ch][0] for val1, [Ch][1] for val2
                quint32 DISPERSION  [12][2]   ; //]4C-63
                qint16  MEANAMPL    [12][2][2]; //]64-7B //[Ch][0][0] for ADC0, [Ch][1][0] for ADC1
                quint32 CH_MASK_DATA          , //]7C
                        CH_BASELINES_NOK      ; //]7D
            };
        };
        union { //block1
            quint32 registers1[block1size] = {0};
            char pointer1[block1size * sizeof(quint32)];
            struct {
                quint32 mainPLLlocked      : 1, //┐
                        TDC1PLLlocked      : 1, //│
                        TDC2PLLlocked      : 1, //│
                        TDC3PLLlocked      : 1, //│
                        GBTlinkPresent     : 1, //│
                        GBTreceiverError   : 1, //│
                        TDC1syncError      : 1, //│
                        TDC2syncError      : 1, //│
                        TDC3syncError      : 1, //│7F
                        RESET_COUNTERS     : 1, //│
                        TRGcountMode       : 1, //│
                        restartDetected    : 1, //│
                        GBTRxPhaseError    : 1, //│
                        BCIDsyncLost       : 1, //│
                        droppingHits       : 1, //│
                                           :17; //┘
                struct ChannelSettings {        //┐
                    quint32 CFD_THRESHOLD  :16, //│
                                           :16; //│
                    qint32  CFD_ZERO       :16, //│
                                           :16, //│
                            ADC_ZERO       :16, //│80-AF
                                           :16; //│
                    quint32 ADC_DELAY      :16, //│
                                           :16; //│
                } Ch[12];                       //┘
                quint32 THRESHOLD_CALIBR[12]  , //]B0-BB
                        boardTemperature   :16, //┐
										   :16; //┘BC
                quint32 boardType          : 2, //┐
                                           : 6, //│
                        SERIAL_NUM         : 8, //│BD
                                           :16, //┘
                        restartReasonCode  : 2, //┐
                                           :30; //┘BE
            };
        };
        GBTunit GBT;                            //]D8-EF
        Timestamp FW_TIME_MCU;					//]F7
        union { //block2
            quint32 registers2[block2size] = {0};
            char pointer2[block2size * sizeof(quint32)];
            struct {
                quint32 FPGAtemperature,        //]FC
                        voltage1,               //]FD
                        voltage1_8;             //]FE
            };
        };
        Timestamp FW_TIME_FPGA;                 //]FF
//calculable parameters
        double
            TEMP_BOARD,
            TEMP_FPGA,
            VOLTAGE_1V,
            VOLTAGE_1_8V,
            RMS_Ch[12][2]; //[Ch][0] for ADC0, [Ch][1] for ADC1
        qint16
            TIME_ALIGN[12]  ,
            TRG_CNT_MODE    ,
            CH_MASK_TRG     ;
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
        static const quint8// size = end_address + 1 - start_address
            block0addr = 0x00, block0size = 0x0C + 1 - block0addr, //13
            block1addr = 0x25, block1size = 0x3D + 1 - block1addr, //25
            block2addr = 0x80, block2size = 0xBB + 1 - block2addr; //60

        union { //block0
            quint32 registers0[block0size] = {0};
            char pointer0[block0size * sizeof(quint32)];
            struct {
                quint32 OR_GATE               ; //]00
                struct TimeAlignment {          //┐
                    qint32                      //│
                        value              :12, //│
                        blockTriggers      : 1, //│01-0C
                                           :19; //│
                }       TIME_ALIGN  [12]      ; //┘
            };
        };
        union { //block1
            quint32 registers1[block1size] = {0};
            char pointer1[block1size * sizeof(quint32)];
            struct {
                quint32 ADC_RANGE   [12][2]    , //]25-3C
                        CFD_SATR               ; //]3D
            };
        };
        quint32 CH_MASK_DATA;                    //]7C
        union { //block2
            quint32 registers2[block2size] = {0};
            char pointer2[block2size * sizeof(quint32)];
            struct {
                struct ChannelSettings {        //┐
                    quint32 CFD_THRESHOLD  :16, //│
                                           :16; //│
                    qint32  CFD_ZERO       :16, //│
                                           :16, //│80-AF
                            ADC_ZERO       :16, //│
                                           :16; //│
                    quint32 ADC_DELAY      :16, //│
                                           :16; //│
                } Ch[12];                       //┘
				quint32  THRESHOLD_CALIBR[12];  //]B0-BB
            };
        };
        GBTunit::ControlData GBT;               //]D8-E7
    } set;

    struct Counters {
        static const quint16
            addressFIFO     = 0x100,
            addressFIFOload = 0x101;
        static const quint8
            number = 24,
            addressDirect   =  0xC0;
        quint16 FIFOload;
        QDateTime newTime, oldTime;
        union {
            quint32 New[number] = {0};
            struct {
                quint32 CFD,
                        TRG;
            } Ch[12];
        };
        quint32 Old[number] = {0};
        union {
			double rate[number] = {0.};
            struct {
				double CFD,
					   TRG;
            } rateCh[12];
        };
        QList<DimService *> services;
    } counters;

    QList<DimService *> services;
    QList<DimCommand *> commands;
    QHash<QString, DimService *> servicesNew;
    quint16 &FEEid = *((quint16 *)(set.GBT.registers+9) + 1);
	const quint16 baseAddress;
    const char *name;

    bool clockSyncOK() { return act.mainPLLlocked && act.TDC1PLLlocked && act.TDC2PLLlocked && act.TDC3PLLlocked && !act.TDC1syncError && !act.TDC2syncError && !act.TDC3syncError; }
    //bool clockSyncOK() { return !((act.registers1[0] ^ 0xF) & 0x1CF); }

    TypePM(quint16 addr, const char *PMname) : baseAddress(addr), name(PMname) {}
};

const QHash<QString, Parameter> PMparameters = {
    //name                  address width shift interval
    {"OR_GATE"              ,  0x00               },
    {"TIME_ALIGN"           , {0x01, 12,  0,    1}},
    {"CH_TRG_BLOCK"         , {0x01,  1, 12,    1}},
    {"ADC0_OFFSET"          , {0x0D, 32,  0,    2}},
    {"ADC1_OFFSET"          , {0x0E, 32,  0,    2}},
    {"ADC0_RANGE"           , {0x25, 32,  0,    2}},
    {"ADC1_RANGE"           , {0x26, 32,  0,    2}},
    {"CFD_SATR"             ,  0x3D               },
    {"CH_MASK_DATA"         ,  0x7C               },
    {"TRG_CNT_MODE"         , {0x7F,  1, 10}      },
    {"CFD_THRESHOLD"        , {0x80, 32,  0,    4}},
    {"CFD_ZERO"             , {0x81, 32,  0,    4}},
    {"ADC_ZERO"             , {0x82, 32,  0,    4}},
    {"ADC_DELAY"            , {0x83, 32,  0,    4}},
    {"THRESHOLD_CALIBR"     , {0xB0, 32,  0,    1}}
};

#endif // PM_H
