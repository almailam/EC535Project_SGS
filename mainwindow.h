// BU EC535 Project - Smart Gardening System (SGS)
// Abdulaziz AlMailam & Samuel Gulinello

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

// Define function slots for use based on triggers
public slots:
    void updateDisplay();
    void pump_button_pressed_response();
    void pump_button_release_response();
    void LED_button_pressed_response();
    void LED_button_release_response();

// Define input signals/interrupts to allow their use
signals:
    void pressed();
    void released();


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
