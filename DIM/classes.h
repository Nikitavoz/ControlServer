#ifndef CLASSES_H
#define CLASSES_H

#include "common.h"


class Base
{
protected:
    QString name;
//    QString prefix;
    MyDimServer* pServer;
    quint16 FEE_id;    //  default is 0xffff
    quint8  CHid;    //  default is 0

public:
    QString prefix;
    void SetServer(MyDimServer* t_pServer){ pServer = t_pServer; }
    void SetFEEid(quint16 t_pm_fee_id){ FEE_id = t_pm_fee_id; }
    Base(QString t_name, QString t_prefix = "")	:	name (t_name), FEE_id(0xFFFF), CHid(0), prefix(t_prefix)
    {/*cout << name << " created" << endl;*/}
    ~Base()	{/*cout << name << " deleted" << endl;*/}
};

//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@  ACT  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

template<class T>
class pm_ACT : public virtual Base
{
protected:
    T actValue;
    DimService* actService = nullptr;
public:
    void updateAct(T val)
    {
        actValue = val;
        actService->updateService();
        cout << "# " << actService->getName() << " updated to" <<
              hex << " H:" << val << dec << " D:" << val << endl << endl;
    }

    void publishService()
    {
        switch (sizeof(T)) {
        case 1:
            actService = new DimService(qPrintable("ACT_"+DIM_name[FEE_id]+prefix+"/"+name),"C:1",&actValue,1);
            outDSs << actService->getName() << endl;
            break;
        case 2:
            actService = new DimService(qPrintable("ACT_"+DIM_name[FEE_id]+prefix+"/"+name),"S:1",&actValue,2);
            outDSs << actService->getName() << endl;
            break;
        case 4:
            actService = new DimService(qPrintable("ACT_"+DIM_name[FEE_id]+prefix+"/"+name),"I:1",&actValue,4);
            outDSs << actService->getName() << endl;
            break;
        case 8:
            actService = new DimService(qPrintable("ACT_"+DIM_name[FEE_id]+prefix+"/"+name),"X:1",&actValue,8);
            outDSs << actService->getName() << endl;
            break;
        default:
            cerr << "\n## Underfined size of T\n";
            Q_ASSERT(1);
        }
    }


    pm_ACT(QString t_name) : Base(t_name), actValue(0){}
    ~pm_ACT(){ delete actService; }
};


template<class T>
class pmch_ACT : public pm_ACT<T>
{
public:
    pmch_ACT(QString t_name) : Base(t_name), pm_ACT<T>::pm_ACT(t_name)
    {
//        this->prefix = "/Ch"+QString("%1").arg(this->CHid,2,10,QLatin1Char('0')) + this->prefix;
    }
};



//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@  NEW  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

template<class T>
class pm_NEW : public virtual Base
{
protected:
    T newValue;
    DimService* newService = nullptr;
public:
    void updateNew(T val)
    {
        newValue = val;
        newService->updateService();
        cout << "# " << newService->getName() << " updated to" <<
              hex << " H:" << val << dec << " D:" << val << endl << endl;
    }

    void publishService()
    {
        switch (sizeof(T)) {
        case 1:
            newService = new DimService(qPrintable("NEW_"+DIM_name[FEE_id]+prefix+"/"+name),"C:1",&newValue,1);
            outDSs << newService->getName() << endl;
            break;
        case 2:
            newService = new DimService(qPrintable("NEW_"+DIM_name[FEE_id]+prefix+"/"+name),"S:1",&newValue,2);
            outDSs << newService->getName() << endl;
            break;
        case 4:
            newService = new DimService(qPrintable("NEW_"+DIM_name[FEE_id]+prefix+"/"+name),"I:1",&newValue,4);
            outDSs << newService->getName() << endl;
            break;
        case 8:
            newService = new DimService(qPrintable("NEW_"+DIM_name[FEE_id]+prefix+"/"+name),"X:1",&newValue,8);
            outDSs << newService->getName() << endl;
            break;
        default:
            cerr << "\n## Underfined size of T\n";
            Q_ASSERT(1);
        }
    }

    T GetNewValue(){return newValue;}

    pm_NEW(QString t_name) : Base(t_name), newValue(0){}
    ~pm_NEW(){ delete newService; }
};

template<class T>
class pmch_NEW : public pm_NEW<T>
{
public:
    pmch_NEW(QString t_name) : Base(t_name), pm_NEW<T>::pm_NEW(t_name)
    {
//        this->prefix = "/Ch"+QString("%1").arg(this->CHid,2,10,QLatin1Char('0')) + this->prefix;
    }
};



//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@  APP  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

class pm_APP : public virtual Base, public DimCommandHandler
{
    DimCommand* appCommand = nullptr;
    pm_pNonValSignal pAPPSignal = nullptr;
    void commandHandler();
public:
    void publishCommand();
    virtual void emitSignalRequest();
    virtual void SetSignal();
    pm_APP(QString t_name);
    virtual ~pm_APP(){ delete appCommand;}
};

class pmch_APP : public pm_APP
{
    pmch_pNonValSignal pAPPSignal = nullptr;
public:
    pmch_APP(QString t_name);
    void emitSignalRequest();
    void SetSignal();
};

template<class T>
class pm_ValAPP : public virtual Base, public DimCommandHandler
{
    DimCommand* appCommand = nullptr;
    pm_pValSignal<T> pAPPSignal = nullptr;
    void commandHandler()
    {
        DimCommand* currCmnd = getCommand();
        if(currCmnd == appCommand) {
            cout << "@ValAPP@ recieved " << currCmnd->getName() <<
                 hex << " H:" << *static_cast<T*>(currCmnd->getData()) <<
                 dec << " D:" << *static_cast<T*>(currCmnd->getData()) << endl;
            emitSignalRequest(currCmnd);
        }
    }
    virtual void emitSignalRequest(DimCommand* currCmnd);


public:
    void publishCommand()
    {
    //    if(pSet!= nullptr)
        switch (sizeof(T)) {
        case 1: appCommand = new DimCommand(qPrintable("APP_"+DIM_name[FEE_id]+prefix+"/"+name),"C:1",this);
            outDCs << appCommand->getName() << endl;
            break;
        case 2: appCommand = new DimCommand(qPrintable("APP_"+DIM_name[FEE_id]+prefix+"/"+name),"S:1",this);
            outDCs << appCommand->getName() << endl;
            break;
        case 4: appCommand = new DimCommand(qPrintable("APP_"+DIM_name[FEE_id]+prefix+"/"+name),"I:1",this);
            outDCs << appCommand->getName() << endl;
            break;
        case 8: appCommand = new DimCommand(qPrintable("APP_"+DIM_name[FEE_id]+prefix+"/"+name),"X:1",this);
            outDCs << appCommand->getName() << endl;
            break;
        default:
            Q_ASSERT(1);
        }
    }

    virtual void SetSignal(){ pAPPSignal = getPMValPointerToSignal<T>(this->name);}
    pm_ValAPP(QString t_name) : Base(t_name){ SetSignal(); }
    virtual ~pm_ValAPP(){ delete appCommand;}
};


//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@  SET  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

template<class T>
class pm_SET : public virtual Base, public DimCommandHandler
{
    DimCommand* setCommand = nullptr;
    pm_pValSignal<T> pSETSignal = nullptr;
    void commandHandler()
    {
        DimCommand* currCmnd = getCommand();
        if(currCmnd == setCommand) {
            cout << "@SET@ recieved " << currCmnd->getName() <<
            hex << " H:" << *static_cast<T*>(currCmnd->getData()) <<
            dec << " D:" << *static_cast<T*>(currCmnd->getData()) << endl;

            emitSignalRequest(currCmnd);
        }
    }
    virtual void emitSignalRequest(DimCommand* currCmnd);

public:
    void publishCommand()
    {
    //    if(pSet!= nullptr)
        switch (sizeof(T)) {
        case 1: setCommand = new DimCommand(qPrintable("SET_"+DIM_name[FEE_id]+prefix+"/"+name),"C:1",this);
            outDCs << setCommand->getName() << endl;
            break;
        case 2: setCommand = new DimCommand(qPrintable("SET_"+DIM_name[FEE_id]+prefix+"/"+name),"S:1",this);
            outDCs << setCommand->getName() << endl;
            break;
        case 4: setCommand = new DimCommand(qPrintable("SET_"+DIM_name[FEE_id]+prefix+"/"+name),"I:1",this);
            outDCs << setCommand->getName() << endl;
            break;
        case 8: setCommand = new DimCommand(qPrintable("SET_"+DIM_name[FEE_id]+prefix+"/"+name),"X:1",this);
            outDCs << setCommand->getName() << endl;
            break;
        default:
            Q_ASSERT(1);
        }
    }

    virtual void SetSignal(){ pSETSignal = getPMValPointerToSignal<T>(this->name);}
    pm_SET(QString t_name) : Base(t_name){ SetSignal(); }
    virtual ~pm_SET(){ delete setCommand;}
};

template<class T>
class pmch_SET : public pm_SET<T>
{
    pmch_pValSignal<T> pSETSignal = nullptr;
public:
    void emitSignalRequest(DimCommand* currCmnd){
        this->pServer->emitSignal(pSETSignal,this->FEE_id,this->CHid,*static_cast<T*>(currCmnd->getData()));
    }

    void SetSignal(){ pSETSignal = getPMCHValPointerToSignal<T>(this->name);}

    pmch_SET(QString t_name) : Base(t_name), pm_SET<T>::pm_SET(t_name)
    {
//        this->prefix = "/Ch"+QString("%1").arg(this->CHid,2,10,QLatin1Char('0')) + this->prefix;
        SetSignal();
    }
};


//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//	@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


template<class T>
class PMfullPar : public pm_ACT<T>, public pm_NEW<T>, public pm_SET<T>, public pm_APP
{
public:
    PMfullPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/GBT/control"), pm_ACT<T>(t_name), pm_NEW<T>(t_name),pm_SET<T>(t_name), pm_APP(t_name)
    {SetServer(t_pServer);}
    void publishServices(){
        pm_ACT<T>::publishService();
        pm_NEW<T>::publishService();
    }
    void publishCommands(){
        pm_SET<T>::publishCommand();
        pm_APP::publishCommand();
    }
};


template<class T>
class PMCHfullPar : public pmch_ACT<T>, public pmch_NEW<T>, public pmch_APP, public pmch_SET<T>
{
public:
    PMCHfullPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/control"), pmch_ACT<T>(t_name), pmch_NEW<T>(t_name), pmch_APP(t_name), pmch_SET<T>(t_name)
    {this->SetServer(t_pServer);}
    void SetCHid(quint8 t_CHid){
        CHid = t_CHid; this->prefix = "/Ch"+QString("%1").arg(CHid,2,10,QLatin1Char('0')) + this->prefix;
    }
    void publishServices(){
        pmch_ACT<T>::publishService();
        pmch_NEW<T>::publishService();
    }
    void publishCommands(){
        pmch_SET<T>::publishCommand();
        pmch_APP::publishCommand();
    }
};

template<class T>
class PMonlyActPar : public pm_ACT<T>
{
public:
    PMonlyActPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/GBT/status"), pm_ACT<T>(t_name)
    {this->SetServer(t_pServer);}
    void publishServices(){ pm_ACT<T>::publishService(); }
};

template<class T>
class PMCHonlyActPar : public pmch_ACT<T>
{
public:
    PMCHonlyActPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/status"), pmch_ACT<T>(t_name)
    {this->SetServer(t_pServer);}
    void publishServices(){ pmch_ACT<T>::publishService(); }
    void SetCHid(quint8 t_CHid){
        this->CHid = t_CHid; this->prefix = "/Ch"+QString("%1").arg(this->CHid,2,10,QLatin1Char('0')) + this->prefix;
    }

};

class PMonlyAppPar : public pm_APP
{
public:
    PMonlyAppPar(QString t_name, MyDimServer* t_pServer);
    void publishCommands();
};

class PMCHonlyAppPar : public pmch_APP
{
public:
    PMCHonlyAppPar(QString t_name, MyDimServer* t_pServer);
    void SetCHid(quint8 t_CHid){
        this->CHid = t_CHid; this->prefix = "/Ch"+QString("%1").arg(this->CHid,2,10,QLatin1Char('0')) + this->prefix;
    }
    void publishCommands();
};


template<class T>
class PMonlyValAppPar : public pm_ValAPP<T>
{
public:
    PMonlyValAppPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/GBT/control"), pm_ValAPP<T>(t_name)
    {this->SetServer(t_pServer);}
    void publishCommands(){ pm_ValAPP<T>::publishCommand(); }
};

template<class T>
class PMActnValAppPar : public PMonlyActPar<T>, public PMonlyValAppPar<T>
{
public:
    PMActnValAppPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/GBT/control"), PMonlyActPar<T>(t_name,t_pServer), PMonlyValAppPar<T>(t_name,t_pServer)
    {this->SetServer(t_pServer);}
};




template<class T>
class tcm_SET : public pm_SET<T>
{
    tcm_pValSignal<T> pSETSignal = nullptr;
public:
    void emitSignalRequest(DimCommand* currCmnd){
        this->pServer->emitSignal(pSETSignal,*static_cast<T*>(currCmnd->getData()));
    }

    void SetSignal(){ pSETSignal = getTCMValPointerToSignal<T>(this->name);}

    tcm_SET(QString t_name) : Base(t_name), pm_SET<T>::pm_SET(t_name)
    {
        SetSignal();
    }

};


class tcm_APP : public pm_APP
{
    tcm_pNonValSignal pAPPSignal = nullptr;
public:
    tcm_APP(QString t_name);
    void emitSignalRequest();
    void SetSignal();
};

template<class T>
class tcm_ValAPP : public pm_ValAPP<T>
{
    tcm_pValSignal<T> pAPPSignal = nullptr;
public:
    void emitSignalRequest(DimCommand* currCmnd){
        this->pServer->emitSignal(pAPPSignal,*static_cast<T*>(currCmnd->getData()));
    }

    void SetSignal(){ pAPPSignal = getTCMValPointerToSignal<T>(this->name);}

    tcm_ValAPP(QString t_name) : Base(t_name), pm_ValAPP<T>(t_name)
    {
        SetSignal();
    }
};



template<class T>
class TCMfullPar : public pm_ACT<T>, public pm_NEW<T>, public tcm_APP, public tcm_SET<T>
{
public:
    TCMfullPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/control"), pm_ACT<T>(t_name), pm_NEW<T>(t_name), tcm_APP(t_name), tcm_SET<T>(t_name)
    {this->SetServer(t_pServer);}

    void publishServices(){
        pm_ACT<T>::publishService();
        pm_NEW<T>::publishService();
    }
    void publishCommands(){
        tcm_SET<T>::publishCommand();
        tcm_APP::publishCommand();
    }
};


template<class T>
class TCMonlyValAppPar : public tcm_ValAPP<T>
{
public:
    TCMonlyValAppPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/control"), tcm_ValAPP<T>(t_name)
    {this->SetServer(t_pServer);}
    void publishCommands(){ tcm_ValAPP<T>::publishCommand(); }
};


template<class T>
class TCMActnValAppPar : public PMonlyActPar<T>, public TCMonlyValAppPar<T>
{
public:
    TCMActnValAppPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/control"), PMonlyActPar<T>(t_name,t_pServer), TCMonlyValAppPar<T>(t_name,t_pServer)
    {this->SetServer(t_pServer);}
};


class str_ACT : public virtual Base
{
protected:
    char actValue[500];
//    std::string actValue;
    DimService* actService = nullptr;
public:
    void updateAct(QString val);

    void publishService();

    str_ACT(QString t_name);
    ~str_ACT();
};

class twoValAPP : public virtual Base, public DimCommandHandler
{
    DimCommand* appCommand = nullptr;
    pTwoValSignal pAPPSignal = nullptr;
    void commandHandler();
//    {
//        DimCommand* currCmnd = getCommand();
//        if(currCmnd == appCommand) {
//            twoVal* data = static_cast<twoVal*>(currCmnd->getData());
//            cout << "@ValAPP@ recieved " << currCmnd->getName()
//                 << hex << " H1:" << data->first
//                 << dec << " D1:" << data->first
//                 << hex << " H2:" << data->second
//                 << dec << " D1:" << data->second
//                 << endl;
//            emitSignalRequest(currCmnd);
//        }
//    }

    virtual void emitSignalRequest(DimCommand* currCmnd);
public:
    void publishCommand();
//    {
//            appCommand = new DimCommand(qPrintable("APP_"+DIM_name[FEE_id]+prefix+"/"+name),"C:2",this);
//            outDCs << appCommand->getName() << endl;
//    }

    void SetSignal(pTwoValSignal pSignal);//{ pAPPSignal = pSignal;}
    twoValAPP(QString t_name);//: Base(t_name){}
    virtual ~twoValAPP();//{ delete appCommand;}
};




#endif // CLASSES_H
