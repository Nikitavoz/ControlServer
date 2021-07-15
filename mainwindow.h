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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QSettings settings;
    FITelectronics FEE;
    QActionGroup *controlsGroup;
    QPixmap
		Green0 = QPixmap(":/0G.png"), //OK
		Green1 = QPixmap(":/1G.png"), //OK
		Red0 = QPixmap(":/0R.png"), //not OK
        Red1 = QPixmap(":/1R.png"), //not OK
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

	QIntValidator *intValidator = new QIntValidator(this);
	QDoubleValidator *doubleValidator = new QDoubleValidator(this);
    QRegExpValidator *uint16Validator = new QRegExpValidator(QRegExp("[0-5]?[0-9]{1,4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]"), this);
    bool ok, laserFreqIsEditing = false;
    quint8 mode;
    quint32 value;
    int sz;
	double prevPhaseStep_ns = 25. / 2048; //value for 40. Mhz clock and production TCM; var is used to detect values change in case of clock source and/or TCM change

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

        applyButtons = ui->centralWidget->findChildren<QPushButton *>(QRegularExpression("buttonApply.*"));
        switchBitButtons = ui->groupBoxLaser->findChildren<QPushButton *>(QRegularExpression("buttonSwitchBit.*")).toVector();
        selectorsPMA = ui->groupBoxBoardSelection->findChildren<QPushButton *>(QRegularExpression("PM_selector_A[0-9]")).toVector();
        selectorsPMC = ui->groupBoxBoardSelection->findChildren<QPushButton *>(QRegularExpression("PM_selector_C[0-9]")).toVector();
        linksPMA = ui->groupBoxBoardSelection->findChildren<QLabel *>(QRegularExpression("labelIconLinkOK_A[0-9]")).toVector();
        linksPMC = ui->groupBoxBoardSelection->findChildren<QLabel *>(QRegularExpression("labelIconLinkOK_C[0-9]")).toVector();
        switchesPMA = ui->groupBoxBoardSelection->findChildren<Switch *>(QRegularExpression("Switcher_A[0-9]")).toVector();
        switchesPMC = ui->groupBoxBoardSelection->findChildren<Switch *>(QRegularExpression("Switcher_C[0-9]")).toVector();
        switchesCh = ui->groupBoxChannels->findChildren<Switch *>(QRegularExpression("SwitcherCh_(0[1-9]|1[0-2])")).toVector();
        labelsTriggersCount = {
            ui->labelValueTriggersCount_ORA,
            ui->labelValueTriggersCount_ORC,
            ui->labelValueTriggersCount_SC ,
            ui->labelValueTriggersCount_C  ,
            ui->labelValueTriggersCount_V  ,
            ui->labelValueTriggersCount_75 ,
            ui->labelValueTriggersCount_76 ,
            ui->labelValueTriggersCount_77 ,
            ui->labelValueTriggersCount_78 ,
            ui->labelValueTriggersCount_79 ,
            ui->labelValueTriggersCount_7A ,
            ui->labelValueTriggersCount_7B ,
            ui->labelValueTriggersCount_7C ,
            ui->labelValueTriggersCount_7D ,
            ui->labelValueTriggersCount_7E ,
        };
        labelsTriggersRate = {
            ui->labelValueTriggersRate_ORA,
            ui->labelValueTriggersRate_ORC,
            ui->labelValueTriggersRate_SC ,
            ui->labelValueTriggersRate_C  ,
            ui->labelValueTriggersRate_V  ,
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
            ui->radioButtonGenerator      ,
            ui->radioButtonStrict         ,
            ui->radioButtonCFDinGate
        };
        allButtons    = ui->centralWidget->findChildren<QPushButton *>(QRegularExpression("button.*")); //except PM selectors
        allSwitches   = ui->centralWidget->findChildren<Switch *>();
        allLineEdits  = ui->centralWidget->findChildren<QLineEdit *>();
        allComboBoxes = ui->centralWidget->findChildren<QComboBox *>();
        allSpinBoxes  = ui->centralWidget->findChildren<QAbstractSpinBox *>();
        allWidgets    = ui->centralWidget->findChildren<QWidget *>();

        ui->groupBoxPM->setVisible(false);
//initial scaling (a label with fontsize 10 (Calibri) has pixelsize of 13 without system scaling and e.g. 20 at 150% scaling so widgets size should be recalculated)
        sz = ui->labelTextLinkOK->fontInfo().pixelSize();
        resize(lround(width()*sz/13.), lround(690*sz/13.)); //mainWindow
        setMaximumSize(size());
        setMinimumSize(size());
        if (sz > 13) { //not default pixelsize for font of size 10
            foreach (QWidget *w, allWidgets) w->setGeometry(lround(w->x()*sz/13.), lround(w->y()*sz/13.), lround(w->width()*sz/13.), lround(w->height()*sz/13.));
			ui->buttonSendSingleValue->setIconSize(QSize(lround(20.*sz/13), lround(20.*sz/13)));
			ui->buttonSendSingleValue->setStyleSheet(QString::asprintf("border-radius: %dpx", lround(10.*sz/13) - 1));
            foreach (QPushButton *b, applyButtons) b->setIconSize(QSize( lround(16.*sz/13), lround(16.*sz/13) ));
            foreach (QPushButton *b, switchBitButtons) b->setIconSize(QSize( lround(22.*sz/13), lround(37.*sz/13) ));
		}

//menus
        QMenu *fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction(QIcon(":/save.png"), "&Save settings to...", this, SLOT(save()), QKeySequence::Save);
        //fileMenu->addAction(QIcon(":/load.png"), "&Load settings from...", this, SLOT(load()), QKeySequence::Open);
        QAction *actionLoad = new QAction(QIcon(":/load.png"), "&Load settings from...", this);
        actionLoad->setShortcut(QKeySequence::Open);
        fileMenu->addAction(actionLoad);
        connect(actionLoad, &QAction::triggered, this, &MainWindow::load);
        actionLoad->setDisabled(true);
        //controlsGroup = new QActionGroup(this);
        QMenu *controlMenu = menuBar()->addMenu("&Control");
        QAction *enableControls = new QAction(QIcon(":/controls.png"), "Disable", this);
        enableControls->setCheckable(true);
        enableControls->setChecked(true);
        controlMenu->addAction(enableControls);
        connect(enableControls, &QAction::triggered, this, [=](bool checked) {
            actionLoad->setEnabled(checked);
            enableControls->setText(checked ? "Disable" : "Enable");
            foreach (QLineEdit        *e, allLineEdits   ) e->setEnabled(checked);
            foreach (QPushButton      *b, allButtons     ) b->setEnabled(checked);
            foreach (Switch           *s, allSwitches    ) s->setEnabled(checked);
            foreach (QComboBox        *c, allComboBoxes  ) c->setEnabled(checked);
            foreach (QAbstractSpinBox *a, allSpinBoxes   ) a->setEnabled(checked);
            foreach (QRadioButton     *r, allRadioButtons) r->setEnabled(checked);
            ui->sliderLaser->setEnabled(checked);
            ui->sliderAttenuation->setEnabled(checked);
        });
        controlMenu->addAction("Read all &PMs", &FEE, &FITelectronics::fullSync);
        QMenu *networkMenu = menuBar()->addMenu("&Network");
		networkMenu->addAction(QIcon(":/recheck.png"), "&Recheck and default", this, SLOT(recheckTarget()), QKeySequence::Refresh);
        networkMenu->addAction("&Change target IP address...", this, SLOT(changeIP()));
//        fileRead(QCoreApplication::applicationName() + ".ini");

//signal-slot conections
        connect(&FEE, &IPbusTarget::error, this, [=](QString message, errorType et) {
            QMessageBox::warning(this, errorTypeName[et], message);
            statusBar()->showMessage(message);
            ui->centralWidget->setDisabled(true);
        });
        connect(&FEE, &IPbusTarget::IPbusStatusOK, this, [=]() {
            ui->centralWidget->setEnabled(true);
            FEE.checkPMlinks();
            FEE.apply_RESET_ERRORS();
            //FEE.fullSyncTimer->start(10000);
        });
        connect(&FEE, &IPbusTarget::noResponse, this, [=]() {
            statusBar()->showMessage(statusBar()->currentMessage() == "" ? FEE.IPaddress + ": no response" : "");
            ui->centralWidget->setDisabled(true);
        });
        connect(&FEE, &FITelectronics::valuesReady, this, [=]() {
            statusBar()->showMessage(statusBar()->currentMessage() == "" ? FEE.IPaddress + ": online" : "");
            updateActualValues();
            if (!enableControls->isChecked()) updateEdits();
        });
        connect(&FEE, &FITelectronics::countersReady, this, &MainWindow::updateCounters);
        connect(ui->labelValuePhase_A,          &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxPhase_A         ->setValue(val.toDouble(     )); });
        connect(ui->labelValuePhase_C,          &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxPhase_C         ->setValue(val.toDouble(     )); });
        connect(ui->labelValueLaserPhase,       &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxLaserPhase      ->setValue(val.toDouble(     )); });
        connect(ui->labelValueAttenuation,      &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxAttenuation     ->setValue(val.toDouble(     )); });
        connect(ui->labelValueLaserFreqDivider, &ActualLabel::doubleclicked, this, [=](QString val) { ui->spinBoxLaserFreqDivider->setValue(val.toUInt(&ok, 16)); });
        foreach (ActualLabel *label, ui->centralWidget->findChildren<ActualLabel *>()) {
            QLineEdit *e = ui->centralWidget->findChild<QLineEdit *>(label->objectName().replace("labelValue", "lineEdit"));
            if (e != nullptr) connect(label, &ActualLabel::doubleclicked, [=](QString text) { e->setText(text); emit e->textEdited(text); });
        }
		//connect(ui->labelValueLaserFrequency, &ActualLabel::doubleclicked, ui->lineEditLaserFrequency, &QLineEdit::textEdited);
        connect(&FEE, &FITelectronics::linksStatusReady, this, [=]() { //disable PMs' selectors and link indicators if no physical link present
            if (!FEE.isTCM && !FEE.PM.contains(FEE.curPM->set.GBT.RDH_FEE_ID)) ui->TCM_selector->toggle();
			for (quint8 i=0; i<=9; ++i) {
				selectorsPMA[i]->setEnabled(FEE.PM.contains(FEE.allPMs[i   ].set.GBT.RDH_FEE_ID));
                selectorsPMC[i]->setEnabled(FEE.PM.contains(FEE.allPMs[i+10].set.GBT.RDH_FEE_ID));
            } 
        });
        for (quint8 i=0; i<=9; ++i) { //PM switchers and selectors
            connect(switchesPMA[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) {
                if (checked) FEE.apply_SwChOff(FEE.TCMid, 0xA0 + i);
                else         FEE.apply_SwChOn (FEE.TCMid, 0xA0 + i);
            });
            connect(switchesPMC[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) {
                if (checked) FEE.apply_SwChOff(FEE.TCMid, 0xC0 + i);
                else         FEE.apply_SwChOn (FEE.TCMid, 0xC0 + i);
            });
            connect(selectorsPMA[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) { if (checked) selectPM(FEE.TCMid + 0xA0 + i); });
            connect(selectorsPMC[i], QOverload<bool>::of(&QPushButton::clicked), this, [=](bool checked) { if (checked) selectPM(FEE.TCMid + 0xC0 + i); });
        }

        for (quint8 i=0; i<12; ++i) { //channels
            connect(editsTimeAlignmentCh  [i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.TIME_ALIGN[i].value = text. toInt(); resetHighlight(); ui->labelTextTimeAlignment  ->setStyleSheet(highlightStyle); });
            connect(editsThresholdCalibrCh[i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.THRESHOLD_CALIBR[i] = text. toInt(); resetHighlight(); ui->labelTextThresholdCalibr->setStyleSheet(highlightStyle); });
            connect(editsADCdelayCh       [i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.Ch[i].ADC_DELAY     = text.toUInt(); resetHighlight(); ui->labelTextADCdelay       ->setStyleSheet(highlightStyle); });
            connect(editsCFDthresholdCh   [i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.Ch[i].CFD_THRESHOLD = text.toUInt(); resetHighlight(); ui->labelTextCFDthreshold   ->setStyleSheet(highlightStyle); });
            connect(editsADCzeroCh        [i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.Ch[i].ADC_ZERO      = text. toInt(); resetHighlight(); ui->labelTextADCzero        ->setStyleSheet(highlightStyle); });
            connect(editsCFDzeroCh        [i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.Ch[i].CFD_ZERO      = text. toInt(); resetHighlight(); ui->labelTextCFDzero        ->setStyleSheet(highlightStyle); });
            connect(editsADC0rangeCh      [i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.ADC_RANGE    [i][0] = text.toUInt(); resetHighlight(); ui->labelTextADC0           ->setStyleSheet(highlightStyle); });
            connect(editsADC1rangeCh      [i], &QLineEdit::textEdited, this, [=](QString text) { FEE.curPM->set.ADC_RANGE    [i][1] = text.toUInt(); resetHighlight(); ui->labelTextADC1           ->setStyleSheet(highlightStyle); });

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

            connect(buttonsTimeAlignmentCh  [i], &QPushButton::clicked, this, [=] { FEE.apply_TIME_ALIGN      (FEE.selectedBoard, i+1); });
            connect(buttonsThresholdCalibrCh[i], &QPushButton::clicked, this, [=] { FEE.apply_THRESHOLD_CALIBR(FEE.selectedBoard, i+1); });
            connect(buttonsADCdelayCh       [i], &QPushButton::clicked, this, [=] { FEE.apply_ADC_DELAY       (FEE.selectedBoard, i+1); });
            connect(buttonsCFDthresholdCh   [i], &QPushButton::clicked, this, [=] { FEE.apply_CFD_THRESHOLD   (FEE.selectedBoard, i+1); });
            connect(buttonsADCzeroCh        [i], &QPushButton::clicked, this, [=] { FEE.apply_ADC_ZERO        (FEE.selectedBoard, i+1); });
            connect(buttonsCFDzeroCh        [i], &QPushButton::clicked, this, [=] { FEE.apply_CFD_ZERO        (FEE.selectedBoard, i+1); });
            connect(buttonsADC0rangeCh      [i], &QPushButton::clicked, this, [=] { FEE.apply_ADC0_RANGE      (FEE.selectedBoard, i+1); });
            connect(buttonsADC1rangeCh      [i], &QPushButton::clicked, this, [=] { FEE.apply_ADC1_RANGE      (FEE.selectedBoard, i+1); });
			connect(switchesCh[i], &Switch::clicked, this, [=](bool checked) { checked ? FEE.apply_SwChOff(FEE.selectedBoard, i+1) : FEE.apply_SwChOn(FEE.selectedBoard, i+1); });
        }
        for (quint8 i=0; i<64; ++i) { //laser pattern bits switching
            connect(switchBitButtons[i], &QPushButton::clicked, this, [=](bool checked) { FEE.apply_SwLaserPatternBit(i, checked); });
        }
//validators
        foreach (QLineEdit *e, editsTimeAlignmentCh + editsThresholdCalibrCh + editsADCdelayCh + editsCFDthresholdCh + editsADCzeroCh + editsADC0rangeCh + editsADC1rangeCh) {
            e->setValidator(intValidator);
        }
        ui->lineEditLaserFrequency->setValidator(doubleValidator);
        ui->lineEditTriggersLevelA_C->setValidator(uint16Validator);
        ui->lineEditTriggersLevelA_SC->setValidator(uint16Validator);
        ui->lineEditTriggersLevelC_C->setValidator(uint16Validator);
        ui->lineEditTriggersLevelC_SC->setValidator(uint16Validator);
		QString IPaddress = settings.value("IPaddress", FEE.IPaddress).toString();
        if (validIPaddressRE.exactMatch(IPaddress)) FEE.IPaddress = IPaddress;

        FEE.reconnect();
//read settings
        if (settings.contains("TCM")) memcpy(&FEE.TCM.set, settings.value("TCM").toByteArray().data(), sizeof(TypeTCM::Settings));
        foreach (TypePM *pm, FEE.PM) if (settings.contains(QString("PM") + pm->name)) memcpy(&(pm->set), settings.value(QString("PM") + pm->name).toByteArray().data(), sizeof(TypePM::Settings));
		updateEdits();
        resetHighlight();
    }

    ~MainWindow() {
        settings.setValue("subdetector", FIT[FEE.subdetector].name);
        settings.setValue("IPaddress", FEE.IPaddress);
		settings.setValue("TCM", QByteArray( (char *)&FEE.TCM.set, sizeof(TypeTCM::Settings) ));
        foreach (TypePM *pm, FEE.PM) settings.setValue(QString("PM") + pm->name, QByteArray( (char *)&(pm->set), sizeof(TypePM::Settings) ));
        delete ui;
    }

public slots:
    bool fileWrite(QString fileName) {
        QSettings PMsettings(fileName, QSettings::IniFormat);
        //PMsettings.setValue("CH_MASK", FEE.curPM)
        return true;
    }

    bool fileRead(QString fileName) {
        QSettings PMsettings(fileName, QSettings::IniFormat);

        return true;
    }

    void load() {
        QFileDialog dialog(this);
		dialog.setWindowModality(Qt::WindowModal);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		if (dialog.exec() != QDialog::Accepted || !fileRead(dialog.selectedFiles().first()))
            statusBar()->showMessage("File not loaded");
        else
            statusBar()->showMessage("File loaded", 2000);
    }

    void save() {
        QFileDialog dialog(this);
		dialog.setWindowModality(Qt::WindowModal);
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		if (dialog.exec() != QDialog::Accepted || !fileWrite(dialog.selectedFiles().first()))
            statusBar()->showMessage("File not saved");
        else
            statusBar()->showMessage("File saved", 2000);
    }

	void recheckTarget() {
		statusBar()->showMessage(FEE.IPaddress + ": status requested...");
        //if (FEE.fullSyncTimer->isActive()) FEE.fullSyncTimer->stop();
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
        ui->labelIconSystemRestarted->setPixmap(FEE.TCM.act.systemRestarted ? Red1 : Green0);
        ui->labelIconSystemRestarting->setPixmap(FEE.TCM.act.resetSystem ? Red1 : Green0);
        if (FEE.TCM.act.forceLocalClock) ui->radioButtonForceLocal->setChecked(true); else ui->radioButtonAuto->setChecked(true);
        ui->labelValueClockSource->setText(FEE.TCM.act.externalClock ? "external" : "local");
        ui->labelValueClockSource->setStyleSheet(FEE.TCM.act.externalClock ? OKstyle : notOKstyle);
        double
            curTemp_board   = FEE.isTCM ? FEE.TCM.act.TEMP_BOARD    : FEE.curPM->act.TEMP_BOARD,
            curTemp_FPGA    = FEE.isTCM ? FEE.TCM.act.TEMP_FPGA     : FEE.curPM->act.TEMP_FPGA ,
            curVoltage_1V   = FEE.isTCM ? FEE.TCM.act.VOLTAGE_1V    : FEE.curPM->act.VOLTAGE_1V,
            curVoltage_1_8V = FEE.isTCM ? FEE.TCM.act.VOLTAGE_1_8V  : FEE.curPM->act.VOLTAGE_1_8V;
        ui->labelValueBoardTemperature->setText(QString::asprintf("%4.1f°C", curTemp_board   ));
        ui->labelValueFPGAtemperature ->setText(QString::asprintf("%5.1f°C", curTemp_FPGA    ));
        ui->labelValueVoltage1V       ->setText(QString::asprintf("%5.3f V", curVoltage_1V   ));
        ui->labelValueVoltage1_8V     ->setText(QString::asprintf("%5.3f V", curVoltage_1_8V ));
        ui->labelValueBoardTemperature->setStyleSheet(fabs(curTemp_board      - 35) > 25  ? notOKstyle : neutralStyle);
        ui->labelValueFPGAtemperature ->setStyleSheet(fabs(curTemp_FPGA       - 35) > 25  ? notOKstyle : neutralStyle);
        ui->labelValueVoltage1V       ->setStyleSheet(fabs(curVoltage_1V  /1.0 - 1) > 0.2 ? notOKstyle : neutralStyle);
        ui->labelValueVoltage1_8V     ->setStyleSheet(fabs(curVoltage_1_8V/1.8 - 1) > 0.2 ? notOKstyle : neutralStyle);

        ui->labelValueSerial          ->setText(QString::asprintf("%d"     , FEE.isTCM ? FEE.TCM.act.SERIAL_NUM    : FEE.curPM->act.SERIAL_NUM   ));
        TypeFITsubdetector bt = TypeFITsubdetector(FEE.isTCM ? FEE.TCM.act.boardType : FEE.curPM->act.boardType);
        ui->labelValueBoardType->setText(QString::asprintf("%d: %s", bt, FIT[bt].name));
        ui->labelValueBoardType->setStyleSheet(bt != FEE.subdetector ? notOKstyle : neutralStyle);
        Timestamp tMCU  = FEE.isTCM ? FEE.TCM.act.FW_TIME_MCU : FEE.curPM->act.FW_TIME_MCU;
        Timestamp tFPGA = FEE.isTCM ? FEE.TCM.act.   FW_TIME_FPGA : FEE.curPM->act.   FW_TIME_FPGA;
        ui->labelValueMCUFWversion ->setText(tMCU .printCode1());
        ui->labelValueFPGAFWversion->setText(tFPGA.printCode1());
        QString tMCUfull  = tMCU .printFull();
        QString tFPGAfull = tFPGA.printFull();
        ui->labelTextMCUFWversion ->setToolTip(tMCUfull);
        ui->labelValueMCUFWversion->setToolTip(tMCUfull);
        ui->labelTextFPGAFWversion ->setToolTip(tFPGAfull);
        ui->labelValueFPGAFWversion->setToolTip(tFPGAfull);
		ui->comboBoxUpdatePeriod->setCurrentIndex(FEE.TCM.act.COUNTERS_UPD_RATE);
        for (quint8 i=0; i<=9; ++i) {
            linksPMA[i]->setPixmap(FEE.TCM.act.TRG_SYNC_A[i].linkOK ? Green1 : Red0); switchesPMA[i]->setChecked(FEE.TCM.act.CH_MASK_A & (1 << i));
            linksPMC[i]->setPixmap(FEE.TCM.act.TRG_SYNC_C[i].linkOK ? Green1 : Red0); switchesPMC[i]->setChecked(FEE.TCM.act.CH_MASK_C & (1 << i));
        }
        switch (FEE.curGBTact->Control.DG_MODE) {
            case GBTunit::DG_noData: ui->buttonDataGeneratorOff ->setChecked(true); break;
            case GBTunit::DG_main  : ui->buttonDataGeneratorMain->setChecked(true); break;
            case GBTunit::DG_Tx    : ui->buttonDataGeneratorTx  ->setChecked(true);
        }
        switch (FEE.curGBTact->Control.TG_MODE) {
            case GBTunit::TG_noTrigger : ui->buttonTriggerGeneratorOff		 ->setChecked(true); break;
            case GBTunit::TG_continuous: ui->buttonTriggerGeneratorContinuous->setChecked(true); break;
            case GBTunit::TG_Tx        : ui->buttonTriggerGeneratorTx        ->setChecked(true);
		}
        ui->labelValueDGtriggerRespondMask  ->setText(QString::asprintf("%08X",      FEE.curGBTact->Control.DG_TRG_RESPOND_MASK  ));
        ui->labelValueDGbunchPattern        ->setText(QString::asprintf("%08X",      FEE.curGBTact->Control.DG_BUNCH_PATTERN     ));
        ui->labelValueDGbunchFrequency      ->setText(QString::asprintf("%04X",      FEE.curGBTact->Control.DG_BUNCH_FREQ        ));
        ui->labelValueDGfrequencyOffset     ->setText(QString::asprintf("%03X",      FEE.curGBTact->Control.DG_FREQ_OFFSET       ));
        ui->labelValueTGcontinuousPattern   ->setText(QString::asprintf("%08X %08X", FEE.curGBTact->Control.TG_PATTERN_1,
                                                                                     FEE.curGBTact->Control.TG_PATTERN_0));
        ui->labelValueTGcontinuousValue     ->setText(QString::asprintf("%08X",      FEE.curGBTact->Control.TG_CONT_VALUE        ));
        ui->labelValueTGbunchFrequency      ->setText(QString::asprintf("%04X",      FEE.curGBTact->Control.TG_BUNCH_FREQ        ));
        ui->labelValueTGfrequencyOffset     ->setText(QString::asprintf("%03X",      FEE.curGBTact->Control.TG_FREQ_OFFSET       ));
        switch (FEE.curGBTact->Control.TG_CTP_EMUL_MODE) {
            case GBTunit::RO_idle       : ui->buttonCTPmodeIdle      ->setChecked(true); break;
            case GBTunit::RO_continuous : ui->buttonCTPmodeContinouos->setChecked(true); break;
            case GBTunit::RO_triggered  : ui->buttonCTPmodeTriggered ->setChecked(true);
        }
        ui->labelValueFEEID                 ->setText(QString::asprintf("%04X",      FEE.curGBTact->Control.RDH_FEE_ID           ));
        ui->labelValueMaxPayload            ->setText(QString::asprintf("%04X",      FEE.curGBTact->Control.RDH_MAX_PAYLOAD      ));
        ui->labelValuePAR                   ->setText(QString::asprintf("%04X",      FEE.curGBTact->Control.RDH_PAR              ));
        ui->labelValueDETfield              ->setText(QString::asprintf("%04X",      FEE.curGBTact->Control.RDH_DET_FIELD        ));
        ui->labelValueCRUtriggerCompareDelay->setText(QString::asprintf("%03X",      FEE.curGBTact->Control.CRU_TRG_COMPARE_DELAY));
        ui->labelValueBCIDdelay             ->setText(QString::asprintf("%03X",      FEE.curGBTact->Control.BCID_DELAY           ));
        ui->labelValueDataSelectTriggerMask ->setText(QString::asprintf("%08X",      FEE.curGBTact->Control.DATA_SEL_TRG_MASK    ));
        ui->SwitcherLockReadout->setChecked(!(FEE.curGBTact->Control.READOUT_LOCK));
        ui->SwitcherBypassMode ->setChecked(  FEE.curGBTact->Control.BYPASS_MODE);
        ui->SwitcherHBresponse ->setChecked(  FEE.curGBTact->Control.HB_RESPONSE);
        switch (FEE.curGBTact->Status.READOUT_MODE) {
            case GBTunit::RO_idle      : ui->labelValueReadoutModeBoard->setText("Idle"      ); break;
            case GBTunit::RO_continuous: ui->labelValueReadoutModeBoard->setText("Continuous"); break;
            case GBTunit::RO_triggered : ui->labelValueReadoutModeBoard->setText("Triggered" );
        }
        switch (FEE.curGBTact->Status.CRU_READOUT_MODE) {
            case GBTunit::RO_idle      : ui->labelValueReadoutModeCRU  ->setText("Idle"      ); break;
            case GBTunit::RO_continuous: ui->labelValueReadoutModeCRU  ->setText("Continuous"); break;
            case GBTunit::RO_triggered : ui->labelValueReadoutModeCRU  ->setText("Triggered" );
        }
        switch (FEE.curGBTact->Status.BCID_SYNC_MODE) {
            case GBTunit::BS_start: ui->labelValueBCIDsync->setText("Start"); ui->labelValueBCIDsync->setStyleSheet(neutralStyle); break;
            case GBTunit::BS_sync : ui->labelValueBCIDsync->setText("Sync" ); ui->labelValueBCIDsync->setStyleSheet(     OKstyle); break;
            case GBTunit::BS_lost : ui->labelValueBCIDsync->setText("Lost" ); ui->labelValueBCIDsync->setStyleSheet(  notOKstyle);
        }

        ui->labelValueCRUorbit           ->setText(QString::asprintf("%08X", FEE.curGBTact->Status.CRU_ORBIT                  ));
        ui->labelValueCRUBC              ->setText(QString::asprintf("%03X", FEE.curGBTact->Status.CRU_BC                     ));
        ui->labelValueSelectorFIFOcount  ->setText(QString::asprintf("%04X", FEE.curGBTact->Status.SEL_FIFO                   ));
        ui->labelValueRawFIFOcount       ->setText(QString::asprintf("%04X", FEE.curGBTact->Status.RAW_FIFO                   ));
        ui->labelValueSelectorFirstHitDO ->setText(QString::asprintf("%08X", FEE.curGBTact->Status.SEL_FIRST_HIT_DROPPED_ORBIT));
        ui->labelValueSelectorLastHitDO  ->setText(QString::asprintf("%08X", FEE.curGBTact->Status.SEL_LAST_HIT_DROPPED_ORBIT ));
        ui->labelValueSelectorHitsDropped->setText(QString::asprintf("%08X", FEE.curGBTact->Status.SEL_HITS_DROPPED           ));
        ui->labelValueReadoutRate        ->setText(QString::asprintf("%04X", FEE.curGBTact->Status.READOUT_RATE               ));
        ui->labelValueRxPhase            ->setText(QString::asprintf("%01X", FEE.curGBTact->Status.RX_PHASE                   ));
        ui->labelIconPhaseAlignerCPLLlock->setPixmap(FEE.curGBTact->Status.BITS & 0x0001 ? Green1 : Red0);
        ui->labelIconRxWorldclkReady     ->setPixmap(FEE.curGBTact->Status.BITS & 0x0002 ? Green1 : Red0);
        ui->labelIconRxFrameclkReady     ->setPixmap(FEE.curGBTact->Status.BITS & 0x0004 ? Green1 : Red0);
        ui->labelIconMGTlinkReady        ->setPixmap(FEE.curGBTact->Status.BITS & 0x0008 ? Green1 : Red0);
        ui->labelIconTxResetDone         ->setPixmap(FEE.curGBTact->Status.BITS & 0x0010 ? Green1 : Red0);
        ui->labelIconTxFSMresetDone      ->setPixmap(FEE.curGBTact->Status.BITS & 0x0020 ? Green1 : Red0);
        ui->labelIconGBTRxReady          ->setPixmap(FEE.curGBTact->Status.BITS & 0x0040 ? Green1 : Red0);
        ui->labelIconGBTRxErrorDetected  ->setPixmap(FEE.curGBTact->Status.BITS & 0x0080 ? Red1 : Green0);
        ui->labelIconGBTRxErrorLatch     ->setPixmap(FEE.curGBTact->Status.BITS & 0x0100 ? Red1 : Green0);
        ui->labelIconRxPhaseError        ->setPixmap(FEE.curGBTact->Status.BITS & 0x0200 ? Red1 : Green0);

        if (FEE.isTCM) {
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
            ui->labelValueChannelMask_A->setText(QString::asprintf("%03X", FEE.TCM.act.CH_MASK_A));
            ui->labelValueChannelMask_C->setText(QString::asprintf("%03X", FEE.TCM.act.CH_MASK_C));
            ui->labelValuePhase_A->setText(QString::asprintf("%7.3f", FEE.TCM.act.delayAside_ns));
            ui->labelValuePhase_C->setText(QString::asprintf("%7.3f", FEE.TCM.act.delayCside_ns));
			if (prevPhaseStep_ns != phaseStep_ns) {
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
            ui->SwitcherExt1->setChecked(FEE.TCM.act.EXT_SW & 1);
            ui->SwitcherExt2->setChecked(FEE.TCM.act.EXT_SW & 2);
            ui->SwitcherExt3->setChecked(FEE.TCM.act.EXT_SW & 4);
            ui->SwitcherExt4->setChecked(FEE.TCM.act.EXT_SW & 8);
            ui->SwitcherExtendedReadout->setChecked(FEE.TCM.act.EXTENDED_READOUT);
			ui->SwitcherAddCdelay->setChecked(FEE.TCM.act.ADD_C_DELAY);
			ui->SwitcherTriggers_SC ->setChecked(FEE.TCM.act.SC_ENABLED );
			ui->SwitcherTriggers_C  ->setChecked(FEE.TCM.act.C_ENABLED  );
            ui->SwitcherTriggers_ORA->setChecked(FEE.TCM.act.ORA_ENABLED);
            ui->SwitcherTriggers_ORC->setChecked(FEE.TCM.act.ORC_ENABLED);
			ui->SwitcherTriggers_V  ->setChecked(FEE.TCM.act.V_ENABLED  );
			ui->comboBoxTriggersMode_SC ->setCurrentIndex(FEE.TCM.act.SC_MODE );
			ui->comboBoxTriggersMode_C  ->setCurrentIndex(FEE.TCM.act.C_MODE  );
            ui->comboBoxTriggersMode_ORA->setCurrentIndex(FEE.TCM.act.ORA_MODE);
            ui->comboBoxTriggersMode_ORC->setCurrentIndex(FEE.TCM.act.ORC_MODE);
			ui->comboBoxTriggersMode_V  ->setCurrentIndex(FEE.TCM.act.V_MODE  );
            ui->labelValueTriggersRandomRate_SC ->setText(QString::asprintf("%08X", FEE.TCM.act.SC_RATE  ));
            ui->labelValueTriggersRandomRate_C  ->setText(QString::asprintf("%08X", FEE.TCM.act.C_RATE   ));
            ui->labelValueTriggersRandomRate_ORA->setText(QString::asprintf("%08X", FEE.TCM.act.ORA_RATE));
            ui->labelValueTriggersRandomRate_ORC->setText(QString::asprintf("%08X", FEE.TCM.act.ORC_RATE));
            ui->labelValueTriggersRandomRate_V  ->setText(QString::asprintf("%08X", FEE.TCM.act.V_RATE   ));
			ui->labelValueTriggersSignature_SC ->setText(QString::asprintf("%d", FEE.TCM.act.SC_SIGN  & 0x7F)); //7 bit of signature
			ui->labelValueTriggersSignature_C  ->setText(QString::asprintf("%d", FEE.TCM.act.C_SIGN   & 0x7F));
			ui->labelValueTriggersSignature_ORA->setText(QString::asprintf("%d", FEE.TCM.act.ORA_SIGN & 0x7F));
			ui->labelValueTriggersSignature_ORC->setText(QString::asprintf("%d", FEE.TCM.act.ORC_SIGN & 0x7F));
			ui->labelValueTriggersSignature_V  ->setText(QString::asprintf("%d", FEE.TCM.act.V_SIGN   & 0x7F));
            ui->labelValueTriggersLevelA_SC->setText(QString::asprintf("%d", FEE.TCM.act.SC_LEVEL_A));
            ui->labelValueTriggersLevelC_SC->setText(QString::asprintf("%d", FEE.TCM.act.SC_LEVEL_C));
            ui->labelValueTriggersLevelA_C ->setText(QString::asprintf("%d", FEE.TCM.act.C_LEVEL_A ));
            ui->labelValueTriggersLevelC_C ->setText(QString::asprintf("%d", FEE.TCM.act.C_LEVEL_C ));
            mode = FEE.TCM.act.C_SC_TRG_MODE;
            allRadioButtons[mode]->setChecked(true);
            ui->labelValueTriggersLevelA_SC->setEnabled(mode != 1); //enabled for modes A&C, A, A+C
            ui->labelValueTriggersLevelA_C ->setEnabled(mode != 1);
            ui->labelValueTriggersLevelC_SC->setEnabled(mode < 2); //enabled for modes A&C, C
            ui->labelValueTriggersLevelC_C ->setEnabled(mode < 2);
            ui->labelValueVertexTimeLow ->setText(QString::asprintf("%d", FEE.TCM.act.VTIME_LOW ));
            ui->labelValueVertexTimeHigh->setText(QString::asprintf("%d", FEE.TCM.act.VTIME_HIGH));
			ui->labelValueAttenuation->setText(QString::asprintf("%5.0f", FEE.TCM.act.attenuation));
            ui->labelIconAttenBusy ->setPixmap(FEE.TCM.act.attenBusy     ? Red1 : Green0);
            ui->labelIconAttenError->setPixmap(FEE.TCM.act.attenNotFound ? Red1 : Green0);
			ui->sliderAttenuation->setDisabled(FEE.TCM.act.attenBusy || FEE.TCM.act.attenNotFound);
            ui->labelValueLaserTriggerMask->setText(QString::asprintf("%08X", FEE.TCM.act.GBT.Control.DG_TRG_RESPOND_MASK));
			FEE.TCM.act.LASER_SOURCE ? ui->radioButtonGenerator->setChecked(true) : ui->radioButtonExternalTrigger->setChecked(true);
            ui->labelValueLaserFreqDivider->setText(QString::asprintf("%06x", FEE.TCM.act.LASER_DIVIDER));
            ui->labelValueLaserFrequency->setText(frequencyFormat(FEE.TCM.act.laserFrequency_Hz));
            ui->labelValueLaserPattern->setText(QString::asprintf("%08X %08X", FEE.TCM.act.LASER_PATTERN_1, FEE.TCM.act.LASER_PATTERN_0));
            ui->labelValueLaserPhase->setText(QString::asprintf("%7.3f", FEE.TCM.act.delayLaser_ns));

            for (quint8 i=0; i<32; ++i) {
                switchBitButtons.at(i     )->setChecked(FEE.TCM.act.LASER_PATTERN_0 & (1 << i));
                switchBitButtons.at(i + 32)->setChecked(FEE.TCM.act.LASER_PATTERN_1 & (1 << i));
            }
        } else { //PM
            ui->labelValueORgate->setText(QString::asprintf("%d", FEE.curPM->act.OR_GATE));
            ui->labelValueCFDsaturation->setText(QString::asprintf("%d", FEE.curPM->act.CFD_SATR));
            switch (FEE.curPM->act.restartReasonCode) {
                case 0 : ui->labelValueRestartCode->setText("power reset"); ui->labelValueRestartCode->setStyleSheet(notOKstyle); break;
                case 1 : ui->labelValueRestartCode->setText("FPGA reset" ); ui->labelValueRestartCode->setStyleSheet(notOKstyle); break;
                case 2 : ui->labelValueRestartCode->setText("PLL relock" ); ui->labelValueRestartCode->setStyleSheet(notOKstyle); break;
                case 3 : ui->labelValueRestartCode->setText("SPI command"); ui->labelValueRestartCode->setStyleSheet(   OKstyle); break;
            }
            FEE.curPM->act.TRGcountMode ? ui->radioButtonCFDinGate->setChecked(true) : ui->radioButtonStrict->setChecked(true);
            ui->labelValueChannelMask->setText(QString::asprintf("%03X", FEE.curPM->act.CH_MASK_DATA));
            ui->labelIconSyncErrorTDC1->setPixmap(FEE.curPM->act.TDC1syncError ? Red1 : Green0);
            ui->labelIconSyncErrorTDC2->setPixmap(FEE.curPM->act.TDC2syncError ? Red1 : Green0);
            ui->labelIconSyncErrorTDC3->setPixmap(FEE.curPM->act.TDC3syncError ? Red1 : Green0);
            ui->labelIconPLLlockedTDC1->setPixmap(FEE.curPM->act.TDC1PLLlocked ? Green1 : Red0);
            ui->labelIconPLLlockedTDC2->setPixmap(FEE.curPM->act.TDC2PLLlocked ? Green1 : Red0);
            ui->labelIconPLLlockedTDC3->setPixmap(FEE.curPM->act.TDC3PLLlocked ? Green1 : Red0);
            ui->labelIconPLLlockedMain->setPixmap(FEE.curPM->act.mainPLLlocked ? Green1 : Red0);
			ui->labelValuePhaseTuningTDC1->setText(QString::asprintf("%.0f", FEE.curPM->act.TDC1tuning * TDCunit_ps * 8/7));
			ui->labelValuePhaseTuningTDC2->setText(QString::asprintf("%.0f", FEE.curPM->act.TDC2tuning * TDCunit_ps * 8/7));
			ui->labelValuePhaseTuningTDC3->setText(QString::asprintf("%.0f", FEE.curPM->act.TDC3tuning * TDCunit_ps * 8/7));

            quint8 iPM = FEE.curPM->baseAddress / 0x200 - 1;
            TRGsyncStatus *s = (iPM > 9 ? FEE.TCM.act.TRG_SYNC_C : FEE.TCM.act.TRG_SYNC_A) + iPM % 10;
            ui->labelIconHDMIsyncError->setPixmap((iPM > 9 ? FEE.TCM.act.syncErrorInLinkC : FEE.TCM.act.syncErrorInLinkA) & (1 << iPM % 10) ?  Red1 : Green0);
            ui->labelIconHDMIlinkOK->setPixmap(s->linkOK ? Green1 : Red0);
            ui->labelIconHDMIbitsOK->setPixmap(s->bitPositionsOK ? Green1 : Red0);
            ui->labelIconHDMIsignalLost0->setPixmap(s->line0signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalLost1->setPixmap(s->line1signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalLost2->setPixmap(s->line2signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalLost3->setPixmap(s->line3signalLost ? Red1 : Green0);
            ui->labelIconHDMIsignalStable0->setPixmap(s->line0signalStable ? Green1 : Red0);
            ui->labelIconHDMIsignalStable1->setPixmap(s->line1signalStable ? Green1 : Red0);
            ui->labelIconHDMIsignalStable2->setPixmap(s->line2signalStable ? Green1 : Red0);
            ui->labelIconHDMIsignalStable3->setPixmap(s->line3signalStable ? Green1 : Red0);
            ui->labelValueHDMIdelay0->setText(QString::asprintf("%4.2f", s->line0delay * TDCunit_ps * 0.006));
            ui->labelValueHDMIdelay1->setText(QString::asprintf("%4.2f", s->line1delay * TDCunit_ps * 0.006));
            ui->labelValueHDMIdelay2->setText(QString::asprintf("%4.2f", s->line2delay * TDCunit_ps * 0.006));
            ui->labelValueHDMIdelay3->setText(QString::asprintf("%4.2f", s->line3delay * TDCunit_ps * 0.006));
            for (quint8 i=0; i<12; ++i) {
                switchesCh[i]->setChecked(FEE.curPM->act.CH_MASK_DATA & (1 << i));
                labelsTimeAlignmentCh    [i]->setText(QString::asprintf("%d", FEE.curPM->act.TIME_ALIGN[i]));
                labelsThresholdCalibrCh  [i]->setText(QString::asprintf("%d", FEE.curPM->act.THRESHOLD_CALIBR[i]));
                labelsADCdelayCh         [i]->setText(QString::asprintf("%d", FEE.curPM->act.Ch[i].ADC_DELAY));
                labelsCFDthresholdCh     [i]->setText(QString::asprintf("%d", FEE.curPM->act.Ch[i].CFD_THRESHOLD));
                labelsADCzeroCh          [i]->setText(QString::asprintf("%d", FEE.curPM->act.Ch[i].ADC_ZERO));
                labelsCFDzeroCh          [i]->setText(QString::asprintf("%d", FEE.curPM->act.Ch[i].CFD_ZERO));
                labelsADC0rangeCh        [i]->setText(QString::asprintf("%d", FEE.curPM->act.ADC_RANGE[i][0]));
                labelsADC1rangeCh        [i]->setText(QString::asprintf("%d", FEE.curPM->act.ADC_RANGE[i][1]));
                labelsADC0baseLineCh     [i]->setText(QString::asprintf("%d", FEE.curPM->act.ADC_BASELINE[i][0]));
                labelsADC1baseLineCh     [i]->setText(QString::asprintf("%d", FEE.curPM->act.ADC_BASELINE[i][1]));
                labelsADC0baseLineCh     [i]->setStyleSheet(FEE.curPM->act.CH_BASELINES_NOK & (1 << i) ? notOKstyle : neutralStyle);
                labelsADC1baseLineCh     [i]->setStyleSheet(FEE.curPM->act.CH_BASELINES_NOK & (1 << i) ? notOKstyle : neutralStyle);
                labelsADC0RMSCh          [i]->setText(QString::asprintf( "%5.1f", FEE.curPM->act.RMS_Ch[i][0]));
                labelsADC1RMSCh          [i]->setText(QString::asprintf( "%5.1f", FEE.curPM->act.RMS_Ch[i][0]));
                labelsADC0meanAmplitudeCh[i]->setText(QString::asprintf("%d", FEE.curPM->act.MEANAMPL[i][0][0]));
                labelsADC1meanAmplitudeCh[i]->setText(QString::asprintf("%d", FEE.curPM->act.MEANAMPL[i][1][0]));
                labelsRawTDCdata1Ch      [i]->setText(QString::asprintf("%02X", FEE.curPM->act.RAW_TDC_DATA[i][0]));
                labelsRawTDCdata2Ch      [i]->setText(QString::asprintf("%02X", FEE.curPM->act.RAW_TDC_DATA[i][1]));
            }
        }
    }

    void updateEdits() {
        if (FEE.isTCM) {
			ui->lineEditChannelMask_A->setText(QString::asprintf("%03X", FEE.TCM.set.CH_MASK_A));
			ui->lineEditChannelMask_C->setText(QString::asprintf("%03X", FEE.TCM.set.CH_MASK_C));
			ui->spinBoxPhase_A->setValue(FEE.TCM.set.delayAside_ns);
			ui->spinBoxPhase_C->setValue(FEE.TCM.set.delayCside_ns);
			ui->lineEditTriggersRandomRate_SC ->setText(QString::asprintf("%08X", FEE.TCM.set. SC_RATE));
			ui->lineEditTriggersRandomRate_C  ->setText(QString::asprintf("%08X", FEE.TCM.set.  C_RATE));
			ui->lineEditTriggersRandomRate_V  ->setText(QString::asprintf("%08X", FEE.TCM.set.  V_RATE));
			ui->lineEditTriggersRandomRate_ORA->setText(QString::asprintf("%08X", FEE.TCM.set.ORA_RATE));
			ui->lineEditTriggersRandomRate_ORC->setText(QString::asprintf("%08X", FEE.TCM.set.ORC_RATE));
			ui->lineEditTriggersSignature_SC ->setText(QString::asprintf("%d", FEE.TCM.set. SC_SIGN));
			ui->lineEditTriggersSignature_C  ->setText(QString::asprintf("%d", FEE.TCM.set.  C_SIGN));
			ui->lineEditTriggersSignature_V  ->setText(QString::asprintf("%d", FEE.TCM.set.  V_SIGN));
			ui->lineEditTriggersSignature_ORA->setText(QString::asprintf("%d", FEE.TCM.set.ORA_SIGN));
			ui->lineEditTriggersSignature_ORC->setText(QString::asprintf("%d", FEE.TCM.set.ORC_SIGN));
			ui->lineEditTriggersLevelA_SC->setText(QString::asprintf("%d", FEE.TCM.set.SC_LEVEL_A));
			ui->lineEditTriggersLevelC_SC->setText(QString::asprintf("%d", FEE.TCM.set.SC_LEVEL_C));
			ui->lineEditTriggersLevelA_C ->setText(QString::asprintf("%d", FEE.TCM.set. C_LEVEL_A));
			ui->lineEditTriggersLevelC_C ->setText(QString::asprintf("%d", FEE.TCM.set. C_LEVEL_C));
			ui->lineEditVertexTimeLow ->setText(QString::asprintf("%d", FEE.TCM.set.VTIME_LOW ));
			ui->lineEditVertexTimeHigh->setText(QString::asprintf("%d", FEE.TCM.set.VTIME_HIGH));
			ui->sliderAttenuation->setValue(FEE.TCM.set.attenSteps);
			if (ui->spinBoxAttenuation->value() != FEE.TCM.set.attenuation) ui->spinBoxAttenuation->setValue(FEE.TCM.set.attenuation);
			ui->lineEditLaserTriggerMask->setText(QString::asprintf("%08X", FEE.TCM.set.GBT.DG_TRG_RESPOND_MASK));
			ui->spinBoxLaserFreqDivider->setValue(FEE.TCM.set.LASER_DIVIDER);
			ui->lineEditLaserPattern->setText(QString::asprintf("%08X %08X", FEE.TCM.set.LASER_PATTERN_1, FEE.TCM.set.LASER_PATTERN_0));
			ui->sliderLaser->setValue(FEE.TCM.set.LASER_DELAY);
			if (ui->spinBoxLaserPhase->value() != FEE.TCM.set.delayLaser_ns) ui->spinBoxLaserPhase->setValue(FEE.TCM.set.delayLaser_ns);
		} else { //PM
			ui->lineEditORgate->setText(QString::asprintf("%d", FEE.curPM->set.OR_GATE));
			ui->lineEditCFDsaturation->setText(QString::asprintf("%d", FEE.curPM->set.CFD_SATR));
            ui->lineEditChannelMask->setText(QString::asprintf("%03X", FEE.curPM->set.CH_MASK_DATA));
			for (quint8 i=0; i<12; ++i) {
                editsTimeAlignmentCh  [i]->setText(QString::asprintf("%d", FEE.curPM->set.TIME_ALIGN[i].value));
				editsThresholdCalibrCh[i]->setText(QString::asprintf("%d", FEE.curPM->set.THRESHOLD_CALIBR[i]));
				editsADCdelayCh       [i]->setText(QString::asprintf("%d", FEE.curPM->set.Ch[i].ADC_DELAY));
				editsCFDthresholdCh   [i]->setText(QString::asprintf("%d", FEE.curPM->set.Ch[i].CFD_THRESHOLD));
				editsADCzeroCh        [i]->setText(QString::asprintf("%d", FEE.curPM->set.Ch[i].ADC_ZERO));
				editsCFDzeroCh        [i]->setText(QString::asprintf("%d", FEE.curPM->set.Ch[i].CFD_ZERO));
				editsADC0rangeCh      [i]->setText(QString::asprintf("%d", FEE.curPM->set.ADC_RANGE[i][0]));
				editsADC1rangeCh      [i]->setText(QString::asprintf("%d", FEE.curPM->set.ADC_RANGE[i][1]));
			}
		}
		ui->lineEditDGtriggerRespondMask  ->setText(QString::asprintf("%08X",	   FEE.curGBTset->DG_TRG_RESPOND_MASK  ));
		ui->lineEditDGbunchPattern        ->setText(QString::asprintf("%08X",      FEE.curGBTset->DG_BUNCH_PATTERN     ));
		ui->lineEditDGbunchFrequency      ->setText(QString::asprintf("%04X",      FEE.curGBTset->DG_BUNCH_FREQ        ));
		ui->lineEditDGfrequencyOffset     ->setText(QString::asprintf("%03X",      FEE.curGBTset->DG_FREQ_OFFSET       ));
		ui->lineEditTGcontinuousPattern   ->setText(QString::asprintf("%08X %08X", FEE.curGBTset->TG_PATTERN_1,
																				   FEE.curGBTset->TG_PATTERN_0		   ));
		ui->lineEditTGcontinuousValue     ->setText(QString::asprintf("%08X",      FEE.curGBTset->TG_CONT_VALUE        ));
		ui->lineEditTGbunchFrequency      ->setText(QString::asprintf("%04X",      FEE.curGBTset->TG_BUNCH_FREQ        ));
		ui->lineEditTGfrequencyOffset     ->setText(QString::asprintf("%03X",      FEE.curGBTset->TG_FREQ_OFFSET       ));
		ui->lineEditFEEID                 ->setText(QString::asprintf("%04X",      FEE.curGBTset->RDH_FEE_ID           ));
		ui->lineEditMaxPayload            ->setText(QString::asprintf("%04X",      FEE.curGBTset->RDH_MAX_PAYLOAD      ));
		ui->lineEditPAR                   ->setText(QString::asprintf("%04X",      FEE.curGBTset->RDH_PAR              ));
		ui->lineEditDETfield              ->setText(QString::asprintf("%04X",      FEE.curGBTset->RDH_DET_FIELD        ));
		ui->lineEditCRUtriggerCompareDelay->setText(QString::asprintf("%03X",      FEE.curGBTset->CRU_TRG_COMPARE_DELAY));
		ui->lineEditBCIDdelay             ->setText(QString::asprintf("%03X",      FEE.curGBTset->BCID_DELAY           ));
		ui->lineEditDataSelectTriggerMask ->setText(QString::asprintf("%08X",      FEE.curGBTset->DATA_SEL_TRG_MASK    ));
        resetHighlight();
    }

    void updateCounters() {
        if (FEE.isTCM) {
            for (quint8 i=0; i<TypeTCM::Counters::number; ++i) {
                labelsTriggersCount[i]->setText(QString::number(FEE.TCM.counters.New[i]));
                labelsTriggersRate[i]->setText(rateFormat(FEE.TCM.counters.rate[i]));
            }
        } else { //PM
            for (quint8 i=0; i<12; ++i) {
                labelsTRGcounterCh[i]->setText(QString::number(FEE.curPM->counters.Ch[i].TRG));
                labelsCFDcounterCh[i]->setText(QString::number(FEE.curPM->counters.Ch[i].CFD));
                labelsTRGcounterRateCh[i]->setText(rateFormat(FEE.curPM->counters.rateCh[i].TRG));
                labelsCFDcounterRateCh[i]->setText(rateFormat(FEE.curPM->counters.rateCh[i].CFD));
            }
        }
    }

    void on_buttonResetDroppingHitCounters_clicked   () { FEE.reset(FEE.selectedBoard, GBTunit::RB_droppingHitCounters  ); }
    void on_buttonResetOffset_clicked                () { FEE.reset(FEE.selectedBoard, GBTunit::RB_generatorsBunchOffset); }
    void on_buttonResetOrbitSync_clicked             () { FEE.reset(FEE.selectedBoard, GBTunit::RB_orbitSync            ); }
    void on_buttonResetGBTerrors_clicked             () { FEE.reset(FEE.selectedBoard, GBTunit::RB_GBTerrors            ); }
    void on_buttonResetGBT_clicked                   () { FEE.reset(FEE.selectedBoard, GBTunit::RB_GBT                  ); }
    void on_buttonResetRxPhaseError_clicked          () { FEE.reset(FEE.selectedBoard, GBTunit::RB_RXphaseError         ); }
    void on_buttonCTPmodeIdle_clicked                () { FEE.apply_TG_CTP_EMUL_MODE(FEE.selectedBoard, GBTunit::RO_idle      ); }
    void on_buttonCTPmodeContinouos_clicked          () { FEE.apply_TG_CTP_EMUL_MODE(FEE.selectedBoard, GBTunit::RO_continuous); }
    void on_buttonCTPmodeTriggered_clicked           () { FEE.apply_TG_CTP_EMUL_MODE(FEE.selectedBoard, GBTunit::RO_triggered ); }
    void on_buttonDataGeneratorOff_clicked           () { FEE.apply_DG_MODE(FEE.selectedBoard, GBTunit::DG_noData); }
    void on_buttonDataGeneratorMain_clicked          () { FEE.apply_DG_MODE(FEE.selectedBoard, GBTunit::DG_main  ); }
    void on_buttonDataGeneratorTx_clicked            () { FEE.apply_DG_MODE(FEE.selectedBoard, GBTunit::DG_Tx    ); }
    void on_buttonApplyDGtriggerRespondMask_clicked  () { FEE.apply_DG_TRG_RESPOND_MASK(FEE.selectedBoard); }
    void on_buttonApplyDGbunchPattern_clicked        () { FEE.apply_DG_BUNCH_PATTERN   (FEE.selectedBoard); }
    void on_buttonApplyDGbunchFrequency_clicked      () { FEE.apply_DG_BUNCH_FREQ      (FEE.selectedBoard); }
    void on_buttonApplyDGfrequencyOffset_clicked     () { FEE.apply_DG_FREQ_OFFSET     (FEE.selectedBoard); }
    void on_buttonTriggerGeneratorOff_clicked        () { FEE.apply_TG_MODE(FEE.selectedBoard, GBTunit::TG_noTrigger ); }
    void on_buttonTriggerGeneratorContinuous_clicked () { FEE.apply_TG_MODE(FEE.selectedBoard, GBTunit::TG_continuous); }
    void on_buttonTriggerGeneratorTx_clicked         () { FEE.apply_TG_MODE(FEE.selectedBoard, GBTunit::TG_Tx        ); }
    void on_buttonApplyTGcontinuousPattern_clicked   () { FEE.apply_TG_PATTERN    (FEE.selectedBoard); }
    void on_buttonApplyTGcontinuousValue_clicked     () { FEE.apply_TG_CONT_VALUE (FEE.selectedBoard); }
    void on_buttonApplyTGbunchFrequency_clicked      () { FEE.apply_TG_BUNCH_FREQ (FEE.selectedBoard); }
    void on_buttonApplyTGfrequencyOffset_clicked     () { FEE.apply_TG_FREQ_OFFSET(FEE.selectedBoard); }
    void on_buttonSendSingleValue_clicked            () { FEE.apply_TG_SEND_SINGLE(FEE.selectedBoard, ui->lineEditSendSingleValue->text().toUInt(&ok, 16)); }
    void on_buttonApplyFEEID_clicked                 () { FEE.apply_RDH_FEE_ID           (FEE.selectedBoard); }
    void on_buttonApplyPAR_clicked                   () { FEE.apply_RDH_PAR              (FEE.selectedBoard); }
    void on_buttonApplyMaxPayload_clicked            () { FEE.apply_RDH_MAX_PAYLOAD      (FEE.selectedBoard); }
    void on_buttonApplyDETfield_clicked              () { FEE.apply_RDH_DET_FIELD        (FEE.selectedBoard); }
    void on_buttonApplyCRUtriggerCompareDelay_clicked() { FEE.apply_CRU_TRG_COMPARE_DELAY(FEE.selectedBoard); }
    void on_buttonApplyBCIDdelay_clicked             () { FEE.apply_BCID_DELAY           (FEE.selectedBoard); }
    void on_buttonApplyDataSelectTriggerMask_clicked () { FEE.apply_DATA_SEL_TRG_MASK    (FEE.selectedBoard); }

    void on_SwitcherLockReadout_clicked(bool checked) { FEE.apply_READOUT_LOCK(FEE.selectedBoard,  checked); }
    void on_SwitcherBypassMode_clicked (bool checked) { FEE.apply_BYPASS_MODE (FEE.selectedBoard, !checked); }
    void on_SwitcherHBresponse_clicked (bool checked) { FEE.apply_HB_RESPONSE (FEE.selectedBoard, !checked); }

    void on_lineEditDGtriggerRespondMask_textEdited  () {
        FEE.curGBTset->DG_TRG_RESPOND_MASK = ui->lineEditDGtriggerRespondMask->displayText().toUInt(&ok, 16);
        ui->lineEditLaserTriggerMask->setText(ui->lineEditDGtriggerRespondMask->displayText());
    }
    void on_lineEditDGbunchPattern_textEdited        () { FEE.curGBTset->DG_BUNCH_PATTERN      = ui->lineEditDGbunchPattern        ->displayText().toUInt(&ok, 16); }
    void on_lineEditDGbunchFrequency_textEdited      () { FEE.curGBTset->DG_BUNCH_FREQ         = ui->lineEditDGbunchFrequency      ->displayText().toUInt(&ok, 16); }
    void on_lineEditDGfrequencyOffset_textEdited     () { FEE.curGBTset->DG_FREQ_OFFSET        = ui->lineEditDGfrequencyOffset     ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGcontinuousValue_textEdited     () { FEE.curGBTset->TG_CONT_VALUE         = ui->lineEditTGcontinuousValue     ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGbunchFrequency_textEdited      () { FEE.curGBTset->TG_BUNCH_FREQ         = ui->lineEditTGbunchFrequency      ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGfrequencyOffset_textEdited     () { FEE.curGBTset->TG_FREQ_OFFSET        = ui->lineEditTGfrequencyOffset     ->displayText().toUInt(&ok, 16); }
    void on_lineEditSendSingleValue_textEdited       () { FEE.curGBTset->TG_SINGLE_VALUE       = ui->lineEditSendSingleValue       ->displayText().toUInt(&ok, 16); }
    void on_lineEditFEEID_textEdited                 () { FEE.curGBTset->RDH_FEE_ID            = ui->lineEditFEEID                 ->displayText().toUInt(&ok, 16); }
    void on_lineEditPAR_textEdited                   () { FEE.curGBTset->RDH_PAR               = ui->lineEditPAR                   ->displayText().toUInt(&ok, 16); }
    void on_lineEditMaxPayload_textEdited            () { FEE.curGBTset->RDH_MAX_PAYLOAD       = ui->lineEditMaxPayload            ->displayText().toUInt(&ok, 16); }
    void on_lineEditDETfield_textEdited              () { FEE.curGBTset->RDH_DET_FIELD         = ui->lineEditDETfield              ->displayText().toUInt(&ok, 16); }
    void on_lineEditCRUtriggerCompareDelay_textEdited() { FEE.curGBTset->CRU_TRG_COMPARE_DELAY = ui->lineEditCRUtriggerCompareDelay->displayText().toUInt(&ok, 16); }
    void on_lineEditBCIDdelay_textEdited             () { FEE.curGBTset->BCID_DELAY            = ui->lineEditBCIDdelay             ->displayText().toUInt(&ok, 16); }
    void on_lineEditDataSelectTriggerMask_textEdited () { FEE.curGBTset->DATA_SEL_TRG_MASK     = ui->lineEditDataSelectTriggerMask ->displayText().toUInt(&ok, 16); }
    void on_lineEditTGcontinuousPattern_textEdited   () {
        FEE.curGBTset->TG_PATTERN_1 = ui->lineEditTGcontinuousPattern->displayText().left(8) .toUInt(&ok, 16);
        FEE.curGBTset->TG_PATTERN_0 = ui->lineEditTGcontinuousPattern->displayText().right(8).toUInt(&ok, 16);
    }

    void on_TCM_selector_toggled(bool checked) { //select TCM
        (checked ? ui->groupBoxTCM : ui->groupBoxPM)->setVisible(true);
        ui->groupBoxReadoutControl->setParent(checked ? ui->groupBoxTCM : ui->groupBoxPM);
        int PMy = ui->groupBoxPM->y();
        ui->groupBoxPM->move(ui->groupBoxPM->x(), ui->groupBoxTCM->y());
        ui->groupBoxTCM->move(ui->groupBoxTCM->x(), PMy);
        ui->groupBoxReadoutControl->move(ui->groupBoxReadoutControl->x(), (checked ? 6 : 120) * sz/13.);
        ui->groupBoxReadoutControl->setVisible(true);
        (checked ? ui->groupBoxPM : ui->groupBoxTCM)->setVisible(false);
        if (checked) {
            FEE.selectedBoard = FEE.TCMid;
            FEE.isTCM = true;
            FEE.curGBTact = &FEE.TCM.act.GBT;
            FEE.curGBTset = &FEE.TCM.set.GBT;
            if (FEE.isOnline) FEE.sync();
			updateEdits();
        }
	}

    void selectPM(quint16 FEEid) {
        ui->groupBoxPM->setTitle(QString("PM") + FEE.PM[FEEid]->name);
        FEE.selectedBoard = FEEid;
        FEE.isTCM = false;
        FEE.curPM = FEE.PM[FEEid];
        FEE.curGBTact = &FEE.curPM->act.GBT;
        FEE.curGBTset = &FEE.curPM->set.GBT;
        if (FEE.isOnline) FEE.sync();
		updateEdits();
    }

    void on_comboBoxUpdatePeriod_activated(int index) { if (FEE.TCM.act.COUNTERS_UPD_RATE != quint32(index)) FEE.apply_COUNTERS_UPD_RATE(index); }
    void on_buttonRestart_clicked() { FEE.apply_RESET_SYSTEM(ui->radioButtonForceLocal->isChecked()); }
    void on_buttonDismissErrors_clicked() { FEE.apply_RESET_ERRORS(); }

	void on_spinBoxAttenuation_valueChanged(double val) { FEE.TCM.set.attenuation = val; FEE.TCM.set.attenSteps = val; }
	void on_sliderAttenuation_valueChanged(int value) { ui->spinBoxAttenuation->setValue(value); }
	void on_buttonApplyAttenuation_clicked() {
		FEE.apply_attenSteps();
		ui->sliderAttenuation->setValue(FEE.TCM.set.attenSteps);
	}
	void on_sliderAttenuation_sliderReleased() { FEE.apply_attenSteps(); }
	void on_radioButtonGenerator_clicked	  () { FEE.apply_LASER_SOURCE(true ); }
	void on_radioButtonExternalTrigger_clicked() { FEE.apply_LASER_SOURCE(false); }
    void on_buttonApplyLaserFrequency_clicked() { FEE.apply_LASER_DIVIDER(); }
    void on_buttonApplyLaserPattern_clicked() { FEE.apply_LaserPattern(); }

    void on_lineEditChannelMask_A_textEdited(const QString &text) { FEE.TCM.set.CH_MASK_A = text.toUShort(&ok, 16); }
    void on_lineEditChannelMask_C_textEdited(const QString &text) { FEE.TCM.set.CH_MASK_C = text.toUShort(&ok, 16); }
    void on_buttonApplyChannelMask_A_clicked() { FEE.apply_CH_MASK_A(); }
    void on_buttonApplyChannelMask_C_clicked() { FEE.apply_CH_MASK_C(); }

    void on_spinBoxLaserFreqDivider_valueChanged(int div) {
        FEE.TCM.set.LASER_DIVIDER = div;
        FEE.TCM.set.laserFrequency_Hz = systemClock_MHz * 1e6 / (div == 0 ? 1 << 24 : div);
        if (!laserFreqIsEditing) ui->lineEditLaserFrequency->setText(frequencyFormat(FEE.TCM.set.laserFrequency_Hz));
    }
    void on_lineEditLaserFrequency_textEdited(const QString &text) {
        laserFreqIsEditing = true;
        double freq = text.toDouble(&ok), high = systemClock_MHz * 1e6, low = systemClock_MHz * 1e6 / (1 << 24);
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
    void on_lineEditLaserPattern_textChanged() {
        FEE.TCM.set.LASER_PATTERN_1 = ui->lineEditLaserPattern->displayText(). left(8).toUInt(&ok, 16);
        FEE.TCM.set.LASER_PATTERN_0 = ui->lineEditLaserPattern->displayText().right(8).toUInt(&ok, 16);
    }

    void on_lineEditLaserTriggerMask_textEdited() {
        FEE.TCM.set.GBT.DG_TRG_RESPOND_MASK = ui->lineEditLaserTriggerMask->displayText().toUInt(&ok, 16);
        ui->lineEditDGtriggerRespondMask->setText(ui->lineEditLaserTriggerMask->displayText());
    }
    void on_buttonApplyLaserTriggerMask_clicked() { FEE.apply_DG_TRG_RESPOND_MASK(FEE.TCMid); }

    void on_spinBoxPhase_A_valueChanged(double val) {
		FEE.TCM.set.DELAY_A = lround(val / phaseStep_ns);
		FEE.TCM.set.delayAside_ns = FEE.TCM.set.DELAY_A * phaseStep_ns;
    }
    void on_spinBoxPhase_C_valueChanged(double val) {
		FEE.TCM.set.DELAY_C = lround(val / phaseStep_ns);
		FEE.TCM.set.delayCside_ns = FEE.TCM.set.DELAY_C * phaseStep_ns;
    }
    void on_spinBoxLaserPhase_valueChanged(double val) {
		FEE.TCM.set.LASER_DELAY = lround(val / phaseStepLaser_ns);
		FEE.TCM.set.delayLaser_ns = FEE.TCM.set.LASER_DELAY * phaseStepLaser_ns;
    }
    void on_buttonApplyPhase_A_clicked() { FEE.apply_DELAY_A(); }
    void on_buttonApplyPhase_C_clicked() { FEE.apply_DELAY_C(); }
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
    void on_SwitcherTriggers_SC_clicked    (bool checked) { FEE.apply_SC_ENABLED      (!checked); }
    void on_SwitcherTriggers_C_clicked     (bool checked) { FEE.apply_C_ENABLED       (!checked); }
    void on_SwitcherTriggers_ORA_clicked   (bool checked) { FEE.apply_ORA_ENABLED     (!checked); }
    void on_SwitcherTriggers_ORC_clicked   (bool checked) { FEE.apply_ORC_ENABLED     (!checked); }
    void on_SwitcherTriggers_V_clicked     (bool checked) { FEE.apply_V_ENABLED       (!checked); }

    void on_radioButtonAandC_clicked(bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(0); }
    void on_radioButtonC_clicked    (bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(1); }
    void on_radioButtonA_clicked    (bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(2); }
    void on_radioButtonSum_clicked  (bool checked) { if (checked) FEE.apply_C_SC_TRG_MODE(3); }

    void on_radioButtonForceLocal_clicked(bool checked) { FEE.apply_RESET_SYSTEM( checked); }
    void on_radioButtonAuto_clicked      (bool checked) { FEE.apply_RESET_SYSTEM(!checked); }

    void on_buttonResetCountersTCM_clicked() { FEE.apply_RESET_COUNTERS(FEE.TCMid); }
    void on_butonResetChCounters_clicked() { FEE.apply_RESET_COUNTERS(FEE.selectedBoard); }

    void on_comboBoxTriggersMode_ORA_activated(int index) { FEE.apply_ORA_MODE(index); }
    void on_comboBoxTriggersMode_ORC_activated(int index) { FEE.apply_ORC_MODE(index); }
    void on_comboBoxTriggersMode_SC_activated (int index) { FEE.apply_SC_MODE (index); }
    void on_comboBoxTriggersMode_C_activated  (int index) { FEE.apply_C_MODE  (index); }
    void on_comboBoxTriggersMode_V_activated  (int index) { FEE.apply_V_MODE  (index); }

    void on_lineEditTriggersRandomRate_ORA_textEdited() { FEE.TCM.set.ORA_RATE = ui->lineEditTriggersRandomRate_ORA->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_ORC_textEdited() { FEE.TCM.set.ORC_RATE = ui->lineEditTriggersRandomRate_ORC->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_SC_textEdited () { FEE.TCM.set. SC_RATE = ui->lineEditTriggersRandomRate_SC ->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_C_textEdited  () { FEE.TCM.set.  C_RATE = ui->lineEditTriggersRandomRate_C  ->displayText().toUInt(&ok, 16); }
    void on_lineEditTriggersRandomRate_V_textEdited  () { FEE.TCM.set.  V_RATE = ui->lineEditTriggersRandomRate_V  ->displayText().toUInt(&ok, 16); }
    void on_buttonApplyTriggersRandomRate_ORA_clicked() { FEE.apply_ORA_RATE(); }
    void on_buttonApplyTriggersRandomRate_ORC_clicked() { FEE.apply_ORC_RATE(); }
    void on_buttonApplyTriggersRandomRate_SC_clicked () { FEE.apply_SC_RATE (); }
    void on_buttonApplyTriggersRandomRate_C_clicked  () { FEE.apply_C_RATE  (); }
    void on_buttonApplyTriggersRandomRate_V_clicked  () { FEE.apply_V_RATE  (); }

	void on_lineEditTriggersSignature_ORA_textEdited() { FEE.TCM.set.ORA_SIGN = ui->lineEditTriggersSignature_ORA->displayText().toUInt(); };
	void on_lineEditTriggersSignature_ORC_textEdited() { FEE.TCM.set.ORC_SIGN = ui->lineEditTriggersSignature_ORC->displayText().toUInt(); };
	void on_lineEditTriggersSignature_SC_textEdited () { FEE.TCM.set. SC_SIGN = ui->lineEditTriggersSignature_SC ->displayText().toUInt(); };
	void on_lineEditTriggersSignature_C_textEdited  () { FEE.TCM.set.  C_SIGN = ui->lineEditTriggersSignature_C  ->displayText().toUInt(); };
	void on_lineEditTriggersSignature_V_textEdited  () { FEE.TCM.set.  V_SIGN = ui->lineEditTriggersSignature_V  ->displayText().toUInt(); };
    void on_buttonApplyTriggersSignature_ORA_clicked() { FEE.apply_ORA_SIGN(); }
    void on_buttonApplyTriggersSignature_ORC_clicked() { FEE.apply_ORC_SIGN(); }
    void on_buttonApplyTriggersSignature_SC_clicked () { FEE.apply_SC_SIGN (); }
    void on_buttonApplyTriggersSignature_C_clicked  () { FEE.apply_C_SIGN  (); }
    void on_buttonApplyTriggersSignature_V_clicked  () { FEE.apply_V_SIGN  (); }

    void on_lineEditTriggersLevelA_SC_textEdited(const QString &text) { FEE.TCM.set.SC_LEVEL_A = text.toUInt(); }
    void on_lineEditTriggersLevelA_C_textEdited (const QString &text) { FEE.TCM.set. C_LEVEL_A = text.toUInt(); }
    void on_lineEditTriggersLevelC_SC_textEdited(const QString &text) { FEE.TCM.set.SC_LEVEL_C = text.toUInt(); }
    void on_lineEditTriggersLevelC_C_textEdited (const QString &text) { FEE.TCM.set. C_LEVEL_C = text.toUInt(); }
    void on_buttonApplyTriggersLevelA_SC_clicked() { FEE.apply_SC_LEVEL_A(); }
    void on_buttonApplyTriggersLevelA_C_clicked () { FEE.apply_C_LEVEL_A (); }
    void on_buttonApplyTriggersLevelC_SC_clicked() { FEE.apply_SC_LEVEL_C(); }
    void on_buttonApplyTriggersLevelC_C_clicked () { FEE.apply_C_LEVEL_C (); }

    void on_lineEditVertexTimeLow_textEdited(const QString text) { FEE.TCM.set.VTIME_LOW  = text.toInt(); }
    void on_lineEditVertexTimeHigh_textEdited (const QString text) { FEE.TCM.set.VTIME_HIGH = text.toInt(); }
    void on_buttonApplyVertexTimeLow_clicked() { FEE.apply_VTIME_LOW (); }
    void on_buttonApplyVertexTimeHigh_clicked () { FEE.apply_VTIME_HIGH(); }

    void on_lineEditORgate_textEdited       (const QString text) { FEE.curPM->set.OR_GATE  = text.toUInt(); }
    void on_lineEditCFDsaturation_textEdited(const QString text) { FEE.curPM->set.CFD_SATR = text.toUInt(); }
    void on_lineEditChannelMask_textEdited  () { FEE.curPM->set.CH_MASK_DATA = ui->lineEditChannelMask->displayText().toUInt(&ok, 16); }
    void on_buttonApplyORgate_clicked       () { FEE.apply_OR_GATE (FEE.selectedBoard); }
    void on_buttonApplyCFDsaturation_clicked() { FEE.apply_CFD_SATR(FEE.selectedBoard); }
    void on_buttonApplyChannelMask_clicked  () { FEE.apply_CH_MASK (FEE.selectedBoard); }

    void on_radioButtonStrict_clicked    () { FEE.apply_TRG_COUNT_MODE(FEE.selectedBoard, false); }
    void on_radioButtonCFDinGate_clicked () { FEE.apply_TRG_COUNT_MODE(FEE.selectedBoard, true ); }

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
