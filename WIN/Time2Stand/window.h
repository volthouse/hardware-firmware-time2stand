#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QTimer>

#include "positiontracker.h"

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
QT_END_NAMESPACE

class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

    void setVisible(bool visible) Q_DECL_OVERRIDE;

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void setIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void handleReset();
    void handlePositionUpdate();

private:
    void createActions();
    void createTrayIcon();
    QString secondsToHhMmSs(uint32_t totalSeconds);

private:
    QLabel *m_sitTime;
    QLabel *m_standTime;
    QLabel *m_standTotalTime;
    QPushButton *m_resetButton;

    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;
};

#endif
