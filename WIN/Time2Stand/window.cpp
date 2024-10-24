#include "window.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QMenu>
#include <Windows.h>
#include <QPushButton>
#include <QDebug>
#include <QThread>

#include <iomanip>
#include <sstream>

#include "appsettings.h"
#include "device.h"

Window::Window()
{
    createActions();
    createTrayIcon();

    m_sitTime = new QLabel();
    m_standTime = new QLabel();
    m_standTotalTime = new QLabel();
    m_resetButton = new QPushButton(tr("Reset"));

    QPushButton* sitButton = new QPushButton("Sit");
    QPushButton* standButton = new QPushButton("Stand");

    connect(sitButton, &QPushButton::clicked, this, [&]() {Device::Instance().TestSit();});
    connect(standButton, &QPushButton::clicked, this, [&]() {Device::Instance().TestStand();});

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->addWidget(new QLabel("Sit:"), 0, 0);
    gridLayout->addWidget(m_sitTime, 0, 1);

    gridLayout->addWidget(new QLabel("Stand:"), 1, 0);
    gridLayout->addWidget(m_standTime, 1, 1);

    gridLayout->addWidget(new QLabel("Stand Total:"), 2, 0);
    gridLayout->addWidget(m_standTotalTime, 2, 1);

    gridLayout->addWidget(m_resetButton, 3, 0, 1, 2);
    // gridLayout->addWidget(sitButton, 4, 0, 1, 2);
    // gridLayout->addWidget(standButton, 5, 0, 1, 2);
    gridLayout->setRowStretch(6, 1);
    setLayout(gridLayout);

    setIcon();
    m_trayIcon->show();

    setWindowTitle(tr("Time2Stand"));
    resize(200, 70);

    handlePositionUpdate();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);
    connect(m_resetButton, &QPushButton::clicked, this, &Window::handleReset);

    PositionTracker::Instance().UpdatePosition().Subscribe("Window",
        std::bind(&Window::handlePositionUpdate, this));

    std::chrono::seconds duration = std::chrono::seconds(AppSettings::Instance().StandTotalDuration());
    PositionTracker::Instance().Start(duration);
}

void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    // if (trayIcon->isVisible()) {
    //     QMessageBox::information(this, tr("Systray"),
    //                              tr("The program will keep running in the "
    //                                 "system tray. To terminate the program, "
    //                                 "choose <b>Quit</b> in the context menu "
    //                                 "of the system tray entry."));
    //     hide();
    //     event->ignore();
    // }
}

void Window::setIcon()
{
    QIcon icon = QIcon(":/images/heart.png");
    m_trayIcon->setIcon(icon);
    setWindowIcon(icon);
    m_trayIcon->setToolTip("Time2Stand");
}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
 //       iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1) % iconComboBox->count());
        showNormal();
        break;
    case QSystemTrayIcon::MiddleClick:
        //showMessage();
        break;
    default:
        ;
    }
}

void Window::handleReset()
{
    PositionTracker::Instance().Reset();
}

void Window::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Window::createTrayIcon()
{
    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(minimizeAction);
    // trayIconMenu->addAction(maximizeAction);
    m_trayIconMenu->addAction(restoreAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
}

void Window::handlePositionUpdate()
{
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(
            this, __func__, Qt::QueuedConnection);
        return;
    }

    auto data = PositionTracker::Instance().Position();

    QString sitStr = secondsToHhMmSs(data.sit.count());
    QString standStr = secondsToHhMmSs(data.stand.count());
    QString standTotalStr = secondsToHhMmSs(data.standTotal.count());

    m_sitTime->setText(sitStr);
    m_standTime->setText(standStr);
    m_standTotalTime->setText(standTotalStr);

    if(PositionState::Sit == data.position) {
        m_trayIcon->setToolTip(QString("Sit: %1").arg(sitStr));
    } else if(PositionState::Stand == data.position) {
        m_trayIcon->setToolTip(QString("Stand: %1").arg(standStr));
    } else {
        m_trayIcon->setToolTip("Time2Stand");
    }
}

QString Window::secondsToHhMmSs(uint32_t totalSeconds)
{
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;

    // Format to hh:mm:ss
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << " h "
        << std::setw(2) << std::setfill('0') << minutes << " m";

    return QString::fromStdString(oss.str());
}

#endif
