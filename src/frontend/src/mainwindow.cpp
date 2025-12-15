#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/backend/HuffmanTree.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_chartWidget(new ChartWidget) {
    ui->setupUi(this);
    setCentralWidget(m_chartWidget);
    connect(ui->selectFileBtn, &QPushButton::clicked, this, &MainWindow::on_selectFileBtn_clicked);
    connect(ui->processBtn, &QPushButton::clicked, this, &MainWindow::on_processBtn_clicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_selectFileBtn_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "文本文件 (*.txt);;图片文件 (*.bmp)");
    if (!filePath.isEmpty()) {
        m_selectedFilePath = filePath.toStdString();
        ui->filePathEdit->setText(filePath);
    }
}

void MainWindow::on_processBtn_clicked() {
    if (m_selectedFilePath.empty()) {
        QMessageBox::warning(this, "警告", "请先选择文件");
        return;
    }

    // 区分文本/图片文件处理
    if (m_selectedFilePath.ends_with(".txt")) {
        try {
            auto freqMap = Text_file_read(m_selectedFilePath);
            m_chartWidget->updateTextFrequency(freqMap);
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "错误", QString("文件处理失败: %1").arg(e.what()));
        }
    } else if (m_selectedFilePath.ends_with(".bmp")) {
        // 此处需调用后端图片读取接口（示例）
        // std::vector<BYTE> imageData = LoadImageData(m_selectedFilePath);
        // auto freqVec = getByteFrequencySorted(imageData);
        // m_chartWidget->updateByteFrequency(freqVec);
    }
}