#ifndef GBT_H
#define GBT_H

#include <QtGlobal>
#include <QHash>
#include <QDateTime>
#include <QVector>
#include "DIM/dim/dis.hxx"
#include <functional>

enum TypeFITsubdetector {FT0=1, FV0=2, FDD=3};
inline TypeFITsubdetector getSubdetectorTypeByName(QString s) {
    if (s == "FV0") return FV0;
    if (s == "FDD") return FDD;
    else return FT0;
}
const struct TypeFITconstants {
    char name[4]; quint16 TCMid; quint8 systemID;
} FIT[4] = {
    {    "???",          0xFFFF,         0 },
    {    "FT0",          0xF000,        34 },
    {    "FV0",          0xF500,        35 },
    {    "FDD",          0xFD00,        33 }
};

struct GBTunit { // 32 registers * 4 bytes = 128 bytes
	union ControlData {
		quint32 registers[16] = {0};
        char pointer[16 * sizeof(quint32)]; // 64 bytes
		struct {
			quint32
                DG_MODE              :  4, //┐
                TG_MODE              :  4, //│
                RESET				 :  8, //│
                TG_CTP_EMUL_MODE     :  4, //│
                HB_RESPONSE			 :  1, //│D8
                BYPASS_MODE			 :  1, //│
                READOUT_LOCK		 :  1, //│
                                     :  9, //┘
                DG_TRG_RESPOND_MASK,       //]D9
                DG_BUNCH_PATTERN,          //]DA
                TG_SINGLE_VALUE,           //]DB
                TG_PATTERN_1,              //]DC
                TG_PATTERN_0,              //]DD
                TG_CONT_VALUE,             //]DE
                DG_BUNCH_FREQ        : 16, //┐
                TG_BUNCH_FREQ        : 16, //┘DF
                DG_FREQ_OFFSET       : 12, //┐
                                     :  4, //│
                TG_FREQ_OFFSET       : 12, //│E0
                                     :  4, //┘
                RDH_PAR              : 16, //┐
                RDH_FEE_ID           : 16, //┘E1
                RDH_DET_FIELD        : 16, //┐
                RDH_MAX_PAYLOAD      : 16, //┘E2
                BCID_DELAY           : 12, //┐
                                     :  4, //│
                CRU_TRG_COMPARE_DELAY: 12, //│E3
                                     :  4, //┘
                DATA_SEL_TRG_MASK        , //]E4
                reserved[3]              ; //]E5-E7
		};
	} Control;
    union StatusData {
		quint32 registers[16] = {0};
        char pointer[16 * sizeof(quint32)]; // 64 bytes
        struct {
            quint32
                BITS                        : 16, //┐
                READOUT_MODE                :  4, //│
                BCID_SYNC_MODE              :  4, //│E8
                RX_PHASE                    :  4, //│
                CRU_READOUT_MODE            :  4, //┘
                CRU_ORBIT                   : 32, //┐
                CRU_BC                      : 12, //┘E9
                                            : 20, //┐
                RAW_FIFO                    : 16, //┘EA
                SEL_FIFO                    : 16, //]EB
                SEL_FIRST_HIT_DROPPED_ORBIT,      //]EC
                SEL_LAST_HIT_DROPPED_ORBIT,       //]ED
                SEL_HITS_DROPPED,                 //]EE
                READOUT_RATE                : 16, //┐
                                            : 16, //┘EF
                reserved[8]                     ; //]F0-F7
        };
    } Status;
    static const quint8
        controlSize   =   13,
        statusSize    =    8,
        controlAddress= 0xD8,
        statusAddress = 0xE8,
    //data generator states
        DG_noData	= 0,
        DG_main		= 1,
        DG_Tx		= 2,
    //trigger generator states
        TG_noTrigger  = 0,
        TG_continuous = 1,
		TG_Tx		  = 2,
    //readout modes
        RO_idle       = 0,
        RO_continuous = 1,
        RO_triggered    = 2,
    //BCID sync modes
        BS_start = 0,
        BS_sync  = 1,
        BS_lost  = 2,
    //positions of reset bits
        RB_orbitSync			=  8,
        RB_droppingHitCounters	=  9,
        RB_generatorsBunchOffset= 10,
        RB_GBTerrors			= 11,
        RB_GBT					= 12,
        RB_RXphaseError			= 13;
    static constexpr quint32 defaults[controlSize] = {
        0x00100000, //D8, HB response is on
              0x20, //D9, actuate laser on 'Pre Pulse' trigger
                 1, //DA, data gen bunch pattern: 1 single event with 1 data word
                 0, //DB
                 0, //DC
                 0, //DD
                 0, //DE
            0xFFFF, //DF, data gen: once per 65535 BCs
                 0, //E0
                 0, //E1
        0x01F40000, //E2, corresponding to 8 kB packet
        0x000F0020, //E3
              0x10  //E4, select data on 'Physics' trigger
    };
};

struct Parameter {
    quint8 address,
           bitwidth,
           bitshift,
           interval; //for PM channels parameters only
    Parameter(quint8 addr = 0, quint8 w = 32, quint8 sh = 0, quint8 i = 0): address(addr), bitwidth(w), bitshift(sh), interval(i) {};
};

const QHash<QString, Parameter> GBTparameters = {
    //name                  address width shift
    {"DG_MODE"              , {0xD8,  4,  0}},
    {"TG_MODE"              , {0xD8,  4,  4}},
    {"TG_CTP_EMUL_MODE"     , {0xD8,  4, 16}},
    {"HB_RESPONSE"          , {0xD8,  1, 20}},
    {"BYPASS_MODE"          , {0xD8,  1, 21}},
    {"READOUT_LOCK"         , {0xD8,  1, 22}},
    {"DG_TRG_RESPOND_MASK"  ,  0xD9         },
    {"DG_BUNCH_PATTERN"     ,  0xDA         },
    {"TG_PATTERN"           , {0xDC, 64,  0}},
    {"TG_CONT_VALUE"        ,  0xDE         },
    {"DG_BUNCH_FREQ"        , {0xDF, 16,  0}},
    {"TG_BUNCH_FREQ"        , {0xDF, 16, 16}},
    {"DG_FREQ_OFFSET"       , {0xE0, 12,  0}},
    {"TG_FREQ_OFFSET"       , {0xE0, 12, 16}},
    {"RDH_PAR"              , {0xE1, 16,  0}},
    {"RDH_FEE_ID"           , {0xE1, 16, 16}},
    {"RDH_DET_FIELD"        , {0xE2, 16,  0}},
    {"RDH_MAX_PAYLOAD"      , {0xE2, 16, 16}},
    {"BCID_DELAY"           , {0xE3, 12,  0}},
    {"CRU_TRG_COMPARE_DELAY", {0xE3, 12, 16}},
    {"DATA_SEL_TRG_MASK"    ,  0xE4         }
};

struct Timestamp {
	quint32 second : 6, //0..59
			minute : 6, //0..59
			hour   : 5, //0..23
			year   : 6, //0..63
			month  : 4, //1..12
			day    : 5; //1..31
	Timestamp(const quint32 v=0) { memcpy(this, &v, 4); }
	Timestamp(quint16 y, quint8 mo, quint8 d, quint8 h, quint8 mi, quint8 s): second(s), minute(mi), hour(h), year(y > 2000 ? y - 2000 : y), month(mo), day(d) {};
	QString printFull() { return QString::asprintf("20%02d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second); }
	QString printISO () { return QString::asprintf("20%02d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, minute, second); }
	static constexpr char alph[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz|~";
	QString printCode1() { //from "011.a0" (start of 2020) to "hCV.xx" (end of 2063), minute accuracy
		return *(quint32 *)(this)==0 ? "000000" : QString::asprintf("%c%X%c.%c%c", alph[(year - 20)%64], month, alph[day], 'a' + hour, alph[minute]);
	}
	QString printCode2() { //from "0GW0" (start of 2020) to "~Ftx" (end of 2035), minute accuracy
		QString res(4);
		quint32 v = ((year - 20) << 20) + (month << 16) + (day << 11) + (hour << 6) + (minute);
		for (qint8 i=3; i>=0; --i, v >>= 6) res[i] = alph[v & 63];
		return res;
	}
};

struct TRGsyncStatus {
    quint32
        line0delay          : 5,
        line0signalLost     : 1,
        line0signalStable   : 1,
                            : 1,
        line1delay          : 5,
        line1signalLost     : 1,
        line1signalStable	: 1,
        syncError           : 1, //this bit should be read from side status register (0x1A or 0x3A)
        line2delay          : 5,
        line2signalLost     : 1,
        line2signalStable	: 1,
        bitPositionsOK		: 1,
        line3delay          : 5,
        line3signalLost     : 1,
        line3signalStable	: 1,
        linkOK              : 1;
};

#endif // GBT_H
