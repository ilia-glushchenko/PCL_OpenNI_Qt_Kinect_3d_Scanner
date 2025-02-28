#include "gui/scannerwidget.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QString>

#include <QDesktopWidget>
#include <QRect>

#include "io/pcdinputiterator.hpp"

ScannerWidget::ScannerWidget(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    QRect screenRect = QDesktopWidget().availableGeometry(this);
    move(screenRect.right() - frameGeometry().right(), 0);

    initializeOpenDialogInterface();
}

void ScannerWidget::reloadSettings()
{
    initializeSettings();

    openniInterface->deleteLater();
    openniInterface = new OpenNiInterface(this, settings);
    reconstructionInterface->reloadSettings();

    initializeMainInterfaceSettings();
}

void ScannerWidget::initializeSettings()
{
    settings = new QSettings(settingsPath, QSettings::IniFormat, this);

    if (settings->value("READING_SETTING/AUTO_SET_RANGE").toBool()) {
        PcdInputIterator it(settings, 0, std::numeric_limits<uint>::max(), 1);
        settings->setValue("READING_SETTING/FROM", it.getLowerBound());
        settings->setValue("READING_SETTING/TO", it.getUpperBound());
        settings->sync();
    }
}

void ScannerWidget::initializeReconstruction()
{
    openniInterface = new OpenNiInterface(this, settings);
    reconstructionInterface = new ReconstructionInterface(this, settings);
}

void ScannerWidget::initializeMainInterfaceSettings()
{
    recCheck->setChecked(settings->value("STREAM_SETTINGS/ENABLE_STREAM_RECORDING").toBool());
    streamFromCheck->setChecked(settings->value("STREAM_SETTINGS/ENABLE_REPLAY_RECORD_STREAM").toBool());
    recToPclDataCheck->setChecked(settings->value("STREAM_SETTINGS/ENABLE_CONVERT_TO_PCD").toBool());
    undistCheck->setChecked(settings->value("STREAM_SETTINGS/ENABLE_UNDISTORTION").toBool());
    bilateralCheck->setChecked(settings->value("STREAM_SETTINGS/ENABLE_BILATERAL_FILTER").toBool());

    reconstructCheck->setChecked(settings->value("PIPELINE_SETTINGS/ENABLE_RECONSTRUCTION").toBool());
    undistrtionCheck->setChecked(settings->value("PIPELINE_SETTINGS/UNDISTORTION").toBool());
    bilateralFilterCheck->setChecked(settings->value("PIPELINE_SETTINGS/OPENCV_BILATERAL_FILTER").toBool());
    statFilterCheck->setChecked(settings->value("PIPELINE_SETTINGS/STATISTICAL_OUTLIER_REMOVAL_FILTER").toBool());
    mlsFilterCheck->setChecked(settings->value("PIPELINE_SETTINGS/MOVING_LEAST_SQUARES_FILTER").toBool());
}

void ScannerWidget::initializeMainInterface()
{
    statusBar = new QStatusBar();
    statusBar->showMessage("Started");
    setStatusBar(statusBar);

    makeProjectButton = new QPushButton("Make Project", this);
    openProjectButton = new QPushButton("Open Project", this);
    connect(makeProjectButton, SIGNAL(clicked()), this, SLOT(slot_make_project()));
    connect(openProjectButton, SIGNAL(clicked()), this, SLOT(slot_open_project()));

    initButton = new QPushButton("Start Stream", this);
    takeImagesButton = new QPushButton("Start Rotation Stream", this);
    takeOpImagesButton = new QPushButton("Take Long Images", this);
    takeOneOpImageButton = new QPushButton("Take One Long Image", this);
    saveDataButton = new QPushButton("Save Long Image Data", this);
    connect(initButton, SIGNAL(clicked()), this, SLOT(slot_start_stream()));
    connect(takeImagesButton, SIGNAL(clicked()), this, SLOT(slot_start_rotation_stream()));
    connect(takeOpImagesButton, SIGNAL(clicked()), this, SLOT(slot_take_long_images()));
    connect(takeOneOpImageButton, SIGNAL(clicked()), this, SLOT(slot_take_one_long_image()));
    connect(saveDataButton, SIGNAL(clicked()), this, SLOT(slot_save_long_image_data()));

    recCheck = new QCheckBox("Record stream", this);
    streamFromCheck = new QCheckBox("Replay recorded stream", this);
    recToPclDataCheck = new QCheckBox("Save stream as PCD", this);
    undistCheck = new QCheckBox("Use lense undistortion", this);
    bilateralCheck = new QCheckBox("Use Bilateral filter", this);
    connect(recCheck, SIGNAL(stateChanged(int)), this, SLOT(slot_record_stream(int)));
    connect(streamFromCheck, SIGNAL(stateChanged(int)), this, SLOT(slot_replay_recording(int)));
    connect(recToPclDataCheck, SIGNAL(stateChanged(int)), this, SLOT(slot_record_pcd(int)));
    connect(undistCheck, SIGNAL(stateChanged(int)), this, SLOT(slot_use_undistortion(int)));
    connect(bilateralCheck, SIGNAL(stateChanged(int)), this, SLOT(slot_use_bilateral(int)));

    drawScene3dModelButton = new QPushButton("Perform Reconstruction", this);
    connect(drawScene3dModelButton, SIGNAL(clicked()), this, SLOT(slot_perform_reconstruction()));

    reconstructCheck = new QCheckBox("Reconstruct", this);
    undistrtionCheck = new QCheckBox("Undistortion", this);
    bilateralFilterCheck = new QCheckBox("Bilateral filter", this);
    statFilterCheck = new QCheckBox("Statistic filter", this);
    mlsFilterCheck = new QCheckBox("Smooth filter", this);
}

void ScannerWidget::initializeOpenDialogInterface()
{
    makeProjectButton = new QPushButton("Make Project");
    connect(makeProjectButton, SIGNAL(clicked()), this, SLOT(slot_make_project()));
    openProjectButton = new QPushButton("Open Project");
    connect(openProjectButton, SIGNAL(clicked()), this, SLOT(slot_open_project()));

    centralWidget = new QWidget;
    vBoxLayout = new QVBoxLayout;

    vBoxLayout->addWidget(makeProjectButton);
    vBoxLayout->addWidget(openProjectButton);

    centralWidget->setLayout(vBoxLayout);
    setCentralWidget(centralWidget);
    setWindowTitle("No project opened...");
}

void ScannerWidget::initializeDebugInterface()
{
    centralWidget->deleteLater();
    vBoxLayout->deleteLater();
    centralWidget = new QWidget;
    vBoxLayout = new QVBoxLayout;

    vBoxLayout->addWidget(makeProjectButton);
    vBoxLayout->addWidget(openProjectButton);

    QFrame* line = new QFrame(this);
    line->setObjectName(QString::fromUtf8("line"));
    line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBoxLayout->addWidget(line);

    vBoxLayout->addWidget(initButton);
    vBoxLayout->addWidget(takeImagesButton);
    vBoxLayout->addWidget(takeOpImagesButton);
    vBoxLayout->addWidget(takeOneOpImageButton);
    vBoxLayout->addWidget(saveDataButton);

    QGroupBox* groupBox = new QGroupBox(tr("Stream settings"));
    QVBoxLayout* groupBoxVBoxLayout = new QVBoxLayout;
    groupBoxVBoxLayout->addWidget(recCheck);
    groupBoxVBoxLayout->addWidget(streamFromCheck);
    groupBoxVBoxLayout->addWidget(recToPclDataCheck);
    groupBoxVBoxLayout->addWidget(undistCheck);
    groupBoxVBoxLayout->addWidget(bilateralCheck);
    groupBox->setLayout(groupBoxVBoxLayout);
    vBoxLayout->addWidget(groupBox);

    QFrame* line2 = new QFrame(this);
    line2->setObjectName(QString::fromUtf8("line"));
    line2->setGeometry(QRect(320, 150, 118, 3));
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    vBoxLayout->addWidget(line2);

    vBoxLayout->addWidget(drawScene3dModelButton);
    QGroupBox* groupBox1 = new QGroupBox(tr("Reconstruction settings"));
    QVBoxLayout* groupBox1VBoxLayout = new QVBoxLayout;
    groupBox1VBoxLayout->addWidget(undistrtionCheck);
    groupBox1VBoxLayout->addWidget(bilateralFilterCheck);
    groupBox1VBoxLayout->addWidget(statFilterCheck);
    groupBox1VBoxLayout->addWidget(mlsFilterCheck);
    groupBox1->setLayout(groupBox1VBoxLayout);
    vBoxLayout->addWidget(groupBox1);

    centralWidget->setLayout(vBoxLayout);
    setCentralWidget(centralWidget);
}

void ScannerWidget::initializeReleaseInterface()
{
    centralWidget->deleteLater();
    vBoxLayout->deleteLater();
    centralWidget = new QWidget;
    vBoxLayout = new QVBoxLayout;

    vBoxLayout->addWidget(makeProjectButton);
    vBoxLayout->addWidget(openProjectButton);

    QFrame* line = new QFrame(this);
    line->setObjectName(QString::fromUtf8("line"));
    line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vBoxLayout->addWidget(line);

    vBoxLayout->addWidget(initButton);
    vBoxLayout->addWidget(takeImagesButton);

    QFrame* line1 = new QFrame(this);
    line1->setObjectName(QString::fromUtf8("line"));
    line1->setGeometry(QRect(320, 150, 118, 3));
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    vBoxLayout->addWidget(line1);

    vBoxLayout->addWidget(drawScene3dModelButton);

    centralWidget->setLayout(vBoxLayout);
    setCentralWidget(centralWidget);
}

void ScannerWidget::slot_make_project()
{
    QString makeProjectPath = QFileDialog::getExistingDirectory(this, "Select new project directory...", "./");
    if (makeProjectPath.isEmpty() == false) {
        QDir dir(makeProjectPath);
        if (!dir.exists() && !dir.mkdir(".")) {
            qDebug() << "Cannot create:" << makeProjectPath;
        }

        //Create calibartion folder
        if (!tools::copyRecursively("./default_project/calibration", QString("%1/calibration").arg(makeProjectPath))) {
            qDebug() << "Cannot create:" << QString("%1/calibration").arg(makeProjectPath);
        }

        //Create stream folder
        dir = QDir(QString("%1/stream").arg(makeProjectPath));
        if (!dir.exists() && !dir.mkdir(".")) {
            qDebug() << "Cannot create:" << QString("%1/stream").arg(makeProjectPath);
        }

        //Create pcd folder
        dir = QDir(QString("%1/pcd").arg(makeProjectPath));
        if (!dir.exists() && !dir.mkdir(".")) {
            qDebug() << "Cannot create:" << QString("%1/pcd").arg(makeProjectPath);
        }

        QFile projectFile("./default_project/project.ini");
        if (projectFile.exists()) {
            projectFile.copy(makeProjectPath + "/project.ini");
        } else {
            qDebug() << "Error: Default project does not exists!";
        }
    }
}

void ScannerWidget::slot_open_project()
{
    QString tempSettingsPath = QFileDialog::getOpenFileName(this, "Select project ini file...", "./", "*.ini");
    if (tempSettingsPath.isEmpty() == false) {
        settingsPath = tempSettingsPath;

        initializeSettings();
        initializeReconstruction();

        setWindowTitle(QString("Project: %1").arg(settings->value("PROJECT_SETTINGS/NAME").toString()));

        initializeMainInterface();
        initializeMainInterfaceSettings();

        if (settings->value("PROJECT_SETTINGS/DEBUG_INTERFACE").toBool()) {
            initializeDebugInterface();
        } else {
            initializeReleaseInterface();
        }
    }
}

void ScannerWidget::slot_start_stream()
{
    if (!openniInterface->isInit()) {
        reloadSettings();
        openniInterface->initialize_interface();

        if (openniInterface->isInit()) {
            recCheck->setDisabled(true);
            streamFromCheck->setDisabled(true);
            recToPclDataCheck->setDisabled(true);

            initButton->setText("Stop Stream");
            openniInterface->start_stream();
        }
    } else {
        openniInterface->shutdown_interface();

        recCheck->setDisabled(false);
        streamFromCheck->setDisabled(false);
        recToPclDataCheck->setDisabled(false);

        initButton->setText("Start Stream");
    }
}

void ScannerWidget::slot_start_rotation_stream()
{
    if (!openniInterface->isInit()) {
        reloadSettings();
        openniInterface->initialize_interface();

        if (openniInterface->isInit()) {
            recCheck->setDisabled(true);
            streamFromCheck->setDisabled(true);
            recToPclDataCheck->setDisabled(true);

            takeImagesButton->setText("Stop Rotation Stream");
            openniInterface->start_rotation_stream();
        }
    } else {
        openniInterface->shutdown_interface();

        recCheck->setDisabled(false);
        streamFromCheck->setDisabled(false);
        recToPclDataCheck->setDisabled(false);

        takeImagesButton->setText("Start Rotation Stream");
    }
}

void ScannerWidget::slot_take_long_images()
{
    if (!openniInterface->isInit()) {
        reloadSettings();
        openniInterface->initialize_interface();

        if (openniInterface->isInit()) {
            recCheck->setDisabled(true);
            streamFromCheck->setDisabled(true);
            recToPclDataCheck->setDisabled(true);

            takeOpImagesButton->setText("Stop Taking Long Images");
            openniInterface->take_long_images();
        }
    } else {
        openniInterface->shutdown_interface();

        recCheck->setDisabled(false);
        streamFromCheck->setDisabled(false);
        recToPclDataCheck->setDisabled(false);

        takeOpImagesButton->setText("Start Taking Long Images");
    }
}

void ScannerWidget::slot_take_one_long_image()
{
    reloadSettings();
    openniInterface->initialize_interface();

    if (openniInterface->isInit()) {
        recCheck->setDisabled(true);
        streamFromCheck->setDisabled(true);
        recToPclDataCheck->setDisabled(true);

        openniInterface->take_one_long_image();

        recCheck->setDisabled(false);
        streamFromCheck->setDisabled(false);
        recToPclDataCheck->setDisabled(false);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    openniInterface->shutdown_interface();
}

void ScannerWidget::slot_save_long_image_data()
{
    openniInterface->save_long_image_data();
}

void ScannerWidget::slot_perform_reconstruction()
{
    reloadSettings();
    reconstructionInterface->slot_perform_reconstruction();
}

void ScannerWidget::slot_record_stream(int state)
{
    settings->setValue("STREAM_SETTINGS/ENABLE_STREAM_RECORDING", state != Qt::Unchecked);
    settings->sync();
}

void ScannerWidget::slot_replay_recording(int state)
{
    settings->setValue("STREAM_SETTINGS/ENABLE_REPLAY_RECORD_STREAM", state != Qt::Unchecked);
    settings->sync();
}

void ScannerWidget::slot_record_pcd(int state)
{
    settings->setValue("STREAM_SETTINGS/ENABLE_CONVERT_TO_PCD", state != Qt::Unchecked);
    settings->sync();
}

void ScannerWidget::slot_use_undistortion(int state)
{
    settings->setValue("STREAM_SETTINGS/ENABLE_UNDISTORTION", state != Qt::Unchecked);
    settings->sync();
}

void ScannerWidget::slot_use_bilateral(int state)
{
    settings->setValue("STREAM_SETTINGS/ENABLE_BILATERAL_FILTER", state != Qt::Unchecked);
    settings->sync();
}
