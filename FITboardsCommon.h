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
																					 {          "Or",              44} }             },
             {       "FDD",        0x0FA0, 0x0FAA, 0x0FFF,              33,        { {     "Central",             110},
                                                                                     {"Semi Central",             111},
                                                                                     {      "Vertex",             112},
                                                                                     {         "OrC",             113},
                                                                                     {         "OrA",             114} }             }
};
const struct {const char *name,										   *description;} COUNTERS[2][15] = {
{//FT0, FDD
			 {           "OrA",                               "TRG-events on A-side"},//5
			 {           "OrC",                               "TRG-events on C-side"},//4
			 {   "SemiCentral","sum charge is between Semicental and Central levels"},//2
			 {       "Central",                         "sum charge > Central level"},//1
			 {        "Vertex",              "(timeC - timeA) is within time limits"},//3
			 {        "NoiseA",                "A-side out-of-gate hits AND NOT OrA"},
			 {        "NoiseC",                "C-side out-of-gate hits AND NOT OrC"},
			 {   "Total noise",                                   "NoiseA OR NoiseC"},
			 {        "CB-OrA",                               "collision-BC AND OrA"},
			 {        "CB-OrC",                               "collision-BC AND OrC"},
			 {   "Interaction",                        "both sides Or (OrA AND OrC)"},
			 {"CB-Interaction",                       "collision-BC AND Interaction"},
			 {     "CB-Vertex",                            "collision-BC AND Vertex"},
			 {   "BackgroundA",                                   "beam1-BC AND OrC"},
			 {   "BackgroundC",                                   "beam2-BC AND OrA"}
}, {//FV0
			 {            "Or",                          "TRG-events in any channel"},//5
			 {    "OuterRings",               "PMA5-9 sum charge > OuterRings level"},//4
			 {     "Nchannels",    "number of channels having TRG-event > Nch level"},//2
			 {        "Charge",                     "sum charge > TotalCharge level"},//1
			 {    "InnerRings",               "PMA0-4 sum charge > InnerRings level"},//3
			 {         "Noise",                        "out-of-gate hits AND NOT Or"},
			 {              "",                                                   ""},
			 {         "Noise",                        "out-of-gate hits AND NOT Or"},
			 {         "CB-Or",                                "collision-BC AND Or"},
			 {              "",                                                   ""},
			 {              "",                                                   ""},
			 {              "",                                                   ""},
			 { "CB-InnerRings",                        "collision-BC AND InnerRings"},
			 {              "",                                                   ""},
			 {   "BackgroundC",                                    "beam2-BC AND Or"}
}};
struct GBTunit { // (13 + 3 + 10) registers * 4 bytes = 104 bytes
    union ControlData {
        quint32 registers[16] = {0};
        struct {
            quint32
                DG_MODE                      :  4, //┐
                TG_MODE                      :  4, //│
                RESET                        :  8, //│
                TG_CTP_EMUL_MODE             :  4, //│
                HB_RESPONSE                  :  1, //│D8
                BYPASS_MODE                  :  1, //│
                READOUT_LOCK                 :  1, //│
                HB_REJECT                    :  1, //│
                shiftRxPhase                 :  1, //│
                                             :  7, //┘
                DG_TRG_RESPOND_MASK,               //]D9
                DG_BUNCH_PATTERN,                  //]DA
                TG_SINGLE_VALUE,                   //]DB
                TG_PATTERN_LSB,                    //]DC
                TG_PATTERN_MSB,                    //]DD
                TG_CONT_VALUE,                     //]DE
                DG_BUNCH_FREQ                : 16, //┐
                TG_BUNCH_FREQ                : 16, //┘DF
                DG_FREQ_OFFSET               : 12, //┐
                                             :  4, //│
                TG_FREQ_OFFSET               : 12, //│E0
                TG_HBr_RATE                  :  4, //┘
                RDH_FEE_ID                   : 16, //┐
                RDH_SYS_ID                   :  8, //│E1
                PRIORITY_BIT                 :  8, //┘
                                             : 32, //]E2
                BCID_DELAY                   : 12, //┐
                                             : 20, //┘E3
                DATA_SEL_TRG_MASK                , //]E4
                _reservedSpace[3]                ; //]E5-E7
        };
    } Control;
    union StatusData {
        quint32 registers[10] = {0};
        struct {
            quint32
                phaseAlignerCPLLlock         :  1, //┐
				RxWordClockReady             :  1, //│
                RxFrameClockReady            :  1, //│
                MGTlinkReady                 :  1, //│
                TxResetDone                  :  1, //│
                TxFSMresetDone               :  1, //│
                GBTRxReady                   :  1, //│
                GBTRxError                   :  1, //│E8
                RxPhaseError                 :  1, //│
                                             :  7, //│
                READOUT_MODE                 :  4, //│
                BCID_SYNC_MODE               :  4, //│
                RX_PHASE                     :  4, //│
                CRU_READOUT_MODE             :  4, //┘
                CRU_ORBIT                        , //]E9
                FIFOempty_header             :  1, //┐
                FIFOempty_data               :  1, //│
                FIFOempty_trg                :  1, //│
                FIFOempty_slct               :  1, //│
                FIFOempty_cntpck             :  1, //│
                FIFOempty_errorReport        :  1, //│
                                             : 10, //│
                slctFIFOemptyWhileRead       :  1, //│
                FIFOnotEmptyOnRunStart_slct  :  1, //│
                FIFOnotEmptyOnRunStart_cntpck:  1, //│
                FIFOnotEmptyOnRunStart_trg   :  1, //│EA
                trgFIFOwasFull               :  1, //│
                FIFOnotEmptyOnRunStart_data  :  1, //│
                FIFOnotEmptyOnRunStart_header:  1, //│
                TCMdataFIFOfull              :  1, //│
                PMpacketCorruptedExtraWord   :  1, //│
                PMpacketCorruptedEarlyHeader :  1, //│
                BCsyncLostInRun              :  1, //│
                                             :  4, //│
                dataFIFOnotReady             :  1, //┘
                CNVdropCount                 : 16, //┐
                CNVFIFOmax                   : 16, //┘EB
                SELdropCount                 : 16, //┐
                SELFIFOmax                   : 16, //┘EC
                wordsCount                   : 32, //]ED
                BCindicatorData              : 12, //┐
                BCmodalityData               :  4, //│
                BCindicatorTrg               : 12, //│EE
                BCmodalityTrg                :  4, //┘
                                             : 16, //┐
                FTMIPbusFIFOcount            : 16, //┘EF
                FTMIPbusFIFOdata                 , //]F0
                eventsCount                      ; //]F1
        };
    } Status;
static const quint8
    controlSize   =   13,
    statusSize    =   10,
    controlAddress= 0xD8,
    statusAddress = 0xE8,
//data generator states
    DG_noData     = 0,
    DG_main       = 1,
    DG_Tx         = 2,
//trigger generator states
    TG_noTrigger  = 0,
    TG_continuous = 1,
    TG_Tx         = 2,
//readout modes
    RO_idle       = 0,
    RO_continuous = 1,
    RO_triggered  = 2,
//BCID sync modes
    BS_start = 0,
    BS_sync  = 1,
    BS_lost  = 2,
//positions of reset bits
    RB_orbitSync            =  8,
    RB_dataCounter          =  9,
    RB_generatorsBunchOffset= 10,
    RB_GBTRxError           = 11,
    RB_GBT                  = 12,
    RB_RXphaseError         = 13,
    RB_readoutFSM           = 14,
    RB_errorReport          = 15;
    static constexpr quint32 defaults[controlSize] = {
        0x00100000, //D8, HB response is on, emulation is off
              0x40, //D9, show BC indicator for 'Calibration' trigger
                 0, //DA, no emulated data
                 0, //DB, (not used anymore)
                 0, //DC, empty trigger pattern
                 0, //DD, empty trigger pattern
                 0, //DE, zero trigger type
                 0, //DF, zero intervals
                 0, //E0, zero offsets
                 0, //E1, RDH settings, to be filled during initialization
                 0, //E2, (not used)
                 0, //E3, individual for each side, to be restored from settings
              0x10  //E4, select data on 'Physics' trigger
    };
    inline bool isOK() const {return
//        Status.GBTRxReady && //this flag indicates errors in past, the actual error bit is read from board status
//        !Status.GBTRxError &&
        !Status.RxPhaseError &&
        Status.READOUT_MODE == Status.CRU_READOUT_MODE &&
        (Control.registers[0] & 1 << 14) == 0 && //FSM reset not locked
        (Status.registers[2] >> 16 & 0x04FF) == 0 && //no FSM errors except PM-specific ones
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
         wordsRate =  wordsNew >=  wordsOld ? ( wordsNew -  wordsOld) * 1000. / time_ms :  wordsNew;
        eventsRate = eventsNew >= eventsOld ? (eventsNew - eventsOld) * 1000. / time_ms : eventsNew;
         wordsOld =  wordsNew;
        eventsOld = eventsNew;
        oldTime = newTime;
    }
};
struct GBTword {
    quint16 p[5];
    QString printHex() const { return QString::asprintf("%04X%04X %04X %04X%04X", p[4], p[3], p[2], p[1], p[0]); }
};
struct GBTerrorReport {
    static const quint32 errCodeBCsyncLostInRun = 0xEEEE000A,
                         errCodePMearlyheader   = 0xEEEE0009,
                         errCodeFIFOoverload    = 0xEEEE0008;
    static const quint8 reportSize = 36, address = 0xF2;
    quint32  errCode;                   //]IPbus word 0
    union {
        struct TypeBCsyncLostInRun {
            struct {
                GBTword data;
                quint16 counter : 12,
                        isData  :  4;
            } w[10];                    //]IPbus word 1-30
            quint16 BC_CRU, BC_Board;   //]IPbus word 31
            quint32 orbitBoard,         //]IPbus word 32
                    orbitCRU,           //]IPbus word 33
                    _reservedSpace[2];  //]IPbus word 34-35
        } SL;
        struct TypePMearlyHeader {
            GBTword w[14];
        } EH;
        struct TypeFIFOoverload {
            GBTword w[13];              //┐
            quint16 _reservedSpace0,    //┘IPbus word 1-33
                    rdRate, wrRate;     //]IPbus word 34
            quint32 _reservedSpace1;    //]IPbus word 35
        } FO;
    };
    quint32 *data = (quint32 *)this;
    static const inline QString LF = QString::asprintf("\n%*s", 24, ""); //a new line with indentation equal to timestamp length in logfile
    QString print() const {
        switch (errCode) {
        case 0: return "error (zero report)";
        case errCodeBCsyncLostInRun:  {
            QString res = "BC sync lost in run";
            res.append(LF + QString::asprintf("Board BCid: %08X  %03X", SL.orbitBoard, SL.BC_Board));
            res.append(LF + QString::asprintf("  CRU BCid: %08X  %03X", SL.orbitCRU  , SL.BC_CRU  ));
            res.append(LF + "#### isData GBTword");
            for (quint8 i=0; i<10; ++i) res.append(LF + QString::asprintf("%4d %-6s ", SL.w[i].counter, SL.w[i].isData ? "true" : "false") + SL.w[i].data.printHex());
            return res;               }
        case errCodePMearlyheader:    {
            QString res = "Input packet corrupted: header too early" + LF + "## GBTword";
            for (quint8 i=0; i<14; ++i) res.append(LF + QString::asprintf("%2d ", i) + EH.w[i].printHex());
            return res;               }
        case errCodeFIFOoverload:     {
            QString res = "raw data FIFO overload";
            res.append(LF + QString::asprintf("%d read and %d write operations in last 1000 cycles", FO.rdRate, FO.wrRate));
            res.append(LF + "## GBTword");
            for (quint8 i=0; i<13; ++i) res.append(LF + QString::asprintf("%2d ", i) + FO.w[i].printHex());
            return res;               }
        default:                      {
            QString res = "Unknown error report:" + LF + "## IPbusWord";
            for (quint8 i=0; i<reportSize; ++i) res.append(LF + QString::asprintf("%2d %08X", i, data[i]));
            return res;               }
        }
    }
};

struct Parameter {
    quint8 address,
           bitwidth,
           bitshift,
           interval; //for PM channels parameters only
    Parameter(quint8 address = 0, quint8 bitwidth = 32, quint8 bitshift = 0, quint8 interval = 0): address(address), bitwidth(bitwidth), bitshift(bitshift), interval(interval) {}
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
    {"shiftRxPhase"         , {0xD8,  1, 24}},
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
    {"RDH_SYS_ID"           , {0xE1,  8, 16}},
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
    Timestamp() = default;
    Timestamp(quint16 y, quint8 mo, quint8 d, quint8 h, quint8 mi, quint8 s): second(s), minute(mi), hour(h), year(y % 2000), month(mo), day(d) {}
    QString printFull() { return QString::asprintf("20%02d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second); }
    QString printISO () { return QString::asprintf("20%02d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, minute, second); }
                                    //          ↓10       ↓20       ↓30       ↓40       ↓50     60↓  ↓63
    static constexpr char alph[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz|~";
    QString printCode1() { //from "011.a0" (beginning of 2020) to "hCV.Nx" (end of 2063), minute accuracy
        return *(quint32 *)(this)==0 ? "000000" : QString::asprintf("%c%c%c.%c%c", alph[(year - 20) % 64], alph[month], alph[day], alph[hour], alph[minute]);
    }
    QString printCode2() { //from "0GW0" (beginning of 2020) to "~Ftx" (end of 2035), minute accuracy
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
        line1signalStable   : 1,
        syncError           : 1, //this bit should be read from side status register (0x1A or 0x3A)
        line2delay          : 5,
        line2signalLost     : 1,
        line2signalStable   : 1,
        bitPositionsOK      : 1,
        line3delay          : 5,
        line3signalLost     : 1,
        line3signalStable   : 1,
        linkOK              : 1;
};

struct regblock {
    const quint8 addr, endAddr;
    inline quint8 size() { return endAddr - addr + 1; }
};

class AdvancedDIMservice {
    DimService *service;
    const bool dataIsExternal;
    const size_t dataSize;
    void *dataNew, *dataOld;
    std::function<void(void *)> dataCollect;
public:
    AdvancedDIMservice(const char *name, const char *format, size_t size, std::function<void(void *)> dataCollectingFunction, void *data = nullptr):
        dataIsExternal(data != nullptr),
        dataSize(size),
        dataNew(dataIsExternal ? data : malloc(dataSize)),
        dataOld(malloc(dataSize)),
        dataCollect(dataCollectingFunction)
    {
        service = new DimService(name, format, dataNew, int(dataSize));
    }
    ~AdvancedDIMservice() {
        delete service;
        if (!dataIsExternal) free(dataNew);
        free(dataOld);
    }
    void updateService(bool onlyIfChanged = true) {
        if (dataCollect != 0) dataCollect(dataNew);
        if (onlyIfChanged == false) {
            service->updateService();
        } else if (memcmp(dataNew, dataOld, dataSize) != 0) {//data has changed
            memcpy(dataOld, dataNew, dataSize);
            service->updateService();
        }
    }
//    void updateAnyway() {
//        if (dataCollect != 0) dataCollect(dataNew);
//        service->updateService();
//    }
//    void updateIfChanged() {
//        if (dataCollect != 0) dataCollect(dataNew);
//        if (memcmp(dataNew, dataOld, dataSize) != 0) {//data has changed
//            memcpy(dataOld, dataNew, dataSize);
//            service->updateService();
//        }
//    }
};

inline quint32 changeNbits(quint32 base, quint8 length, quint8 shift, quint32 value) {
    if (length == 0) return base;
    if (length >= 32) return value;
    if (length == 1) return value ? base | 1 << shift : base & ~(1 << shift);
    quint32 mask = (1 << length) - 1;
    base &= ~(mask << shift);
    base |= (mask & value) << shift;
    return base;
}

#endif // GBT_H
