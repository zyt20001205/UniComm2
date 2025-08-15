#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDockWidget>
#include <QMainWindow>
#include <QWidget>

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override = default;

private:
    void uiInit();
};

#endif //MAINWINDOW_H
