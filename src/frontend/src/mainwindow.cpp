#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "HuffmanTree.h"  // 后端头文件

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. 初始化中心容器和主布局
    m_centralWidget = new QWidget(this);
    this->setCentralWidget(m_centralWidget);
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_btnLayout = new QHBoxLayout();

    // 2. 创建控件（替代UI文件的控件）
    QLabel *fileLabel = new QLabel("选中文件路径：", this);
    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setReadOnly(true);  // 路径只读
    m_selectFileBtn = new QPushButton("选择文件", this);
    m_processBtn = new QPushButton("处理文件", this);
    m_chartWidget = new ChartWidget(this);

    // 3. 组装布局
    m_btnLayout->addWidget(m_selectFileBtn);
    m_btnLayout->addWidget(m_processBtn);
    m_mainLayout->addWidget(fileLabel);
    m_mainLayout->addWidget(m_filePathEdit);
    m_mainLayout->addLayout(m_btnLayout);
    m_mainLayout->addWidget(m_chartWidget);

    // 4. 绑定按钮点击事件
    connect(m_selectFileBtn, &QPushButton::clicked, this, &MainWindow::on_selectFileBtn_clicked);
    connect(m_processBtn, &QPushButton::clicked, this, &MainWindow::on_processBtn_clicked);

    // 5. 设置窗口属性（标题、大小、位置）
    this->setWindowTitle("哈夫曼词频分析工具");
    this->resize(800, 600);  // 初始大小：800x600像素
    this->move(100, 100);    // 初始位置：屏幕左上角(100,100)
}

MainWindow::~MainWindow() {
    // 手动释放控件（父控件会自动释放子控件，此处可选）
    delete m_chartWidget;
    delete m_selectFileBtn;
    delete m_processBtn;
    delete m_filePathEdit;
    delete m_btnLayout;
    delete m_mainLayout;
    delete m_centralWidget;
}

void MainWindow::on_selectFileBtn_clicked() {
    // 打开文件选择对话框（支持文本和图片）
    QString filePath = QFileDialog::getOpenFileName(
        this, 
        "选择文件", 
        "", 
        "文本文件 (*.txt);;图片文件 (*.bmp)"
    );
    if (!filePath.isEmpty()) {
        m_selectedFilePath = filePath.toStdString();
        m_filePathEdit->setText(filePath);  // 显示路径
    }
}

void MainWindow::on_processBtn_clicked() {
    if (m_selectedFilePath.empty()) {
        QMessageBox::warning(this, "警告", "请先选择文件！");
        return;
    }

    // 修复文件后缀判断逻辑（原find()会匹配任意位置，改用rfind()匹配末尾）
    bool isTxt = (m_selectedFilePath.rfind(".txt") == m_selectedFilePath.size() - 4);
    bool isBmp = (m_selectedFilePath.rfind(".bmp") == m_selectedFilePath.size() - 4);

    try {
        if (isTxt) {
            // 文本文件：调用后端Text_file_read统计字符频率
            auto freqMap = Text_file_read(m_selectedFilePath);
            m_chartWidget->updateTextFrequency(freqMap);
        } else if (isBmp) {
            // 图片文件：提示需后端补充LoadImageData实现（当前后端无该函数）
            QMessageBox::information(this, "提示", "图片处理需后端补充LoadImageData函数！");
        } else {
            QMessageBox::warning(this, "警告", "不支持的文件格式！仅支持.txt和.bmp");
        }
    } catch (const std::exception& e) {
        // 捕获文件读取异常（如文件不存在、权限不足）
        QMessageBox::critical(this, "错误", QString("文件处理失败：%1").arg(e.what()));
    }
}