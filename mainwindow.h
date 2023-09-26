#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "FITelectronics.h"
#include "actualLabel.h"
#include "switch.h"
#include <QMainWindow>
#include <QtWidgets>
#include <cmath> //lround

static QString frequencyFormat(const double f) { return QString::asprintf("%.*f", f < 1e5 ? (f < 1e4 ? 3 : 2) : (f < 1e6 ? 1 : 0), f); } //e.g. 1.234, 12.345, 123.456, 1234.567, 12345.67, 123456.7, 1234567, 40078970
static QString rateFormat(const double f) { return f < 999999.95 ? QString::asprintf("%.1f", f) : QString::asprintf("%.3f M", f/1e6); } //e.g. 1.0, 1000.0, 999999.9, 1.000 M, 40.079 M

extern double systemClock_MHz; //40
extern double TDCunit_ps; // 13
extern double halfBC_ns; // 12.5
extern double phaseStepLaser_ns, phaseStep_ns;

class StrictIntValidator: public QIntValidator {
public:
    StrictIntValidator(int minimum, int maximum, QObject *parent = nullptr): QIntValidator(minimum, maximum, parent) {}
    QValidator::State validate(QString &input, int &pos) const {
        bool neg = false, ok;
        if (input.startsWith('-')) {
            if (bottom() >= 0) return QValidator::Invalid;
            neg = true;
            input.remove(0, 1);
        }
        if (!QRegExp("[0-9]*").exactMatch(input)) return QValidator::Invalid;
        while (input.length() > 0 && input.startsWith('0')) { if (pos) --pos; input.remove(0, 1); }
        if (input.isEmpty()) { input = "0"; pos = 1; }
        if (neg) { input.prepend('-'); if (input == "-0") pos = 2; }
        qint32 value = input.toInt(&ok);
        return ok && value >= bottom() && value <= top() ? QValidator::Acceptable : QValidator::Invalid;
    }
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSettings settings;
    FITelectronics FEE;
    QPixmap
        Green0 = QPixmap(":/0G.png"), //OK
        Green1 = QPixmap(":/1G.png"), //OK
        Red0 = QPixmap(":/0R.png"), //not OK
        Red1 = QPixmap(":/1R.png"), //not OK
        RedDash = Red1.transformed(QTransform().rotate(90)),
        SwOff = QPixmap(":/SW0.png"), //isOff
        SwOn  = QPixmap(":/SW1.png"); //isOn
    QFont  regularValueFont = QFont("Consolas", 10, QFont::Normal);
    QFont selectedValueFont = QFont("Consolas", 10, QFont::Bold  );
    QString
        OKstyle    = QString::asprintf("background-color: rgba(%d, %d, %d, 127)", OKcolor   .red(), OKcolor   .green(), OKcolor   .blue()),
        notOKstyle = QString::asprintf("background-color: rgba(%d, %d, %d, 127)", notOKcolor.red(), notOKcolor.green(), notOKcolor.blue()),
        neutralStyle   = QString::asprintf("background-color: rgba(255, 255, 255,   0)"),
        highlightStyle = QString::asprintf("background-color: rgba(255, 255,   0, 127)");
    QRegExp validIPaddressRE {"([1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.(([1-9]?[0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){2}([1-9][0-9]?|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"};
    QList<QLineEdit *> allLineEdits;
    QVector<QLineEdit *> editsTimeAlignmentCh  ,
                         editsThresholdCalibrCh,
                         editsADCdelayCh       ,
                         editsCFDthresholdCh   ,
                         editsADCzeroCh        ,
                         editsCFDzeroCh        ,
                         editsADC0rangeCh      ,
                         editsADC1rangeCh      ;
    QList<QPushButton *> applyButtons, allButtons;
    QVector<QPushButton *> buttonsTimeAlignmentCh  ,
                           buttonsThresholdCalibrCh,
                           buttonsADCdelayCh       ,
                           buttonsCFDthresholdCh   ,
                           buttonsADCzeroCh        ,
                           buttonsCFDzeroCh        ,
                           buttonsADC0rangeCh      ,
                           buttonsADC1rangeCh      ;
    QVector<QPushButton *> switchBitButtons, selectorsPMA, selectorsPMC;
    QVector<QLabel *> linksPMA, linksPMC,
                      labelsTriggersCount       ,
                      labelsTriggersRate        ,
                      labelsADC0baseLineCh      ,
                      labelsADC1baseLineCh      ,
                      labelsADC0RMSCh           ,
                      labelsADC1RMSCh           ,
                      labelsADC0meanAmplitudeCh ,
                      labelsADC1meanAmplitudeCh ,
                      labelsRawTDCdata1Ch       ,
                      labelsRawTDCdata2Ch       ,
                      labelsTRGcounterCh        ,
                      labelsCFDcounterCh        ,
                      labelsTRGcounterRateCh    ,
                      labelsCFDcounterRateCh    ;
    QVector<Switch *> switchesPMA, switchesPMC, switchesCh;
    QVector<QCheckBox *> noTRGCh;
    QVector<ActualLabel *>  labelsTimeAlignmentCh  ,
                            labelsThresholdCalibrCh,
                            labelsADCdelayCh       ,
                            labelsCFDthresholdCh   ,
                            labelsADCzeroCh        ,
                            labelsCFDzeroCh        ,
                            labelsADC0rangeCh      ,
                            labelsADC1rangeCh      ;
    QVector<QRadioButton *> allRadioButtons;
    QList<Switch *> allSwitches;
    QList<QComboBox *> allComboBoxes;
    QList<QAbstractSpinBox *> allSpinBoxes;
    QList<QWidget *> allWidgets;
    QAction *enableControls;

    QIntValidator *intValidator = new QIntValidator(this);
    QDoubleValidator *doubleValidator = new QDoubleValidator(this);
//    QRegExpValidator *uint16Validator = new QRegExpValidator(QRegExp("[0-5]?[0-9]{1,4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]"), this);
//    QRegExpValidator *uint8Validator  = new QRegExpValidator(QRegExp("[0-1]?[0-9]{1,2}|2[0-4][0-9]|25[0-5]"), this);
    bool ok, laserFreqIsEditing = false, laserIsShuttling = false;
    QTimer shuttleTimer;
    quint8 mode;
    quint32 value;
    int fontSize_px;
    double prevPhaseStep_ns = 25. / 2048; //value for 40. Mhz clock and production TCM; var is used to detect values change in case of clock source and/or TCM change
    GBTunit *curGBTact = &FEE.TCM.act.GBT;
    GBTunit::ControlData *curGBTset = &FEE.TCM.set.GBT;
    GBTcounters *curGBTcnt = &FEE.TCM.counters.GBT;
    TypePM *curPM = FEE.allPMs;
    quint16 curFEEid;
    inline bool isTCM() { return curFEEid == FEE.TCMid; } //is TCM selected

public:
    void resetHighlight() {
        ui->labelTextTimeAlignment  ->setStyleSheet(neutralStyle);
        ui->labelTextThresholdCalibr->setStyleSheet(neutralStyle);
        ui->labelTextADCdelay       ->setStyleSheet(neutralStyle);
        ui->labelTextCFDthreshold   ->setStyleSheet(neutralStyle);
        ui->labelTextADCzero        ->setStyleSheet(neutralStyle);
        ui->labelTextCFDzero        ->setStyleSheet(neutralStyle);
        ui->labelTextADC0           ->setStyleSheet(neutralStyle);
        ui->labelTextADC1           ->setStyleSheet(neutralStyle);
    }

    explicit MainWindow(QWidget *parent = nullptr):
        QMainWindow(parent),
        settings(QCoreApplication::applicationName() + ".ini", QSettings::IniFormat),
        FEE(getSubdetectorTypeByName( settings.value("subdetector").toString() )),
        ui(new Ui::MainWindow)
    {
        ui->setupUi(this);

//initialization and lists creation
        setWindowTitle(QCoreApplication::applicationName() + " v" + QCoreApplication::applicationVersion() + ": " + FIT[FEE.subdetector].name + " settings");
        curFEEid = FEE.TCMid;
        applyButtons = ui->centralWidget->findChildren<QPushButton *>(QRegularExpression("buttonApply.*"));
        switchBitButtons = ui->groupBoxLaser->findChildren<QPushButton *>(QRegularExpression("buttonSwitchBit.*")).toVector();
        selectorsPMA = ui->groupBoxBoardSelection->findChildren<QPushButton *>(QRegularExpression("PM_selector_A[0-9]")).toVector();
        selectorsPMC = ui->groupBoxBoardSelection->findChildren<QPushButton *>(QRegularExpression("PM_selector_C[0-9]")).toVector();
        linksPMA = ui->groupBoxBoardSelection->findChildren<QLabel *>(QRegularExpression("labelIconLinkOK_A[0-9]")).toVector();
        linksPMC = ui->groupBoxBoardSelection->findChildren<QLabel *>(QRegularExpression("labelIconLinkOK_C[0-9]")).toVector();
        switchesPMA = ui->groupBoxBoardSelection->findChildren<Switch *>(QRegularExpression("Switcher_A[0-9]")).toVector();
        switchesPMC = ui->groupBoxBoardSelection->findChildren<Switch *>(QRegularExpression("Switcher_C[0-9]")).toVector();
        switchesCh = ui->groupBoxChannels->findChildren<Switch *>(QRegularExpression("SwitcherCh_(0[1-9]|1[0-2])")).toVector();
        noTRGCh = ui->groupBoxChannels->findChildren<QCheckBox *>(QRegularExpression("checkBoxNoTRG_(0[1-9]|1[0-2])")).toVector();
        labelsTriggersCount = {
            ui->labelValueTriggersCount_5 ,
            ui->labelValueTriggersCount_4 ,
            ui->labelValueTriggersCount_2 ,
            ui->labelValueTriggersCount_1 ,
            ui->labelValueTriggersCount_3 ,
            ui->labelValueTriggersCount_75,
            ui->labelValueTriggersCount_76,
            ui->labelValueTriggersCount_77,
            ui->labelValueTriggersCount_78,
            ui->labelValueTriggersCount_79,
            ui->labelValueTriggersCount_7A,
            ui->labelValueTriggersCount_7B,
            ui->labelValueTriggersCount_7C,
            ui->labelValueTriggersCount_7D,
            ui->labelValueTriggersCount_7E,
        };
        labelsTriggersRate = {
			ui->labelValueTriggersRate_5  ,
			ui->labelValueTriggersRate_4  ,
			ui->labelValueTriggersRate_2  ,
			ui->labelValueTriggersRate_1  ,
			ui->labelValueTriggersRate_3  ,
            ui->labelValueTriggersRate_75 ,
            ui->labelValueTriggersRate_76 ,
            ui->labelValueTriggersRate_77 ,
            ui->labelValueTriggersRate_78 ,
            ui->labelValueTriggersRate_79 ,
            ui->labelValueTriggersRate_7A ,
            ui->labelValueTriggersRate_7B ,
            ui->labelValueTriggersRate_7C ,
            ui->labelValueTriggersRate_7D ,
            ui->labelValueTriggersRate_7E ,
        };
        labelsTimeAlignmentCh      = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueTimeAlignment"    "_(0[1-9]|1[0-2])")).toVector();
        labelsThresholdCalibrCh    = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueThresholdCalibr"  "_(0[1-9]|1[0-2])")).toVector();
        labelsADCdelayCh           = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueADCdelay"         "_(0[1-9]|1[0-2])")).toVector();
        labelsCFDthresholdCh       = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueCFDthreshold"     "_(0[1-9]|1[0-2])")).toVector();
        labelsADCzeroCh            = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueADCzero"          "_(0[1-9]|1[0-2])")).toVector();
        labelsCFDzeroCh            = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueCFDzero"          "_(0[1-9]|1[0-2])")).toVector();
        labelsADC0rangeCh          = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueADC0range"        "_(0[1-9]|1[0-2])")).toVector();
        labelsADC1rangeCh          = ui->groupBoxChannels->findChildren<ActualLabel *>(QRegularExpression("labelValueADC1range"        "_(0[1-9]|1[0-2])")).toVector();
        labelsADC0baseLineCh       = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueADC0baseLine"     "_(0[1-9]|1[0-2])")).toVector();
        labelsADC1baseLineCh       = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueADC1baseLine"     "_(0[1-9]|1[0-2])")).toVector();
        labelsADC0RMSCh            = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueADC0RMS"          "_(0[1-9]|1[0-2])")).toVector();
        labelsADC1RMSCh            = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueADC1RMS"          "_(0[1-9]|1[0-2])")).toVector();
        labelsADC0meanAmplitudeCh  = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueADC0meanAmplitude""_(0[1-9]|1[0-2])")).toVector();
        labelsADC1meanAmplitudeCh  = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueADC1meanAmplitude""_(0[1-9]|1[0-2])")).toVector();
        labelsRawTDCdata1Ch        = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueRawTDCdata1"      "_(0[1-9]|1[0-2])")).toVector();
        labelsRawTDCdata2Ch        = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueRawTDCdata2"      "_(0[1-9]|1[0-2])")).toVector();
        labelsTRGcounterCh         = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueTRGcounter"       "_(0[1-9]|1[0-2])")).toVector();
        labelsCFDcounterCh         = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueCFDcounter"       "_(0[1-9]|1[0-2])")).toVector();
        labelsTRGcounterRateCh     = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueTRGcounterRate"   "_(0[1-9]|1[0-2])")).toVector();
        labelsCFDcounterRateCh     = ui->groupBoxChannels->findChildren<QLabel *>     (QRegularExpression("labelValueCFDcounterRate"   "_(0[1-9]|1[0-2])")).toVector();
        editsTimeAlignmentCh       = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditTimeAlignment"      "_(0[1-9]|1[0-2])")).toVector();
        editsThresholdCalibrCh     = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditThresholdCalibr"    "_(0[1-9]|1[0-2])")).toVector();
        editsADCdelayCh            = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditADCdelay"           "_(0[1-9]|1[0-2])")).toVector();
        editsCFDthresholdCh        = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditCFDthreshold"       "_(0[1-9]|1[0-2])")).toVector();
        editsADCzeroCh             = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditADCzero"            "_(0[1-9]|1[0-2])")).toVector();
        editsCFDzeroCh             = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditCFDzero"            "_(0[1-9]|1[0-2])")).toVector();
        editsADC0rangeCh           = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditADC0range"          "_(0[1-9]|1[0-2])")).toVector();
        editsADC1rangeCh           = ui->groupBoxChannels->findChildren<QLineEdit *>  (QRegularExpression("lineEditADC1range"          "_(0[1-9]|1[0-2])")).toVector();
        buttonsTimeAlignmentCh     = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyTimeAlignment"   "_(0[1-9]|1[0-2])")).toVector();
        buttonsThresholdCalibrCh   = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyThresholdCalibr" "_(0[1-9]|1[0-2])")).toVector();
        buttonsADCdelayCh          = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyADCdelay"        "_(0[1-9]|1[0-2])")).toVector();
        buttonsCFDthresholdCh      = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyCFDthreshold"    "_(0[1-9]|1[0-2])")).toVector();
        buttonsADCzeroCh           = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyADCzero"         "_(0[1-9]|1[0-2])")).toVector();
        buttonsCFDzeroCh           = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyCFDzero"         "_(0[1-9]|1[0-2])")).toVector();
        buttonsADC0rangeCh         = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyADC0range"       "_(0[1-9]|1[0-2])")).toVector();
        buttonsADC1rangeCh         = ui->groupBoxChannels->findChildren<QPushButton *>(QRegularExpression("buttonApplyADC1range"       "_(0[1-9]|1[0-2])")).toVector();
        allRadioButtons = {
            ui->radioButtonAandC          ,
            ui->radioButtonC              ,
            ui->radioButtonA              ,
            ui->radioButtonSum            ,
            ui->radioButtonAuto           ,
            ui->radioButtonForceLocal     ,
            ui->radioButtonExternalTrigger,
            ui->radioButtonGenerator
        };
        allButtons    = ui->centralWidget->findChildren<QPushButton *>(QRegularExpression("button.*")); //except PM selectors
        allSwitches   = ui->centralWidget->findChildren<Switch *>();
        allLineEdits  = ui->centralWidget->findChildren<QLineEdit *>();
        allComboBoxes = ui->centralWidget->findChildren<QComboBox *>();
        allSpinBoxes  = ui->centralWidget->findChildren<QAbstractSpinBox *>();
        allWidgets    = ui->centralWidget->findChildren<QWidget *>();
        foreach(QPushButton *b, applyButtons) b->setToolTip("Apply");
        foreach(QWidget *w, QList<QWidget *>({
            ui->  lineEditDataSelectTriggerMask,
            ui->labelValueDGtriggerRespondMask ,
            ui->  lineEditDGtriggerRespondMask ,
            ui->   labelValueTGcontinuousValue ,
            ui->     lineEditTGcontinuousValue ,
        })) {
            w->setToolTip(ui->labelValueDataSelectTriggerMask->toolTip());
            w->setToolTipDuration(INT_MAX);
        }
        ui->groupBoxPM->hide();
        ui->labelTextEarlyHeader->hide();
        ui->labelIconEarlyHeader->hide();
        ui->labelTextTriggers_1->setText(QString("1: ") + FIT[FEE.subdetector].triggers[0].name);
        ui->labelTextTriggers_2->setText(QString("2: ") + FIT[FEE.subdetector].triggers[1].name);
        ui->labelTextTriggers_3->setText(QString("3: ") + FIT[FEE.subdetector].triggers[2].name);
        ui->labelTextTriggers_4->setText(QString("4: ") + FIT[FEE.subdetector].triggers[3].name);
        ui->labelTextTriggers_5->setText(QString("5: ") + FIT[FEE.subdetector].triggers[4].name);
        if (FEE.subdetector == FV0) {
            ui-> labelValueTriggersLevelC_2->move(ui-> labelValueTriggersLevelA_1 ->x(), 30 * 3 + 1);
            ui-> labelValueTriggersLevelC_1->move(ui-> labelValueTriggersLevelA_1 ->x(), 30 * 4 + 1);
            ui->buttonApplyTriggersLevelC_2->move(ui->buttonApplyTriggersLevelA_1 ->x(), 30 * 3 + 1);
            ui->buttonApplyTriggersLevelC_1->move(ui->buttonApplyTriggersLevelA_1 ->x(), 30 * 4 + 1);
            ui->   lineEditTriggersLevelC_2->move(ui->   lineEditTriggersLevelA_1 ->x(), 30 * 3 + 2);
            ui->   lineEditTriggersLevelC_1->move(ui->   lineEditTriggersLevelA_1 ->x(), 30 * 4 + 2);
            foreach (QWidget *w, QList<QWidget *>({
                ui->groupBoxCentralityMode       ,
                ui->labelTextVertexTimeThresholds,
                ui->labelTextVertexTimeLow       ,
                ui->labelTextVertexTimeHigh      ,
                ui->labelValueVertexTimeLow      ,
                ui->labelValueVertexTimeHigh     ,
                ui->lineEditVertexTimeLow        ,
                ui->lineEditVertexTimeHigh       ,
                ui->buttonApplyVertexTimeLow     ,
                ui->buttonApplyVertexTimeHigh    ,
                ui->labelTextTriggersLevelC      ,
                ui->buttonSCcharge               ,
                ui->buttonSCNchan                ,
            })) w->hide();
            foreach(QWidget *w, ui->groupBoxSecondaryCounters->findChildren<QWidget *>(QRegularExpression("label.*_7[679A-E]")) + QList<QWidget *>({
                ui->groupBoxSide_C              ,
                ui->labelTextTriggersCount_1    ,
                ui->labelTextTriggersRate_1     ,
                ui->labelTextAverageTime_A      ,
                ui->SwitcherAddCdelay           ,
                ui->labelTextAddCdelay
            })) w->setEnabled(false);
        }

        resize(width(), 10 + ui->groupBoxBoardSelection->height()+ui->groupBoxTCM->height()+ui->statusBar->height());
//initial scaling (a label with fontsize 10 (Calibri) has pixelsize of 13 without system scaling and e.g. 20 at 150% scaling so widgets size should be recalculated)
        fontSize_px = ui->labelTextPMOK->fontInfo().pixelSize();
        if (fontSize_px > 13) { //not default pixelsize for font of size 10
            double f = fontSize_px / 13.;
            resize(size()*f); //mainWindow
            foreach (QWidget *w, allWidgets) { w->resize(w->size()*f); w->move(w->pos()*f); }
            foreach (QPushButton *b,     applyButtons) b->setIconSize(b->iconSize()*f);
            foreach (QPushButton *b, switchBitButtons) b->setIconSize(b->iconSize()*f);
        }
        setFixedSize(size());

//menus
        QMenu *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction(QIcon(":/save.png"), "&Save settings to...", this, SLOT(save()), QKeySequence::Save);
        QAction *actionLoad = new QAction(QIcon(":/load.png"), "&Load settings from...", this);
        actionLoad->setShortcut(QKeySequence::Open);
        fileMenu->addAction(actionLoad);
        connect(actionLoad, &QAction::triggered, this, &MainWindow::load);
        actionLoad->setDisabled(false);
        QAction *actionLoadApply = new QAction(QIcon(":/load.png"), "&Load and apply...", this);
        actionLoad->setShortcut(QKeySequence::Open);
        fileMenu->addAction(actionLoadApply);
        connect(actionLoadApply, &QAction::triggered, this, [=]() { load(true); } );
        actionLoadApply->setDisabled(false);
        QMenu *controlMenu = menuBar()->addMenu("&Control");
        enableControls = new QAction(QIcon(":/controls.png"), "Disable", this);
        enableControls->setCheckable(true);
        controlMenu->addAction(enableControls);
        controlMenu->addAction("Copy ALL actual values to settings", this, [=]() { FEE.copyActualToSettingsAll(); updateEdits(); });
        QAction *applyAll = controlMenu->addAction(QIcon(":/write.png"), "Apply ALL settings to FEE", &FEE, &FITelectronics::applySettingsAll);
		QAction *resetAllPMsCount = controlMenu->addAction("Reset count in all PMs", this, [=]() { FEE.resetCounts(-2); });
		QAction *resetAllBoardsCount = controlMenu->addAction("Reset count in all PMs and TCM", this, [=]() { FEE.resetCounts(-1); });
        connect(enableControls, &QAction::triggered, this, [=](bool checked) {
            enableControls->setText(checked ? "Disable" : "Enable");
            foreach (QLineEdit        *e, allLineEdits   ) e->setEnabled(checked);
            foreach (QPushButton      *b, allButtons     ) b->setEnabled(checked);
            foreach (Switch           *s, allSwitches    ) s->setEnabled(checked);
            foreach (QComboBox        *c, allComboBoxes  ) c->setEnabled(checked);
            foreach (QAbstractSpinBox *a, allSpinBoxes   ) a->setEnabled(checked);
            foreach (QRadioButton     *r, allRadioButtons) r->setEnabled(checked);
            foreach (QCheckBox        *c, noTRGCh        ) c->setEnabled(checked);
            ui->sliderLaser->setEnabled(checked);
            ui->sliderAttenuation->setEnabled(checked);
            actionLoadApply->setEnabled(checked);
            applyAll->setEnabled(checked);
			resetAllPMsCount->setEnabled(checked);
			resetAllBoardsCount->setEnabled(checked);
        });
        QMenu *networkMenu = menuBar()->addMenu("&Network");
        networkMenu->addAction(QIcon(":/recheck.png"), "&Recheck and default", this, SLOT(recheckTarget()), QKeySequence::Refresh);
        networkMenu->addAction("&Change target IP address...", this, SLOT(changeIP()));
        QAction *enableDebugActions = new QAction("Activate debug menu", this);
        networkMenu->addAction(enableDebugActions);
        enableDebugActions->setShortcut(QKeySequence("Ctrl+!"));
        connect(enableDebugActions, &QAction::triggered, this, [=]() {
            networkMenu->removeAction(enableDebugActions);
            QMenu *debugMenu = menuBar()->addMenu("&Debug");
            debugMenu->addAction("Adjust &PM treshholds", this, [=]() { //decrease thresholds to noise levels to see counting without signals
                if (curFEEid == FEE.TCMid) QMessageBox::warning(this, "Warning", "This operation is not applicable for TCM!");
                else FEE.adjustThresholds(curPM, QInputDialog::getDouble(this, "Adjusting " + ui->groupBoxPM->title() + " thresholds", "Set CFD hits target rate", 20, 1, 1e6, 0)); }
            );
            debugMenu->addAction("Randomize OrbitFillMask", this, [=]() { for (quint8 i=0; i<213; ++i) FEE.TCM.ORBIT_FILL_MASK[i] = QRandomGenerator::global()->generate(); FEE.apply_ORBIT_FILL_MASK(); });
            QAction *shuttleLaser = new QAction("Start laser phase shuttling");
            connect(&shuttleTimer, &QTimer::timeout, this, [&]() {
                qint16 start = FEE.shuttleStartPhase, end = -start;
                double r = FEE.shuttleTimer->remainingTime() / 2100.;
                ui->sliderLaser->setValue( qRound(start * r + end * (1-r)) );
            });
            connect(shuttleLaser, &QAction::triggered, this, [=]() {
                if (laserIsShuttling) {
                    this->shuttleTimer.stop();
                    this->FEE.shuttleTimer->stop();
                    FEE.TCM.set.LASER_DELAY = this->settings.value("LASER_DELAY", FEE.TCM.set.LASER_DELAY).toInt();
                } else {
                    this->settings.setValue("LASER_DELAY", FEE.TCM.set.LASER_DELAY);
                    this->FEE.inverseLaserPhase();
                    this->FEE.shuttleTimer->start(2100);
                    this->shuttleTimer.start(100);
                }
                this->laserIsShuttling = !this->laserIsShuttling;
                this->ui->spinBoxLaserPhase->setDisabled(laserIsShuttling);
                this->ui->sliderLaser->setDisabled(laserIsShuttling);
                this->ui->buttonApplyLaserPhase->setDisabled(laserIsShuttling);
                shuttleLaser->setText(laserIsShuttling ? "Stop shuttling laser phase" : "Start shuttling laser phase");
            });
            debugMenu->addAction(shuttleLaser);
        });

//signal-slot conections
        connect(&FEE, &IPbusTarget::IPbusStatusOK, this, [=]() {
            ui->centralWidget->setEnabled(true);
        });
        connect(&FEE, &IPbusTarget::noResponse, this, [=](QString message) {
			ui->centralWidget->setDisabled(true);
			QString msg = FEE.IPaddress + ": " + message;
			if (FEE.updateTimer->isActive()) statusBar()->showMessage(statusBar()->currentMessage() == msg ? "" : msg);
        });
        connect(&FEE, &FITelectronics::valuesReady, this, [=]() {
			QString msg = FEE.IPaddress + ": online";
			if (FEE.updateTimer->isActive()) statusBar()->showMessage(statusBar()->currentMessage() == msg ? "" : msg);
            updateActualValues();
            if (!enableControls->isChecked()) updateEdits();
        });
		connect(&FEE, &IPbusTarget::error, this, [=](QString message, errorType et) {
//            QMessageBox::warning(this, errorTypeName[et], message);
			ui->centralWidget->setDisabled(true);
			statusBar()->showMessage(message + " (" + errorTypeName[et] + ")");
		});
        connect(&FEE, &FITelectronics::countersReady, this, &MainWindow::updateCounters);
		connect(ui->labelValueORgate_A,         &ActualLabel::doubleclicked, this, [=](QString val) { double v = val.toDouble(&ok); if (ok) ui->spinBoxORgate_A->setValue(v); });
        connect(ui->labelValueORgate_C,         &ActualLabel::doubleclicked, this, [=](QString val) { double v = val.toDouble(&ok); if (ok) ui->spinBoxORgate_C->setValue(v); });
		connect(ui->labelValueAverageTime_A,    &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxPhase_A         ->setValue(ui->labelValuePhase_A->text().toDouble() + val.toDouble()); });
		connect(ui->labelValueAverageTime_C,    &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxPhase_C         ->setValue(ui->labelValuePhase_C->text().toDouble() + val.toDouble()); });
		connect(ui->labelValuePhase_A,          &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxPhase_A         ->setValue(val.toDouble(     )); });
        connect(ui->labelValuePhase_C,          &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxPhase_C         ->setValue(val.toDouble(     )); });
		connect(ui->labelValueLaserPhase,       &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxLaserPhase      ->setValue(val.toDouble(     )); });
        connect(ui->labelValueAttenuation,      &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxAttenuation     ->setValue(val.toDouble(     )); });
        connect(ui->labelValueLaserFreqDivider, &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxLaserFreqDivider->setValue(val.toUInt(&ok, 16)); });
        connect(ui->labelValueSuppressDuration, &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxSuppressDuration->setValue(val.toUInt()       ); });
        connect(ui->labelValueSuppressDelayBC , &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxSuppressDelayBC ->setValue(val.toUInt()       ); });

        foreach (ActualLabel *label, ui->centralWidget->findChildren<ActualLabel *>()) {
            QLineEdit *e = ui->centralWidget->findChild<QLineEdit *>(label->objectName().replace("labelValue", "lineEdit"));
            if (e != nullptr) connect(label, &ActualLabel::doubleclicked, [=](QString text) { e->setText( text.right(e->maxLength()) ); emit e->textEdited(text); });
        }

        connect(&FEE, &FITelectronics::linksStatusReady, this, [=]() { //disable PMs' selectors and link indicators if no physical link present
            //if (!isTCM() && !FEE.PM.contains(curPM->FEEid)) ui->TCM_selector->toggle();
            for (quint8 i=0; i<=9; ++i) {
                selectorsPMA[i]->setEnabled(FEE.PM.contains(FEE.allPMs[i   ].FEEid));
                selectorsPMC[i]->setEnabled(FEE.PM.contains(FEE.allPMs[i+10].FEEid));
            }
        });
        for (quint8 i=0; i<=9; ++i) { //PM switchers and selectors
            connect(switchesPMA[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) { FEE.switchTRGsyncPM(i     , !checked); });
            connect(switchesPMC[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) { FEE.switchTRGsyncPM(i + 10, !checked); });
            connect(selectorsPMA[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) { if (checked) selectPM((FEE.allPMs + i     )->FEEid); });
            connect(selectorsPMC[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) { if (checked) selectPM((FEE.allPMs + i + 10)->FEEid); });
        }

        for (quint8 i=0; i<12; ++i) { //channels
            connect(editsTimeAlignmentCh  [i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.TIME_ALIGN[i].value = text. toInt(); resetHighlight(); ui->labelTextTimeAlignment  ->setStyleSheet(highlightStyle); });
            connect(editsThresholdCalibrCh[i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.THRESHOLD_CALIBR[i] = text.toUInt(); resetHighlight(); ui->labelTextThresholdCalibr->setStyleSheet(highlightStyle); });
            connect(editsADCdelayCh       [i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.Ch[i].ADC_DELAY     = text.toUInt(); resetHighlight(); ui->labelTextADCdelay       ->setStyleSheet(highlightStyle); });
            connect(editsCFDthresholdCh   [i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.Ch[i].CFD_THRESHOLD = text.toUInt(); resetHighlight(); ui->labelTextCFDthreshold   ->setStyleSheet(highlightStyle); });
            connect(editsADCzeroCh        [i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.Ch[i].ADC_ZERO      = text. toInt(); resetHighlight(); ui->labelTextADCzero        ->setStyleSheet(highlightStyle); });
            connect(editsCFDzeroCh        [i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.Ch[i].CFD_ZERO      = text. toInt(); resetHighlight(); ui->labelTextCFDzero        ->setStyleSheet(highlightStyle); });
            connect(editsADC0rangeCh      [i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.ADC_RANGE    [i][0] = text.toUInt(); resetHighlight(); ui->labelTextADC0           ->setStyleSheet(highlightStyle); });
            connect(editsADC1rangeCh      [i], &QLineEdit::textEdited, this, [=](QString text) { curPM->set.ADC_RANGE    [i][1] = text.toUInt(); resetHighlight(); ui->labelTextADC1           ->setStyleSheet(highlightStyle); });

            connect(editsTimeAlignmentCh  [i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextTimeAlignment  ->setStyleSheet(highlightStyle); });
            connect(editsThresholdCalibrCh[i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextThresholdCalibr->setStyleSheet(highlightStyle); });
            connect(editsADCdelayCh       [i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextADCdelay       ->setStyleSheet(highlightStyle); });
            connect(editsCFDthresholdCh   [i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextCFDthreshold   ->setStyleSheet(highlightStyle); });
            connect(editsADCzeroCh        [i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextADCzero        ->setStyleSheet(highlightStyle); });
            connect(editsCFDzeroCh        [i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextCFDzero        ->setStyleSheet(highlightStyle); });
            connect(editsADC0rangeCh      [i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextADC0           ->setStyleSheet(highlightStyle); });
            connect(editsADC1rangeCh      [i], &QLineEdit::cursorPositionChanged, this, [=]() { resetHighlight(); ui->labelTextADC1           ->setStyleSheet(highlightStyle); });
            connect(editsTimeAlignmentCh  [i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });
            connect(editsThresholdCalibrCh[i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });
            connect(editsADCdelayCh       [i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });
            connect(editsCFDthresholdCh   [i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });
            connect(editsADCzeroCh        [i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });
            connect(editsCFDzeroCh        [i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });
            connect(editsADC0rangeCh      [i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });
            connect(editsADC1rangeCh      [i], &QLineEdit::editingFinished      , this, [=]() { resetHighlight(); });

            connect(buttonsTimeAlignmentCh  [i], &QPushButton::clicked, this, [=] { curPM->set.TIME_ALIGN[i].value = editsTimeAlignmentCh  [i]->text(). toInt(); FEE.apply_TIME_ALIGN      (curFEEid, i+1); });
            connect(buttonsThresholdCalibrCh[i], &QPushButton::clicked, this, [=] { curPM->set.THRESHOLD_CALIBR[i] = editsThresholdCalibrCh[i]->text().toUInt(); FEE.apply_THRESHOLD_CALIBR(curFEEid, i+1); });
            connect(buttonsADCdelayCh       [i], &QPushButton::clicked, this, [=] { curPM->set.Ch[i].ADC_DELAY     = editsADCdelayCh       [i]->text().toUInt(); FEE.apply_ADC_DELAY       (curFEEid, i+1); });
            connect(buttonsCFDthresholdCh   [i], &QPushButton::clicked, this, [=] { curPM->set.Ch[i].CFD_THRESHOLD = editsCFDthresholdCh   [i]->text().toUInt(); FEE.apply_CFD_THRESHOLD   (curFEEid, i+1); });
            connect(buttonsADCzeroCh        [i], &QPushButton::clicked, this, [=] { curPM->set.Ch[i].ADC_ZERO      = editsADCzeroCh        [i]->text(). toInt(); FEE.apply_ADC_ZERO        (curFEEid, i+1); });
            connect(buttonsCFDzeroCh        [i], &QPushButton::clicked, this, [=] { curPM->set.Ch[i].CFD_ZERO      = editsCFDzeroCh        [i]->text(). toInt(); FEE.apply_CFD_ZERO        (curFEEid, i+1); });
            connect(buttonsADC0rangeCh      [i], &QPushButton::clicked, this, [=] { curPM->set.ADC_RANGE    [i][0] = editsADC0rangeCh      [i]->text().toUInt(); FEE.apply_ADC0_RANGE      (curFEEid, i+1); });
            connect(buttonsADC1rangeCh      [i], &QPushButton::clicked, this, [=] { curPM->set.ADC_RANGE    [i][1] = editsADC1rangeCh      [i]->text().toUInt(); FEE.apply_ADC1_RANGE      (curFEEid, i+1); });

            connect(switchesCh[i], &Switch::clicked, this, [=](bool checked) { FEE.switchPMchannel     (curPM - FEE.allPMs, i + 1, !checked); });
            connect(noTRGCh   [i], &Switch::clicked, this, [=](bool checked) { FEE.apply_PMchannelNoTRG(curPM - FEE.allPMs, i + 1,  checked); });
        }
        for (quint8 i=0; i<64; ++i) { //laser pattern bits switching
			connect(switchBitButtons[i], &QPushButton::clicked, this, [=](bool checked) { FEE.apply_SwLaserPatternBit(i, checked); ui->lineEditLaserPattern->setText(QString::asprintf("%016llX", FEE.TCM.set.LASER_PATTERN)); });
        }
//validators
        ui->lineEditLaserFrequency->setValidator(doubleValidator);
        ui->lineEditTriggersLevelA_1->setValidator(new StrictIntValidator(   0, 65535, this));
        ui->lineEditTriggersLevelA_2->setValidator(new StrictIntValidator(   0, 65535, this));
        ui->lineEditTriggersLevelC_1->setValidator(new StrictIntValidator(   0, 65535, this));
        ui->lineEditTriggersLevelC_2->setValidator(new StrictIntValidator(   0, 65535, this));
        ui->lineEditVertexTimeLow   ->setValidator(new StrictIntValidator(-512,   511, this));
        ui->lineEditVertexTimeHigh  ->setValidator(new StrictIntValidator(-512,   511, this));
        ui->lineEditBCIDdelayDec    ->setValidator(new StrictIntValidator(   0,  4095, this));
        ui->lineEditORgate          ->setValidator(new StrictIntValidator(   0,   255, this));
        ui->lineEditTGHBrRate       ->setValidator(new StrictIntValidator(   0,    15, this));
        ui->lineEditChargeHi        ->setValidator(new StrictIntValidator(   0,  4095, this));
        ui->lineEditChargeLo        ->setValidator(new StrictIntValidator(   0,    15, this));
        foreach (QLineEdit *e, editsTimeAlignmentCh               ) e->setValidator(new StrictIntValidator(-2048,  2047, this));
        foreach (QLineEdit *e, editsThresholdCalibrCh             ) e->setValidator(new StrictIntValidator(    0,  4000, this));
        foreach (QLineEdit *e, editsADCdelayCh                    ) e->setValidator(new StrictIntValidator(    0, 20000, this));
        foreach (QLineEdit *e, editsCFDthresholdCh                ) e->setValidator(new      QIntValidator(  300, 30000, this));
        foreach (QLineEdit *e, editsADCzeroCh   + editsCFDzeroCh  ) e->setValidator(new StrictIntValidator( -500,   500, this));
        foreach (QLineEdit *e, editsADC0rangeCh + editsADC1rangeCh) e->setValidator(new StrictIntValidator(    0,  4095, this));

//read settings
        QString IPaddress = settings.value("IPaddress", FEE.IPaddress).toString();
        if (validIPaddressRE.exactMatch(IPaddress)) FEE.IPaddress = IPaddress;
        FEE.fileRead(QCoreApplication::applicationName() + ".ini");
        updateEdits();
        FEE.reconnect();
        resetHighlight();
		enableControls->setChecked(true);
//		emit enableControls->triggered(true);
    }

    ~MainWindow() {
        settings.clear();
        settings.setValue("IPaddress", FEE.IPaddress);
        settings.setValue("subdetector", FIT[FEE.subdetector].name);
        FEE.fileWrite(QCoreApplication::applicationName() + ".ini");
        delete ui;
    }

public slots:
    void load(bool doApply = false) {
        QFileDialog dialog(this);
        dialog.setWindowModality(Qt::WindowModal);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (dialog.exec() != QDialog::Accepted)
            statusBar()->showMessage("File not loaded");
        else {
            FEE.fileRead(dialog.selectedFiles().first(), doApply);
            updateEdits();
            if (!doApply) statusBar()->showMessage("File loaded");
        }
    }

    void save() {
        QFileDialog dialog(this);
        dialog.setWindowModality(Qt::WindowModal);
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec() != QDialog::Accepted)
            statusBar()->showMessage("File not saved");
        else {
            FEE.fileWrite(dialog.selectedFiles().first());
            statusBar()->showMessage("File saved");
        }
    }

    void recheckTarget() {
        statusBar()->showMessage(FEE.IPaddress + ": status requested...");
        if (FEE.countersTimer->isActive()) FEE.countersTimer->stop();
        FEE.reconnect();
    }

    void changeIP() {
        QString text = QInputDialog::getText(this, "Changing target", "Enter new target's IP address", QLineEdit::Normal, FEE.IPaddress, &ok);
        if (ok && !text.isEmpty()) {
            if (validIPaddressRE.exactMatch(text)) {
                FEE.IPaddress = text;
                FEE.reconnect();
            } else QMessageBox::warning(this, "Warning", text + ": invalid IP address. Continue with previous target");
        }
    }

    void updateActualValues() {
        for (quint8 i=0; i<=9; ++i) {
            linksPMA[i]->setPixmap(FEE.allPMs[i   ].isOK() ? (FEE.allPMs[i   ].GBTisOK() ? Green1 : Red0) : RedDash); switchesPMA[i]->setChecked(FEE.TCM.act.CH_MASK_A & (1 << i));
            linksPMC[i]->setPixmap(FEE.allPMs[i+10].isOK() ? (FEE.allPMs[i+10].GBTisOK() ? Green1 : Red0) : RedDash); switchesPMC[i]->setChecked(FEE.TCM.act.CH_MASK_C & (1 << i));
        }
        ui->labelIconSystemRestarted->setPixmap(FEE.TCM.act.systemRestarted ? Red1 : Green0);
        ui->labelIconSystemRestarting->setPixmap(FEE.TCM.act.resetSystem ? Red1 : Green0);
//        ok = true;
//        foreach (TypePM *pm, FEE.PM) if (!pm->isOK() || !pm->GBTisOK()) { ok = false; break; }
//        ui->labelIconSystemErrors->setPixmap(ok && FEE.TCM.isOK() && FEE.TCM.act.GBT.isOK() ? Green0 : Red1);
        ui->labelIconSystemErrors->setPixmap(FEE.BOARDS_OK >> 20 && (FEE.TCM.act.PM_MASK_TRG() & FEE.BOARDS_OK) == FEE.TCM.act.PM_MASK_TRG() ? Green0 : Red1);
		ui->TCM_selector->setStyleSheet(FEE.TCM.isOK() && FEE.TCM.GBTisOK() ? "" : notOKstyle);
        ui->labelValueClockSource->setText(FEE.TCM.act.externalClock ? "external" : (FEE.TCM.act.forceLocalClock ? "force local" : "local"));
        ui->labelValueClockSource->setStyleSheet(FEE.TCM.act.externalClock ? OKstyle : (FEE.TCM.act.forceLocalClock ? neutralStyle : notOKstyle));
        double
            curTemp_board   = isTCM() ? FEE.TCM.act.TEMP_BOARD    : curPM->act.TEMP_BOARD,
            curTemp_FPGA    = isTCM() ? FEE.TCM.act.TEMP_FPGA     : curPM->act.TEMP_FPGA ,
            curVoltage_1V   = isTCM() ? FEE.TCM.act.VOLTAGE_1V    : curPM->act.VOLTAGE_1V,
            curVoltage_1_8V = isTCM() ? FEE.TCM.act.VOLTAGE_1_8V  : curPM->act.VOLTAGE_1_8V;
        ui->labelValueBoardTemperature->setText(QString::asprintf("%4.1f°C", curTemp_board   ));
        ui->labelValueFPGAtemperature ->setText(QString::asprintf("%5.1f°C", curTemp_FPGA    ));
        ui->labelValueVoltage1V       ->setText(QString::asprintf("%5.3f V", curVoltage_1V   ));
        ui->labelValueVoltage1_8V     ->setText(QString::asprintf("%5.3f V", curVoltage_1_8V ));
        ui->labelValueBoardTemperature->setStyleSheet(fabs(curTemp_board      - 35) > 25  ? notOKstyle : neutralStyle);
        ui->labelValueFPGAtemperature ->setStyleSheet(fabs(curTemp_FPGA       - 35) > 25  ? notOKstyle : neutralStyle);
        ui->labelValueVoltage1V       ->setStyleSheet(fabs(curVoltage_1V  /1.0 - 1) > 0.2 ? notOKstyle : neutralStyle);
        ui->labelValueVoltage1_8V     ->setStyleSheet(fabs(curVoltage_1_8V/1.8 - 1) > 0.2 ? notOKstyle : neutralStyle);

        ui->labelValueSerial          ->setText(QString::asprintf("%d"     , isTCM() ? FEE.TCM.act.SERIAL_NUM : curPM->act.SERIAL_NUM));
        TypeFITsubdetector bt = TypeFITsubdetector(isTCM() ? FEE.TCM.act.boardType : curPM->act.boardType);
        ui->labelValueBoardType->setText(QString::asprintf("%d: %s", bt, FIT[bt].name));
        ui->labelValueBoardType->setStyleSheet(bt != FEE.subdetector ? notOKstyle : neutralStyle);
        Timestamp tMCU  = isTCM() ? FEE.TCM.act.FW_TIME_MCU  : curPM->act.FW_TIME_MCU;
        Timestamp tFPGA = isTCM() ? FEE.TCM.act.FW_TIME_FPGA : curPM->act.FW_TIME_FPGA;
        ui->labelValueMCUFWversion ->setText(tMCU .printCode1());
        ui->labelValueFPGAFWversion->setText(tFPGA.printCode1());
        QString tMCUfull  = tMCU .printFull();
        QString tFPGAfull = tFPGA.printFull();
        ui->labelTextMCUFWversion ->setToolTip(tMCUfull);
        ui->labelValueMCUFWversion->setToolTip(tMCUfull);
        ui->labelTextFPGAFWversion ->setToolTip(tFPGAfull);
        ui->labelValueFPGAFWversion->setToolTip(tFPGAfull);
        ui->comboBoxUpdatePeriod->setCurrentIndex(FEE.TCM.act.COUNTERS_UPD_RATE);
        switch (curGBTact->Control.DG_MODE) {
            case GBTunit::DG_noData: ui->buttonDataGeneratorOff ->setChecked(true); break;
            case GBTunit::DG_main  : ui->buttonDataGeneratorMain->setChecked(true); break;
            case GBTunit::DG_Tx    : ui->buttonDataGeneratorTx  ->setChecked(true);
        }
        switch (curGBTact->Control.TG_MODE) {
            case GBTunit::TG_noTrigger : ui->buttonTriggerGeneratorOff		 ->setChecked(true); break;
            case GBTunit::TG_continuous: ui->buttonTriggerGeneratorContinuous->setChecked(true); break;
            case GBTunit::TG_Tx        : ui->buttonTriggerGeneratorTx        ->setChecked(true);
        }
        ui->labelValueDGtriggerRespondMask  ->setText(QString::asprintf("0x%08X" , curGBTact->Control.DG_TRG_RESPOND_MASK));
        ui->labelValueDGbunchPattern        ->setText(QString::asprintf("0x%08X" , curGBTact->Control.DG_BUNCH_PATTERN   ));
        ui->labelValueDGbunchFrequency      ->setText(QString::asprintf("0x%04X" , curGBTact->Control.DG_BUNCH_FREQ      ));
        ui->labelValueDGfrequencyOffset     ->setText(QString::asprintf("0x%03X" , curGBTact->Control.DG_FREQ_OFFSET     ));
        ui->labelValueTGcontinuousValue     ->setText(QString::asprintf("0x%08X" , curGBTact->Control.TG_CONT_VALUE      ));
        ui->labelValueTGbunchFrequency      ->setText(QString::asprintf("0x%04X" , curGBTact->Control.TG_BUNCH_FREQ      ));
        ui->labelValueTGfrequencyOffset     ->setText(QString::asprintf("0x%03X" , curGBTact->Control.TG_FREQ_OFFSET     ));
        ui->labelValueHBrRate               ->setText(QString::asprintf("%d"     , curGBTact->Control.TG_HBr_RATE        ));
        ui->labelValueTGcontinuousPattern   ->setText(QString::asprintf("0x%08X%08X", curGBTact->Control.TG_PATTERN_MSB, curGBTact->Control.TG_PATTERN_LSB));
        ui->comboBoxLTUemuReadoutMode->setCurrentIndex(curGBTact->Control.TG_CTP_EMUL_MODE);
        ui->labelValueFEEID                 ->setText(QString::asprintf("0x%04X = %d", curGBTact->Control.RDH_FEE_ID, curGBTact->Control.RDH_FEE_ID));
        ui->labelValueSystemID              ->setText(QString::asprintf("0x%02X = %d", curGBTact->Control.RDH_SYS_ID, curGBTact->Control.RDH_SYS_ID));
        ui->labelValueBCIDdelayHex          ->setText(QString::asprintf("0x%03X",      curGBTact->Control.BCID_DELAY           ));
        ui->labelValueBCIDdelayDec          ->setText(QString::asprintf("%d"  ,        curGBTact->Control.BCID_DELAY           ));
        ui->labelValueDataSelectTriggerMask ->setText(QString::asprintf("0x%08X",      curGBTact->Control.DATA_SEL_TRG_MASK    ));
        ui->SwitcherLockReadout->setChecked( (curGBTact->Control.registers[0] & 1 << 14) == 0);
        ui->SwitcherBypassMode ->setChecked(  curGBTact->Control.BYPASS_MODE == 0);
        ui->SwitcherHBresponse ->setChecked(  curGBTact->Control.HB_RESPONSE);
        ui->SwitcherHBreject   ->setChecked(  curGBTact->Control.HB_REJECT);
        switch (curGBTact->Status.READOUT_MODE) {
            case GBTunit::RO_idle      : ui->labelValueReadoutModeBoard->setText("Idle"      ); break;
            case GBTunit::RO_continuous: ui->labelValueReadoutModeBoard->setText("Continuous"); break;
            case GBTunit::RO_triggered : ui->labelValueReadoutModeBoard->setText("Triggered" );
        }
        switch (curGBTact->Status.CRU_READOUT_MODE) {
            case GBTunit::RO_idle      : ui->labelValueReadoutModeCRU  ->setText("Idle"      ); break;
            case GBTunit::RO_continuous: ui->labelValueReadoutModeCRU  ->setText("Continuous"); break;
            case GBTunit::RO_triggered : ui->labelValueReadoutModeCRU  ->setText("Triggered" );
        }
        switch (curGBTact->Status.BCID_SYNC_MODE) {
            case GBTunit::BS_start: ui->labelValueBCIDsync->setText("Start"); ui->labelValueBCIDsync->setStyleSheet(neutralStyle); break;
            case GBTunit::BS_sync : ui->labelValueBCIDsync->setText("Sync" ); ui->labelValueBCIDsync->setStyleSheet(     OKstyle); break;
            case GBTunit::BS_lost : ui->labelValueBCIDsync->setText("Lost" ); ui->labelValueBCIDsync->setStyleSheet(  notOKstyle);
        }
		ui->labelValueCRUorbit->setText(QString::asprintf("%08X", curGBTact->Status.CRU_ORBIT));
		ui->labelValueRxPhase ->setText(QString::asprintf("%d"  , curGBTact->Status.RX_PHASE ));
		bool shift = curGBTact->Control.shiftRxPhase;
		quint8 phase = curGBTact->Status.RX_PHASE;
		ui->SwitcherShiftRxPhase->setChecked(shift);
		ui->labelValueRxPhase ->setStyleSheet(phase/2 == (shift ? 0 : 2) ? notOKstyle : neutralStyle);


        ui->labelIconEmptyFIFOheader->setPixmap(curGBTact->Status.FIFOempty_header ? Green1 : Red0);
        ui->labelIconEmptyFIFOdata  ->setPixmap(curGBTact->Status.FIFOempty_data   ? Green1 : Red0);
        ui->labelIconEmptyFIFOtrg   ->setPixmap(curGBTact->Status.FIFOempty_trg    ? Green1 : Red0);
        ui->labelIconEmptyFIFOslct  ->setPixmap(curGBTact->Status.FIFOempty_slct   ? Green1 : Red0);
        ui->labelIconEmptyFIFOcntpck->setPixmap(curGBTact->Status.FIFOempty_cntpck ? Green1 : Red0);
        ui->labelIconEmptyFIFOheader->setEnabled(curGBTact->Status.READOUT_MODE == GBTunit::RO_idle);
        ui->labelIconEmptyFIFOdata  ->setEnabled(curGBTact->Status.READOUT_MODE == GBTunit::RO_idle);
        ui->labelIconEmptyFIFOtrg   ->setEnabled(curGBTact->Status.READOUT_MODE == GBTunit::RO_idle);
        ui->labelIconEmptyFIFOslct  ->setEnabled(curGBTact->Status.READOUT_MODE == GBTunit::RO_idle);
        ui->labelIconEmptyFIFOcntpck->setEnabled(curGBTact->Status.READOUT_MODE == GBTunit::RO_idle);
        ui->labelIconFIFOsNotReady  ->setEnabled(curGBTact->Status.READOUT_MODE == GBTunit::RO_idle);
        ui->labelIconNotEmptyFIFOheader->setPixmap(curGBTact->Status.FIFOnotEmptyOnRunStart_header ? Red1 : Green0);
        ui->labelIconNotEmptyFIFOdata  ->setPixmap(curGBTact->Status.FIFOnotEmptyOnRunStart_data   ? Red1 : Green0);
        ui->labelIconNotEmptyFIFOtrg   ->setPixmap(curGBTact->Status.FIFOnotEmptyOnRunStart_trg    ? Red1 : Green0);
        ui->labelIconNotEmptyFIFOslct  ->setPixmap(curGBTact->Status.FIFOnotEmptyOnRunStart_slct   ? Red1 : Green0);
        ui->labelIconNotEmptyFIFOcntpck->setPixmap(curGBTact->Status.FIFOnotEmptyOnRunStart_cntpck ? Red1 : Green0);
        ui->labelIconFIFOtrgWasFull     ->setPixmap(curGBTact->Status.trgFIFOwasFull         ? Red1 : Green0);
        ui->labelIconFIFOslctEmptyOnRead->setPixmap(curGBTact->Status.slctFIFOemptyWhileRead ? Red1 : Green0);
        ui->labelIconFIFOsNotReady      ->setPixmap(curGBTact->Status.dataFIFOnotReady       ? Red1 : Green0);
        ui->labelIconBCIDsyncLostInRun  ->setPixmap(curGBTact->Status.BCsyncLostInRun        ? Red1 : Green0);
        if (isTCM())
            ui->labelIconExtraWord->setPixmap(curGBTact->Status.TCMdataFIFOfull ? Red1 : Green0);
        else {
            ui->labelIconExtraWord->setPixmap(curGBTact->Status.PMpacketCorruptedExtraWord ? Red1 : Green0);
            ui->labelIconEarlyHeader->setPixmap(curGBTact->Status.PMpacketCorruptedEarlyHeader ? Red1 : Green0);
        }

        ui->labelValueFIFOmaxConverter->setText(QString::asprintf("%u", curGBTact->Status.CNVFIFOmax));
        ui->labelValueFIFOmaxSelector ->setText(QString::asprintf("%u", curGBTact->Status.SELFIFOmax));
        ui->labelValueDropCountConverter->setText(QString::asprintf("%u", curGBTact->Status.CNVdropCount));
        ui->labelValueDropCountSelector ->setText(QString::asprintf("%u", curGBTact->Status.SELdropCount));
        ui->labelValueGBTwords->setText(QString::asprintf("%u", curGBTact->Status.wordsCount ));
        ui->labelValueEvents  ->setText(QString::asprintf("%u", curGBTact->Status.eventsCount));
        ui->labelValueGBTwordsRate->setText(rateFormat(isTCM() ? FEE.TCM.counters.GBT. wordsRate : curPM->counters.GBT. wordsRate));
        ui->labelValueEventsRate  ->setText(rateFormat(isTCM() ? FEE.TCM.counters.GBT.eventsRate : curPM->counters.GBT.eventsRate));
		ui->labelValueBCdata->setText(QString::asprintf("%4d", curGBTact->Status.BCindicatorData));
		ui->labelValueBCtrg ->setText(QString::asprintf("%4d", curGBTact->Status.BCindicatorTrg ));
        ui->labelValueBCdataModality->setText(QString::asprintf("%d/15", curGBTact->Status.BCmodalityData));
        ui->labelValueBCtrgModality ->setText(QString::asprintf("%d/15", curGBTact->Status.BCmodalityTrg ));
        bool isDataBCindicatorActual = (isTCM() ? FEE.TCM.counters.GBT.eventsRate : curPM->counters.GBT.eventsRate) >= 2.;
        ui->labelValueBCdata        ->setEnabled(isDataBCindicatorActual);
        ui->labelValueBCdataModality->setEnabled(isDataBCindicatorActual);

        ui->labelIconPhaseAlignerCPLLlock->setPixmap(curGBTact->Status.phaseAlignerCPLLlock ? Green1 : Red0);
        ui->labelIconRxWorldclkReady     ->setPixmap(curGBTact->Status.RxWorkClockReady ? Green1 : Red0);
        ui->labelIconRxFrameclkReady     ->setPixmap(curGBTact->Status.RxFrameClockReady ? Green1 : Red0);
        ui->labelIconMGTlinkReady        ->setPixmap(curGBTact->Status.MGTlinkReady ? Green1 : Red0);
        ui->labelIconTxResetDone         ->setPixmap(curGBTact->Status.TxResetDone ? Green1 : Red0);
        ui->labelIconTxFSMresetDone      ->setPixmap(curGBTact->Status.TxFSMresetDone ? Green1 : Red0);
		ui->labelIconGBTRxReady          ->setPixmap((isTCM() ? FEE.TCM.act.GBTRxReady : curPM->act.GBTRxReady) ? Green1 : Red0);
		ui->labelTextGBTRxReady          ->setStyleSheet(curGBTact->Status.GBTRxReady ? "" : "color: red");
		ui->labelTextGBTRxError          ->setStyleSheet(curGBTact->Status.GBTRxError ? "color: red" : "");
        ui->labelIconRxPhaseError        ->setPixmap(curGBTact->Status.RxPhaseError ? Red1 : Green0);

        ui->labelValueFSMerror->setText(QString::asprintf("0x%03X", curGBTact->Status.registers[2] >> 16 & 0xFFF));

        if (isTCM()) {
            if (FEE.subdetector != FV0) {
                ui->buttonSCcharge->setChecked(FEE.TCM.act.SC_EVAL_MODE == 0);
                ui->buttonSCNchan ->setChecked(FEE.TCM.act.SC_EVAL_MODE == 1);
            }
            ui->labelIconEnabled_A             ->setPixmap(FEE.TCM.act.sideAenabled     ? Green1 : Red0);
            ui->labelIconEnabled_C             ->setPixmap(FEE.TCM.act.sideCenabled     ? Green1 : Red0);
            ui->labelIconLinksOKready_A        ->setPixmap(FEE.TCM.act.sideAready       ? Green1 : Red0);
            ui->labelIconLinksOKready_C        ->setPixmap(FEE.TCM.act.sideCready       ? Green1 : Red0);
            ui->labelIconMasterLinkDelayError_A->setPixmap(FEE.TCM.act.masterLinkErrorA ? Red1 : Green0);
            ui->labelIconMasterLinkDelayError_C->setPixmap(FEE.TCM.act.masterLinkErrorC ? Red1 : Green0);
            ui->labelIconPLLlock_A             ->setPixmap(FEE.TCM.act.PLLlockA         ? Green1 : Red0);
            ui->labelIconPLLlock_C             ->setPixmap(FEE.TCM.act.PLLlockC         ? Green1 : Red0);
            ui->labelIconReadinessChanged_A    ->setPixmap(FEE.TCM.act.readinessChangeA ? Red1 : Green0);
            ui->labelIconReadinessChanged_C    ->setPixmap(FEE.TCM.act.readinessChangeC ? Red1 : Green0);
            ui->labelIconDelayRangeError_A     ->setPixmap(FEE.TCM.act.delayRangeErrorA ? Red1 : Green0);
            ui->labelIconDelayRangeError_C     ->setPixmap(FEE.TCM.act.delayRangeErrorC ? Red1 : Green0);
            if (prevPhaseStep_ns != phaseStep_ns) {
                ui->spinBoxORgate_A->setMaximum(TDCunit_ps * 255 / 1000);
                ui->spinBoxORgate_C->setMaximum(TDCunit_ps * 255 / 1000);
                ui->spinBoxORgate_A->setSingleStep(TDCunit_ps / 1000);
                ui->spinBoxORgate_C->setSingleStep(TDCunit_ps / 1000);
                ui->spinBoxPhase_A->setMinimum(-halfBC_ns);
                ui->spinBoxPhase_C->setMinimum(-halfBC_ns);
                ui->spinBoxPhase_A->setMaximum( halfBC_ns);
                ui->spinBoxPhase_C->setMaximum( halfBC_ns);
                ui->spinBoxPhase_A->setSingleStep(phaseStep_ns);
                ui->spinBoxPhase_C->setSingleStep(phaseStep_ns);
                ui->spinBoxLaserPhase->setMinimum(-halfBC_ns);
                ui->spinBoxLaserPhase->setMaximum( halfBC_ns);
                ui->spinBoxLaserPhase->setSingleStep(phaseStepLaser_ns);
                prevPhaseStep_ns = phaseStep_ns;
            }
            ui->labelValueAverageTime_A->setText(QString::asprintf("%7.3f", FEE.TCM.act.averageTimeA_ns));
            ui->labelValueAverageTime_C->setText(QString::asprintf("%7.3f", FEE.TCM.act.averageTimeC_ns));
            ui->labelValueAverageTime_A->setEnabled(FEE.TCM.counters.rate[0xA] >= 100.); //average time is calculated only on interactions (OrA AND OrC)
            ui->labelValueAverageTime_C->setEnabled(FEE.TCM.counters.rate[0xA] >= 100.);
            ui->labelValuePhase_A->setText(QString::asprintf("%7.3f", FEE.TCM.act.delayAside_ns));
            ui->labelValuePhase_C->setText(QString::asprintf("%7.3f", FEE.TCM.act.delayCside_ns));
            if (FEE.PMsA.isEmpty()) {
                ui->labelValueORgate_A->setText("noPM");
                ui->labelValueORgate_A->setToolTip("no PM available");
            } else {
                bool equalOrGate = true;
                quint8 orGate = FEE.PMsA.first()->act.OR_GATE;
                foreach (TypePM *pm, FEE.PMsA) { if (orGate != pm->act.OR_GATE) equalOrGate = false; break; }
                ui->labelValueORgate_A->setText(equalOrGate ? QString::asprintf("±%5.3f", orGate * TDCunit_ps / 1000) : "diff");
                ui->labelValueORgate_A->setToolTip(equalOrGate ? QString::asprintf("±%d TDC units", orGate) : "differs between PMs");
            }
            if (FEE.PMsC.isEmpty()) {
                ui->labelValueORgate_C->setText("noPM");
                ui->labelValueORgate_C->setToolTip("no PM available");
            } else {
                bool equalOrGate = true;
                quint8 orGate = FEE.PMsC.first()->act.OR_GATE;
                foreach (TypePM *pm, FEE.PMsC) { if (orGate != pm->act.OR_GATE) equalOrGate = false; break; }
                ui->labelValueORgate_C->setText(equalOrGate ? QString::asprintf("±%5.3f", orGate * TDCunit_ps / 1000) : "diff");
                ui->labelValueORgate_C->setToolTip(equalOrGate ? QString::asprintf("±%d TDC units", orGate) : "differs between PMs");
            }
            ui->SwitcherExt1->setChecked(FEE.TCM.act.EXT_SW & 1);
            ui->SwitcherExt2->setChecked(FEE.TCM.act.EXT_SW & 2);
            ui->SwitcherExt3->setChecked(FEE.TCM.act.EXT_SW & 4);
            ui->SwitcherExt4->setChecked(FEE.TCM.act.EXT_SW & 8);
            ui->SwitcherExtendedReadout->setChecked(FEE.TCM.act.EXTENDED_READOUT);
            ui->SwitcherAddCdelay->setChecked(FEE.TCM.act.ADD_C_DELAY);
            ui->SwitcherTriggers_1->setChecked(FEE.TCM.act.T1_ENABLED);
            ui->SwitcherTriggers_2->setChecked(FEE.TCM.act.T2_ENABLED);
            ui->SwitcherTriggers_3->setChecked(FEE.TCM.act.T3_ENABLED);
            ui->SwitcherTriggers_4->setChecked(FEE.TCM.act.T4_ENABLED);
            ui->SwitcherTriggers_5->setChecked(FEE.TCM.act.T5_ENABLED);
            ui->comboBoxTriggersMode_1->setCurrentIndex(FEE.TCM.act.T1_MODE);
            ui->comboBoxTriggersMode_2->setCurrentIndex(FEE.TCM.act.T2_MODE);
            ui->comboBoxTriggersMode_3->setCurrentIndex(FEE.TCM.act.T3_MODE);
            ui->comboBoxTriggersMode_4->setCurrentIndex(FEE.TCM.act.T4_MODE);
            ui->comboBoxTriggersMode_5->setCurrentIndex(FEE.TCM.act.T5_MODE);
            ui->labelValueTriggersRandomRate_1->setText(QString::asprintf("%08X", FEE.TCM.act.T1_RATE));
            ui->labelValueTriggersRandomRate_2->setText(QString::asprintf("%08X", FEE.TCM.act.T2_RATE));
            ui->labelValueTriggersRandomRate_3->setText(QString::asprintf("%08X", FEE.TCM.act.T3_RATE));
            ui->labelValueTriggersRandomRate_4->setText(QString::asprintf("%08X", FEE.TCM.act.T4_RATE));
            ui->labelValueTriggersRandomRate_5->setText(QString::asprintf("%08X", FEE.TCM.act.T5_RATE));
            ui->labelValueTriggersSignature_1->setText(QString::asprintf("%d", FEE.TCM.act.T1_SIGN));
            ui->labelValueTriggersSignature_2->setText(QString::asprintf("%d", FEE.TCM.act.T2_SIGN));
            ui->labelValueTriggersSignature_3->setText(QString::asprintf("%d", FEE.TCM.act.T3_SIGN));
            ui->labelValueTriggersSignature_4->setText(QString::asprintf("%d", FEE.TCM.act.T4_SIGN));
            ui->labelValueTriggersSignature_5->setText(QString::asprintf("%d", FEE.TCM.act.T5_SIGN));
            ui->labelValueTriggersLevelA_1->setText(QString::asprintf("%d", FEE.TCM.act.T1_LEVEL_A));
            ui->labelValueTriggersLevelC_1->setText(QString::asprintf("%d", FEE.TCM.act.T1_LEVEL_C));
            ui->labelValueTriggersLevelA_2->setText(QString::asprintf("%d", FEE.TCM.act.T2_LEVEL_A));
            ui->labelValueTriggersLevelC_2->setText(QString::asprintf("%d", FEE.TCM.act.T2_LEVEL_C));
            mode = FEE.TCM.act.C_SC_TRG_MODE;
            allRadioButtons[mode]->setChecked(true);
            ui->labelValueTriggersLevelA_1->setEnabled(mode != 1); //enabled for modes A&C, A, A+C
            ui->labelValueTriggersLevelA_2->setEnabled(mode != 1);
            ui->labelValueTriggersLevelC_1->setEnabled(mode < 2); //enabled for modes A&C, C
            ui->labelValueTriggersLevelC_2->setEnabled(mode < 2);
            ui->labelValueVertexTimeLow ->setText(QString::asprintf("%d", FEE.TCM.act.VTIME_LOW ));
            ui->labelValueVertexTimeHigh->setText(QString::asprintf("%d", FEE.TCM.act.VTIME_HIGH));
            ui->labelValueAttenuation->setText(QString::asprintf("%d", FEE.TCM.act.attenSteps));
            ui->labelIconAttenBusy ->setPixmap(FEE.TCM.act.attenBusy     ? Red1 : Green0);
            ui->labelIconAttenError->setPixmap(FEE.TCM.act.attenNotFound ? Red1 : Green0);
            if (enableControls->isChecked()) ui->sliderAttenuation->setDisabled(FEE.TCM.act.attenBusy || FEE.TCM.act.attenNotFound);
            ui->SwitcherLaser->setChecked(FEE.TCM.act.LASER_ENABLED);
            FEE.TCM.act.LASER_SOURCE ? ui->radioButtonGenerator->setChecked(true) : ui->radioButtonExternalTrigger->setChecked(true);
            ui->labelValueLaserFreqDivider->setText(QString::asprintf("0x%06x", FEE.TCM.act.LASER_DIVIDER));
            ui->labelValueLaserFrequency->setText(frequencyFormat(FEE.TCM.act.laserFrequency_Hz));
			ui->labelValueLaserPattern->setText(QString::asprintf("0x%016llX", FEE.TCM.act.LASER_PATTERN));
            ui->labelValueLaserPhase->setText(QString::asprintf("%7.3f", FEE.TCM.act.delayLaser_ns));
            for (quint8 i=0; i<64; ++i) { switchBitButtons.at(i)->setChecked(FEE.TCM.act.LASER_PATTERN & (1ULL << i)); }
            ui->labelValueSuppressDuration->setText(QString::asprintf("%d", FEE.TCM.act.lsrTrgSupprDur));
            ui->labelValueSuppressDelayBC ->setText(QString::asprintf("%d", FEE.TCM.act.lsrTrgSupprDelay));
            ui->labelValueSuppressDelay_ns->setText(QString::asprintf("%.1f", FEE.TCM.act.lsrTrgSupprDelay * 2 * halfBC_ns));
        } else { //PM
            ui->labelIconHDMIsyncError->setPixmap(curPM->TRGsync.syncError ?  Red1 : Green0);
            ui->labelIconHDMIlinkOK->setPixmap(curPM->TRGsync.linkOK ? Green1 : Red0);
            ui->labelIconHDMIbitsOK->setPixmap(curPM->TRGsync.bitPositionsOK ? Green1 : Red0);
            ui->labelIconHDMIsignalLost0->setPixmap(curPM->TRGsync.line0signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalLost1->setPixmap(curPM->TRGsync.line1signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalLost2->setPixmap(curPM->TRGsync.line2signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalLost3->setPixmap(curPM->TRGsync.line3signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalStable0->setPixmap(curPM->TRGsync.line0signalStable ? Green1 : Red0);
            ui->labelIconHDMIsignalStable1->setPixmap(curPM->TRGsync.line1signalStable ? Green1 : Red0);
            ui->labelIconHDMIsignalStable2->setPixmap(curPM->TRGsync.line2signalStable ? Green1 : Red0);
            ui->labelIconHDMIsignalStable3->setPixmap(curPM->TRGsync.line3signalStable ? Green1 : Red0);
            ui->labelValueHDMIdelay0->setText(QString::asprintf("%4.2f", curPM->TRGsync.line0delay * TDCunit_ps * 0.006));
            ui->labelValueHDMIdelay1->setText(QString::asprintf("%4.2f", curPM->TRGsync.line1delay * TDCunit_ps * 0.006));
            ui->labelValueHDMIdelay2->setText(QString::asprintf("%4.2f", curPM->TRGsync.line2delay * TDCunit_ps * 0.006));
            ui->labelValueHDMIdelay3->setText(QString::asprintf("%4.2f", curPM->TRGsync.line3delay * TDCunit_ps * 0.006));
            bool pmUpdated = FEE.PM.contains(curPM->FEEid);
            foreach (QGroupBox *g, QList<QGroupBox *>({ui->groupBoxChannels, ui->groupBoxPMControl, ui->groupBoxTDCStatus, ui->groupBoxReadoutControl})) g->setEnabled(pmUpdated);
            if (!pmUpdated) return;
            ui->labelValueORgate->setText   (QString::asprintf("±%3d"    , curPM->act.OR_GATE));
            ui->labelValueORgate->setToolTip(QString::asprintf("±%.3f ns", curPM->act.OR_GATE * TDCunit_ps / 1000.));
            ui->labelValueChargeHi->setText(QString::asprintf("%d", curPM->act.TRGchargeLevelHi));
            ui->labelValueChargeLo->setText(QString::asprintf("%d", curPM->act.TRGchargeLevelLo));
            switch (curPM->act.restartReasonCode) {
                case 0 : ui->labelValueRestartCode->setText("power reset"); ui->labelValueRestartCode->setStyleSheet(notOKstyle); break;
                case 1 : ui->labelValueRestartCode->setText("FPGA reset" ); ui->labelValueRestartCode->setStyleSheet(notOKstyle); break;
                case 2 : ui->labelValueRestartCode->setText("PLL relock" ); ui->labelValueRestartCode->setStyleSheet(notOKstyle); break;
                case 3 : ui->labelValueRestartCode->setText("SPI command"); ui->labelValueRestartCode->setStyleSheet(   OKstyle); break;
            }
//            curPM->act.TRG_CNT_MODE ? ui->radioButtonCFDinGate->setChecked(true) : ui->radioButtonStrict->setChecked(true);
            ui->buttonCFDinGate->setChecked( curPM->act.TRG_CNT_MODE);
            ui->buttonStrict   ->setChecked(!curPM->act.TRG_CNT_MODE);
            ui->labelIconSyncErrorTDC1->setPixmap(curPM->act.TDC1syncError ? Red1 : Green0);
            ui->labelIconSyncErrorTDC2->setPixmap(curPM->act.TDC2syncError ? Red1 : Green0);
            ui->labelIconSyncErrorTDC3->setPixmap(curPM->act.TDC3syncError ? Red1 : Green0);
            ui->labelIconPLLlockedTDC1->setPixmap(curPM->act.TDC1PLLlocked ? Green1 : Red0);
            ui->labelIconPLLlockedTDC2->setPixmap(curPM->act.TDC2PLLlocked ? Green1 : Red0);
            ui->labelIconPLLlockedTDC3->setPixmap(curPM->act.TDC3PLLlocked ? Green1 : Red0);
            ui->labelIconPLLlockedMain->setPixmap(curPM->act.mainPLLlocked ? Green1 : Red0);
            ui->labelValuePhaseTuningTDC1->setText(QString::asprintf("%.0f", curPM->act.TDC1tuning * TDCunit_ps * 8/7));
            ui->labelValuePhaseTuningTDC2->setText(QString::asprintf("%.0f", curPM->act.TDC2tuning * TDCunit_ps * 8/7));
            ui->labelValuePhaseTuningTDC3->setText(QString::asprintf("%.0f", curPM->act.TDC3tuning * TDCunit_ps * 8/7));
            for (quint8 iCh=0; iCh<12; ++iCh) {
                switchesCh[iCh]->setChecked(curPM->act.CH_MASK_DATA & (1 << iCh));
                noTRGCh[iCh]->setChecked(curPM->act.timeAlignment[iCh].blockTriggers);
                labelsTimeAlignmentCh    [iCh]->setText(QString::asprintf("%d", curPM->act.TIME_ALIGN[iCh]));
                labelsThresholdCalibrCh  [iCh]->setText(QString::asprintf("%d", curPM->act.THRESHOLD_CALIBR[iCh]));
                labelsADCdelayCh         [iCh]->setText(QString::asprintf("%d", curPM->act.Ch[iCh].ADC_DELAY));
                labelsCFDthresholdCh     [iCh]->setText(QString::asprintf("%d", curPM->act.Ch[iCh].CFD_THRESHOLD));
                labelsADCzeroCh          [iCh]->setText(QString::asprintf("%d", curPM->act.Ch[iCh].ADC_ZERO));
                labelsCFDzeroCh          [iCh]->setText(QString::asprintf("%d", curPM->act.Ch[iCh].CFD_ZERO));
                labelsADC0rangeCh        [iCh]->setText(QString::asprintf("%d", curPM->act.ADC_RANGE[iCh][0]));
                labelsADC1rangeCh        [iCh]->setText(QString::asprintf("%d", curPM->act.ADC_RANGE[iCh][1]));
                labelsADC0baseLineCh     [iCh]->setText(QString::asprintf("%d", curPM->act.ADC_BASELINE[iCh][0]));
                labelsADC1baseLineCh     [iCh]->setText(QString::asprintf("%d", curPM->act.ADC_BASELINE[iCh][1]));
                labelsADC0baseLineCh     [iCh]->setStyleSheet(curPM->act.CH_BASELINES_NOK & (1 << iCh) ? notOKstyle : neutralStyle);
                labelsADC1baseLineCh     [iCh]->setStyleSheet(curPM->act.CH_BASELINES_NOK & (1 << iCh) ? notOKstyle : neutralStyle);
                labelsADC0RMSCh          [iCh]->setText(QString::asprintf( "%5.1f", curPM->act.RMS_Ch[iCh][0]));
                labelsADC1RMSCh          [iCh]->setText(QString::asprintf( "%5.1f", curPM->act.RMS_Ch[iCh][1]));
                labelsADC0meanAmplitudeCh[iCh]->setText(QString::asprintf("%d", curPM->act.MEANAMPL[iCh][0][0]));
                labelsADC1meanAmplitudeCh[iCh]->setText(QString::asprintf("%d", curPM->act.MEANAMPL[iCh][1][0]));
                labelsRawTDCdata1Ch      [iCh]->setText(QString::asprintf("%02X", curPM->act.RAW_TDC_DATA[iCh][0]));
                labelsRawTDCdata2Ch      [iCh]->setText(QString::asprintf("%02X", curPM->act.RAW_TDC_DATA[iCh][1]));
            }
        }
    }

    void updateEdits() {
        if (isTCM()) {
            ui->spinBoxPhase_A->setValue(FEE.TCM.set.DELAY_A * phaseStep_ns);
            ui->spinBoxPhase_C->setValue(FEE.TCM.set.DELAY_C * phaseStep_ns);
            ui->spinBoxORgate_A->setValue(FEE.allPMs[ 0].set.OR_GATE * TDCunit_ps / 1000);
            ui->spinBoxORgate_C->setValue(FEE.allPMs[10].set.OR_GATE * TDCunit_ps / 1000);
            if (!FEE.PM.isEmpty()) {
                quint8 iPM = FEE.PM.first() - FEE.allPMs; if (iPM <  10) ui->spinBoxORgate_A->setValue(FEE.allPMs[iPM].set.OR_GATE * TDCunit_ps / 1000);
                       iPM = FEE.PM.last () - FEE.allPMs; if (iPM >= 10) ui->spinBoxORgate_C->setValue(FEE.allPMs[iPM].set.OR_GATE * TDCunit_ps / 1000);
            }
            ui->lineEditTriggersRandomRate_1->setText(QString::asprintf("%08X", FEE.TCM.set.T1_RATE));
            ui->lineEditTriggersRandomRate_2->setText(QString::asprintf("%08X", FEE.TCM.set.T2_RATE));
            ui->lineEditTriggersRandomRate_3->setText(QString::asprintf("%08X", FEE.TCM.set.T3_RATE));
            ui->lineEditTriggersRandomRate_4->setText(QString::asprintf("%08X", FEE.TCM.set.T4_RATE));
            ui->lineEditTriggersRandomRate_5->setText(QString::asprintf("%08X", FEE.TCM.set.T5_RATE));
            ui->lineEditTriggersLevelA_1->setText(QString::asprintf("%d", FEE.TCM.set.T1_LEVEL_A));
            ui->lineEditTriggersLevelC_1->setText(QString::asprintf("%d", FEE.TCM.set.T1_LEVEL_C));
            ui->lineEditTriggersLevelA_2->setText(QString::asprintf("%d", FEE.TCM.set.T2_LEVEL_A));
            ui->lineEditTriggersLevelC_2->setText(QString::asprintf("%d", FEE.TCM.set.T2_LEVEL_C));
            ui->lineEditVertexTimeLow ->setText(QString::asprintf("%d", FEE.TCM.set.VTIME_LOW ));
            ui->lineEditVertexTimeHigh->setText(QString::asprintf("%d", FEE.TCM.set.VTIME_HIGH));
            ui->sliderAttenuation->setValue(FEE.TCM.set.attenSteps);
            if (ui->spinBoxAttenuation->value() != FEE.TCM.set.attenSteps) ui->spinBoxAttenuation->setValue(FEE.TCM.set.attenSteps);
            ui->spinBoxLaserFreqDivider->setValue(FEE.TCM.set.LASER_DIVIDER);
			ui->lineEditLaserPattern->setText(QString::asprintf("%016llX", FEE.TCM.set.LASER_PATTERN));
            ui->sliderLaser->setValue(FEE.TCM.set.LASER_DELAY);
            if (ui->spinBoxLaserPhase->value() != FEE.TCM.set.delayLaser_ns) ui->spinBoxLaserPhase->setValue(FEE.TCM.set.delayLaser_ns);
            ui->spinBoxSuppressDuration->setValue(FEE.TCM.set.lsrTrgSupprDur);
            ui->spinBoxSuppressDelayBC->setValue(FEE.TCM.set.lsrTrgSupprDelay);
        } else { //PM
            ui->lineEditORgate->setText(QString::asprintf("%d", curPM->set.OR_GATE));
            ui->lineEditChargeHi->setText(QString::asprintf("%d", curPM->set.TRGchargeLevelHi));
            ui->lineEditChargeLo->setText(QString::asprintf("%d", curPM->set.TRGchargeLevelLo));
            for (quint8 i=0; i<12; ++i) {
                editsTimeAlignmentCh  [i]->setText(QString::asprintf("%d", curPM->set.TIME_ALIGN[i].value));
                editsThresholdCalibrCh[i]->setText(QString::asprintf("%d", curPM->set.THRESHOLD_CALIBR[i]));
                editsADCdelayCh       [i]->setText(QString::asprintf("%d", curPM->set.Ch[i].ADC_DELAY));
                editsCFDthresholdCh   [i]->setText(QString::asprintf("%d", curPM->set.Ch[i].CFD_THRESHOLD));
                editsADCzeroCh        [i]->setText(QString::asprintf("%d", curPM->set.Ch[i].ADC_ZERO));
                editsCFDzeroCh        [i]->setText(QString::asprintf("%d", curPM->set.Ch[i].CFD_ZERO));
                editsADC0rangeCh      [i]->setText(QString::asprintf("%d", curPM->set.ADC_RANGE[i][0]));
                editsADC1rangeCh      [i]->setText(QString::asprintf("%d", curPM->set.ADC_RANGE[i][1]));
            }
        }
        ui->lineEditDGtriggerRespondMask  ->setText(QString::asprintf("%08X", curGBTset->DG_TRG_RESPOND_MASK  ));
        ui->lineEditDGbunchPattern        ->setText(QString::asprintf("%08X", curGBTset->DG_BUNCH_PATTERN	  ));
        ui->lineEditDGbunchFrequency      ->setText(QString::asprintf("%04X", curGBTset->DG_BUNCH_FREQ		  ));
        ui->lineEditDGfrequencyOffset     ->setText(QString::asprintf("%03X", curGBTset->DG_FREQ_OFFSET		  ));
        ui->lineEditTGcontinuousValue     ->setText(QString::asprintf("%08X", curGBTset->TG_CONT_VALUE		  ));
        ui->lineEditTGbunchFrequency      ->setText(QString::asprintf("%04X", curGBTset->TG_BUNCH_FREQ		  ));
        ui->lineEditTGfrequencyOffset     ->setText(QString::asprintf("%03X", curGBTset->TG_FREQ_OFFSET		  ));
        ui->lineEditBCIDdelayHex          ->setText(QString::asprintf("%03X", curGBTset->BCID_DELAY			  ));
        ui->lineEditBCIDdelayDec          ->setText(QString::asprintf("%d"  , curGBTset->BCID_DELAY			  ));
        ui->lineEditDataSelectTriggerMask ->setText(QString::asprintf("%08X", curGBTset->DATA_SEL_TRG_MASK	  ));
        ui->lineEditTGHBrRate             ->setText(QString::asprintf("%d"  , curGBTset->TG_HBr_RATE    	  ));
        ui->lineEditTGcontinuousPattern   ->setText(QString::asprintf("%08X%08X", curGBTset->TG_PATTERN_MSB, curGBTset->TG_PATTERN_LSB));
        resetHighlight();
    }

    void updateCounters(quint16 FEEid) {
        if (FEEid != curFEEid) return;
        if (isTCM()) {
            for (quint8 i=0; i<TypeTCM::Counters::number; ++i) {
                labelsTriggersCount[i]->setText(QString::number(FEE.TCM.counters.New[i]));
                labelsTriggersRate[i]->setText(rateFormat(FEE.TCM.counters.rate[i]));
            }
        } else { //PM
            for (quint8 i=0; i<12; ++i) {
                labelsTRGcounterCh[i]->setText(QString::number(curPM->counters.Ch[i].TRG));
                labelsCFDcounterCh[i]->setText(QString::number(curPM->counters.Ch[i].CFD));
                labelsTRGcounterRateCh[i]->setText(rateFormat(curPM->counters.rateCh[i].TRG));
                labelsCFDcounterRateCh[i]->setText(rateFormat(curPM->counters.rateCh[i].CFD));
            }
        }
    }

    void on_buttonResetCounters_clicked() {
        curGBTact->Status. wordsCount = FEE.readRegister((isTCM() ? 0 : curPM->baseAddress) + 0xED);
        curGBTact->Status.eventsCount = FEE.readRegister((isTCM() ? 0 : curPM->baseAddress) + 0xF1);
        curGBTcnt->calculateRate(curGBTact->Status.wordsCount, curGBTact->Status.eventsCount);
        curGBTcnt-> wordsOld = 0;
        curGBTcnt->eventsOld = 0;
        FEE.reset(curFEEid, GBTunit::RB_dataCounter);
    }
    void on_buttonResetOffset_clicked                () { FEE.reset(curFEEid, GBTunit::RB_generatorsBunchOffset); }
    void on_buttonResetOrbitSync_clicked             () { FEE.reset(curFEEid, GBTunit::RB_orbitSync            ); }
    void on_buttonResetGBTRxErrors_clicked             () { FEE.reset(curFEEid, GBTunit::RB_GBTRxError           ); }
    void on_buttonResetGBT_clicked                   () { FEE.reset(curFEEid, GBTunit::RB_GBT                  ); }
    void on_buttonResetRxPhaseError_clicked          () { FEE.reset(curFEEid, GBTunit::RB_RXphaseError         ); }
    void on_buttonDataGeneratorOff_clicked           () { FEE.apply_DG_MODE(curFEEid, GBTunit::DG_noData); }
    void on_buttonDataGeneratorMain_clicked          () { FEE.apply_DG_MODE(curFEEid, GBTunit::DG_main  ); }
    void on_buttonDataGeneratorTx_clicked            () { FEE.apply_DG_MODE(curFEEid, GBTunit::DG_Tx    ); }
    void on_buttonApplyDGtriggerRespondMask_clicked  () { FEE.apply_DG_TRG_RESPOND_MASK(curFEEid); }
    void on_buttonApplyDGbunchPattern_clicked        () { on_lineEditDGbunchPattern_textEdited   (); FEE.apply_DG_BUNCH_PATTERN   (curFEEid); }
    void on_buttonApplyDGbunchFrequency_clicked      () { on_lineEditDGbunchFrequency_textEdited (); FEE.apply_DG_BUNCH_FREQ      (curFEEid); }
    void on_buttonApplyDGfrequencyOffset_clicked     () { on_lineEditDGfrequencyOffset_textEdited(); FEE.apply_DG_FREQ_OFFSET     (curFEEid); }
    void on_buttonTriggerGeneratorOff_clicked        () { FEE.apply_TG_MODE(curFEEid, GBTunit::TG_noTrigger ); }
    void on_buttonTriggerGeneratorContinuous_clicked () { FEE.apply_TG_MODE(curFEEid, GBTunit::TG_continuous); }
    void on_buttonTriggerGeneratorTx_clicked         () { FEE.apply_TG_MODE(curFEEid, GBTunit::TG_Tx        ); }
    void on_buttonApplyTGcontinuousPattern_clicked   () { on_lineEditTGcontinuousPattern_textEdited  (); FEE.apply_TG_PATTERN       (curFEEid); }
    void on_buttonApplyTGcontinuousValue_clicked     () { on_lineEditTGcontinuousValue_textEdited    (); FEE.apply_TG_CONT_VALUE    (curFEEid); }
    void on_buttonApplyTGbunchFrequency_clicked      () { on_lineEditTGbunchFrequency_textEdited     (); FEE.apply_TG_BUNCH_FREQ    (curFEEid); }
    void on_buttonApplyTGfrequencyOffset_clicked     () { on_lineEditTGfrequencyOffset_textEdited    (); FEE.apply_TG_FREQ_OFFSET   (curFEEid); }
    void on_buttonApplyBCIDdelay_clicked             () { on_lineEditBCIDdelayHex_textEdited         (); FEE.apply_BCID_DELAY       (curFEEid); }
    void on_buttonApplyDataSelectTriggerMask_clicked () { on_lineEditDataSelectTriggerMask_textEdited(); FEE.apply_DATA_SEL_TRG_MASK(curFEEid); }
    void on_buttonApplyTGHBrRate_clicked             () { on_lineEditTGHBrRate_textEdited            (); FEE.apply_TG_HBr_RATE      (curFEEid); }
    void on_SwitcherLockReadout_clicked() {
        ui->SwitcherLockReadout->setChecked(false);
        FEE.apply_RESET_FSM(curFEEid,  false);
    }

    void on_SwitcherBypassMode_clicked (bool checked) { FEE.apply_BYPASS_MODE (curFEEid,  checked); }
    void on_SwitcherHBresponse_clicked (bool checked) { FEE.apply_HB_RESPONSE (curFEEid, !checked); }
    void on_SwitcherHBreject_clicked   (bool checked) { FEE.apply_HB_REJECT   (curFEEid, !checked); }
	void on_SwitcherShiftRxPhase_clicked (bool checked) { FEE.apply_shiftRxPhase(curFEEid, !checked); }
    void on_lineEditDGtriggerRespondMask_textEdited  () { curGBTset->DG_TRG_RESPOND_MASK = ui->lineEditDGtriggerRespondMask->displayText().toUInt(&ok, 16); }
    void on_lineEditDGbunchPattern_textEdited        () { curGBTset->DG_BUNCH_PATTERN  = ui->lineEditDGbunchPattern        ->displayText().toUInt(&ok, 16); }
    void on_lineEditDGbunchFrequency_textEdited      () { curGBTset->DG_BUNCH_FREQ     = ui->lineEditDGbunchFrequency      ->displayText().toUInt(&ok, 16); }
    void on_lineEditDGfrequencyOffset_textEdited     () { curGBTset->DG_FREQ_OFFSET    = ui->lineEditDGfrequencyOffset     ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGcontinuousValue_textEdited     () { curGBTset->TG_CONT_VALUE     = ui->lineEditTGcontinuousValue     ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGbunchFrequency_textEdited      () { curGBTset->TG_BUNCH_FREQ     = ui->lineEditTGbunchFrequency      ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGfrequencyOffset_textEdited     () { curGBTset->TG_FREQ_OFFSET    = ui->lineEditTGfrequencyOffset     ->displayText().toUInt(&ok, 16); }
    void on_lineEditDataSelectTriggerMask_textEdited () { curGBTset->DATA_SEL_TRG_MASK = ui->lineEditDataSelectTriggerMask ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGHBrRate_textEdited             () { curGBTset->TG_HBr_RATE       = ui->lineEditTGHBrRate             ->displayText().toUInt()       ; }
    void on_lineEditTGcontinuousPattern_textEdited   () { curGBTset->TG_PATTERN_LSB = ui->lineEditTGcontinuousPattern->displayText().right(8).toUInt(&ok, 16);
                                                          curGBTset->TG_PATTERN_MSB = ui->lineEditTGcontinuousPattern->displayText().left (8).toUInt(&ok, 16); }
    void on_comboBoxLTUemuReadoutMode_activated(int index) { curGBTset->TG_CTP_EMUL_MODE = index; FEE.apply_TG_CTP_EMUL_MODE(curFEEid, index); }
    void on_lineEditBCIDdelayHex_textEdited() {
        curGBTset->BCID_DELAY = ui->lineEditBCIDdelayHex->displayText().toUInt(&ok, 16);
        ui->lineEditBCIDdelayDec->setText(QString::asprintf("%d"  , curGBTset->BCID_DELAY));
    }
    void on_lineEditBCIDdelayDec_textEdited() {
        curGBTset->BCID_DELAY = ui->lineEditBCIDdelayDec->displayText().toUInt(&ok);
        ui->lineEditBCIDdelayHex->setText(QString::asprintf("%03X"  , curGBTset->BCID_DELAY));
    }


    void on_TCM_selector_toggled(bool checked) { //select TCM
        (checked ? ui->groupBoxTCM : ui->groupBoxPM)->setVisible(true);
        ui->groupBoxReadoutControl->setParent(checked ? ui->groupBoxTCM : ui->groupBoxPM);
        int PMy = ui->groupBoxPM->y();
        ui->groupBoxPM->move(ui->groupBoxPM->x(), ui->groupBoxTCM->y());
        ui->groupBoxTCM->move(ui->groupBoxTCM->x(), PMy);
        ui->groupBoxReadoutControl->setVisible(true);
        ui->labelTextEarlyHeader->setVisible(!checked);
        ui->labelIconEarlyHeader->setVisible(!checked);
        ui->labelTextExtraWord->setText(checked ? "TCM data FIFO full" : "PM: extra word");
        (checked ? ui->groupBoxPM : ui->groupBoxTCM)->setVisible(false);
        if (checked) {
            curFEEid = FEE.TCMid;
            ui->groupBoxBoardStatus->setTitle("Board status (TCM)");
            curGBTact = &FEE.TCM.act.GBT;
            curGBTset = &FEE.TCM.set.GBT;
            curGBTcnt = &FEE.TCM.counters.GBT;
            ui->groupBoxReadoutControl->setEnabled(true);
            updateEdits();
            updateCounters(FEE.TCMid);
            if (FEE.isOnline) FEE.sync();
        }
    }

    void selectPM(quint16 FEEid) {
        ui->groupBoxPM->setTitle(QString("PM") + FEE.PM[FEEid]->name);
        ui->groupBoxBoardStatus->setTitle(QString::asprintf("Board status (PM%s)", FEE.PM[FEEid]->name));
        curFEEid = FEEid;
        curPM = FEE.PM[FEEid];
        curGBTact = &curPM->act.GBT;
        curGBTset = &curPM->set.GBT;
        curGBTcnt = &curPM->counters.GBT;
        updateEdits();
        updateCounters(curFEEid);
        if (FEE.isOnline) FEE.sync();
    }

    void on_comboBoxUpdatePeriod_activated(int index) { /*if (FEE.TCM.act.COUNTERS_UPD_RATE != quint32(index))*/ FEE.apply_COUNTERS_UPD_RATE(index); }
    void on_buttonRestart_clicked() { FEE.apply_RESET_SYSTEM(ui->radioButtonForceLocal->isChecked()); }
    void on_buttonDismissErrors_clicked() { FEE.apply_RESET_ERRORS(); }

    void on_spinBoxAttenuation_valueChanged(double val) { FEE.TCM.set.attenSteps = val; }
    void on_sliderAttenuation_valueChanged(int value) { ui->spinBoxAttenuation->setValue(value); }
    void on_buttonApplyAttenuation_clicked() {
        FEE.apply_attenSteps();
        ui->sliderAttenuation->setValue(FEE.TCM.set.attenSteps);
    }
    void on_sliderAttenuation_sliderReleased() { FEE.apply_attenSteps(); }
    void on_SwitcherLaser_clicked (bool checked) { FEE.apply_LASER_ENABLED(!checked); }
    void on_radioButtonGenerator_clicked	  () { FEE.apply_LASER_SOURCE(true ); }
    void on_radioButtonExternalTrigger_clicked() { FEE.apply_LASER_SOURCE(false); }
    void on_buttonApplyLaserFrequency_clicked() { FEE.TCM.set.LASER_DIVIDER = ui->spinBoxLaserFreqDivider->value(); FEE.apply_LASER_DIVIDER(); }
	void on_buttonApplyLaserPattern_clicked() { on_lineEditLaserPattern_textEdited(); FEE.apply_LASER_PATTERN(); }

    void on_spinBoxLaserFreqDivider_valueChanged(int div) {
        FEE.TCM.set.LASER_DIVIDER = div;
        FEE.TCM.set.laserFrequency_Hz = systemClock_MHz * 1e6 / (div == 0 ? 1 << 24 : div);
        if (!laserFreqIsEditing) ui->lineEditLaserFrequency->setText(frequencyFormat(FEE.TCM.set.laserFrequency_Hz));
    }
    void on_lineEditLaserFrequency_textEdited() {
        laserFreqIsEditing = true;
        double freq = ui->lineEditLaserFrequency->displayText().toDouble(&ok), high = systemClock_MHz * 1e6, low = systemClock_MHz * 1e6 / (1 << 24);
        ui->buttonApplyLaserFrequency->setEnabled(ok);
        ui->spinBoxLaserFreqDivider->setEnabled(ok);
        if (ok) {
            if (freq > high)
                ui->spinBoxLaserFreqDivider->setValue(1);
            else if (freq < low)
                ui->spinBoxLaserFreqDivider->setValue(0);
            else
                ui->spinBoxLaserFreqDivider->setValue(lround(high / freq));
        }
    }
    void on_lineEditLaserFrequency_editingFinished() { laserFreqIsEditing = false; };
	void on_lineEditLaserPattern_textEdited() { FEE.TCM.set.LASER_PATTERN = ui->lineEditLaserPattern->displayText().toULongLong(&ok, 16); }
    void on_spinBoxSuppressDuration_valueChanged(int div) { FEE.TCM.set.lsrTrgSupprDur   = div; }
    void on_spinBoxSuppressDelayBC_valueChanged (int div) { FEE.TCM.set.lsrTrgSupprDelay = div; }
    void on_buttonApplySuppressDuration_clicked() { FEE.TCM.set.lsrTrgSupprDur   = ui->spinBoxSuppressDuration->value(); FEE.apply_LSR_TRG_SUPPR_DUR  (); }
    void on_buttonApplySuppressDelayBC_clicked () { FEE.TCM.set.lsrTrgSupprDelay = ui->spinBoxSuppressDelayBC ->value(); FEE.apply_LSR_TRG_SUPPR_DELAY(); }

    void on_spinBoxORgate_A_valueChanged(double val) { for (quint8 iPM= 0; iPM<10; ++iPM) FEE.allPMs[iPM].set.OR_GATE = lround(val * 1000 / TDCunit_ps); }
    void on_spinBoxORgate_C_valueChanged(double val) { for (quint8 iPM=10; iPM<20; ++iPM) FEE.allPMs[iPM].set.OR_GATE = lround(val * 1000 / TDCunit_ps); }
    void on_buttonApplyORgate_A_clicked() { on_spinBoxORgate_A_valueChanged(ui->spinBoxORgate_A->value()); FEE.apply_OR_GATE_sideA(FEE.allPMs[ 0].set.OR_GATE); }
    void on_buttonApplyORgate_C_clicked() { on_spinBoxORgate_C_valueChanged(ui->spinBoxORgate_C->value()); FEE.apply_OR_GATE_sideC(FEE.allPMs[10].set.OR_GATE); }

    void on_spinBoxPhase_A_valueChanged(double val) { FEE.TCM.set.DELAY_A = lround(val / phaseStep_ns); }
    void on_spinBoxPhase_C_valueChanged(double val) { FEE.TCM.set.DELAY_C = lround(val / phaseStep_ns); }
    void on_spinBoxLaserPhase_valueChanged(double val) {
        FEE.TCM.set.LASER_DELAY = lround(val / phaseStepLaser_ns);
        FEE.TCM.set.delayLaser_ns = FEE.TCM.set.LASER_DELAY * phaseStepLaser_ns;
    }
    void on_buttonApplyPhase_A_clicked() { on_spinBoxPhase_A_valueChanged(ui->spinBoxPhase_A->value()); FEE.apply_DELAY_A(); }
    void on_buttonApplyPhase_C_clicked() { on_spinBoxPhase_C_valueChanged(ui->spinBoxPhase_C->value()); FEE.apply_DELAY_C(); }
    void on_buttonApplyLaserPhase_clicked() {
        FEE.apply_LASER_DELAY();
        ui->sliderLaser->setValue(FEE.TCM.set.LASER_DELAY);
    }
    void on_sliderLaser_sliderReleased() { FEE.apply_LASER_DELAY(); }
    void on_sliderLaser_valueChanged(int value) { ui->spinBoxLaserPhase->setValue(value * phaseStepLaser_ns); }

    void on_SwitcherExt1_clicked(bool checked) { FEE.apply_SW_EXT(1, !checked); }
    void on_SwitcherExt2_clicked(bool checked) { FEE.apply_SW_EXT(2, !checked); }
    void on_SwitcherExt3_clicked(bool checked) { FEE.apply_SW_EXT(3, !checked); }
    void on_SwitcherExt4_clicked(bool checked) { FEE.apply_SW_EXT(4, !checked); }

    void on_SwitcherExtendedReadout_clicked(bool checked) { FEE.apply_EXTENDED_READOUT(!checked); }
    void on_SwitcherAddCdelay_clicked      (bool checked) { FEE.apply_ADD_C_DELAY     (!checked); }
    void on_SwitcherTriggers_1_clicked (bool checked) { FEE.apply_T1_ENABLED(!checked); }
    void on_SwitcherTriggers_2_clicked (bool checked) { FEE.apply_T2_ENABLED(!checked); }
    void on_SwitcherTriggers_3_clicked (bool checked) { FEE.apply_T3_ENABLED(!checked); }
    void on_SwitcherTriggers_4_clicked (bool checked) { FEE.apply_T4_ENABLED(!checked); }
    void on_SwitcherTriggers_5_clicked (bool checked) { FEE.apply_T5_ENABLED(!checked); }

    void on_radioButtonAandC_clicked(bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(0); }
    void on_radioButtonC_clicked    (bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(1); }
    void on_radioButtonA_clicked    (bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(2); }
    void on_radioButtonSum_clicked  (bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(3); }

	void on_buttonResetCountersTCM_clicked() { FEE.resetCounts(FEE.TCMid); }
	void on_buttonResetChCounters_clicked() { FEE.resetCounts(curFEEid); }

    void on_comboBoxTriggersMode_1_activated(int index) { FEE.apply_T1_MODE(index); }
    void on_comboBoxTriggersMode_2_activated(int index) { FEE.apply_T2_MODE(index); }
    void on_comboBoxTriggersMode_3_activated(int index) { FEE.apply_T3_MODE(index); }
    void on_comboBoxTriggersMode_4_activated(int index) { FEE.apply_T4_MODE(index); }
    void on_comboBoxTriggersMode_5_activated(int index) { FEE.apply_T5_MODE(index); }

    void on_lineEditTriggersRandomRate_1_textEdited() { FEE.TCM.set.T1_RATE = ui->lineEditTriggersRandomRate_1->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_2_textEdited() { FEE.TCM.set.T2_RATE = ui->lineEditTriggersRandomRate_2->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_3_textEdited() { FEE.TCM.set.T3_RATE = ui->lineEditTriggersRandomRate_3->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_4_textEdited() { FEE.TCM.set.T4_RATE = ui->lineEditTriggersRandomRate_4->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_5_textEdited() { FEE.TCM.set.T5_RATE = ui->lineEditTriggersRandomRate_5->displayText().toUInt(&ok, 16); }
    void on_buttonApplyTriggersRandomRate_1_clicked() { FEE.apply_T1_RATE(); }
    void on_buttonApplyTriggersRandomRate_2_clicked() { FEE.apply_T2_RATE(); }
    void on_buttonApplyTriggersRandomRate_3_clicked() { FEE.apply_T3_RATE(); }
    void on_buttonApplyTriggersRandomRate_4_clicked() { FEE.apply_T4_RATE(); }
    void on_buttonApplyTriggersRandomRate_5_clicked() { FEE.apply_T5_RATE(); }

    void on_lineEditTriggersLevelA_1_textEdited(const QString &text) { FEE.TCM.set.T1_LEVEL_A = text.toUInt(); }
    void on_lineEditTriggersLevelC_1_textEdited(const QString &text) { FEE.TCM.set.T1_LEVEL_C = text.toUInt(); }
    void on_lineEditTriggersLevelA_2_textEdited(const QString &text) { FEE.TCM.set.T2_LEVEL_A = text.toUInt(); }
    void on_lineEditTriggersLevelC_2_textEdited(const QString &text) { FEE.TCM.set.T2_LEVEL_C = text.toUInt(); }
    void on_buttonApplyTriggersLevelA_1_clicked() { FEE.apply_T1_LEVEL_A(); }
    void on_buttonApplyTriggersLevelC_1_clicked() { FEE.apply_T1_LEVEL_C(); }
    void on_buttonApplyTriggersLevelA_2_clicked() { FEE.apply_T2_LEVEL_A(); }
    void on_buttonApplyTriggersLevelC_2_clicked() { FEE.apply_T2_LEVEL_C(); }

    void on_lineEditVertexTimeLow_textEdited(const QString text) { FEE.TCM.set.VTIME_LOW  = text.toInt(); }
    void on_lineEditVertexTimeHigh_textEdited (const QString text) { FEE.TCM.set.VTIME_HIGH = text.toInt(); }
    void on_buttonApplyVertexTimeLow_clicked() { FEE.apply_VTIME_LOW (); }
    void on_buttonApplyVertexTimeHigh_clicked () { FEE.apply_VTIME_HIGH(); }

    void on_lineEditORgate_textEdited       (const QString text) { curPM->set.OR_GATE  = text.toUInt(); }
    void on_lineEditChargeHi_textEdited(const QString text) { curPM->set.TRGchargeLevelHi = text.toUInt(); }
    void on_lineEditChargeLo_textEdited(const QString text) { curPM->set.TRGchargeLevelLo = text.toUInt(); }
    void on_buttonApplyORgate_clicked       () { FEE.apply_OR_GATE_PM (curFEEid); }
    void on_buttonApplyChargeHi_clicked() { FEE.apply_TRGchargeLevelHi(curPM - FEE.allPMs, ui->lineEditChargeHi->text().toUInt()); }
    void on_buttonApplyChargeLo_clicked() { FEE.apply_TRGchargeLevelLo(curPM - FEE.allPMs, ui->lineEditChargeLo->text().toUInt()); }

    void on_buttonStrict_clicked   () { FEE.apply_TRG_CNT_MODE(curFEEid, false); }
    void on_buttonCFDinGate_clicked() { FEE.apply_TRG_CNT_MODE(curFEEid, true ); }

    void on_buttonSCNchan_clicked () { FEE.apply_SC_EVAL_MODE(true ); }
    void on_buttonSCcharge_clicked() { FEE.apply_SC_EVAL_MODE(false); }

    void on_buttonCopyActual_clicked() { FEE.copyActualToSettingsPM(curPM); updateEdits(); }
    void on_buttonApplyAll_clicked() { FEE.applySettingsPM(curPM); }

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
