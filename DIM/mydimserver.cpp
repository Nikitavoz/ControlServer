#include "mydimserver.h"

//  ==========================================  MyDimServer  =============================================

MyDimServer::MyDimServer(QString dns_node,QString server_name)  :
    QObject(nullptr)//,
{
    dnsNode = dns_node;
    serverName = server_name;
    setDnsNode(qPrintable(dnsNode));

    fillPMValHash();
    fillPMCHValHash();
    fillPMNonValHash();
    fillPMCHNonValHash();
    fillTCMValHash();
    fillTCMNonValHash();

    emit apply_SET_OPTIONCODE_requested(1,1);
}

MyDimServer::~MyDimServer()
{
    stopServer();
    for(quint8 i=0;i<Npms;i++){
        delete pm[i];
    }
    delete tcm;

    PMValHash<quint8>.clear();      PMValHash<quint16>.clear();     PMValHash<quint32>.clear();
    PMValHash<qint8>.clear();       PMValHash<qint16>.clear();      PMValHash<qint32>.clear();
    PMCHValHash<quint8>.clear();    PMCHValHash<quint16>.clear();   PMCHValHash<quint32>.clear();
    PMCHValHash<qint8>.clear();     PMCHValHash<qint16>.clear();    PMCHValHash<qint32>.clear();
    PMNonValHash.clear();
    PMCHNonValHash.clear();

}

void MyDimServer::startServer()
{
    OpenOutFile();

    setDnsNode(qPrintable(dnsNode));
    start(qPrintable(serverName));
    cout << "Start DIM server on " << dnsNode << Qt::endl;

    tcm = new TCMPars(this);
    tcm->TCM_FEE_id = 0xF000;
    tcm->publish();

    for(quint8 i=0;i<Npms;i++) {
         pm[i] = new PMPars(this);
         pm[i]->PM_FEE_id = FT0_FEE_ID[i+1];
         pm[i]->publish();
     }

}

void MyDimServer::stopServer()
{
    CloseOutFile();
    this->stop();
    cout << "Stop DIM server "  << Qt::endl;
}

void MyDimServer::OpenOutFile()
{
    DimServicesFile.open(QIODevice::WriteOnly);
    DimCommandsFile.open(QIODevice::WriteOnly);
}

void MyDimServer::CloseOutFile()
{
    DimServicesFile.close();
    DimCommandsFile.close();
}

void MyDimServer::setNChannels(quint8 nch)      // Doesn't work yet
{
    for(quint8 i=0;i<Npms;i++){
        pm[i]->Nchannels = nch;
//        cout << pm[i]->Nchannels <<Qt::endl;
    }
}

template<class Y>
void MyDimServer::emitSignal(pmch_pValSignal<Y> pSignal,quint16 _FEE_id,quint8 Chid, Y val)
{
    if(pSignal != nullptr)
        emit (this->*pSignal)(_FEE_id,Chid,val);
}

void MyDimServer::emitSignal(pmch_pNonValSignal pSignal, quint16 FEEid, quint8 Chid)
{
    if(pSignal != nullptr)
        emit (this->*pSignal)(FEEid,Chid);
}

template<class Y>
void MyDimServer::emitSignal(pm_pValSignal<Y> pSignal,quint16 _FEEid, Y val)
{
    if(pSignal != nullptr)
        emit (this->*pSignal)(_FEEid,val);
}

void MyDimServer::emitSignal(pm_pNonValSignal pSignal, quint16 _FEEid)
{
    if(pSignal != nullptr)
        emit (this->*pSignal)(_FEEid);
}

template <class Y>
void MyDimServer::emitSignal(tcm_pValSignal<Y> pSignal, Y val)
{
    if(pSignal != nullptr)
        emit (this->*pSignal)(val);
}

void MyDimServer::emitSignal(tcm_pNonValSignal pSignal)
{
    if(pSignal != nullptr)
        emit (this->*pSignal)();
}

void MyDimServer::emitSignal(pTwoValSignal pSignal,quint8 first_data,quint8 second_data)
{
//    if(this == nullptr) { qDebug() << "Bad thing happened";}
//    else if(pSignal != nullptr)
//        qDebug() << "pServer != nullptr";
//        emit (this->*pSignal)(first_data,second_data);
}

//  ===================================================================================================

//  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PMPars @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

PMPars::PMPars(MyDimServer* _server):
            PM_FEE_id(0xFFFF)                   //  0xffff is initial value instead of zero value
{
//    cout << "PMPARS" << Nchannels << Qt::endl;

    adczero.resize(Nchannels);
    adcdelay.resize(Nchannels);
    adc0offset.resize(Nchannels);
    adc1offset.resize(Nchannels);
    adc0range.resize(Nchannels);
    adc1range.resize(Nchannels);
    timealign.resize(Nchannels);
    cfdthreshold.resize(Nchannels);
    cfdzero.resize(Nchannels);
    thresholdcalibr.resize(Nchannels);

    adc0meanampl.resize(Nchannels);
    adc1meanampl.resize(Nchannels);
    adc0zerolvl.resize(Nchannels);
    adc1zerolvl.resize(Nchannels);
    rawtdcdata.resize(Nchannels);
    cntcfd.resize(Nchannels);
    cntcfdrate.resize(Nchannels);
    cnttrg.resize(Nchannels);
    cnttrgrate.resize(Nchannels);

    for( quint8 i=0; i<Nchannels; i++){
        adczero[i]          = new PMCHfullPar<qint16>("ADC_ZERO",_server);
        adcdelay[i]         = new PMCHfullPar<qint16>("ADC_DELAY",_server);
        adc0offset[i]       = new PMCHfullPar<quint16>("ADC0_OFFSET",_server);
        adc1offset[i]       = new PMCHfullPar<quint16>("ADC1_OFFSET",_server);
        adc0range[i]        = new PMCHfullPar<quint16>("ADC0_RANGE",_server);
        adc1range[i]        = new PMCHfullPar<quint16>("ADC1_RANGE",_server);
        timealign[i]        = new PMCHfullPar<quint16>("TIME_ALIGN",_server);
        cfdthreshold[i]     = new PMCHfullPar<qint16>("CFD_THRESHOLD",_server);
        cfdzero[i]          = new PMCHfullPar<qint16>("CFD_ZERO",_server);
        thresholdcalibr[i]  = new PMCHfullPar<qint16>("THRESHOLD_CALIBR",_server);

        adc0meanampl[i]     = new  PMCHonlyActPar<quint16>("ADC0_MEANAMPL",_server);
        adc1meanampl[i]     = new  PMCHonlyActPar<quint16>("ADC1_MEANAMPL",_server);
        adc0zerolvl[i]      = new  PMCHonlyActPar<quint16>("ADC0_ZEROLVL",_server);
        adc1zerolvl[i]      = new  PMCHonlyActPar<quint16>("ADC1_ZEROLVL",_server);
        rawtdcdata[i]       = new  PMCHonlyActPar<quint16>("RAW_TDC_DATA",_server);
        cntcfd[i]           = new  PMCHonlyActPar<quint32>("CNT_CFD",_server);
        cntcfdrate[i]       = new  PMCHonlyActPar<quint32>("CNT_CFD_RATE",_server);
        cnttrg[i]           = new  PMCHonlyActPar<quint32>("CNT_TRG",_server);
        cnttrgrate[i]       = new  PMCHonlyActPar<quint32>("CNT_TRG_RATE",_server);
    }

    //  PM

    orgate              = new PMfullPar<quint16>("OR_GATE",_server);    orgate->prefix = "/control";
    cfdsatr             = new PMfullPar<quint16>("CFD_SATR",_server);   cfdsatr->prefix = "/control";
    chmask              = new PMfullPar<quint8>("CH_MASK",_server);     chmask->prefix = "/control";
    swchon              = new PMonlyValAppPar<quint8>("SwChOn",_server);     swchon->prefix = "/control";
    swchoff             = new PMonlyValAppPar<quint8>("SwChOff",_server);     swchoff->prefix = "/control";
    resetcounters       = new PMonlyAppPar("RESET_COUNTERS",_server);   resetcounters->prefix = "/control";
    zerolvlcalibr       = new PMonlyAppPar("ZERO_LVL_CALIBR",_server);  zerolvlcalibr->prefix = "/control";
    alltopm             = new PMonlyAppPar("ALLtoPM",_server);          alltopm->prefix = "/control";

    linkstatus          = new PMonlyActPar<quint32>("LINK_STATUS",_server);     linkstatus->prefix = "/status";
    boardstatus         = new PMonlyActPar<quint16>("BOARD_STATUS",_server);    boardstatus->prefix = "/status";
    temperature         = new PMonlyActPar<quint16>("TEMPERATURE",_server);     temperature->prefix = "/status";
    serialnum           = new PMonlyActPar<quint16>("SERIAL_NUM",_server);     serialnum->prefix = "/status";
    fwversion           = new PMonlyActPar<quint32>("FW_VERSION",_server);     fwversion->prefix = "/status";

    //  GBT Readout unit

    resetorbitsync      = new PMonlyAppPar("RESET_ORBIT_SYNC",_server);
    resetdrophitcnts    = new PMonlyAppPar("RESET_DROPPING_HIT_COUNTERS",_server);
    resetgenbunchoffset = new PMonlyAppPar("RESET_GEN_BUNCH_OFFSET",_server);
    resetgbterrors      = new PMonlyAppPar("RESET_GBT_ERRORS",_server);
    resetgbt            = new PMonlyAppPar("RESET_GBT",_server);
    resetrxphaseerror   = new PMonlyAppPar("RESET_RX_PHASE_ERROR",_server);
    sendreadoutcommand  = new PMonlyValAppPar<quint8>("SEND_READOUT_COMMAND",_server);
    tgsendsingle        = new PMonlyValAppPar<quint32>("TG_SEND_SINGLE",_server);


    tgpattern1          = new PMfullPar<quint32>("TG_PATTERN_1",_server);
    tgpattern0          = new PMfullPar<quint32>("TG_PATTERN_0",_server);
    tgcontvalue         = new PMfullPar<quint8>("TG_CONT_VALUE",_server);
    tgbunchfreq         = new PMfullPar<quint16>("TG_BUNCH_FREQ",_server);
    tgfreqoffset        = new PMfullPar<quint16>("TG_FREQ_OFFSET",_server);
    tgmode              = new PMActnValAppPar<quint8>("TG_MODE",_server);
    hbresponse          = new PMActnValAppPar<quint8>("HB_RESPONSE",_server);
    dgmode              = new PMActnValAppPar<quint8>("DG_MODE",_server);
    dgtrgrespondmask    = new PMfullPar<quint32>("DG_TRG_RESPOND_MASK",_server);
    dgbunchpattern      = new PMfullPar<quint32>("DG_BUNCH_PATTERN",_server);
    dgbunchfreq         = new PMfullPar<quint16>("DG_BUNCH_FREQ",_server);
    dgfreqoffset        = new PMfullPar<quint16>("DG_FREQ_OFFSET",_server);
    rdhfeeid            = new PMfullPar<quint16>("RDH_FEE_ID",_server);
    rdhpar              = new PMfullPar<quint16>("RDH_PAR",_server);
    rdhmaxpayload       = new PMfullPar<quint16>("RDH_MAX_PAYLOAD",_server);
    rdhdetfield         = new PMfullPar<quint16>("RDH_DET_FIELD",_server);
    crutrgcomparedelay  = new PMfullPar<quint16>("CRU_TRG_COMPARE_DELAY",_server);
    bciddelay           = new PMfullPar<quint16>("BCID_DELAY",_server);
    dataseltrgmask      = new PMfullPar<quint32>("DATA_SEL_TRG_MASK",_server);

    bits                = new PMonlyActPar<quint16>("BITS",_server);
    readoutmode         = new PMonlyActPar<quint8>("READOUT_MODE",_server);
    bcidsyncmode        = new PMonlyActPar<quint8>("BCID_SYNC_MODE",_server);
    rxphase             = new PMonlyActPar<quint8>("RX_PHASE",_server);
    cruorbit            = new PMonlyActPar<quint32>("CRU_ORBIT",_server);
    crubc               = new PMonlyActPar<quint16>("CRU_BC",_server);
    rawfifo             = new PMonlyActPar<quint16>("RAW_FIFO",_server);
    selfifo             = new PMonlyActPar<quint16>("SEL_FIFO",_server);
    selfirsthit         = new PMonlyActPar<quint32>("SEL_FIRST_HIT_DROPPED_ORBIT",_server);
    sellasthit          = new PMonlyActPar<quint32>("SEL_LAST_HIT_DROPPED_ORBIT",_server);
    selhitsdropped      = new PMonlyActPar<quint32>("SEL_HITS_DROPPED",_server);
    readoutrate         = new PMonlyActPar<quint16>("READOUT_RATE",_server);

    pServer = _server;
}

PMPars::~PMPars()
{
    for( quint8 i=0; i<Nchannels; i++){
        delete adczero[i];
        delete adcdelay[i];
        delete adc0offset[i];
        delete adc1offset[i];
        delete adc0range[i];
        delete adc1range[i];
        delete timealign[i];
        delete cfdthreshold[i];
        delete cfdzero[i];
        delete thresholdcalibr[i];

        delete adc0meanampl[i];
        delete adc1meanampl[i];
        delete adc0zerolvl[i];
        delete adc1zerolvl[i];
        delete rawtdcdata[i];
        delete cntcfd[i];
        delete cntcfdrate[i];
        delete cnttrg[i];
        delete cnttrgrate[i];
    }

    delete orgate;
    delete cfdsatr;
    delete chmask;
    delete swchon;
    delete swchoff;
    delete resetcounters;
    delete zerolvlcalibr;
    delete alltopm;

    delete linkstatus;
    delete boardstatus;
    delete temperature;
    delete serialnum;
    delete fwversion;

    delete resetorbitsync;
    delete resetdrophitcnts;
    delete resetgenbunchoffset;
    delete resetgbterrors;
    delete resetgbt;
    delete resetrxphaseerror;
    delete sendreadoutcommand;
    delete tgsendsingle;

    delete tgpattern1;
    delete tgpattern0;
    delete  tgcontvalue;
    delete tgbunchfreq;
    delete tgfreqoffset;
    delete  tgmode;
    delete  hbresponse;
    delete dgmode;
    delete dgtrgrespondmask;
    delete dgbunchpattern;
    delete dgbunchfreq;
    delete dgfreqoffset;
    delete rdhfeeid;
    delete rdhpar;
    delete rdhmaxpayload;
    delete rdhdetfield;
    delete crutrgcomparedelay;
    delete bciddelay;
    delete dataseltrgmask;

    delete bits;
    delete readoutmode;
    delete bcidsyncmode;
    delete rxphase;
    delete cruorbit;
    delete crubc;
    delete rawfifo;
    delete selfifo;
    delete selfirsthit;
    delete sellasthit;
    delete selhitsdropped;
    delete readoutrate;

}

void PMPars::publish()
{

    orgate->SetFEEid(PM_FEE_id);                   orgate->publishCommands();                    orgate->publishServices();
    cfdsatr->SetFEEid(PM_FEE_id);                  cfdsatr->publishCommands();                   cfdsatr->publishServices();
    chmask->SetFEEid(PM_FEE_id);                   chmask->publishCommands();                    chmask->publishServices();
    swchon->SetFEEid(PM_FEE_id);                   swchon->publishCommands();
    swchoff->SetFEEid(PM_FEE_id);                   swchoff->publishCommands();
    resetcounters->SetFEEid(PM_FEE_id);            resetcounters->publishCommands();
    zerolvlcalibr->SetFEEid(PM_FEE_id);            zerolvlcalibr->publishCommands();
    alltopm->SetFEEid(PM_FEE_id);                  alltopm->publishCommands();

    linkstatus->SetFEEid(PM_FEE_id);                                                           linkstatus->publishServices();
    boardstatus->SetFEEid(PM_FEE_id);                                                          boardstatus->publishServices();
    temperature->SetFEEid(PM_FEE_id);                                                          temperature->publishServices();
    serialnum->SetFEEid(PM_FEE_id);                                                            serialnum->publishServices();
    fwversion->SetFEEid(PM_FEE_id);                                                            fwversion->publishServices();

    //  loop for channels
        for(quint8 i=0; i<Nchannels; i++) {

            adczero[i]->SetCHid(i+1);        adczero[i]->SetFEEid(PM_FEE_id);       adczero[i]->publishCommands();        adczero[i]->publishServices();
            adcdelay[i]->SetCHid(i+1);       adcdelay[i]->SetFEEid(PM_FEE_id);      adcdelay[i]->publishCommands();       adcdelay[i]->publishServices();
            adc0offset[i]->SetCHid(i+1);     adc0offset[i]->SetFEEid(PM_FEE_id);    adc0offset[i]->publishCommands();     adc0offset[i]->publishServices();
            adc1offset[i]->SetCHid(i+1);     adc1offset[i]->SetFEEid(PM_FEE_id);    adc1offset[i]->publishCommands();     adc1offset[i]->publishServices();
            adc0range[i]->SetCHid(i+1);      adc0range[i]->SetFEEid(PM_FEE_id);     adc0range[i]->publishCommands();      adc0range[i]->publishServices();
            adc1range[i]->SetCHid(i+1);      adc1range[i]->SetFEEid(PM_FEE_id);     adc1range[i]->publishCommands();      adc1range[i]->publishServices();
            timealign[i]->SetCHid(i+1);       timealign[i]->SetFEEid(PM_FEE_id);      timealign[i]->publishCommands();       timealign[i]->publishServices();
            cfdthreshold[i]->SetCHid(i+1);    cfdthreshold[i]->SetFEEid(PM_FEE_id);   cfdthreshold[i]->publishCommands();    cfdthreshold[i]->publishServices();
            cfdzero[i]->SetCHid(i+1);        cfdzero[i]->SetFEEid(PM_FEE_id);       cfdzero[i]->publishCommands();        cfdzero[i]->publishServices();
            thresholdcalibr[i]->SetCHid(i+1); thresholdcalibr[i]->SetFEEid(PM_FEE_id);thresholdcalibr[i]->publishCommands(); thresholdcalibr[i]->publishServices();

            if(!pServer->excludeForWinCC) {adc0meanampl[i]->SetCHid(i+1);   adc0meanampl[i]->SetFEEid(PM_FEE_id);                                      adc0meanampl[i]->publishServices();}
            if(!pServer->excludeForWinCC) {adc1meanampl[i]->SetCHid(i+1);   adc1meanampl[i]->SetFEEid(PM_FEE_id);                                      adc1meanampl[i]->publishServices();}
            adc0zerolvl[i]->SetCHid(i+1);    adc0zerolvl[i]->SetFEEid(PM_FEE_id);                                       adc0zerolvl[i]->publishServices();
            adc1zerolvl[i]->SetCHid(i+1);    adc1zerolvl[i]->SetFEEid(PM_FEE_id);                                       adc1zerolvl[i]->publishServices();
            if(!pServer->excludeForWinCC) {rawtdcdata[i]->SetCHid(i+1);     rawtdcdata[i]->SetFEEid(PM_FEE_id);                                        rawtdcdata[i]->publishServices();}
            cntcfd[i]->SetCHid(i+1);         cntcfd[i]->SetFEEid(PM_FEE_id);                                            cntcfd[i]->publishServices();
            cntcfdrate[i]->SetCHid(i+1);     cntcfdrate[i]->SetFEEid(PM_FEE_id);                                        cntcfdrate[i]->publishServices();
            cnttrg[i]->SetCHid(i+1);         cnttrg[i]->SetFEEid(PM_FEE_id);                                            cnttrg[i]->publishServices();
            cnttrgrate[i]->SetCHid(i+1);     cnttrgrate[i]->SetFEEid(PM_FEE_id);                                        cnttrgrate[i]->publishServices();

        }


        resetorbitsync->SetFEEid(PM_FEE_id);           resetorbitsync->publishCommands();
        resetdrophitcnts->SetFEEid(PM_FEE_id);         resetdrophitcnts->publishCommands();
        resetgenbunchoffset->SetFEEid(PM_FEE_id);      resetgenbunchoffset->publishCommands();
        resetgbterrors->SetFEEid(PM_FEE_id);           resetgbterrors->publishCommands();
        resetgbt->SetFEEid(PM_FEE_id);                 resetgbt->publishCommands();
        resetrxphaseerror->SetFEEid(PM_FEE_id);        resetrxphaseerror->publishCommands();
        if(!pServer->excludeForWinCC) {sendreadoutcommand->SetFEEid(PM_FEE_id);       sendreadoutcommand->publishCommands();}
        if(!pServer->excludeForWinCC) {tgsendsingle->SetFEEid(PM_FEE_id);             tgsendsingle->publishCommands();}

        if(!pServer->excludeForWinCC) {tgpattern1->SetFEEid(PM_FEE_id);               tgpattern1->publishCommands();                tgpattern1->publishServices();}
        if(!pServer->excludeForWinCC) {tgpattern0->SetFEEid(PM_FEE_id);               tgpattern0->publishCommands();                tgpattern0->publishServices();}
        if(!pServer->excludeForWinCC) {tgcontvalue->SetFEEid(PM_FEE_id);              tgcontvalue->publishCommands();               tgcontvalue->publishServices();}
        if(!pServer->excludeForWinCC) {tgbunchfreq->SetFEEid(PM_FEE_id);              tgbunchfreq->publishCommands();               tgbunchfreq->publishServices();}
        if(!pServer->excludeForWinCC) {tgfreqoffset->SetFEEid(PM_FEE_id);             tgfreqoffset->publishCommands();              tgfreqoffset->publishServices();}
        if(!pServer->excludeForWinCC) {tgmode->SetFEEid(PM_FEE_id);                   tgmode->publishCommands();                    tgmode->publishServices();}
        hbresponse->SetFEEid(PM_FEE_id);               hbresponse->publishCommands();                hbresponse->publishServices();
        if(!pServer->excludeForWinCC) {dgmode->SetFEEid(PM_FEE_id);                   dgmode->publishCommands();                    dgmode->publishServices();}
        if(!pServer->excludeForWinCC) {dgtrgrespondmask->SetFEEid(PM_FEE_id);         dgtrgrespondmask->publishCommands();          dgtrgrespondmask->publishServices();}
        if(!pServer->excludeForWinCC) {dgbunchpattern->SetFEEid(PM_FEE_id);           dgbunchpattern->publishCommands();            dgbunchpattern->publishServices();}
        if(!pServer->excludeForWinCC) {dgbunchfreq->SetFEEid(PM_FEE_id);              dgbunchfreq->publishCommands();               dgbunchfreq->publishServices();}
        if(!pServer->excludeForWinCC) {dgfreqoffset->SetFEEid(PM_FEE_id);             dgfreqoffset->publishCommands();              dgfreqoffset->publishServices();}
        rdhfeeid->SetFEEid(PM_FEE_id);                 rdhfeeid->publishCommands();                  rdhfeeid->publishServices();
        rdhpar->SetFEEid(PM_FEE_id);                   rdhpar->publishCommands();                    rdhpar->publishServices();
        rdhmaxpayload->SetFEEid(PM_FEE_id);            rdhmaxpayload->publishCommands();             rdhmaxpayload->publishServices();
        rdhdetfield->SetFEEid(PM_FEE_id);              rdhdetfield->publishCommands();               rdhdetfield->publishServices();
        if(!pServer->excludeForWinCC) {crutrgcomparedelay->SetFEEid(PM_FEE_id);       crutrgcomparedelay->publishCommands();        crutrgcomparedelay->publishServices();}
        bciddelay->SetFEEid(PM_FEE_id);                bciddelay->publishCommands();                 bciddelay->publishServices();
        dataseltrgmask->SetFEEid(PM_FEE_id);           dataseltrgmask->publishCommands();            dataseltrgmask->publishServices();


        bits->SetFEEid(PM_FEE_id);                                                                 bits->publishServices();
        readoutmode->SetFEEid(PM_FEE_id);                                                          readoutmode->publishServices();
        bcidsyncmode->SetFEEid(PM_FEE_id);                                                         bcidsyncmode->publishServices();
        rxphase->SetFEEid(PM_FEE_id);                                                              rxphase->publishServices();
        if(!pServer->excludeForWinCC) {cruorbit->SetFEEid(PM_FEE_id);                                                             cruorbit->publishServices();}
        if(!pServer->excludeForWinCC) {crubc->SetFEEid(PM_FEE_id);                                                                crubc->publishServices();}
        rawfifo->SetFEEid(PM_FEE_id);                                                              rawfifo->publishServices();
        selfifo->SetFEEid(PM_FEE_id);                                                              selfifo->publishServices();
        selfirsthit->SetFEEid(PM_FEE_id);                                                          selfirsthit->publishServices();
        sellasthit->SetFEEid(PM_FEE_id);                                                           sellasthit->publishServices();
        selhitsdropped->SetFEEid(PM_FEE_id);                                                       selhitsdropped->publishServices();
        readoutrate->SetFEEid(PM_FEE_id);                                                          readoutrate->publishServices();
}

//  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

TCMPars::TCMPars(MyDimServer* _server):
            TCM_FEE_id(0xFFFF)                   //  0xffff is initial value instead of zero value
{
    countersupdrate = new TCMActnValAppPar<quint8>("COUNTERS_UPD_RATE",_server);    countersupdrate->prefix = "/TCM/control";
    extendedreadout = new TCMActnValAppPar<quint8>("EXTENDED_READOUT",_server);     extendedreadout->prefix = "/TCM/control";
    scsumsides = new TCMActnValAppPar<quint8>("SC_SUM_SIDES",_server);              scsumsides->prefix = "/TCM/control";
    csumsides = new TCMActnValAppPar<quint8>("C_SUM_SIDES",_server);                csumsides->prefix = "/TCM/control";
    addcdelay = new TCMActnValAppPar<quint8>("ADD_C_DELAY",_server);                addcdelay->prefix = "/TCM/control";
    ressw1 = new TCMActnValAppPar<quint8>("RES_SW_1",_server);                      ressw1->prefix = "/TCM/control";
    ressw2 = new TCMActnValAppPar<quint8>("RES_SW_2",_server);                      ressw2->prefix = "/TCM/control";
    ressw3 = new TCMActnValAppPar<quint8>("RES_SW_3",_server);                      ressw3->prefix = "/TCM/control";
    ressw4 = new TCMActnValAppPar<quint8>("RES_SW_4",_server);                      ressw4->prefix = "/TCM/control";

    delayA = new TCMfullPar<qint16>("DELAY_A",_server);                             delayA->prefix = "/TCM/control";
    delayC = new TCMfullPar<qint16>("DELAY_C",_server);                             delayC->prefix = "/TCM/control";
    vtimelow = new TCMfullPar<qint16>("VTIME_LOW",_server);                         vtimelow->prefix = "/TCM/control";
    vtimehight = new TCMfullPar<qint16>("VTIME_HIGH",_server);                      vtimehight->prefix = "/TCM/control";
    sclevelA = new TCMfullPar<quint16>("SC_LEVEL_A",_server);                       sclevelA->prefix = "/TCM/control";
    sclevelC = new TCMfullPar<quint16>("SC_LEVEL_C",_server);                       sclevelC->prefix = "/TCM/control";
    clevelA = new TCMfullPar<quint16>("C_LEVEL_A",_server);                         clevelA->prefix = "/TCM/control";
    clevelC = new TCMfullPar<quint16>("C_LEVEL_C",_server);                         clevelC->prefix = "/TCM/control";
    chmfskA = new TCMfullPar<quint16>("CH_MASK_A",_server);                         chmfskA->prefix = "/TCM/control";
    chmfskC = new TCMfullPar<quint16>("CH_MASK_C",_server);                         chmfskC->prefix = "/TCM/control";

    swchon              = new PMonlyValAppPar<quint8>("SwChOn",_server);     swchon->prefix = "/TCM/control";
    swchoff             = new PMonlyValAppPar<quint8>("SwChOff",_server);     swchoff->prefix = "/TCM/control";
    resetcounters       = new PMonlyAppPar("RESET_COUNTERS",_server);   resetcounters->prefix = "/TCM/control";
    resetsystem         = new PMonlyAppPar("RESET_SYSTEM",_server);   resetsystem->prefix = "/TCM/control";

    boardstatus         = new PMonlyActPar<quint16>("BOARD_STATUS",_server);    boardstatus->prefix = "/TCM/status";
    temperature         = new PMonlyActPar<quint16>("TEMPERATURE",_server);     temperature->prefix = "/TCM/status";
    serialnum           = new PMonlyActPar<quint16>("SERIAL_NUM",_server);     serialnum->prefix = "/TCM/status";
    fwversion           = new PMonlyActPar<quint32>("FW_VERSION",_server);     fwversion->prefix = "/TCM/status";

    sideAstatus = new PMonlyActPar<quint16>("SIDE_A_STATUS",_server);       sideAstatus->prefix = "/TCM/status";
    sideCstatus = new PMonlyActPar<quint16>("SIDE_C_STATUS",_server);       sideCstatus->prefix = "/TCM/status";
    cntorA = new PMonlyActPar<quint32>("CNT_OR_A",_server);                 cntorA->prefix = "/TCM/status";
    cntorArate = new PMonlyActPar<quint32>("CNT_OR_A_RATE",_server);        cntorArate->prefix = "/TCM/status";
    cntorC = new PMonlyActPar<quint32>("CNT_OR_C",_server);                 cntorC->prefix = "/TCM/status";
    cntorCrate = new PMonlyActPar<quint32>("CNT_OR_C_RATE",_server);        cntorCrate->prefix = "/TCM/status";
    cntsc = new PMonlyActPar<quint32>("CNT_SC",_server);                    cntsc->prefix = "/TCM/status";
    cntscrate = new PMonlyActPar<quint32>("CNT_SC_RATE",_server);           cntscrate->prefix = "/TCM/status";
    cntc = new PMonlyActPar<quint32>("CNT_C",_server);                      cntc->prefix = "/TCM/status";
    cntcrate = new PMonlyActPar<quint32>("CNT_C_RATE",_server);             cntcrate->prefix = "/TCM/status";
    cntv = new PMonlyActPar<quint32>("CNT_V",_server);                      cntv->prefix = "/TCM/status";
    cntvrate = new PMonlyActPar<quint32>("CNT_V_RATE",_server);             cntvrate->prefix = "/TCM/status";

    //  GBT Readout unit

    resetorbitsync      = new PMonlyAppPar("RESET_ORBIT_SYNC",_server);                 resetorbitsync->prefix = "/TCM/GBT/control";
    resetdrophitcnts    = new PMonlyAppPar("RESET_DROPPING_HIT_COUNTERS",_server);      resetdrophitcnts->prefix = "/TCM/GBT/control";
    resetgenbunchoffset = new PMonlyAppPar("RESET_GEN_BUNCH_OFFSET",_server);           resetgenbunchoffset->prefix = "/TCM/GBT/control";
    resetgbterrors      = new PMonlyAppPar("RESET_GBT_ERRORS",_server);                 resetgbterrors->prefix = "/TCM/GBT/control";
    resetgbt            = new PMonlyAppPar("RESET_GBT",_server);                        resetgbt->prefix = "/TCM/GBT/control";
    resetrxphaseerror   = new PMonlyAppPar("RESET_RX_PHASE_ERROR",_server);             resetrxphaseerror->prefix = "/TCM/GBT/control";
    sendreadoutcommand  = new PMonlyValAppPar<quint8>("SEND_READOUT_COMMAND",_server);  sendreadoutcommand->prefix = "/TCM/GBT/control";
    tgsendsingle        = new PMonlyValAppPar<quint32>("TG_SEND_SINGLE",_server);       tgsendsingle->prefix = "/TCM/GBT/control";


    tgpattern1          = new PMfullPar<quint32>("TG_PATTERN_1",_server);               tgpattern1->prefix = "/TCM/GBT/control";
    tgpattern0          = new PMfullPar<quint32>("TG_PATTERN_0",_server);               tgpattern0->prefix = "/TCM/GBT/control";
    tgcontvalue         = new PMfullPar<quint8>("TG_CONT_VALUE",_server);               tgcontvalue->prefix = "/TCM/GBT/control";
    tgbunchfreq         = new PMfullPar<quint16>("TG_BUNCH_FREQ",_server);              tgbunchfreq->prefix = "/TCM/GBT/control";
    tgfreqoffset        = new PMfullPar<quint16>("TG_FREQ_OFFSET",_server);             tgfreqoffset->prefix = "/TCM/GBT/control";
    tgmode              = new PMActnValAppPar<quint8>("TG_MODE",_server);               tgmode->prefix = "/TCM/GBT/control";
    hbresponse          = new PMActnValAppPar<quint8>("HB_RESPONSE",_server);           hbresponse->prefix = "/TCM/GBT/control";
    dgmode              = new PMActnValAppPar<quint8>("DG_MODE",_server);               dgmode->prefix = "/TCM/GBT/control";
    dgtrgrespondmask    = new PMfullPar<quint32>("DG_TRG_RESPOND_MASK",_server);        dgtrgrespondmask->prefix = "/TCM/GBT/control";
    dgbunchpattern      = new PMfullPar<quint32>("DG_BUNCH_PATTERN",_server);           dgbunchpattern->prefix = "/TCM/GBT/control";
    dgbunchfreq         = new PMfullPar<quint16>("DG_BUNCH_FREQ",_server);              dgbunchfreq->prefix = "/TCM/GBT/control";
    dgfreqoffset        = new PMfullPar<quint16>("DG_FREQ_OFFSET",_server);             dgfreqoffset->prefix = "/TCM/GBT/control";
    rdhfeeid            = new PMfullPar<quint16>("RDH_FEE_ID",_server);                 rdhfeeid->prefix = "/TCM/GBT/control";
    rdhpar              = new PMfullPar<quint16>("RDH_PAR",_server);                    rdhpar->prefix = "/TCM/GBT/control";
    rdhmaxpayload       = new PMfullPar<quint16>("RDH_MAX_PAYLOAD",_server);            rdhmaxpayload->prefix = "/TCM/GBT/control";
    rdhdetfield         = new PMfullPar<quint16>("RDH_DET_FIELD",_server);              rdhdetfield->prefix = "/TCM/GBT/control";
    crutrgcomparedelay  = new PMfullPar<quint16>("CRU_TRG_COMPARE_DELAY",_server);      crutrgcomparedelay->prefix = "/TCM/GBT/control";
    bciddelay           = new PMfullPar<quint16>("BCID_DELAY",_server);                 bciddelay->prefix = "/TCM/GBT/control";
    dataseltrgmask      = new PMfullPar<quint32>("DATA_SEL_TRG_MASK",_server);          dataseltrgmask->prefix = "/TCM/GBT/control";

    bits                = new PMonlyActPar<quint16>("BITS",_server);                    bits->prefix = "/TCM/GBT/status";
    readoutmode         = new PMonlyActPar<quint8>("READOUT_MODE",_server);             readoutmode->prefix = "/TCM/GBT/status";
    bcidsyncmode        = new PMonlyActPar<quint8>("BCID_SYNC_MODE",_server);           bcidsyncmode->prefix = "/TCM/GBT/status";
    rxphase             = new PMonlyActPar<quint8>("RX_PHASE",_server);                 rxphase->prefix = "/TCM/GBT/status";
    cruorbit            = new PMonlyActPar<quint32>("CRU_ORBIT",_server);               cruorbit->prefix = "/TCM/GBT/status";
    crubc               = new PMonlyActPar<quint16>("CRU_BC",_server);                  crubc->prefix = "/TCM/GBT/status";
    rawfifo             = new PMonlyActPar<quint16>("RAW_FIFO",_server);                rawfifo->prefix = "/TCM/GBT/status";
    selfifo             = new PMonlyActPar<quint16>("SEL_FIFO",_server);                selfifo->prefix = "/TCM/GBT/status";
    selfirsthit         = new PMonlyActPar<quint32>("SEL_FIRST_HIT_DROPPED_ORBIT",_server); selfirsthit->prefix = "/TCM/GBT/status";
    sellasthit          = new PMonlyActPar<quint32>("SEL_LAST_HIT_DROPPED_ORBIT",_server);  sellasthit->prefix = "/TCM/GBT/status";
    selhitsdropped      = new PMonlyActPar<quint32>("SEL_HITS_DROPPED",_server);        selhitsdropped->prefix = "/TCM/GBT/status";
    readoutrate         = new PMonlyActPar<quint16>("READOUT_RATE",_server);            readoutrate->prefix = "/TCM/GBT/status";


    orAsign = new TCMfullPar<quint16>("OR_A_SIGN",_server);     orAsign->prefix = "/TCM/TRG";
    orArate = new TCMfullPar<quint32>("OR_A_RATE",_server);     orArate->prefix = "/TCM/TRG";
    orCsign = new TCMfullPar<quint16>("OR_C_SIGN",_server);     orCsign->prefix = "/TCM/TRG";
    orCrate = new TCMfullPar<quint32>("OR_C_RATE",_server);     orCrate->prefix = "/TCM/TRG";
    scsign = new TCMfullPar<quint16>("SC_SIGN",_server);        scsign->prefix = "/TCM/TRG";
    scrate = new TCMfullPar<quint32>("SC_RATE",_server);        scrate->prefix = "/TCM/TRG";
    csign = new TCMfullPar<quint16>("C_SIGN",_server);          csign->prefix = "/TCM/TRG";
    crate = new TCMfullPar<quint32>("C_RATE",_server);          crate->prefix = "/TCM/TRG";
    vsign = new TCMfullPar<quint16>("V_SIGN",_server);          vsign->prefix = "/TCM/TRG";
    vrate = new TCMfullPar<quint32>("V_RATE",_server);          vrate->prefix = "/TCM/TRG";

    statusoptioncode = new str_ACT("STATUS_OPTIONCODE");        statusoptioncode->prefix = "";
    setoptioncode = new twoValAPP("SET_OPTIONCODE");    setoptioncode->prefix = "";     setoptioncode->SetSignal(&MyDimServer::apply_SET_OPTIONCODE_requested);

    orAenabled = new TCMActnValAppPar<quint8>("OR_A_ENABLED",_server);      orAenabled->prefix = "/TCM/TRG";
    orCenabled = new TCMActnValAppPar<quint8>("OR_C_ENABLED",_server);      orCenabled->prefix = "/TCM/TRG";
    scenabled = new TCMActnValAppPar<quint8>("SC_ENABLED",_server);         scenabled->prefix = "/TCM/TRG";
    cenabled = new TCMActnValAppPar<quint8>("C_ENABLED",_server);           cenabled->prefix = "/TCM/TRG";
    venabled = new TCMActnValAppPar<quint8>("V_ENABLED",_server);           venabled->prefix = "/TCM/TRG";

    laseron = new TCMActnValAppPar<quint8>("LASER_ON",_server);             laseron->prefix = "";
    laserdiv = new TCMfullPar<quint32>("LASER_DIV",_server);                laserdiv->prefix = "";
    laserdelay = new TCMfullPar<qint16>("LASER_DELAY",_server);            laserdelay->prefix = "";
    laserpattern1 = new TCMfullPar<quint32>("LASER_PATTERN_1",_server);     laserpattern1->prefix = "";
    laserpattern0 = new TCMfullPar<quint32>("LASER_PATTERN_0",_server);     laserpattern0->prefix = "";
    attenvalue = new TCMfullPar<quint16>("ATTEN_VALUE",_server);            attenvalue->prefix = "";
    attenstatus = new PMonlyActPar<quint8>("ATTEN_STATUS",_server);         attenstatus->prefix = "";

    pServer = _server;
}

TCMPars::~TCMPars()
{
    delete countersupdrate;
    delete extendedreadout;
    delete scsumsides;
    delete csumsides;
    delete addcdelay;
    delete ressw1;
    delete ressw2;
    delete ressw3;
    delete ressw4;

    delete delayA;
    delete delayC;
    delete vtimelow;
    delete vtimehight;
    delete sclevelA;
    delete sclevelC;
    delete clevelA;
    delete clevelC;
    delete chmfskA;
    delete chmfskC;

    delete swchon;
    delete swchoff;
    delete resetcounters;
    delete resetsystem;

    delete boardstatus;
    delete temperature;
    delete serialnum;
    delete fwversion;
    delete sideAstatus;
    delete sideCstatus;
    delete cntorA;
    delete cntorArate;
    delete cntorC;
    delete cntorCrate;
    delete cntsc;
    delete cntscrate;
    delete cntc;
    delete cntcrate;
    delete cntv;
    delete cntvrate;

    delete resetorbitsync;
    delete resetdrophitcnts;
    delete resetgenbunchoffset;
    delete resetgbterrors;
    delete resetgbt;
    delete resetrxphaseerror;
    delete sendreadoutcommand;
    delete tgsendsingle;

    delete tgpattern1;
    delete tgpattern0;
    delete  tgcontvalue;
    delete tgbunchfreq;
    delete tgfreqoffset;
    delete  tgmode;
    delete  hbresponse;
    delete dgmode;
    delete dgtrgrespondmask;
    delete dgbunchpattern;
    delete dgbunchfreq;
    delete dgfreqoffset;
    delete rdhfeeid;
    delete rdhpar;
    delete rdhmaxpayload;
    delete rdhdetfield;
    delete crutrgcomparedelay;
    delete bciddelay;
    delete dataseltrgmask;

    delete bits;
    delete readoutmode;
    delete bcidsyncmode;
    delete rxphase;
    delete cruorbit;
    delete crubc;
    delete rawfifo;
    delete selfifo;
    delete selfirsthit;
    delete sellasthit;
    delete selhitsdropped;
    delete readoutrate;


    delete orAsign;
    delete orArate;
    delete orCsign;
    delete orCrate;
    delete scsign;
    delete scrate;
    delete csign;
    delete crate;
    delete vsign;
    delete vrate;

    delete statusoptioncode;
    delete setoptioncode;

    delete orAenabled;
    delete orCenabled;
    delete scenabled;
    delete cenabled;
    delete venabled;

    delete laseron;
    delete laserdiv;
    delete laserdelay;
    delete laserpattern1;
    delete laserpattern0;
    delete attenvalue;
    delete attenstatus;

}


void TCMPars::publish()
{
    countersupdrate->SetFEEid(TCM_FEE_id);      countersupdrate->publishCommands();     countersupdrate->publishServices();
    extendedreadout->SetFEEid(TCM_FEE_id);               extendedreadout->publishCommands();              extendedreadout->publishServices();
    scsumsides->SetFEEid(TCM_FEE_id);               scsumsides->publishCommands();              scsumsides->publishServices();
    csumsides->SetFEEid(TCM_FEE_id);               csumsides->publishCommands();              csumsides->publishServices();
    addcdelay->SetFEEid(TCM_FEE_id);               addcdelay->publishCommands();              addcdelay->publishServices();
    ressw1->SetFEEid(TCM_FEE_id);               ressw1->publishCommands();              ressw1->publishServices();
    ressw2->SetFEEid(TCM_FEE_id);               ressw2->publishCommands();              ressw2->publishServices();
    ressw3->SetFEEid(TCM_FEE_id);               ressw3->publishCommands();              ressw3->publishServices();
    ressw4->SetFEEid(TCM_FEE_id);               ressw4->publishCommands();              ressw4->publishServices();

    delayA->SetFEEid(TCM_FEE_id);               delayA->publishCommands();              delayA->publishServices();
    delayC->SetFEEid(TCM_FEE_id);               delayC->publishCommands();              delayC->publishServices();
    vtimelow->SetFEEid(TCM_FEE_id);               vtimelow->publishCommands();              vtimelow->publishServices();
    vtimehight->SetFEEid(TCM_FEE_id);               vtimehight->publishCommands();              vtimehight->publishServices();
    sclevelA->SetFEEid(TCM_FEE_id);               sclevelA->publishCommands();              sclevelA->publishServices();
    sclevelC->SetFEEid(TCM_FEE_id);               sclevelC->publishCommands();              sclevelC->publishServices();
    clevelA->SetFEEid(TCM_FEE_id);               clevelA->publishCommands();              clevelA->publishServices();
    clevelC->SetFEEid(TCM_FEE_id);               clevelC->publishCommands();              clevelC->publishServices();
    chmfskA->SetFEEid(TCM_FEE_id);               chmfskA->publishCommands();              chmfskA->publishServices();
    chmfskC->SetFEEid(TCM_FEE_id);               chmfskC->publishCommands();              chmfskC->publishServices();

    swchon->SetFEEid(TCM_FEE_id);                   swchon->publishCommands();
    swchoff->SetFEEid(TCM_FEE_id);                   swchoff->publishCommands();
    resetcounters->SetFEEid(TCM_FEE_id);            resetcounters->publishCommands();
    resetsystem->SetFEEid(TCM_FEE_id);            resetsystem->publishCommands();

    boardstatus->SetFEEid(TCM_FEE_id);                                                          boardstatus->publishServices();
    temperature->SetFEEid(TCM_FEE_id);                                                          temperature->publishServices();
    serialnum->SetFEEid(TCM_FEE_id);                                                            serialnum->publishServices();
    fwversion->SetFEEid(TCM_FEE_id);                                                            fwversion->publishServices();

    sideAstatus->SetFEEid(TCM_FEE_id);                                                            sideAstatus->publishServices();
    sideCstatus->SetFEEid(TCM_FEE_id);                                                            sideCstatus->publishServices();
    cntorA->SetFEEid(TCM_FEE_id);                                                            cntorA->publishServices();
    cntorArate->SetFEEid(TCM_FEE_id);                                                            cntorArate->publishServices();
    cntorC->SetFEEid(TCM_FEE_id);                                                            cntorC->publishServices();
    cntorCrate->SetFEEid(TCM_FEE_id);                                                            cntorCrate->publishServices();
    cntsc->SetFEEid(TCM_FEE_id);                                                            cntsc->publishServices();
    cntscrate->SetFEEid(TCM_FEE_id);                                                            cntscrate->publishServices();
    cntc->SetFEEid(TCM_FEE_id);                                                            cntc->publishServices();
    cntcrate->SetFEEid(TCM_FEE_id);                                                            cntcrate->publishServices();
    cntv->SetFEEid(TCM_FEE_id);                                                            cntv->publishServices();
    cntvrate->SetFEEid(TCM_FEE_id);                                                            cntvrate->publishServices();

    resetorbitsync->SetFEEid(TCM_FEE_id);           resetorbitsync->publishCommands();
    resetdrophitcnts->SetFEEid(TCM_FEE_id);         resetdrophitcnts->publishCommands();
    resetgenbunchoffset->SetFEEid(TCM_FEE_id);      resetgenbunchoffset->publishCommands();
    resetgbterrors->SetFEEid(TCM_FEE_id);           resetgbterrors->publishCommands();
    resetgbt->SetFEEid(TCM_FEE_id);                 resetgbt->publishCommands();
    resetrxphaseerror->SetFEEid(TCM_FEE_id);        resetrxphaseerror->publishCommands();
    if(!pServer->excludeForWinCC) {sendreadoutcommand->SetFEEid(TCM_FEE_id);       sendreadoutcommand->publishCommands();}
    if(!pServer->excludeForWinCC) {tgsendsingle->SetFEEid(TCM_FEE_id);             tgsendsingle->publishCommands();}

    if(!pServer->excludeForWinCC) {tgpattern1->SetFEEid(TCM_FEE_id);               tgpattern1->publishCommands();                tgpattern1->publishServices();}
    if(!pServer->excludeForWinCC) {tgpattern0->SetFEEid(TCM_FEE_id);               tgpattern0->publishCommands();                tgpattern0->publishServices();}
    if(!pServer->excludeForWinCC) {tgcontvalue->SetFEEid(TCM_FEE_id);              tgcontvalue->publishCommands();               tgcontvalue->publishServices();}
    if(!pServer->excludeForWinCC) {tgbunchfreq->SetFEEid(TCM_FEE_id);              tgbunchfreq->publishCommands();               tgbunchfreq->publishServices();}
    if(!pServer->excludeForWinCC) {tgfreqoffset->SetFEEid(TCM_FEE_id);             tgfreqoffset->publishCommands();              tgfreqoffset->publishServices();}
    if(!pServer->excludeForWinCC) {tgmode->SetFEEid(TCM_FEE_id);                   tgmode->publishCommands();                    tgmode->publishServices();}
    hbresponse->SetFEEid(TCM_FEE_id);               hbresponse->publishCommands();                hbresponse->publishServices();
    if(!pServer->excludeForWinCC) {dgmode->SetFEEid(TCM_FEE_id);                   dgmode->publishCommands();                    dgmode->publishServices();}
    if(!pServer->excludeForWinCC) {dgtrgrespondmask->SetFEEid(TCM_FEE_id);         dgtrgrespondmask->publishCommands();          dgtrgrespondmask->publishServices();}
    if(!pServer->excludeForWinCC) {dgbunchpattern->SetFEEid(TCM_FEE_id);           dgbunchpattern->publishCommands();            dgbunchpattern->publishServices();}
    if(!pServer->excludeForWinCC) {dgbunchfreq->SetFEEid(TCM_FEE_id);              dgbunchfreq->publishCommands();               dgbunchfreq->publishServices();}
    if(!pServer->excludeForWinCC) {dgfreqoffset->SetFEEid(TCM_FEE_id);             dgfreqoffset->publishCommands();              dgfreqoffset->publishServices();}
    rdhfeeid->SetFEEid(TCM_FEE_id);                 rdhfeeid->publishCommands();                  rdhfeeid->publishServices();
    rdhpar->SetFEEid(TCM_FEE_id);                   rdhpar->publishCommands();                    rdhpar->publishServices();
    rdhmaxpayload->SetFEEid(TCM_FEE_id);            rdhmaxpayload->publishCommands();             rdhmaxpayload->publishServices();
    rdhdetfield->SetFEEid(TCM_FEE_id);              rdhdetfield->publishCommands();               rdhdetfield->publishServices();
    if(!pServer->excludeForWinCC) {crutrgcomparedelay->SetFEEid(TCM_FEE_id);       crutrgcomparedelay->publishCommands();        crutrgcomparedelay->publishServices();}
    bciddelay->SetFEEid(TCM_FEE_id);                bciddelay->publishCommands();                 bciddelay->publishServices();
    dataseltrgmask->SetFEEid(TCM_FEE_id);           dataseltrgmask->publishCommands();            dataseltrgmask->publishServices();


    bits->SetFEEid(TCM_FEE_id);                                                                 bits->publishServices();
    readoutmode->SetFEEid(TCM_FEE_id);                                                          readoutmode->publishServices();
    bcidsyncmode->SetFEEid(TCM_FEE_id);                                                         bcidsyncmode->publishServices();
    rxphase->SetFEEid(TCM_FEE_id);                                                              rxphase->publishServices();
    if(!pServer->excludeForWinCC) {cruorbit->SetFEEid(TCM_FEE_id);                                                             cruorbit->publishServices();}
    if(!pServer->excludeForWinCC) {crubc->SetFEEid(TCM_FEE_id);                                                                crubc->publishServices();}
    rawfifo->SetFEEid(TCM_FEE_id);                                                              rawfifo->publishServices();
    selfifo->SetFEEid(TCM_FEE_id);                                                              selfifo->publishServices();
    selfirsthit->SetFEEid(TCM_FEE_id);                                                          selfirsthit->publishServices();
    sellasthit->SetFEEid(TCM_FEE_id);                                                           sellasthit->publishServices();
    selhitsdropped->SetFEEid(TCM_FEE_id);                                                       selhitsdropped->publishServices();
    readoutrate->SetFEEid(TCM_FEE_id);                                                          readoutrate->publishServices();


    orAsign->SetFEEid(TCM_FEE_id);      orAsign->publishCommands();     orAsign->publishServices();
    orArate->SetFEEid(TCM_FEE_id);      orArate->publishCommands();     orArate->publishServices();
    orCsign->SetFEEid(TCM_FEE_id);      orCsign->publishCommands();     orCsign->publishServices();
    orCrate->SetFEEid(TCM_FEE_id);      orCrate->publishCommands();     orCrate->publishServices();
    scsign->SetFEEid(TCM_FEE_id);      scsign->publishCommands();     scsign->publishServices();
    scrate->SetFEEid(TCM_FEE_id);      scrate->publishCommands();     scrate->publishServices();
    csign->SetFEEid(TCM_FEE_id);      csign->publishCommands();     csign->publishServices();
    crate->SetFEEid(TCM_FEE_id);      crate->publishCommands();     crate->publishServices();
    vsign->SetFEEid(TCM_FEE_id);      vsign->publishCommands();     vsign->publishServices();
    vrate->SetFEEid(TCM_FEE_id);      vrate->publishCommands();     vrate->publishServices();

    orAenabled ->SetFEEid(TCM_FEE_id);  orAenabled->publishCommands();                 orAenabled ->publishServices();
    orCenabled ->SetFEEid(TCM_FEE_id);  orCenabled->publishCommands();                 orCenabled ->publishServices();
    scenabled ->SetFEEid(TCM_FEE_id);   scenabled->publishCommands();                  scenabled ->publishServices();
    cenabled->SetFEEid(TCM_FEE_id);     cenabled->publishCommands();                   cenabled->publishServices();
    venabled->SetFEEid(TCM_FEE_id);     venabled->publishCommands();                   venabled->publishServices();

    statusoptioncode->SetFEEid(TCM_FEE_id);         statusoptioncode->publishService();
    setoptioncode->SetFEEid(TCM_FEE_id);            setoptioncode->publishCommand();

    laseron->SetFEEid(TCM_FEE_id);     laseron->publishCommands();                        laseron->publishServices();
    laserdiv->SetFEEid(TCM_FEE_id);     laserdiv->publishCommands();                      laserdiv->publishServices();
    laserdelay->SetFEEid(TCM_FEE_id);     laserdelay->publishCommands();                  laserdelay->publishServices();
    laserpattern1->SetFEEid(TCM_FEE_id);     laserpattern1->publishCommands();            laserpattern1->publishServices();
    laserpattern0->SetFEEid(TCM_FEE_id);     laserpattern0->publishCommands();            laserpattern0->publishServices();
    attenvalue->SetFEEid(TCM_FEE_id);     attenvalue->publishCommands();                  attenvalue->publishServices();
    attenstatus->SetFEEid(TCM_FEE_id);                                                    attenstatus->publishServices();

}




//  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@




void fillPMValHash()
{
    PMValHash<quint8>.insert("CH_MASK",                &MyDimServer::set_CH_MASK_requested);
    PMValHash<quint16>.insert("CFD_SATR",               &MyDimServer::set_CFD_SATR_requested);
    PMValHash<quint16>.insert("OR_GATE",                 &MyDimServer::set_OR_GATE_requested);
    PMValHash<quint32>.insert("TG_PATTERN_1",             &MyDimServer::set_TG_PATTERN_1_requested);
    PMValHash<quint32>.insert("TG_PATTERN_0",             &MyDimServer::set_TG_PATTERN_0_requested);
    PMValHash<quint8>.insert("TG_CONT_VALUE",           &MyDimServer::set_TG_CONT_VALUE_requested);
    PMValHash<quint16>.insert("TG_BUNCH_FREQ",          &MyDimServer::set_TG_BUNCH_FREQ_requested);
    PMValHash<quint16>.insert("TG_FREQ_OFFSET",         &MyDimServer::set_TG_FREQ_OFFSET_requested);
    PMValHash<quint32>.insert("DG_TRG_RESPOND_MASK",    &MyDimServer::set_DG_TRG_RESPOND_MASK_requested);
    PMValHash<quint32>.insert("DG_BUNCH_PATTERN",       &MyDimServer::set_DG_BUNCH_PATTERN_requested);
    PMValHash<quint16>.insert("DG_BUNCH_FREQ",          &MyDimServer::set_DG_BUNCH_FREQ_requested);
    PMValHash<quint16>.insert("DG_FREQ_OFFSET",         &MyDimServer::set_DG_FREQ_OFFSET_requested);
    PMValHash<quint16>.insert("RDH_FEE_ID",             &MyDimServer::set_RDH_FEE_ID_requested);
    PMValHash<quint16>.insert("RDH_PAR",                &MyDimServer::set_RDH_PAR_requested);
    PMValHash<quint16>.insert("RDH_MAX_PAYLOAD",        &MyDimServer::set_RDH_MAX_PAYLOAD_requested);
    PMValHash<quint16>.insert("RDH_DET_FIELD",          &MyDimServer::set_RDH_DET_FIELD_requested);
    PMValHash<quint16>.insert("CRU_TRG_COMPARE_DELAY",  &MyDimServer::set_CRU_TRG_COMPARE_DELAY_requested);
    PMValHash<quint16>.insert("BCID_DELAY",             &MyDimServer::set_BCID_DELAY_requested);
    PMValHash<quint32>.insert("DATA_SEL_TRG_MASK",      &MyDimServer::set_DATA_SEL_TRG_MASK_requested);


    PMValHash<quint8>.insert("SEND_READOUT_COMMAND",    &MyDimServer::apply_SEND_READOUT_COMMAND_requested);
    PMValHash<quint32>.insert("TG_SEND_SINGLE",         &MyDimServer::apply_TG_SEND_SINGLE_requested);
    PMValHash<quint8>.insert("TG_MODE",                 &MyDimServer::apply_TG_MODE_requested);
    PMValHash<quint8>.insert("DG_MODE",                 &MyDimServer::apply_DG_MODE_requested);
    PMValHash<quint8>.insert("SwChOn",                  &MyDimServer::apply_SwChOn_requested);
    PMValHash<quint8>.insert("SwChOff",                 &MyDimServer::apply_SwChOff_requested);
    PMValHash<quint8>.insert("HB_RESPONSE",             &MyDimServer::apply_HB_RESPONSE_requested);

};

void fillPMCHValHash()
{
    PMCHValHash<qint16>.insert("ADC_ZERO",    &MyDimServer::set_ADC_ZERO_requested);
    PMCHValHash<qint16>.insert("ADC_DELAY",   &MyDimServer::set_ADC_DELAY_requested);
    PMCHValHash<quint16>.insert("ADC0_OFFSET", &MyDimServer::set_ADC0_OFFSET_requested);
    PMCHValHash<quint16>.insert("ADC1_OFFSET", &MyDimServer::set_ADC1_OFFSET_requested);
    PMCHValHash<quint16>.insert("ADC0_RANGE",  &MyDimServer::set_ADC0_RANGE_requested);
    PMCHValHash<quint16>.insert("ADC1_RANGE",  &MyDimServer::set_ADC1_RANGE_requested);
    PMCHValHash<quint16>.insert("TIME_ALIGN",   &MyDimServer::set_TIME_ALIGN_requested);
    PMCHValHash<qint16>.insert("CFD_THRESHOLD",&MyDimServer::set_CFD_THRESHOLD_requested);
    PMCHValHash<qint16>.insert("CFD_ZERO",    &MyDimServer::set_CFD_ZERO_requested);
    PMCHValHash<qint16>.insert("THRESHOLD_CALIBR",&MyDimServer::set_THRESHOLD_CALIBR_requested);

}
QHash<QString,pm_pNonValSignal> PMNonValHash;

void fillPMNonValHash()
{
    PMNonValHash.insert("CH_MASK",         &MyDimServer::apply_CH_MASK_requested);
    PMNonValHash.insert("CFD_SATR",        &MyDimServer::apply_CFD_SATR_requested);
    PMNonValHash.insert("OR_GATE",         &MyDimServer::apply_OR_GATE_requested);
    PMNonValHash.insert("RESET_COUNTERS",  &MyDimServer::apply_RESET_COUNTERS_requested);
    PMNonValHash.insert("ZERO_LVL_CALIBR", &MyDimServer::apply_ZERO_LVL_CALIBR_requested);

    PMNonValHash.insert("RESET_ORBIT_SYNC",            &MyDimServer::apply_RESET_ORBIT_SYNC_requested);
    PMNonValHash.insert("RESET_DROPPING_HIT_COUNTERS", &MyDimServer::apply_RESET_DROPPING_HIT_COUNTERS_requested);
    PMNonValHash.insert("RESET_GEN_BUNCH_OFFSET",      &MyDimServer::apply_RESET_GEN_BUNCH_OFFSET_requested);
    PMNonValHash.insert("RESET_GBT_ERRORS",            &MyDimServer::apply_RESET_GBT_ERRORS_requested);
    PMNonValHash.insert("RESET_GBT",                   &MyDimServer::apply_RESET_GBT_requested);
    PMNonValHash.insert("RESET_RX_PHASE_ERROR",        &MyDimServer::apply_RESET_RX_PHASE_ERROR_requested);
    PMNonValHash.insert("TG_PATTERN_1",                 &MyDimServer::apply_TG_PATTERN_1_requested);
    PMNonValHash.insert("TG_PATTERN_0",                 &MyDimServer::apply_TG_PATTERN_0_requested);
    PMNonValHash.insert("TG_CONT_VALUE",               &MyDimServer::apply_TG_CONT_VALUE_requested);
    PMNonValHash.insert("TG_BUNCH_FREQ",               &MyDimServer::apply_TG_BUNCH_FREQ_requested);
    PMNonValHash.insert("TG_FREQ_OFFSET",              &MyDimServer::apply_TG_FREQ_OFFSET_requested);
    PMNonValHash.insert("DG_TRG_RESPOND_MASK",         &MyDimServer::apply_DG_TRG_RESPOND_MASK_requested);
    PMNonValHash.insert("DG_BUNCH_PATTERN",            &MyDimServer::apply_DG_BUNCH_PATTERN_requested);
    PMNonValHash.insert("DG_BUNCH_FREQ",               &MyDimServer::apply_DG_BUNCH_FREQ_requested);
    PMNonValHash.insert("DG_FREQ_OFFSET",              &MyDimServer::apply_DG_FREQ_OFFSET_requested);
    PMNonValHash.insert("RDH_FEE_ID",                  &MyDimServer::apply_RDH_FEE_ID_requested);
    PMNonValHash.insert("RDH_PAR",                     &MyDimServer::apply_RDH_PAR_requested);
    PMNonValHash.insert("RDH_MAX_PAYLOAD",             &MyDimServer::apply_RDH_MAX_PAYLOAD_requested);
    PMNonValHash.insert("RDH_DET_FIELD",               &MyDimServer::apply_RDH_DET_FIELD_requested);
    PMNonValHash.insert("CRU_TRG_COMPARE_DELAY",       &MyDimServer::apply_CRU_TRG_COMPARE_DELAY_requested);
    PMNonValHash.insert("BCID_DELAY",                  &MyDimServer::apply_BCID_DELAY_requested);
    PMNonValHash.insert("ALLtoPM",                     &MyDimServer::apply_ALLtoPM_requested);

    PMNonValHash.insert("DATA_SEL_TRG_MASK",           &MyDimServer::apply_DATA_SEL_TRG_MASK_requested);



    PMNonValHash.insert("RESET_SYSTEM",  &MyDimServer::apply_RESET_SYSTEM_requested);
}

QHash<QString,pmch_pNonValSignal> PMCHNonValHash;

void fillPMCHNonValHash()
{
     PMCHNonValHash.insert("ADC_ZERO",    &MyDimServer::apply_ADC_ZERO_requested);
     PMCHNonValHash.insert("ADC_DELAY",   &MyDimServer::apply_ADC_DELAY_requested);
     PMCHNonValHash.insert("ADC0_OFFSET", &MyDimServer::apply_ADC0_OFFSET_requested);
     PMCHNonValHash.insert("ADC1_OFFSET", &MyDimServer::apply_ADC1_OFFSET_requested);
     PMCHNonValHash.insert("ADC0_RANGE",  &MyDimServer::apply_ADC0_RANGE_requested);
     PMCHNonValHash.insert("ADC1_RANGE",  &MyDimServer::apply_ADC1_RANGE_requested);
     PMCHNonValHash.insert("TIME_ALIGN",   &MyDimServer::apply_TIME_ALIGN_requested);
     PMCHNonValHash.insert("CFD_THRESHOLD",&MyDimServer::apply_CFD_THRESHOLD_requested);
     PMCHNonValHash.insert("CFD_ZERO",    &MyDimServer::apply_CFD_ZERO_requested);
     PMCHNonValHash.insert("THRESHOLD_CALIBR",&MyDimServer::apply_THRESHOLD_CALIBR_requested);
}



void fillTCMValHash()
{
    TCMValHash<quint8>.insert("COUNTERS_UPD_RATE",&MyDimServer::apply_COUNTERS_UPD_RATE_requested);
    TCMValHash<quint8>.insert("EXTENDED_READOUT",&MyDimServer::apply_EXTENDED_READOUT_requested);
    TCMValHash<quint8>.insert("SC_SUM_SIDES",&MyDimServer::apply_SC_SUM_SIDES_requested);
    TCMValHash<quint8>.insert("C_SUM_SIDES",&MyDimServer::apply_C_SUM_SIDES_requested);
    TCMValHash<quint8>.insert("ADD_C_DELAY",&MyDimServer::apply_ADD_C_DELAY_requested);
    TCMValHash<quint8>.insert("RES_SW_1",&MyDimServer::apply_RES_SW_1_requested);
    TCMValHash<quint8>.insert("RES_SW_2",&MyDimServer::apply_RES_SW_2_requested);
    TCMValHash<quint8>.insert("RES_SW_3",&MyDimServer::apply_RES_SW_3_requested);
    TCMValHash<quint8>.insert("RES_SW_4",&MyDimServer::apply_RES_SW_4_requested);

    TCMValHash<qint16>.insert("DELAY_A",&MyDimServer::set_DELAY_A_requested);
    TCMValHash<qint16>.insert("DELAY_C",&MyDimServer::set_DELAY_C_requested);
    TCMValHash<qint16>.insert("VTIME_LOW",&MyDimServer::set_VTIME_LOW_requested);
    TCMValHash<qint16>.insert("VTIME_HIGH",&MyDimServer::set_VTIME_HIGH_requested);
    TCMValHash<quint16>.insert("SC_LEVEL_A",&MyDimServer::set_SC_LEVEL_A_requested);
    TCMValHash<quint16>.insert("SC_LEVEL_C",&MyDimServer::set_SC_LEVEL_C_requested);
    TCMValHash<quint16>.insert("C_LEVEL_A",&MyDimServer::set_C_LEVEL_A_requested);
    TCMValHash<quint16>.insert("C_LEVEL_C",&MyDimServer::set_C_LEVEL_C_requested);
    TCMValHash<quint16>.insert("CH_MASK_A",&MyDimServer::set_CH_MASK_A_requested);
    TCMValHash<quint16>.insert("CH_MASK_C",&MyDimServer::set_CH_MASK_C_requested);

    TCMValHash<quint16>.insert("OR_A_SIGN",&MyDimServer::set_OR_A_SIGN_requested);
    TCMValHash<quint32>.insert("OR_A_RATE",&MyDimServer::set_OR_A_RATE_requested);
    TCMValHash<quint16>.insert("OR_C_SIGN",&MyDimServer::set_OR_C_SIGN_requested);
    TCMValHash<quint32>.insert("OR_C_RATE",&MyDimServer::set_OR_C_RATE_requested);
    TCMValHash<quint16>.insert("SC_SIGN",&MyDimServer::set_SC_SIGN_requested);
    TCMValHash<quint32>.insert("SC_RATE",&MyDimServer::set_SC_RATE_requested);
    TCMValHash<quint16>.insert("C_SIGN",&MyDimServer::set_C_SIGN_requested);
    TCMValHash<quint32>.insert("C_RATE",&MyDimServer::set_C_RATE_requested);
    TCMValHash<quint16>.insert("V_SIGN",&MyDimServer::set_V_SIGN_requested);
    TCMValHash<quint32>.insert("V_RATE",&MyDimServer::set_V_RATE_requested);

    TCMValHash<quint8>.insert("OR_A_ENABLED",&MyDimServer::apply_OR_A_ENABLED_requested);
    TCMValHash<quint8>.insert("OR_C_ENABLED",&MyDimServer::apply_OR_C_ENABLED_requested);
    TCMValHash<quint8>.insert("SC_ENABLED",&MyDimServer::apply_SC_ENABLED_requested);
    TCMValHash<quint8>.insert("C_ENABLED",&MyDimServer::apply_C_ENABLED_requested);
    TCMValHash<quint8>.insert("V_ENABLED",&MyDimServer::apply_V_ENABLED_requested);

    TCMValHash<quint8>.insert("LASER_ON",&MyDimServer::apply_LASER_ON_requested);
    TCMValHash<quint32>.insert("LASER_DIV",&MyDimServer::set_LASER_DIV_requested);
    TCMValHash<qint16>.insert("LASER_DELAY",&MyDimServer::set_LASER_DELAY_requested);
    TCMValHash<quint32>.insert("LASER_PATTERN_1",&MyDimServer::set_LASER_PATTERN_1_requested);
    TCMValHash<quint32>.insert("LASER_PATTERN_0",&MyDimServer::set_LASER_PATTERN_0_requested);
    TCMValHash<quint16>.insert("ATTEN_VALUE",&MyDimServer::set_ATTEN_VALUE_requested);

}

QHash<QString,tcm_pNonValSignal> TCMNonValHash;
void fillTCMNonValHash()
{
    TCMNonValHash.insert("DELAY_A",&MyDimServer::apply_DELAY_A_requested);
    TCMNonValHash.insert("DELAY_C",&MyDimServer::apply_DELAY_C_requested);
    TCMNonValHash.insert("VTIME_LOW",&MyDimServer::apply_VTIME_LOW_requested);
    TCMNonValHash.insert("VTIME_HIGH",&MyDimServer::apply_VTIME_HIGH_requested);
    TCMNonValHash.insert("SC_LEVEL_A",&MyDimServer::apply_SC_LEVEL_A_requested);
    TCMNonValHash.insert("SC_LEVEL_C",&MyDimServer::apply_SC_LEVEL_C_requested);
    TCMNonValHash.insert("C_LEVEL_A",&MyDimServer::apply_C_LEVEL_A_requested);
    TCMNonValHash.insert("C_LEVEL_C",&MyDimServer::apply_C_LEVEL_C_requested);
    TCMNonValHash.insert("CH_MASK_A",&MyDimServer::apply_CH_MASK_A_requested);
    TCMNonValHash.insert("CH_MASK_C",&MyDimServer::apply_CH_MASK_C_requested);

    TCMNonValHash.insert("OR_A_SIGN",&MyDimServer::apply_OR_A_SIGN_requested);
    TCMNonValHash.insert("OR_A_RATE",&MyDimServer::apply_OR_A_RATE_requested);
    TCMNonValHash.insert("OR_C_SIGN",&MyDimServer::apply_OR_C_SIGN_requested);
    TCMNonValHash.insert("OR_C_RATE",&MyDimServer::apply_OR_C_RATE_requested);
    TCMNonValHash.insert("SC_SIGN",&MyDimServer::apply_SC_SIGN_requested);
    TCMNonValHash.insert("SC_RATE",&MyDimServer::apply_SC_RATE_requested);
    TCMNonValHash.insert("C_SIGN",&MyDimServer::apply_C_SIGN_requested);
    TCMNonValHash.insert("C_RATE",&MyDimServer::apply_C_RATE_requested);
    TCMNonValHash.insert("V_SIGN",&MyDimServer::apply_V_SIGN_requested);
    TCMNonValHash.insert("V_RATE",&MyDimServer::apply_V_RATE_requested);

    TCMNonValHash.insert("LASER_DIV",&MyDimServer::apply_LASER_DIV_requested);
    TCMNonValHash.insert("LASER_DELAY",&MyDimServer::apply_LASER_DELAY_requested);
    TCMNonValHash.insert("LASER_PATTERN_1",&MyDimServer::apply_LASER_PATTERN_1_requested);
    TCMNonValHash.insert("LASER_PATTERN_0",&MyDimServer::apply_LASER_PATTERN_0_requested);
    TCMNonValHash.insert("ATTEN_VALUE",&MyDimServer::apply_ATTEN_VALUE_requested);

}





