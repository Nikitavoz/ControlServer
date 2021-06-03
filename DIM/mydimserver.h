#ifndef MYDIMSERVER_H
#define MYDIMSERVER_H


#include "classes.h"
#include "common.h"

class TCMPars
{
public:
    TCMPars(MyDimServer* _server);
    ~TCMPars();

    MyDimServer* pServer;
    quint16 TCM_FEE_id;
    void publish();         //  Here the order of publishing is defined

    TCMActnValAppPar<quint8>* countersupdrate;
    TCMActnValAppPar<quint8>* extendedreadout;
    TCMActnValAppPar<quint8>* scsumsides;
    TCMActnValAppPar<quint8>* csumsides;
    TCMActnValAppPar<quint8>* addcdelay;
    TCMActnValAppPar<quint8>* ressw1;
    TCMActnValAppPar<quint8>* ressw2;
    TCMActnValAppPar<quint8>* ressw3;
    TCMActnValAppPar<quint8>* ressw4;


    TCMfullPar<qint16>* delayA;
    TCMfullPar<qint16>* delayC;
    TCMfullPar<qint16>* vtimelow;
    TCMfullPar<qint16>* vtimehight;
    TCMfullPar<quint16>* sclevelA;
    TCMfullPar<quint16>* sclevelC;
    TCMfullPar<quint16>* clevelA;
    TCMfullPar<quint16>* clevelC;
    TCMfullPar<quint16>* chmfskA;
    TCMfullPar<quint16>* chmfskC;

    PMonlyValAppPar<quint8>* swchon;
    PMonlyValAppPar<quint8>* swchoff;
    PMonlyAppPar* resetcounters;
    PMonlyAppPar* resetsystem;

    PMonlyActPar<quint16>* boardstatus;
    PMonlyActPar<quint16>* temperature;
    PMonlyActPar<quint16>* serialnum;
    PMonlyActPar<quint32>* fwversion;

    PMonlyActPar<quint16>* sideAstatus;
    PMonlyActPar<quint16>* sideCstatus;
    PMonlyActPar<quint32>* cntorA;
    PMonlyActPar<quint32>* cntorArate;
    PMonlyActPar<quint32>* cntorC;
    PMonlyActPar<quint32>* cntorCrate;
    PMonlyActPar<quint32>* cntsc;
    PMonlyActPar<quint32>* cntscrate;
    PMonlyActPar<quint32>* cntc;
    PMonlyActPar<quint32>* cntcrate;
    PMonlyActPar<quint32>* cntv;
    PMonlyActPar<quint32>* cntvrate;

    //                                                                          GBT Readout unit
    //                                                                          ================

    //                                                      control

    PMonlyAppPar* resetorbitsync;
    PMonlyAppPar* resetdrophitcnts;
    PMonlyAppPar* resetgenbunchoffset;
    PMonlyAppPar* resetgbterrors;
    PMonlyAppPar* resetgbt;
    PMonlyAppPar* resetrxphaseerror;
    PMonlyValAppPar<quint8>* sendreadoutcommand;
    PMonlyValAppPar<quint32>* tgsendsingle;

    PMfullPar<quint32>* tgpattern1;
    PMfullPar<quint32>* tgpattern0;
    PMfullPar<quint8>*  tgcontvalue;
    PMfullPar<quint16>* tgbunchfreq;
    PMfullPar<quint16>* tgfreqoffset;
    PMActnValAppPar<quint8>*  tgmode;
    PMActnValAppPar<quint8>*  hbresponse;
    PMActnValAppPar<quint8>* dgmode;
    PMfullPar<quint32>* dgtrgrespondmask;
    PMfullPar<quint32>* dgbunchpattern;
    PMfullPar<quint16>* dgbunchfreq;
    PMfullPar<quint16>* dgfreqoffset;
    PMfullPar<quint16>* rdhfeeid;
    PMfullPar<quint16>* rdhpar;
    PMfullPar<quint16>* rdhmaxpayload;
    PMfullPar<quint16>* rdhdetfield;
    PMfullPar<quint16>* crutrgcomparedelay;
    PMfullPar<quint16>* bciddelay;
    PMfullPar<quint32>* dataseltrgmask;

    //                                                      status

    PMonlyActPar<quint16>* bits;
    PMonlyActPar<quint8>* readoutmode;
    PMonlyActPar<quint8>* bcidsyncmode;
    PMonlyActPar<quint8>* rxphase;
    PMonlyActPar<quint32>* cruorbit;
    PMonlyActPar<quint16>* crubc;
    PMonlyActPar<quint16>* rawfifo;
    PMonlyActPar<quint16>* selfifo;
    PMonlyActPar<quint32>* selfirsthit;
    PMonlyActPar<quint32>* sellasthit;
    PMonlyActPar<quint32>* selhitsdropped;
    PMonlyActPar<quint16>* readoutrate;


    TCMfullPar<quint16>* orAsign;
    TCMfullPar<quint32>* orArate;
    TCMfullPar<quint16>* orCsign;
    TCMfullPar<quint32>* orCrate;
    TCMfullPar<quint16>* scsign;
    TCMfullPar<quint32>* scrate;
    TCMfullPar<quint16>* csign;
    TCMfullPar<quint32>* crate;
    TCMfullPar<quint16>* vsign;
    TCMfullPar<quint32>* vrate;

    TCMActnValAppPar<quint8>* orAenabled;
    TCMActnValAppPar<quint8>* orCenabled;
    TCMActnValAppPar<quint8>* scenabled;
    TCMActnValAppPar<quint8>* cenabled;
    TCMActnValAppPar<quint8>* venabled;

    str_ACT* statusoptioncode;
    twoValAPP* setoptioncode;

    TCMActnValAppPar<quint8>* laseron;
    TCMfullPar<quint32>* laserdiv;
    TCMfullPar<qint16>* laserdelay;
    TCMfullPar<quint32>* laserpattern1;
    TCMfullPar<quint32>* laserpattern0;
    TCMfullPar<quint16>* attenvalue;
    PMonlyActPar<quint8>* attenstatus;
};

class PMPars
{
public:
    PMPars(MyDimServer* _server);
    ~PMPars();

    MyDimServer* pServer;
    quint8 Nchannels = 12;
    quint16 PM_FEE_id;
    void publish();         //  Here the order of publishing is defined

    //                                                                        PM
    //                                                                       ====
    //                                                      control

    PMfullPar<quint16>* orgate;
    PMfullPar<quint16>* cfdsatr;
    PMfullPar<quint8>* chmask;
    PMonlyValAppPar<quint8>* swchon;
    PMonlyValAppPar<quint8>* swchoff;
    PMonlyAppPar* resetcounters;
    PMonlyAppPar* zerolvlcalibr;
    PMonlyAppPar* alltopm;

    //                                                      status

    PMonlyActPar<quint32>* linkstatus;
    PMonlyActPar<quint16>* boardstatus;
    PMonlyActPar<quint16>* temperature;
    PMonlyActPar<quint16>* serialnum;
    PMonlyActPar<quint32>* fwversion;

    //                                                                        PM  channel
    //                                                                        ===========
    //                                                      control

    QVector<PMCHfullPar<qint16>*> adczero;
    QVector<PMCHfullPar<qint16>*> adcdelay;
    QVector<PMCHfullPar<quint16>*> adc0offset;
    QVector<PMCHfullPar<quint16>*> adc1offset;
    QVector<PMCHfullPar<quint16>*> adc0range;
    QVector<PMCHfullPar<quint16>*> adc1range;
    QVector<PMCHfullPar<quint16>*> timealign;
    QVector<PMCHfullPar<qint16>*> cfdthreshold;
    QVector<PMCHfullPar<qint16>*> cfdzero;
    QVector<PMCHfullPar<qint16>*>  thresholdcalibr;

    //                                                      status

    QVector<PMCHonlyActPar<quint16>*> adc0meanampl;
    QVector<PMCHonlyActPar<quint16>*> adc1meanampl;
    QVector<PMCHonlyActPar<quint16>*> adc0zerolvl;
    QVector<PMCHonlyActPar<quint16>*> adc1zerolvl;
    QVector<PMCHonlyActPar<quint16>*> rawtdcdata;
    QVector<PMCHonlyActPar<quint32>*> cntcfd;
    QVector<PMCHonlyActPar<quint32>*> cntcfdrate;
    QVector<PMCHonlyActPar<quint32>*> cnttrg;
    QVector<PMCHonlyActPar<quint32>*> cnttrgrate;

    //                                                                          GBT Readout unit
    //                                                                          ================

    //                                                      control

    PMonlyAppPar* resetorbitsync;
    PMonlyAppPar* resetdrophitcnts;
    PMonlyAppPar* resetgenbunchoffset;
    PMonlyAppPar* resetgbterrors;
    PMonlyAppPar* resetgbt;
    PMonlyAppPar* resetrxphaseerror;
    PMonlyValAppPar<quint8>* sendreadoutcommand;
    PMonlyValAppPar<quint32>* tgsendsingle;

    PMfullPar<quint32>* tgpattern1;
    PMfullPar<quint32>* tgpattern0;
    PMfullPar<quint8>*  tgcontvalue;
    PMfullPar<quint16>* tgbunchfreq;
    PMfullPar<quint16>* tgfreqoffset;
    PMActnValAppPar<quint8>*  tgmode;
    PMActnValAppPar<quint8>*  hbresponse;
    PMActnValAppPar<quint8>* dgmode;
    PMfullPar<quint32>* dgtrgrespondmask;
    PMfullPar<quint32>* dgbunchpattern;
    PMfullPar<quint16>* dgbunchfreq;
    PMfullPar<quint16>* dgfreqoffset;
    PMfullPar<quint16>* rdhfeeid;
    PMfullPar<quint16>* rdhpar;
    PMfullPar<quint16>* rdhmaxpayload;
    PMfullPar<quint16>* rdhdetfield;
    PMfullPar<quint16>* crutrgcomparedelay;
    PMfullPar<quint16>* bciddelay;
    PMfullPar<quint32>* dataseltrgmask;

    //                                                      status

    PMonlyActPar<quint16>* bits;
    PMonlyActPar<quint8>* readoutmode;
    PMonlyActPar<quint8>* bcidsyncmode;
    PMonlyActPar<quint8>* rxphase;
    PMonlyActPar<quint32>* cruorbit;
    PMonlyActPar<quint16>* crubc;
    PMonlyActPar<quint16>* rawfifo;
    PMonlyActPar<quint16>* selfifo;
    PMonlyActPar<quint32>* selfirsthit;
    PMonlyActPar<quint32>* sellasthit;
    PMonlyActPar<quint32>* selhitsdropped;
    PMonlyActPar<quint16>* readoutrate;

};

//  #####################################################################################

class MyDimServer   :  public  QObject, public DimServer
{
    Q_OBJECT
public:
    MyDimServer(QString dns_node = "localhost", QString server_name="FIT DIM SERVER");
    ~MyDimServer();

//    QFile* DimServicesFile;
//    QFile* DimCommandsFile;

    bool excludeForWinCC=0;

    QString serverName;
    QString dnsNode;
    quint8 Npms = 1;
    PMPars* pm[18];
    TCMPars* tcm;


    void startServer();
    void stopServer();

    void OpenOutFile();
    void CloseOutFile();

    void setNChannels(quint8 nch);

    template<class Y>
    void emitSignal(pmch_pValSignal<Y> pSignal,quint16 FEEid,quint8 Chid, Y val);
    void emitSignal(pmch_pNonValSignal pSignal,quint16 FEEid,quint8 Chid);

    template<class Y>
    void emitSignal(pm_pValSignal<Y> pSignal,quint16 FEEid, Y val);
    void emitSignal(pm_pNonValSignal pSignal,quint16 FEEid);

    template<class Y>
    void emitSignal(tcm_pValSignal<Y>, Y val);

    void emitSignal(tcm_pNonValSignal pSignal);

    void emitSignal(pTwoValSignal,quint8,quint8);


signals:
    void set_ADC_ZERO_requested(quint16 FEEid, quint8 Ch, qint16 val);
    void set_ADC_DELAY_requested(quint16 FEEid, quint8 Ch, qint16 val);
    void set_ADC0_OFFSET_requested(quint16 FEEid, quint8 Ch, quint16 val);
    void set_ADC1_OFFSET_requested(quint16 FEEid, quint8 Ch, quint16 val);
    void set_ADC0_RANGE_requested(quint16 FEEid, quint8 Ch, quint16 val);
    void set_ADC1_RANGE_requested(quint16 FEEid, quint8 Ch, quint16 val);
    void set_TIME_ALIGN_requested(quint16 FEEid, quint8 Ch, quint16 val);
    void set_CFD_THRESHOLD_requested(quint16 FEEid, quint8 Ch, qint16 val);
    void set_CFD_ZERO_requested(quint16 FEEid, quint8 Ch, qint16 val);
    void set_THRESHOLD_CALIBR_requested(quint16 FEEid, quint8 Ch, qint16 val);

    void set_TG_PATTERN_1_requested(quint16 FEEid, quint32 val);
    void set_TG_PATTERN_0_requested(quint16 FEEid, quint32 val);
    void set_TG_CONT_VALUE_requested(quint16 FEEid, quint8 val);
    void set_TG_BUNCH_FREQ_requested(quint16 FEEid, quint16 val);
    void set_TG_FREQ_OFFSET_requested(quint16 FEEid, quint16 val);

    void set_DG_TRG_RESPOND_MASK_requested(quint16 FEEid, quint32 val);
    void set_DG_BUNCH_PATTERN_requested(quint16 FEEid, quint32 val);
    void set_DG_BUNCH_FREQ_requested(quint16 FEEid, quint16 val);
    void set_DG_FREQ_OFFSET_requested(quint16 FEEid, quint16 val);
    void set_RDH_FEE_ID_requested(quint16 FEEid, quint16 val);
    void set_RDH_PAR_requested(quint16 FEEid, quint16 val);
    void set_RDH_MAX_PAYLOAD_requested(quint16 FEEid, quint16 val);
    void set_RDH_DET_FIELD_requested(quint16 FEEid, quint16 val);
    void set_CRU_TRG_COMPARE_DELAY_requested(quint16 FEEid, quint16 val);
    void set_BCID_DELAY_requested(quint16 FEEid, quint16 val);
    void set_DATA_SEL_TRG_MASK_requested(quint16 FEEid, quint32 val);

    void set_OR_GATE_requested(quint16 FEEid, quint16 val);
    void set_CFD_SATR_requested(quint16 FEEid, quint16 val);
    void set_CH_MASK_requested(quint16 FEEid, quint8 val);

    void apply_ADC_ZERO_requested(quint16 FEEid, quint8 Ch);
    void apply_ADC_DELAY_requested(quint16 FEEid, quint8 Ch);
    void apply_ADC0_OFFSET_requested(quint16 FEEid, quint8 Ch);
    void apply_ADC1_OFFSET_requested(quint16 FEEid, quint8 Ch);
    void apply_ADC0_RANGE_requested(quint16 FEEid, quint8 Ch);
    void apply_ADC1_RANGE_requested(quint16 FEEid, quint8 Ch);
    void apply_TIME_ALIGN_requested(quint16 FEEid, quint8 Ch);
    void apply_CFD_THRESHOLD_requested(quint16 FEEid, quint8 Ch);
    void apply_CFD_ZERO_requested(quint16 FEEid, quint8 Ch);
    void apply_THRESHOLD_CALIBR_requested(quint16 FEEid, quint8 Ch);

    void apply_RESET_ORBIT_SYNC_requested(quint16 FEEid);
    void apply_RESET_DROPPING_HIT_COUNTERS_requested(quint16 FEEid);
    void apply_RESET_GEN_BUNCH_OFFSET_requested(quint16 FEEid);
    void apply_RESET_GBT_ERRORS_requested(quint16 FEEid);
    void apply_RESET_GBT_requested(quint16 FEEid);
    void apply_RESET_RX_PHASE_ERROR_requested(quint16 FEEid);
    void apply_SEND_READOUT_COMMAND_requested(quint16 FEEid, quint8 cmd);
    void apply_TG_SEND_SINGLE_requested(quint16 FEEid, quint32 val);
    void apply_TG_PATTERN_1_requested(quint16 FEEid);
    void apply_TG_PATTERN_0_requested(quint16 FEEid);
    void apply_TG_CONT_VALUE_requested(quint16 FEEid);
    void apply_TG_BUNCH_FREQ_requested(quint16 FEEid);
    void apply_TG_FREQ_OFFSET_requested(quint16 FEEid);
    void apply_TG_MODE_requested(quint16 FEEid, quint8 val);
    void apply_HB_RESPONSE_requested(quint16 FEEid, quint8 isOn);
    void apply_DG_MODE_requested(quint16 FEEid, quint8 val);
    void apply_DG_TRG_RESPOND_MASK_requested(quint16 FEEid);
    void apply_DG_BUNCH_PATTERN_requested(quint16 FEEid);
    void apply_DG_BUNCH_FREQ_requested(quint16 FEEid);
    void apply_DG_FREQ_OFFSET_requested(quint16 FEEid);
    void apply_RDH_FEE_ID_requested(quint16 FEEid);
    void apply_RDH_PAR_requested(quint16 FEEid);
    void apply_RDH_MAX_PAYLOAD_requested(quint16 FEEid);
    void apply_RDH_DET_FIELD_requested(quint16 FEEid);
    void apply_CRU_TRG_COMPARE_DELAY_requested(quint16 FEEid);
    void apply_BCID_DELAY_requested(quint16 FEEid);
    void apply_DATA_SEL_TRG_MASK_requested(quint16 FEEid);

    void apply_OR_GATE_requested(quint16 FEEid);
    void apply_CFD_SATR_requested(quint16 FEEid);
    void apply_CH_MASK_requested(quint16 FEEid);
    void apply_SwChOn_requested(quint16 FEEid, quint8 Ch);
    void apply_SwChOff_requested(quint16 FEEid, quint8 Ch);
    void apply_RESET_COUNTERS_requested(quint16 FEEid);
    void apply_ZERO_LVL_CALIBR_requested(quint16 FEEid);
    void apply_ALLtoPM_requested(quint16 FEEid);


public slots:
//    void test_slot0(quint16 FEEid){ cout << "Slot0 is reached " << FEEid << endl; }
//    void test_slot1(quint16 FEEid, quint16 val){ cout << "Slot1 is reached " << FEEid << " " << val << endl; }
//    void test_slot2(quint16 FEEid, quint8 CHid, quint16 val){ cout << "Slot2 is reached " << FEEid << " " << CHid << " " << val << endl; }
//    void test_slot3(quint16 FEEid, quint8 CHid, qint16 val){ cout << "Slot2 is reached " << FEEid << " " << CHid << " " << val << endl; }


    void update_act_ADC_ZERO    (quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adczero[Ch-1]->updateAct(val);}
    void update_act_ADC_DELAY   (quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adcdelay[Ch-1]->updateAct(val);}
    void update_act_ADC0_OFFSET (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc0offset[Ch-1]->updateAct(val);}
    void update_act_ADC1_OFFSET (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc1offset[Ch-1]->updateAct(val);}
    void update_act_ADC0_RANGE  (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc0range[Ch-1]->updateAct(val);}
    void update_act_ADC1_RANGE  (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc1range[Ch-1]->updateAct(val);}
    void update_act_TIME_ALIGN   (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->timealign[Ch-1]->updateAct(val);}
    void update_act_CFD_THRESHOLD(quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->cfdthreshold[Ch-1]->updateAct(val);}
    void update_act_CFD_ZERO    (quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->cfdzero[Ch-1]->updateAct(val);}
    void update_act_THRESHOLD_CALIBR(quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->thresholdcalibr[Ch-1]->updateAct(val);}

    void update_new_ADC_ZERO    (quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adczero[Ch-1]->updateNew(val);}
    void update_new_ADC_DELAY   (quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adcdelay[Ch-1]->updateNew(val);}
    void update_new_ADC0_OFFSET (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc0offset[Ch-1]->updateNew(val);}
    void update_new_ADC1_OFFSET (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc1offset[Ch-1]->updateNew(val);}
    void update_new_ADC0_RANGE  (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc0range[Ch-1]->updateNew(val);}
    void update_new_ADC1_RANGE  (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc1range[Ch-1]->updateNew(val);}
    void update_new_TIME_ALIGN   (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->timealign[Ch-1]->updateNew(val);}
    void update_new_CFD_THRESHOLD(quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->cfdthreshold[Ch-1]->updateNew(val);}
    void update_new_CFD_ZERO    (quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->cfdzero[Ch-1]->updateNew(val);}
    void update_new_THRESHOLD_CALIBR(quint16 FEEid, quint8 Ch, qint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->thresholdcalibr[Ch-1]->updateNew(val);}

    void update_act_ADC0_MEANAMPL   (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc0meanampl[Ch-1]->updateAct(val);}
    void update_act_ADC1_MEANAMPL   (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc1meanampl[Ch-1]->updateAct(val);}
    void update_act_ADC0_ZEROLVL    (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc0zerolvl[Ch-1]->updateAct(val);}
    void update_act_ADC1_ZEROLVL    (quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->adc1zerolvl[Ch-1]->updateAct(val);}
    void update_act_RAW_TDC_DATA(quint16 FEEid, quint8 Ch, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->rawtdcdata[Ch-1]->updateAct(val);}
    void update_act_CNT_CFD         (quint16 FEEid, quint8 Ch, quint32 val){pm[FT0_FEE_ID.key(FEEid)-1]->cntcfd[Ch-1]->updateAct(val);}
    void update_act_CNT_CFD_RATE    (quint16 FEEid, quint8 Ch, quint32 val){pm[FT0_FEE_ID.key(FEEid)-1]->cntcfdrate[Ch-1]->updateAct(val);}
    void update_act_CNT_TRG         (quint16 FEEid, quint8 Ch, quint32 val){pm[FT0_FEE_ID.key(FEEid)-1]->cnttrg[Ch-1]->updateAct(val);}
    void update_act_CNT_TRG_RATE    (quint16 FEEid, quint8 Ch, quint32 val){pm[FT0_FEE_ID.key(FEEid)-1]->cnttrgrate[Ch-1]->updateAct(val);}

    void update_act_TG_PATTERN_1(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->tgpattern1->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgpattern1->updateAct(val);}
    void update_act_TG_PATTERN_0(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->tgpattern0->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgpattern0->updateAct(val);}
    void update_act_TG_CONT_VALUE(quint16 FEEid, quint8 val){if(FEEid == 0xF000) tcm->tgcontvalue->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgcontvalue->updateAct(val);}
    void update_act_TG_BUNCH_FREQ(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->tgbunchfreq->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgbunchfreq->updateAct(val);}
    void update_act_TG_FREQ_OFFSET(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->tgfreqoffset->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgfreqoffset->updateAct(val);}
    void update_act_TG_MODE(quint16 FEEid, quint8 val){if(FEEid == 0xF000) tcm->tgmode->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgmode->updateAct(val);}
    void update_act_HB_RESPONSE(quint16 FEEid, quint8 isOn){if(FEEid == 0xF000) tcm->hbresponse->updateAct(isOn) ; else pm[FT0_FEE_ID.key(FEEid)-1]->hbresponse->updateAct(isOn);}
    void update_act_DG_MODE(quint16 FEEid, quint8 val){if(FEEid == 0xF000) tcm->dgmode->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgmode->updateAct(val);}
    void update_act_DG_TRG_RESPOND_MASK(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->dgtrgrespondmask->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgtrgrespondmask->updateAct(val);}
    void update_act_DG_BUNCH_PATTERN(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->dgbunchpattern->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgbunchpattern->updateAct(val);}
    void update_act_DG_BUNCH_FREQ(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->dgbunchfreq->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgbunchfreq->updateAct(val);}
    void update_act_DG_FREQ_OFFSET(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->dgfreqoffset->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgfreqoffset->updateAct(val);}
    void update_act_RDH_FEE_ID(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhfeeid->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhfeeid->updateAct(val);}
    void update_act_RDH_PAR(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhpar->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhpar->updateAct(val);}
    void update_act_RDH_MAX_PAYLOAD(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhmaxpayload->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhmaxpayload->updateAct(val);}
    void update_act_RDH_DET_FIELD(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhdetfield->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhdetfield->updateAct(val);}
    void update_act_CRU_TRG_COMPARE_DELAY(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->crutrgcomparedelay->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->crutrgcomparedelay->updateAct(val);}
    void update_act_BCID_DELAY(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->bciddelay->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->bciddelay->updateAct(val);}
    void update_act_DATA_SEL_TRG_MASK(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->dataseltrgmask->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dataseltrgmask->updateAct(val);}
    void update_act_BITS(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->bits->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->bits->updateAct(val);}
    void update_act_READOUT_MODE(quint16 FEEid, quint8 val){if(FEEid == 0xF000) tcm->readoutmode->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->readoutmode->updateAct(val);}
    void update_act_BCID_SYNC_MODE(quint16 FEEid, quint8 val){if(FEEid == 0xF000) tcm->bcidsyncmode->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->bcidsyncmode->updateAct(val);}
    void update_act_RX_PHASE(quint16 FEEid, quint8 val){if(FEEid == 0xF000) tcm->rxphase->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rxphase->updateAct(val);}
    void update_act_CRU_ORBIT(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->cruorbit->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->cruorbit->updateAct(val);}
    void update_act_CRU_BC(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->crubc->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->crubc->updateAct(val);}
    void update_act_RAW_FIFO(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rawfifo->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rawfifo->updateAct(val);}
    void update_act_SEL_FIFO(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->selfifo->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->selfifo->updateAct(val);}
    void update_act_SEL_FIRST_HIT_DROPPED_ORBIT(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->selfirsthit->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->selfirsthit->updateAct(val);}
    void update_act_SEL_LAST_HIT_DROPPED_ORBIT(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->sellasthit->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->sellasthit->updateAct(val);}
    void update_act_SEL_HITS_DROPPED(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->selhitsdropped->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->selhitsdropped->updateAct(val);}
    void update_act_READOUT_RATE(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->readoutrate->updateAct(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->readoutrate->updateAct(val);}

    void update_act_OR_GATE(quint16 FEEid, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->orgate->updateAct(val);}
    void update_act_CFD_SATR(quint16 FEEid, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->cfdsatr->updateAct(val);}
    void update_act_CH_MASK(quint16 FEEid, quint8 val){pm[FT0_FEE_ID.key(FEEid)-1]->chmask->updateAct(val);}

    void update_new_TG_PATTERN_1(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->tgpattern1->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgpattern1->updateNew(val);}
    void update_new_TG_PATTERN_0(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->tgpattern0->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgpattern0->updateNew(val);}
    void update_new_TG_CONT_VALUE(quint16 FEEid, quint8 val){if(FEEid == 0xF000) tcm->tgcontvalue->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgcontvalue->updateNew(val);}
    void update_new_TG_BUNCH_FREQ(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->tgbunchfreq->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgbunchfreq->updateNew(val);}
    void update_new_TG_FREQ_OFFSET(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->tgfreqoffset->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->tgfreqoffset->updateNew(val);}

    void update_new_DG_TRG_RESPOND_MASK(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->dgtrgrespondmask->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgtrgrespondmask->updateNew(val);}
    void update_new_DG_BUNCH_PATTERN(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->dgbunchpattern->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgbunchpattern->updateNew(val);}
    void update_new_DG_BUNCH_FREQ(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->dgbunchfreq->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgbunchfreq->updateNew(val);}
    void update_new_DG_FREQ_OFFSET(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->dgfreqoffset->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dgfreqoffset->updateNew(val);}
    void update_new_RDH_FEE_ID(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhfeeid->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhfeeid->updateNew(val);}
    void update_new_RDH_PAR(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhpar->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhpar->updateNew(val);}
    void update_new_RDH_MAX_PAYLOAD(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhmaxpayload->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhmaxpayload->updateNew(val);}
    void update_new_RDH_DET_FIELD(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->rdhdetfield->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->rdhdetfield->updateNew(val);}
    void update_new_CRU_TRG_COMPARE_DELAY(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->crutrgcomparedelay->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->crutrgcomparedelay->updateNew(val);}
    void update_new_BCID_DELAY(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->bciddelay->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->bciddelay->updateNew(val);}
    void update_new_DATA_SEL_TRG_MASK(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->dataseltrgmask->updateNew(val) ; else pm[FT0_FEE_ID.key(FEEid)-1]->dataseltrgmask->updateNew(val);}

    void update_new_OR_GATE(quint16 FEEid, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->orgate->updateNew(val);}
    void update_new_CFD_SATR(quint16 FEEid, quint16 val){pm[FT0_FEE_ID.key(FEEid)-1]->cfdsatr->updateNew(val);}
    void update_new_CH_MASK(quint16 FEEid, quint8 val){pm[FT0_FEE_ID.key(FEEid)-1]->chmask->updateNew(val);}

    void update_act_LINK_STATUS(quint16 FEEid, quint32 val){pm[FT0_FEE_ID.key(FEEid)-1]->linkstatus->updateAct(val);}
    void update_act_BOARD_STATUS(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->boardstatus->updateAct(val); else pm[FT0_FEE_ID.key(FEEid)-1]->boardstatus->updateAct(val);}
    void update_act_TEMPERATURE(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->temperature->updateAct(val); else pm[FT0_FEE_ID.key(FEEid)-1]->temperature->updateAct(val);}
    void update_act_SERIAL_NUM(quint16 FEEid, quint16 val){if(FEEid == 0xF000) tcm->serialnum->updateAct(val); else pm[FT0_FEE_ID.key(FEEid)-1]->serialnum->updateAct(val);}
    void update_act_FW_VERSION(quint16 FEEid, quint32 val){if(FEEid == 0xF000) tcm->fwversion->updateAct(val); else pm[FT0_FEE_ID.key(FEEid)-1]->fwversion->updateAct(val);}


signals:
    void apply_COUNTERS_UPD_RATE_requested(quint8 val);
    void apply_EXTENDED_READOUT_requested(quint8 isOn);
    void apply_SC_SUM_SIDES_requested(quint8 isOn);
    void apply_C_SUM_SIDES_requested(quint8 isOn);
    void apply_ADD_C_DELAY_requested(quint8 isOn);
    void apply_RES_SW_1_requested(quint8 isOn);
    void apply_RES_SW_2_requested(quint8 isOn);
    void apply_RES_SW_3_requested(quint8 isOn);
    void apply_RES_SW_4_requested(quint8 isOn);

    void set_DELAY_A_requested(qint16 val);
    void set_DELAY_C_requested(qint16 val);
    void set_VTIME_LOW_requested(qint16 val);
    void set_VTIME_HIGH_requested(qint16 val);
    void set_SC_LEVEL_A_requested(quint16 val);
    void set_SC_LEVEL_C_requested(quint16 val);
    void set_C_LEVEL_A_requested(quint16 val);
    void set_C_LEVEL_C_requested(quint16 val);
    void set_CH_MASK_A_requested(quint16 val);
    void set_CH_MASK_C_requested(quint16 val);

    void apply_DELAY_A_requested();
    void apply_DELAY_C_requested();
    void apply_VTIME_LOW_requested();
    void apply_VTIME_HIGH_requested();
    void apply_SC_LEVEL_A_requested();
    void apply_SC_LEVEL_C_requested();
    void apply_C_LEVEL_A_requested();
    void apply_C_LEVEL_C_requested();
    void apply_CH_MASK_A_requested();
    void apply_CH_MASK_C_requested();

////    void apply_SwChOn_requested(quint16 FEEid, quint8 Ch);
////    void apply_SwChOff_requested(quint16 FEEid, quint8 Ch);
////    void apply_RESET_COUNTERS_requested(quint16 FEEid);
    void apply_RESET_SYSTEM_requested(quint16 FEEid);

    void set_OR_A_SIGN_requested(quint16 val);
    void set_OR_A_RATE_requested(quint32 val);
    void set_OR_C_SIGN_requested(quint16 val);
    void set_OR_C_RATE_requested(quint32 val);
    void set_SC_SIGN_requested(quint16 val);
    void set_SC_RATE_requested(quint32 val);
    void set_C_SIGN_requested(quint16 val);
    void set_C_RATE_requested(quint32 val);
    void set_V_SIGN_requested(quint16 val);
    void set_V_RATE_requested(quint32 val);

    void apply_OR_A_SIGN_requested();
    void apply_OR_A_RATE_requested();
    void apply_OR_C_SIGN_requested();
    void apply_OR_C_RATE_requested();
    void apply_SC_SIGN_requested();
    void apply_SC_RATE_requested();
    void apply_C_SIGN_requested();
    void apply_C_RATE_requested();
    void apply_V_SIGN_requested();
    void apply_V_RATE_requested();

    void apply_OR_A_ENABLED_requested(quint8 isOn);
    void apply_OR_C_ENABLED_requested(quint8 isOn);
    void apply_SC_ENABLED_requested(quint8 isOn);
    void apply_C_ENABLED_requested(quint8 isOn);
    void apply_V_ENABLED_requested(quint8 isOn);

    void apply_LASER_ON_requested(quint8 isOn);
    void apply_LASER_DIV_requested();
    void apply_LASER_DELAY_requested();
    void apply_LASER_PATTERN_1_requested();
    void apply_LASER_PATTERN_0_requested();
    void apply_ATTEN_VALUE_requested();

    void set_LASER_DIV_requested(quint32 val);
    void set_LASER_DELAY_requested(qint16 val);
    void set_LASER_PATTERN_1_requested(quint32 val);
    void set_LASER_PATTERN_0_requested(quint32 val);
    void set_ATTEN_VALUE_requested(quint16 val);

    void apply_SET_OPTIONCODE_requested(quint8 mode,quint8 Ch);


public slots:
    void update_act_COUNTERS_UPD_RATE(quint8 val)   {tcm->countersupdrate->updateAct(val);}
    void update_act_EXTENDED_READOUT(quint8 isOn)   {tcm->extendedreadout->updateAct(isOn);}
    void update_act_SC_SUM_SIDES(quint8 isOn)       {tcm->scsumsides->updateAct(isOn);}
    void update_act_C_SUM_SIDES(quint8 isOn)        {tcm->csumsides->updateAct(isOn);}
    void update_act_ADD_C_DELAY(quint8 isOn)        {tcm->addcdelay->updateAct(isOn);}
    void update_act_RES_SW_1(quint8 isOn)           {tcm->ressw1->updateAct(isOn);}
    void update_act_RES_SW_2(quint8 isOn)           {tcm->ressw2->updateAct(isOn);}
    void update_act_RES_SW_3(quint8 isOn)           {tcm->ressw3->updateAct(isOn);}
    void update_act_RES_SW_4(quint8 isOn)           {tcm->ressw4->updateAct(isOn);}

    void update_new_DELAY_A(qint16 val){tcm->delayA->updateNew(val);}
    void update_new_DELAY_C(qint16 val){tcm->delayC->updateNew(val);}
    void update_new_VTIME_LOW(qint16 val){tcm->vtimelow->updateNew(val);}
    void update_new_VTIME_HIGH(qint16 val){tcm->vtimehight->updateNew(val);}
    void update_new_SC_LEVEL_A(quint16 val){tcm->sclevelA->updateNew(val);}
    void update_new_SC_LEVEL_C(quint16 val){tcm->sclevelC->updateNew(val);}
    void update_new_C_LEVEL_A(quint16 val){tcm->clevelA->updateNew(val);}
    void update_new_C_LEVEL_C(quint16 val){tcm->clevelC->updateNew(val);}
    void update_new_CH_MASK_A(quint16 val){tcm->chmfskA->updateNew(val);}
    void update_new_CH_MASK_C(quint16 val){tcm->chmfskC->updateNew(val);}

    void update_act_DELAY_A(qint16 val){tcm->delayA->updateAct(val);}
    void update_act_DELAY_C(qint16 val){tcm->delayC->updateAct(val);}
    void update_act_VTIME_LOW(qint16 val){tcm->vtimelow->updateAct(val);}
    void update_act_VTIME_HIGH(qint16 val){tcm->vtimehight->updateAct(val);}
    void update_act_SC_LEVEL_A(quint16 val){tcm->sclevelA->updateAct(val);}
    void update_act_SC_LEVEL_C(quint16 val){tcm->sclevelC->updateAct(val);}
    void update_act_C_LEVEL_A(quint16 val){tcm->clevelA->updateAct(val);}
    void update_act_C_LEVEL_C(quint16 val){tcm->clevelC->updateAct(val);}
    void update_act_CH_MASK_A(quint16 val){tcm->chmfskA->updateAct(val);}
    void update_act_CH_MASK_C(quint16 val){tcm->chmfskC->updateAct(val);}

//    void update_act_BOARD_STATUS(quint16 FEEid, quint16 val){tcm->updateAct(val);}
//    void update_act_TEMPERATURE(quint16 FEEid, quint16 val){tcm->updateAct(val);}
//    void update_act_SERIAL_NUM(quint16 FEEid, quint16 val){tcm->updateAct(val);}
//    void update_act_FW_VERSION(quint16 FEEid, quint32 val){tcm->updateAct(val);}
    void update_act_SIDE_A_STATUS(quint16 val){tcm->sideAstatus->updateAct(val);}
    void update_act_SIDE_C_STATUS(quint16 val){tcm->sideCstatus->updateAct(val);}
    void update_act_CNT_OR_A(quint32 val){tcm->cntorA->updateAct(val);}
    void update_act_CNT_OR_A_RATE(quint32 val){tcm->cntorArate->updateAct(val);}
    void update_act_CNT_OR_C(quint32 val){tcm->cntorC->updateAct(val);}
    void update_act_CNT_OR_C_RATE(quint32 val){tcm->cntorCrate->updateAct(val);}
    void update_act_CNT_SC(quint32 val){tcm->cntsc->updateAct(val);}
    void update_act_CNT_SC_RATE(quint32 val){tcm->cntscrate->updateAct(val);}
    void update_act_CNT_C(quint32 val){tcm->cntc->updateAct(val);}
    void update_act_CNT_C_RATE(quint32 val){tcm->cntcrate->updateAct(val);}
    void update_act_CNT_V(quint32 val){tcm->cntv->updateAct(val);}
    void update_act_CNT_V_RATE(quint32 val){tcm->cntvrate->updateAct(val);}

    void update_new_OR_A_SIGN(quint16 val){tcm->orAsign->updateNew(val);}
    void update_new_OR_A_RATE(quint32 val){tcm->orArate->updateNew(val);}
    void update_new_OR_C_SIGN(quint16 val){tcm->orCsign->updateNew(val);}
    void update_new_OR_C_RATE(quint32 val){tcm->orCrate->updateNew(val);}
    void update_new_SC_SIGN(quint16 val){tcm->scsign->updateNew(val);}
    void update_new_SC_RATE(quint32 val){tcm->scrate->updateNew(val);}
    void update_new_C_SIGN(quint16 val){tcm->csign->updateNew(val);}
    void update_new_C_RATE(quint32 val){tcm->crate->updateNew(val);}
    void update_new_V_SIGN(quint16 val){tcm->vsign->updateNew(val);}
    void update_new_V_RATE(quint32 val){tcm->vrate->updateNew(val);}

    void update_act_OR_A_SIGN(quint16 val){tcm->orAsign->updateAct(val);}
    void update_act_OR_A_RATE(quint32 val){tcm->orArate->updateAct(val);}
    void update_act_OR_C_SIGN(quint16 val){tcm->orAsign->updateAct(val);}
    void update_act_OR_C_RATE(quint32 val){tcm->orCrate->updateAct(val);}
    void update_act_SC_SIGN(quint16 val){tcm->scsign->updateAct(val);}
    void update_act_SC_RATE(quint32 val){tcm->scrate->updateAct(val);}
    void update_act_C_SIGN(quint16 val){tcm->csign->updateAct(val);}
    void update_act_C_RATE(quint32 val){tcm->crate->updateAct(val);}
    void update_act_V_SIGN(quint16 val){tcm->vsign->updateAct(val);}
    void update_act_V_RATE(quint32 val){tcm->vrate->updateAct(val);}

    void update_act_STATUS_OPTIONCODE(QString mode){tcm->statusoptioncode->updateAct(mode);}

    void update_act_OR_A_ENABLED(quint8 isOn){tcm->orAenabled->updateAct(isOn);}
    void update_act_OR_C_ENABLED(quint8 isOn){tcm->orCenabled->updateAct(isOn);}
    void update_act_SC_ENABLED(quint8 isOn){tcm->scenabled->updateAct(isOn);}
    void update_act_C_ENABLED(quint8 isOn){tcm->cenabled->updateAct(isOn);}
    void update_act_V_ENABLED(quint8 isOn){tcm->venabled->updateAct(isOn);}

    void update_act_LASER_ON(quint8 isOn){tcm->laseron->updateAct(isOn);}

    void update_act_LASER_DIV(quint32 val){tcm->laserdiv->updateAct(val);}
    void update_act_LASER_DELAY(qint16 val){tcm->laserdelay->updateAct(val);}
    void update_act_LASER_PATTERN_1(quint32 val){tcm->laserpattern1->updateAct(val);}
    void update_act_LASER_PATTERN_0(quint32 val){tcm->laserpattern0->updateAct(val);}
    void update_act_ATTEN_VALUE(quint16 val){tcm->attenvalue->updateAct(val);}

    void update_new_LASER_DIV(quint32 val){tcm->laserdiv->updateNew(val);}
    void update_new_LASER_DELAY(qint16 val){tcm->laserdelay->updateNew(val);}
    void update_new_LASER_PATTERN_1(quint32 val){tcm->laserpattern1->updateNew(val);}
    void update_new_LASER_PATTERN_0(quint32 val){tcm->laserpattern0->updateNew(val);}
    void update_new_ATTEN_VALUE(quint16 val){tcm->attenvalue->updateNew(val);}

    void update_act_ATTEN_STATUS(quint8 val){tcm->attenstatus->updateAct(val);}
};


//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

template<class T>
void pm_SET<T>::emitSignalRequest(DimCommand* currCmnd){
    pServer->emitSignal(pSETSignal,FEE_id,*static_cast<T*>(currCmnd->getData()));
}

template<class T>
void pm_ValAPP<T>::emitSignalRequest(DimCommand* currCmnd){
    pServer->emitSignal(pAPPSignal,FEE_id,*static_cast<T*>(currCmnd->getData()));
}

//template<class T>
//void twoValAPP<T>::emitSignalRequest(DimCommand* currCmnd)
//{
//    twoVal* data = static_cast<twoVal*>(currCmnd->getData());
//    this->pServer->emitSignal(pAPPSignal,static_cast<T>(data->first),static_cast<T>(data->second));

//}


#endif // MYDIMSERVER_H
