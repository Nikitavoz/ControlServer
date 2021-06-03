#include "common.h"

QFile DimServicesFile("ServicesList_v2_0.txt");
QFile DimCommandsFile("CommandsList_v2_0.txt");

QTextStream outDSs(&DimServicesFile);
QTextStream outDCs(&DimCommandsFile);

pm_pNonValSignal getPMNonValPointerToSignal(QString PARname){
//    if(PMNonValHash[PARname] == nullptr) qDebug() << "Can't find" << PARname << "in PMNonValHash";
    return PMNonValHash[PARname];
};
pmch_pNonValSignal getPMCHNonValPointerToSignal(QString PARname){
    //    if(PMCHNonValHash[PARname] == nullptr) qDebug() << "Can't find" << PARname << "in PMCHNonValHash";
    return PMCHNonValHash[PARname];
};

tcm_pNonValSignal getTCMNonValPointerToSignal(QString PARname){
    if(TCMNonValHash[PARname] == nullptr) qDebug() << "Can't find" << PARname << "in TCMNonValHash";
    return TCMNonValHash[PARname];
};



void OpenOutFiles()
{
    DimServicesFile.open(QIODevice::WriteOnly);
    DimCommandsFile.open(QIODevice::WriteOnly);

}
