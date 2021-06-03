#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QObject>
#include <QTextStream>
#include <QFile>
#include <QVector>
#include <QObject>

#include "dim/dis.hxx"

class MyDimServer;

template <class T>
using pm_pValSignal = void(MyDimServer::*)(quint16,T);
using pm_pNonValSignal = void(MyDimServer::*)(quint16);

template <class T>
using pmch_pValSignal = void(MyDimServer::*)(quint16,quint8,T);
using pmch_pNonValSignal = void(MyDimServer::*)(quint16,quint8);

template <class T>
using tcm_pValSignal = void(MyDimServer::*)(T);
using tcm_pNonValSignal = void(MyDimServer::*)();

using pTwoValSignal = void(MyDimServer::*)(quint8,quint8);

static QTextStream cout(stdout);
static QTextStream cin(stdin);
static QTextStream cerr(stderr);

extern QFile DimServicesFile;
extern QFile DimCommandsFile;

//static QTextStream outDSs;
//static QTextStream outDCs;
extern QTextStream outDSs;
extern QTextStream outDCs;


void OpenOutFiles();

template<class T>
QHash<QString,pm_pValSignal<T>> PMValHash;
extern void fillPMValHash();
template<class T>
pm_pValSignal<T> getPMValPointerToSignal(QString PARname){
//    if(PMValHash<T>[PARname] == nullptr) qDebug() << "Can't find" << PARname << "in PMValHash";
    return PMValHash<T>[PARname];
};

template<class T>
QHash<QString,pmch_pValSignal<T>> PMCHValHash;
extern void fillPMCHValHash();
template<class T>
pmch_pValSignal<T> getPMCHValPointerToSignal(QString PARname){
//    if(PMCHValHash<T>[PARname] == nullptr) qDebug() << "Can't find" << PARname << "in PMCHValHash";
    return PMCHValHash<T>[PARname];
};

extern QHash<QString,pm_pNonValSignal> PMNonValHash;
extern void fillPMNonValHash();
pm_pNonValSignal getPMNonValPointerToSignal(QString PARname);

extern QHash<QString,pmch_pNonValSignal> PMCHNonValHash;
extern void fillPMCHNonValHash();
pmch_pNonValSignal getPMCHNonValPointerToSignal(QString PARname);



template<class T>
QHash<QString,tcm_pValSignal<T>> TCMValHash;
extern void fillTCMValHash();
template<class T>
tcm_pValSignal<T> getTCMValPointerToSignal(QString PARname){
    if(TCMValHash<T>[PARname] == nullptr) qDebug() << "Can't find" << PARname << "in TCMValHash";
    return TCMValHash<T>[PARname];
};

extern QHash<QString,tcm_pNonValSignal> TCMNonValHash;
extern void fillTCMNonValHash();
tcm_pNonValSignal getTCMNonValPointerToSignal(QString PARname);


const QMap<quint16,QString> DIM_name{
                                        {0xF000,"FT0"},
                                        {0xF0A0,"FT0/PMA0"},
                                        {0xF0A1,"FT0/PMA1"},
                                        {0xF0A2,"FT0/PMA2"},
                                        {0xF0A3,"FT0/PMA3"},
                                        {0xF0A4,"FT0/PMA4"},
                                        {0xF0A5,"FT0/PMA5"},
                                        {0xF0A6,"FT0/PMA6"},
                                        {0xF0A7,"FT0/PMA7"},
                                        {0xF0C0,"FT0/PMC0"},
                                        {0xF0C1,"FT0/PMC1"},
                                        {0xF0C2,"FT0/PMC2"},
                                        {0xF0C3,"FT0/PMC3"},
                                        {0xF0C4,"FT0/PMC4"},
                                        {0xF0C5,"FT0/PMC5"},
                                        {0xF0C6,"FT0/PMC6"},
                                        {0xF0C7,"FT0/PMC7"},
                                        {0xF0C8,"FT0/PMC8"},
                                        {0xF0C9,"FT0/PMC9"}
                                    };

const QMap<quint8,quint16> FT0_FEE_ID{  {0,0xF000},
                                        {1,0xF0A0},
                                        {2,0xF0A1},
                                        {3,0xF0A2},
                                        {4,0xF0A3},
                                        {5,0xF0A4},
                                        {6,0xF0A5},
                                        {7,0xF0A6},
                                        {8,0xF0A7},
                                        {9,0xF0C0},
                                        {10,0xF0C1},
                                        {11,0xF0C2},
                                        {12,0xF0C3},
                                        {13,0xF0C4},
                                        {14,0xF0C5},
                                        {15,0xF0C6},
                                        {16,0xF0C7},
                                        {17,0xF0C8},
                                        {18,0xF0C9}
                                     };

struct twoVal
{
  quint8 first;
  quint8 second;
};

#endif // COMMON_H
