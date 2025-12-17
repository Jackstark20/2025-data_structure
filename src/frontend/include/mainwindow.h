#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTextEncodeDecodeClicked();
    void onImageEncodeDecodeClicked();

private:
    void initUI();
    
    QPushButton *textButton;
    QPushButton *imageButton;
};