#include "classes.h"
#include "mydimserver.h"

pm_APP::pm_APP(QString t_name) : Base(t_name)
{
//////    pAPPSignal = returnPMAppPointerToSignal(name);
}

void pm_APP::publishCommand()
{
    appCommand = new DimCommand(qPrintable("APP_"+DIM_name[FEE_id]
                                        +prefix+"/"+name),"C:1",this);
    outDCs << appCommand->getName() << endl;
    SetSignal();
}

void pm_APP::commandHandler()
{
    DimCommand* currCmnd = getCommand();
    if(currCmnd == appCommand) {
        cout << "@APP@ recieved " << currCmnd->getName() << endl;
        emitSignalRequest();
    }
}

void pm_APP::emitSignalRequest()
{
    this->pServer->emitSignal(pAPPSignal,FEE_id);
}

void pm_APP::SetSignal(){ pAPPSignal = getPMNonValPointerToSignal(this->name);}


pmch_APP::pmch_APP(QString t_name) : Base(t_name), pm_APP::pm_APP(t_name)
{
    SetSignal();
}

void pmch_APP::emitSignalRequest()
{
    this->pServer->emitSignal(pAPPSignal,this->FEE_id,this->CHid);
}

void pmch_APP::SetSignal(){ pAPPSignal = getPMCHNonValPointerToSignal(this->name);}

PMonlyAppPar::PMonlyAppPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/GBT/control"), pm_APP(t_name)
{this->SetServer(t_pServer);}

void PMonlyAppPar::publishCommands()
{ pm_APP::publishCommand(); }

PMCHonlyAppPar::PMCHonlyAppPar(QString t_name, MyDimServer* t_pServer) : Base(t_name,"/control"), pmch_APP(t_name)
{this->SetServer(t_pServer);}

void PMCHonlyAppPar::publishCommands()
{ pmch_APP::publishCommand(); }


tcm_APP::tcm_APP(QString t_name) : Base(t_name), pm_APP::pm_APP(t_name)
{
    SetSignal();
}

void tcm_APP::emitSignalRequest()
{
    this->pServer->emitSignal(pAPPSignal);
}

void tcm_APP::SetSignal(){ pAPPSignal = getTCMNonValPointerToSignal(this->name);}


str_ACT::str_ACT(QString t_name) : Base(t_name), actValue("My default string"){}

str_ACT::~str_ACT(){ delete actService; }

void str_ACT::updateAct(QString val)
{
    qstrncpy(actValue,qPrintable(val),499);
        actService->updateService();
        cout << "# " << actService->getName() << " updated to "
             << "\"" << val << "\"" << endl << endl;
}

void str_ACT::publishService()
{
            actService = new DimService(qPrintable("ACT_"+DIM_name[FEE_id]+prefix+"/"+name),actValue);
            outDSs << actService->getName() << endl;
}

twoValAPP::twoValAPP(QString t_name) : Base(t_name){}
twoValAPP::~twoValAPP(){ delete appCommand;}
void twoValAPP::publishCommand()
{
        appCommand = new DimCommand(qPrintable("APP_"+DIM_name[FEE_id]+prefix+"/"+name),"C:1;C:1",this);
        outDCs << appCommand->getName() << endl;
}

void twoValAPP::SetSignal(pTwoValSignal pSignal){ pAPPSignal = pSignal;}

void twoValAPP::commandHandler()
    {
        DimCommand* currCmnd = getCommand();
        if(currCmnd == appCommand) {
            twoVal* data = static_cast<twoVal*>(currCmnd->getData());
            cout << "@ValAPP@ recieved " << currCmnd->getName()
                 << hex << " H1:" << data->first
                 << dec << " D1:" << data->first
                 << hex << " H2:" << data->second
                 << dec << " D1:" << data->second
                 << endl;
            emitSignalRequest(currCmnd);
        }

    }

void twoValAPP::emitSignalRequest(DimCommand* currCmnd)
{
    twoVal* data = static_cast<twoVal*>(currCmnd->getData());
    this->pServer->emitSignal(pAPPSignal,data->first,data->second);

}
