#ifndef GBT_H
#define GBT_H

#include <QtGlobal>
#include <QHash>
#include <QDateTime>
#include <QVector>
#include "DIM/dis.hxx"
#include <functional>

enum TypeFITsubdetector {_0_=0, FT0=1, FV0=2, FDD=3};
inline TypeFITsubdetector getSubdetectorTypeByName(QString s) {
    if (s == "FT0") return FT0;
    if (s == "FV0") return FV0;
    if (s == "FDD") return FDD;
    else return _0_;
}
const struct {char name[4]; quint16 TCMid, PMA0id, PMC0id; quint8 systemID; struct {const char *name; qint16 signature;} triggers[5];} FIT[4] = { //global static constants
             {       "???",        0xFFFF, 0x0000, 0x000A,               0,        { {    "Trigger1",              75},
                                                                                     {    "Trigger2",              76},
                                                                                     {    "Trigger3",              77},
                                                                                     {    "Trigger4",              78},
                                                                                     {    "Trigger5",              79} }             },
             {       "FT0",        0xFA00, 0xFAA0, 0xFAF0,              34,        { {     "Central",              70},
                                                                                     {"Semi Central",              71},
                                                                                     {      "Vertex",              72},
                                                                                     {         "OrC",              73},
                                                                                     {         "OrA",              74} }             },
             {       "FV0",        0x55F0, 0x55A0, 0x55AA,              35,        { {      "Charge",              40},
                                                                                     {   "Nchannels",              41},
                                                                                     {  "InnerRings",              42},
                                                                                     {  "OuterRings",              43},
                                                                                     {         "OrA",              44} }             },
             {       "FDD",        0x0FA0, 0x0FAA, 0x0FFF,              33,        { {     "Central",             110},
                                                                                     {"Semi Central",             111},
                                                                                     {      "Vertex",             112},
                                                                                     {         "OrC",             113},
                                                                                     {         "OrA",             114} }             }
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
                HB_REJECT            :  1, //│
                                     :  8, //┘
                DG_TRG_RESPOND_MASK,       //]D9
                DG_BUNCH_PATTERN,          //]DA
                TG_SINGLE_VALUE,           //]DB
                TG_PATTERN_LSB,            //]DC
                TG_PATTERN_MSB,            //]DD
                TG_CONT_VALUE,             //]DE
                DG_BUNCH_FREQ        : 16, //┐
                TG_BUNCH_FREQ        : 16, //┘DF
                DG_FREQ_OFFSET       : 12, //┐
                                     :  4, //│
                TG_FREQ_OFFSET       : 12, //│E0
                TG_HBr_RATE          :  4, //┘
                RDH_FEE_ID           : 16, //┐
                RDH_SYS_ID           :  8, //│E1
                PRIORITY_BIT         :  8, //┘
                                     : 32, //]E2
                BCID_DELAY           : 12, //┐
                                     : 20, //┘E3
                DATA_SEL_TRG_MASK        , //]E4
                _reserved[3]             ; //]E5-E7
        };
    } Control;
    union StatusData {
        quint32 registers[10] = {0};
        char pointer[10 * sizeof(quint32)]; // 64 bytes
        struct {
            quint32
                phaseAlignerCPLLlock        :  1, //┐
                RxWorkClockReady            :  1, //│
                RxFrameClockReady           :  1, //│
                MGTlinkReady                :  1, //│
                TxResetDone                 :  1, //│
                TxFSMresetDone              :  1, //│
                GBTRxReady                  :  1, //│
                GBTRxError                  :  1, //│E8
                RxPhaseError                :  1, //│
                                            :  7, //│
                READOUT_MODE                :  4, //│
                BCID_SYNC_MODE              :  4, //│
                RX_PHASE                    :  4, //│
                CRU_READOUT_MODE            :  4, //┘
                CRU_ORBIT                       , //]E9
                FIFOempty_header            :  1, //┐
                FIFOempty_data              :  1, //│
                FIFOempty_trg               :  1, //│
                FIFOempty_slct              :  1, //│
                FIFOempty_cntpck            :  1, //│
                                            : 11, //│
                slctFIFOemptyWhileRead      :  1, //│
                FIFOnotEmptyOnRunStart_slct :  1, //│
                FIFOnotEmptyOnRunStart_cntpck: 1, //│
                FIFOnotEmptyOnRunStart_trg  :  1, //│EA
                trgFIFOwasFull              :  1, //│
                FIFOnotEmptyOnRunStart_data :  1, //│
                FIFOnotEmptyOnRunStart_header: 1, //│
                TCMdataFIFOfull             :  1, //│
                PMpacketCorruptedExtraWord  :  1, //│
                PMpacketCorruptedEarlyHeader:  1, //│
                BCsyncLostInRun             :  1, //│
                                            :  4, //│
                dataFIFOnotReady            :  1, //┘
                CNVdropCount                : 16, //┐
                CNVFIFOmax                  : 16, //┘EB
                SELdropCount                : 16, //┐
                SELFIFOmax                  : 16, //┘EC
                wordsCount                  : 32, //]ED
                BCindicatorData             : 12, //┐
                BCpurityData                :  4, //│
                BCindicatorTrg              : 12, //│EE
                BCpurityTrg                 :  4, //┘
                                            : 16, //┐
                FTMIPbusFIFOcount           : 16, //┘EF
                FTMIPbusFIFOdata                , //]F0
                eventsCount                     ; //]F1
        };
    } Status;
static const quint8
    controlSize   =   13,
    statusSize    =   10,
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
    RO_triggered  = 2,
//BCID sync modes
    BS_start = 0,
    BS_sync  = 1,
    BS_lost  = 2,
//positions of reset bits
    RB_orbitSync			=  8,
    RB_dataCounter      	=  9,
    RB_generatorsBunchOffset= 10,
    RB_GBTRxError			= 11,
    RB_GBT					= 12,
    RB_RXphaseError			= 13,
    RB_readoutFSM           = 14;
    static constexpr quint32 defaults[controlSize] = {
        0x00100000, //D8, HB response is on
              0x40, //D9, actuate laser on 'Calibration' trigger
                 0, //DA, data gen bunch pattern: 1 single event with 1 data word
                 0, //DB
                 0, //DC
                 0, //DD
                 0, //DE
                 0, //DF
                 0, //E0
                 0, //E1, to be filled during initialization
                 0, //E2
        0x000F0020, //E3
              0x10  //E4, select data on 'Physics' trigger
    };
    inline bool isOK() {return
        !Status.GBTRxError &&
        !Status.RxPhaseError &&
        Status.READOUT_MODE == Status.CRU_READOUT_MODE &&
        (Control.registers[0] & 1 << 14) == 0 && //GBT reset not locked
        (Status.registers[2] >> 16 & 0x7FFF) == 0 && //no FSM reset errors
        (!Status.dataFIFOnotReady || Status.READOUT_MODE != RO_idle); //FIFOs must me ready in Idle
    }
};

struct GBTcounters {
    QDateTime oldTime = QDateTime::currentDateTime(), newTime;
    quint32 wordsOld  = 0 , eventsOld  = 0;
    double  wordsRate = 0., eventsRate = 0.;
    void calculateRate(quint32 wordsNew, quint32 eventsNew) {
        newTime = QDateTime::currentDateTime();
        quint32 time_ms = oldTime.msecsTo(newTime);
        if (time_ms < 100) return;
         wordsRate = ( wordsNew -  wordsOld) * 1000. / time_ms;
        eventsRate = (eventsNew - eventsOld) * 1000. / time_ms;
         wordsOld =  wordsNew;
        eventsOld = eventsNew;
        oldTime = newTime;
    }
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
    {"HB_REJECT"            , {0xD8,  1, 23}},
    {"DG_TRG_RESPOND_MASK"  ,  0xD9         },
    {"DG_BUNCH_PATTERN"     ,  0xDA         },
    {"TG_PATTERN"           , {0xDC, 64,  0}},
    {"TG_CONT_VALUE"        ,  0xDE         },
    {"DG_BUNCH_FREQ"        , {0xDF, 16,  0}},
    {"TG_BUNCH_FREQ"        , {0xDF, 16, 16}},
    {"DG_FREQ_OFFSET"       , {0xE0, 12,  0}},
    {"TG_FREQ_OFFSET"       , {0xE0, 12, 16}},
    {"TG_HBr_RATE"          , {0xE0,  4, 28}},
    {"RDH_FEE_ID"           , {0xE1, 16,  0}},
    {"SYSTEM_ID"            , {0xE1,  8, 16}},
    {"BCID_DELAY"           , {0xE3, 12,  0}},
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
	QString printCode1() { //from "011.a0" (start of 2020) to "hCV.Nx" (end of 2063), minute accuracy
		return *(quint32 *)(this)==0 ? "000000" : QString::asprintf("%c%c%c.%c%c", alph[(year - 20) % 64], alph[month], alph[day], alph[hour], alph[minute]);
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

//class CustomDIMservice {
//    DimService *service;
//public:
//    CustomDIMservice(DimService *s) {}
//    int updateService() {}
//    template<typename... Args> int updateService(Args... args) { return service->updateService(args...); }
//};

#endif // GBT_H
