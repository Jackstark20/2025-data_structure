#include "mainwindow.h"
#include "textencodedecodewindow.h"
#include "imageencodedecodewindow.h"
#include <QMessageBox>
//主窗口实现
//构建 “哈夫曼编码工具” 的主界面
//提供文本 / 图片编码解码的功能入口

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    initUI();
}

MainWindow::~MainWindow() {
}

void MainWindow::initUI() {
    setWindowTitle("哈夫曼编码工具");
    setGeometry(100, 100, 800, 600);
    
    // 创建主布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    // 添加标题
    QLabel *titleLabel = new QLabel("哈夫曼编码工具");
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // 添加功能选择按钮
    textButton = new QPushButton("文本编码解码");
    textButton->setStyleSheet("font-size: 20px; padding: 20px;");
    connect(textButton, &QPushButton::clicked, this, &MainWindow::onTextEncodeDecodeClicked);
    mainLayout->addWidget(textButton);
    
    imageButton = new QPushButton("图片编码解码");
    imageButton->setStyleSheet("font-size: 20px; padding: 20px;");
    connect(imageButton, &QPushButton::clicked, this, &MainWindow::onImageEncodeDecodeClicked);
    mainLayout->addWidget(imageButton);
}

void MainWindow::onTextEncodeDecodeClicked() {
    TextEncodeDecodeWindow *textWindow = new TextEncodeDecodeWindow();
    textWindow->show();
    // 可以选择隐藏主窗口
    // this->hide();
}

void MainWindow::onImageEncodeDecodeClicked() {
    ImageEncodeDecodeWindow *imageWindow = new ImageEncodeDecodeWindow();
    imageWindow->show();
    // 可以选择隐藏主窗口
    // this->hide();
}