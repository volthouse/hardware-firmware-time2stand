#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QDir>
#include <QMessageBox>

#include "window.h"
#include "device.h"
#include "positiontracker.h"
#include "appsettings.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(time2stand);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Time2Stand"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    QString appDir = QCoreApplication::applicationDirPath();
    QString settingsFilePath = QDir(appDir).filePath("settings.json");

    AppSettings::Instance().Init(settingsFilePath.toStdString());
    AppSettings::Instance().Load();

    auto err = Device::Instance().Open(AppSettings::Instance().PortName(), 9600);
    if(err) {
        QMessageBox::warning(nullptr, "Comport Error", "Failed to open com port");
    }

    Window window;
    window.show();
    return app.exec();
}

#else

#include <QLabel>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString text("QSystemTrayIcon is not supported on this platform");

    QLabel *label = new QLabel(text);
    label->setWordWrap(true);

    label->show();
    qDebug() << text;

    app.exec();
}

#endif
