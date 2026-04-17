/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qcustomgraph.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QCustomGraph *graphView;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QTableWidget *devicesList;
    QHBoxLayout *horizontalLayout_2;
    QLabel *realTimeLabel;
    QSpinBox *realTimeNumeric;
    QLabel *liveTimeLabel;
    QSpinBox *liveTimeNumeric;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *startStopButton;
    QPushButton *clearButton;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(631, 571);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy1);
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        graphView = new QCustomGraph(centralWidget);
        graphView->setObjectName(QString::fromUtf8("graphView"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(1);
        sizePolicy2.setHeightForWidth(graphView->sizePolicy().hasHeightForWidth());
        graphView->setSizePolicy(sizePolicy2);

        verticalLayout_2->addWidget(graphView);

        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        groupBox->setMinimumSize(QSize(0, 100));
        groupBox->setMaximumSize(QSize(16777215, 200));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        devicesList = new QTableWidget(groupBox);
        devicesList->setObjectName(QString::fromUtf8("devicesList"));
        devicesList->setAlternatingRowColors(true);
        devicesList->setSelectionMode(QAbstractItemView::SingleSelection);
        devicesList->setSelectionBehavior(QAbstractItemView::SelectRows);
        devicesList->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

        verticalLayout->addWidget(devicesList);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        realTimeLabel = new QLabel(groupBox);
        realTimeLabel->setObjectName(QString::fromUtf8("realTimeLabel"));

        horizontalLayout_2->addWidget(realTimeLabel);

        realTimeNumeric = new QSpinBox(groupBox);
        realTimeNumeric->setObjectName(QString::fromUtf8("realTimeNumeric"));
        realTimeNumeric->setMaximum(99999);

        horizontalLayout_2->addWidget(realTimeNumeric);

        liveTimeLabel = new QLabel(groupBox);
        liveTimeLabel->setObjectName(QString::fromUtf8("liveTimeLabel"));

        horizontalLayout_2->addWidget(liveTimeLabel);

        liveTimeNumeric = new QSpinBox(groupBox);
        liveTimeNumeric->setObjectName(QString::fromUtf8("liveTimeNumeric"));
        liveTimeNumeric->setMaximum(999999);

        horizontalLayout_2->addWidget(liveTimeNumeric);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        startStopButton = new QPushButton(groupBox);
        startStopButton->setObjectName(QString::fromUtf8("startStopButton"));
        startStopButton->setEnabled(false);

        horizontalLayout->addWidget(startStopButton);

        clearButton = new QPushButton(groupBox);
        clearButton->setObjectName(QString::fromUtf8("clearButton"));
        clearButton->setEnabled(false);

        horizontalLayout->addWidget(clearButton);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addWidget(groupBox);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "USB Spectrometer Example", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Attached Devices", nullptr));
        realTimeLabel->setText(QCoreApplication::translate("MainWindow", "Real Time:", nullptr));
        liveTimeLabel->setText(QCoreApplication::translate("MainWindow", "Live Time:", nullptr));
        liveTimeNumeric->setSuffix(QString());
        startStopButton->setText(QCoreApplication::translate("MainWindow", "Start Acquisition", nullptr));
        clearButton->setText(QCoreApplication::translate("MainWindow", "Clear Data", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
