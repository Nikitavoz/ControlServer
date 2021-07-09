#ifndef TCM_H
#define TCM_H

#include "FITboardsCommon.h"

const double LHCclock_MHz = 40.0785; //reference value
//const double LHCclock_MHz = 40.; //lab value
const quint16 countersUpdatePeriod_ms[8] = {0, 100, 200, 500, 1000, 2000, 5000, 10000};
extern double systemClock_MHz; //40
extern double TDCunit_ps; // 13
extern double halfBC_ns; // 25
extern double phaseStepLaser_ns, phaseStep_ns;

struct TypeTCM {
    struct ActualValues {
        static const quint8// size = end_address + 1 - start_address
            block0addr = 0x00, block0size = 0x1D + 1 - block0addr, //30
            block1addr = 0x30, block1size = 0x3A + 1 - block1addr, //11
            block2addr = 0x60, block2size = 0x6A + 1 - block2addr, //11
			block3addr = 0xFC, block3size = 0xFE + 1 - block3addr; // 3

        union { //block0
            quint32 registers0[block0size] = {0};
            char pointer0[block0size * sizeof(quint32)];
            struct {
                qint32  DELAY_A         :16, //┐
                                        :16, //┘00
                        DELAY_C         :16, //┐
                                        :16, //┘01
                        LASER_DELAY     :16, //┐
                                        :16; //┘02
				quint32 attenSteps      :14, //┐
						attenBusy       : 1, //│
						attenNotFound   : 1, //│03
                                        :16, //┘
                        EXT_SW          : 4, //┐
                                        :28; //┘04
                qint32  boardTemperature:16, //┐
                                        :16, //┘05
                                        :32; //]06
                quint32 boardType       : 2, //┐
                                        : 6, //│
                        SERIAL_NUM      : 8, //│07
                                        :16; //┘
				qint32  VTIME_LOW       :10, //┐
										:22, //┘08
						VTIME_HIGH      :10, //┐
										:22; //┘09
                quint32 SC_LEVEL_A      :16, //┐
                                        :16, //┘0A
                        SC_LEVEL_C      :16, //┐
                                        :16, //┘0B
                        C_LEVEL_A       :16, //┐
                                        :16, //┘0C
                        C_LEVEL_C       :16, //┐
                                        :16, //┘0D
                        ADD_C_DELAY     : 1, //┐
                        C_SC_TRG_MODE   : 2, //│
                        EXTENDED_READOUT: 1, //│0E
                                        :28, //┘
                        PLLlockC        : 1, //┐
                        PLLlockA        : 1, //│
                        systemRestarted : 1, //│
                        externalClock   : 1, //│
                        GBTRxReady      : 1, //│
                        GBTRxError      : 1, //│
                        GBTRxPhaseError : 1, //│0F
                        BCIDsyncLost    : 1, //│
                        droppingHits    : 1, //│
                        resetCounters   : 1, //│
                        forceLocalClock : 1, //│
                        resetSystem     : 1, //│
                        PMstatusChanged :20; //┘
                TRGsyncStatus TRG_SYNC_A[10];//]10-19
                quint32 CH_MASK_A       :10, //┐
                                        : 7, //│
                        syncErrorInLinkA:10, //│
                        masterLinkErrorA: 1, //│
                        sideAenabled    : 1, //│1A
                        delayRangeErrorA: 1, //│
                        readinessChangeA: 1, //│
                        sideAready      : 1, //┘
                        LASER_DIVIDER   :24, //┐
                                        : 7, //│1B
                        LASER_SOURCE    : 1, //┘
                        LASER_PATTERN_1    , //]1C
                        LASER_PATTERN_0    ; //]1D
            };
        };
        quint32 PM_MASK_SPI                ; //]1E
        union { //block1
            quint32 registers1[block1size] = {0};
            char pointer1[block1size * sizeof(quint32)];
            struct {
                TRGsyncStatus TRG_SYNC_C[10]; //]30-39
                quint32 CH_MASK_C       :10, //┐
                                        : 7, //│
                        syncErrorInLinkC:10, //│
                        masterLinkErrorC: 1, //│
                        sideCenabled    : 1, //│3A
                        delayRangeErrorC: 1, //│
                        readinessChangeC: 1, //│
                        sideCready      : 1; //┘
            };
        };
		quint32 COUNTERS_UPD_RATE;			 //]50
        union { //block2
            quint32 registers2[block2size] = {0};
            char pointer2[block2size * sizeof(quint32)];
            struct {
                quint32 ORA_SIGN        :14, //┐
                                        :18, //┘60
                        ORA_RATE        :31, //┐
                                        : 1, //┘61
                        ORC_SIGN        :14, //┐
                                        :18, //┘62
                        ORC_RATE        :31, //┐
                                        : 1, //┘63
                        SC_SIGN         :14, //┐
                                        :18, //┘64
                        SC_RATE         :31, //┐
                                        : 1, //┘65
                        C_SIGN          :14, //┐
                                        :18, //┘66
                        C_RATE          :31, //┐
                                        : 1, //┘67
                        V_SIGN          :14, //┐
                                        :18, //┘68
                        V_RATE          :31, //┐
                                        : 1, //┘69
                        ORA_MODE        : 2, //┐
                        ORA_ENABLED     : 1, //│
                        ORC_MODE        : 2, //│
                        ORC_ENABLED     : 1, //│
                        SC_MODE         : 2, //│
                        SC_ENABLED      : 1, //│6A
                        C_MODE          : 2, //│
                        C_ENABLED       : 1, //│
                        V_MODE          : 2, //│
                        V_ENABLED       : 1, //│
                                        :17; //┘
            };
        };
		GBTunit GBT;						 //]D8-EF
		Timestamp FW_TIME_MCU;				 //]F7
        union { //block3
            quint32 registers3[block3size] = {0};
            char pointer3[block3size * sizeof(quint32)];
            struct {
                quint32 FPGAtemperature,     //]FC
                        voltage1,            //]FD
                        voltage1_8;          //]FE
            };
        };
        Timestamp FW_TIME_FPGA;              //]FF
//calculable parameters
        double
            TEMP_BOARD,
            TEMP_FPGA,
            VOLTAGE_1V,
            VOLTAGE_1_8V,
            delayLaser_ns,
            delayAside_ns,
            delayCside_ns,
            laserFrequency_Hz,
            attenuation;
        char BOARD_TYPE[4] = {0};
        void calculateValues() {
            TEMP_BOARD   = boardTemperature / 10.;
            TEMP_FPGA    = FPGAtemperature  * 503.975 / 65536 - 273.15;
            VOLTAGE_1V   = voltage1         * 3.      / 65536;
            VOLTAGE_1_8V = voltage1_8       * 3.      / 65536;
            memcpy(BOARD_TYPE, FIT[boardType].name, 4);
            //on reset only
            systemClock_MHz = externalClock ? LHCclock_MHz : 40.;
            TDCunit_ps = 1e6 / 30 / 64 / systemClock_MHz;
            halfBC_ns = 500. / systemClock_MHz;
			phaseStepLaser_ns = halfBC_ns / 1024;
			phaseStep_ns = halfBC_ns / (SERIAL_NUM ? 1024 : 1280); //TCM prototype has SERIAL_NUM == 0 and sides delay regvalue range of [-1280 .. 1280] instead of [-1024 .. 1024] in production TCMs
			delayLaser_ns = LASER_DELAY * phaseStepLaser_ns;
			delayAside_ns = DELAY_A     * phaseStep_ns;
			delayCside_ns = DELAY_C     * phaseStep_ns;
            laserFrequency_Hz = systemClock_MHz * 1e6 / (LASER_DIVIDER == 0 ? 1 << 24 : LASER_DIVIDER);
            //
            attenuation = attenSteps;
            for (quint8 i=0; i<10; ++i) {
                TRG_SYNC_A[i].syncError = syncErrorInLinkA & 1 << i;
                TRG_SYNC_C[i].syncError = syncErrorInLinkC & 1 << i;
            }
        }
    } act;

    struct Settings {
        static const quint8// size = end_address + 1 - start_address
            block0addr = 0x00, block0size = 0x04 + 1 - block0addr, // 5
            block1addr = 0x08, block1size = 0x0E + 1 - block1addr, // 7
            block2addr = 0x1A, block2size = 0x1D + 1 - block2addr, // 4
            block3addr = 0x60, block3size = 0x6A + 1 - block3addr; //11

        union { //block0
            quint32 registers0[block0size] = {0};
            char pointer0[block0size * sizeof(quint32)];
            struct {
                qint32  DELAY_A         :16, //┐
                                        :16, //┘00
                        DELAY_C         :16, //┐
                                        :16, //┘01
                        LASER_DELAY     :16, //┐
                                        :16; //┘02
				quint32 attenSteps      :14, //┐
										:18, //┘03
                        EXT_SW          : 4, //┐
                                        :28; //┘04
            };
        };
        union { //block1
            quint32 registers1[block1size] = {0};
            char pointer1[block1size * sizeof(quint32)];
            struct {
				qint32  VTIME_LOW       :10, //┐
										:22, //┘08
						VTIME_HIGH      :10, //┐
										:22; //┘09
                quint32 SC_LEVEL_A      :16, //┐
                                        :16, //┘0A
                        SC_LEVEL_C      :16, //┐
                                        :16, //┘0B
                        C_LEVEL_A       :16, //┐
                                        :16, //┘0C
                        C_LEVEL_C       :16, //┐
                                        :16, //┘0D
                        ADD_C_DELAY     : 1, //┐
                        C_SC_TRG_MODE   : 2, //│
                        EXTENDED_READOUT: 1, //│0E
                                        :28; //┘
            };
        };
        union { //block2
            quint32 registers2[block2size] = {0};
            char pointer2[block2size * sizeof(quint32)];
            struct {
                quint32 CH_MASK_A       :10, //┐
                                        :22, //┘1A
                        LASER_DIVIDER   :24, //┐
                                        : 7, //│1B
                        LASER_SOURCE    : 1, //┘
                        LASER_PATTERN_1,     //]1C
                        LASER_PATTERN_0;     //]1D
            };
        };
        quint32 CH_MASK_C,                   //]3A
				COUNTERS_UPD_RATE;			 //]50
        union { //block3
            quint32 registers3[block3size] = {0};
            char pointer3[block3size * sizeof(quint32)];
            struct {
                quint32 ORA_SIGN        :14, //┐
                                        :18, //┘60
                        ORA_RATE        :31, //┐
                                        : 1, //┘61
                        ORC_SIGN        :14, //┐
                                        :18, //┘62
                        ORC_RATE        :31, //┐
                                        : 1, //┘63
                        SC_SIGN         :14, //┐
                                        :18, //┘64
                        SC_RATE         :31, //┐
                                        : 1, //┘65
                        C_SIGN          :14, //┐
                                        :18, //┘66
                        C_RATE          :31, //┐
                                        : 1, //┘67
                        V_SIGN          :14, //┐
                                        :18, //┘68
                        V_RATE          :31, //┐
                                        : 1, //┘69
                        ORA_MODE        : 2, //┐
                        ORA_ENABLED     : 1, //│
                        ORC_MODE        : 2, //│
                        ORC_ENABLED     : 1, //│
                        SC_MODE         : 2, //│
                        SC_ENABLED      : 1, //│6A
                        C_MODE          : 2, //│
                        C_ENABLED       : 1, //│
                        V_MODE          : 2, //│
                        V_ENABLED       : 1, //│
                                        :17; //┘
            };
        };
        GBTunit::ControlData GBT;            //]D8-E7
        double //calculated values
            delayLaser_ns,
            delayAside_ns,
            delayCside_ns,
            laserFrequency_Hz,
            attenuation;
    } set;

    struct Counters {
        static const quint16
            addressFIFO     = 0x100,
            addressFIFOload = 0x101;
        static const quint8
            number = 15,
            addressDirect   =  0x70;
        quint16 FIFOload;
        QDateTime newTime, oldTime;
        union {
            quint32 New[number] = {0};
            struct {
                quint32 CNT_ORA,
                        CNT_ORC,
                        CNT_SC,
                        CNT_C,
                        CNT_V,
                        CNT_bgA,
                        CNT_bgC,
                        CNT_bgA_and_bgC,
                        CNT_bgA_or_bgC,
                        CNT_orA_or_orC,
                        CNT_orA_and_orC,
                        CNT_bgA_and_not_orA,
                        CNT_bgC_and_not_orC,
                        CNT_bgA_and_not_orA_OR_bgC_and_not_orC,
                        CNT_bgA_and_not_orA_AND_bgC_and_not_orC;
            };
        };
        quint32 Old[number] = {0};
		double rate[number] = {0.};
    } counters;

    QList<DimService *> services;
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
    {"SC_LEVEL_A"           ,  0x0A         },
    {"SC_LEVEL_C"           ,  0x0B         },
    {"C_LEVEL_A"            ,  0x0C         },
    {"C_LEVEL_C"            ,  0x0D         },
    {"ADD_C_DELAY"          , {0x0E,  1,  0}},
    {"C_SC_TRG_MODE"        , {0x0E,  2,  1}},
    {"EXTENDED_READOUT"     , {0x0E,  1,  3}},
    {"CH_MASK_A"            ,  0x1A         },
    {"LASER_DIVIDER"        , {0x1B, 24,  0}},
    {"LASER_SOURCE"         , {0x1B,  1, 31}},
    {"LASER_PATTERN_1"      ,  0x1C         },
    {"LASER_PATTERN_0"      ,  0x1D         },
    {"SPI_LINKS_MASK"       ,  0x1E         },
    {"CH_MASK_C"            ,  0x3A         },
    {"COUNTERS_UPD_RATE"    ,  0x50         },
    {"ORA_SIGN"             ,  0x60         },
    {"ORA_RATE"             ,  0x61         },
    {"ORC_SIGN"             ,  0x62         },
    {"ORC_RATE"             ,  0x63         },
    {"SC_SIGN"              ,  0x64         },
    {"SC_RATE"              ,  0x65         },
    {"C_SIGN"               ,  0x66         },
    {"C_RATE"               ,  0x67         },
    {"V_SIGN"               ,  0x68         },
    {"V_RATE"               ,  0x69         },
    {"ORA_MODE"             , {0x6A,  2,  0}},
    {"ORA_ENABLED"          , {0x6A,  1,  2}},
    {"ORC_MODE"             , {0x6A,  2,  3}},
    {"ORC_ENABLED"          , {0x6A,  1,  5}},
    {"SC_MODE"              , {0x6A,  2,  6}},
    {"SC_ENABLED"           , {0x6A,  1,  8}},
    {"C_MODE"               , {0x6A,  2,  9}},
    {"C_ENABLED"            , {0x6A,  1, 11}},
    {"V_MODE"               , {0x6A,  2, 12}},
    {"V_ENABLED"            , {0x6A,  1, 14}}
};

#endif // TCM_H
