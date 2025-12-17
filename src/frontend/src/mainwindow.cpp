#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    setWindowTitle("哈夫曼编码工具");
    setGeometry(100, 100, 1200, 800);
    
    // 创建主布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // ==================== 文本编码部分 ====================
    textGroupBox = new QGroupBox("文本编码");
    QVBoxLayout *textLayout = new QVBoxLayout(textGroupBox);
    textLayout->setSpacing(15);
    
    // 文本输入区域
    textInputLabel = new QLabel("输入要编码的文本：");
    textInputEdit = new QTextEdit();
    textInputEdit->setPlaceholderText("请输入文本...");
    textInputEdit->setMinimumHeight(100);
    
    encodeTextButton = new QPushButton("编码文本");
    connect(encodeTextButton, &QPushButton::clicked, this, &MainWindow::onEncodeTextClicked);
    
    // 文本文件浏览区域
    QHBoxLayout *textFileLayout = new QHBoxLayout();
    textFileLabel = new QLabel("或选择本地文本文件：");
    textFilePathEdit = new QLineEdit();
    textFilePathEdit->setPlaceholderText("文本文件路径");
    browseTextFileButton = new QPushButton("浏览");
    connect(browseTextFileButton, &QPushButton::clicked, this, &MainWindow::onBrowseTextFileClicked);
    encodeTextFileButton = new QPushButton("编码文本文件");
    connect(encodeTextFileButton, &QPushButton::clicked, this, &MainWindow::onEncodeTextFileClicked);
    
    textFileLayout->addWidget(textFilePathEdit);
    textFileLayout->addWidget(browseTextFileButton);
    textFileLayout->addWidget(encodeTextFileButton);
    
    // 结果展示区域
    textResultLabel = new QLabel("编码结果：");
    textResultEdit = new QPlainTextEdit();
    textResultEdit->setReadOnly(true);
    textResultEdit->setMinimumHeight(200);
    
    exportTextHufButton = new QPushButton("导出为.huf文件");
    connect(exportTextHufButton, &QPushButton::clicked, this, &MainWindow::onExportTextHufClicked);
    
    // 将文本编码部分的组件添加到布局
    textLayout->addWidget(textInputLabel);
    textLayout->addWidget(textInputEdit);
    textLayout->addWidget(encodeTextButton);
    textLayout->addWidget(textFileLabel);
    textLayout->addLayout(textFileLayout);
    textLayout->addWidget(textResultLabel);
    textLayout->addWidget(textResultEdit);
    textLayout->addWidget(exportTextHufButton);
    
    // ==================== 图片编码部分 ====================
    imageGroupBox = new QGroupBox("图片编码");
    QVBoxLayout *imageLayout = new QVBoxLayout(imageGroupBox);
    imageLayout->setSpacing(15);
    
    // 图片浏览区域
    QHBoxLayout *imagePathLayout = new QHBoxLayout();
    imagePathLabel = new QLabel("选择图片文件：");
    imagePathEdit = new QLineEdit();
    imagePathEdit->setPlaceholderText("图片文件路径");
    browseImageButton = new QPushButton("浏览");
    connect(browseImageButton, &QPushButton::clicked, this, &MainWindow::onBrowseImageClicked);
    encodeImageButton = new QPushButton("编码图片");
    connect(encodeImageButton, &QPushButton::clicked, this, &MainWindow::onEncodeImageClicked);
    
    imagePathLayout->addWidget(imagePathEdit);
    imagePathLayout->addWidget(browseImageButton);
    imagePathLayout->addWidget(encodeImageButton);
    
    // 图片预览区域
    imagePreviewLabel = new QLabel("图片预览：");
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setMinimumHeight(200);
    imagePreviewLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
    
    // 图片编码结果区域
    imageResultLabel = new QLabel("编码结果摘要：");
    imageResultEdit = new QPlainTextEdit();
    imageResultEdit->setReadOnly(true);
    imageResultEdit->setMinimumHeight(150);
    
    exportImageHufButton = new QPushButton("导出为.huf文件");
    connect(exportImageHufButton, &QPushButton::clicked, this, &MainWindow::onExportImageHufClicked);
    
    // 将图片编码部分的组件添加到布局
    imageLayout->addWidget(imagePathLabel);
    imageLayout->addLayout(imagePathLayout);
    imageLayout->addWidget(imagePreviewLabel);
    imageLayout->addWidget(imageResultLabel);
    imageLayout->addWidget(imageResultEdit);
    imageLayout->addWidget(exportImageHufButton);
    
    // 将左右两部分添加到主布局
    mainLayout->addWidget(textGroupBox);
    mainLayout->addWidget(imageGroupBox);
    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 1);
}

// 文本编码相关槽函数实现
// 修改mainwindow.cpp文件中的onEncodeTextClicked函数
void MainWindow::onEncodeTextClicked()
{
    QString text = textInputEdit->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要编码的文本！");
        return;
    }
    
    // 使用后端API进行编码（直接使用UTF-8版本）
    std::string utf8Text = text.toUtf8().toStdString();
    std::string encoded = backend_api::encodeTextUtf8(utf8Text);
    if (encoded.empty()) {
        QMessageBox::warning(this, "警告", "编码失败！");
        return;
    }
    
    currentEncodedText = encoded;
    textResultEdit->setPlainText(QString::fromUtf8(encoded.c_str()));
    QMessageBox::information(this, "提示", "文本编码成功！");
}

void MainWindow::onBrowseTextFileClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择文本文件", "", "Text Files (*.txt);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        textFilePathEdit->setText(filePath);
    }
}

void MainWindow::onEncodeTextFileClicked()
{
    QString filePath = textFilePathEdit->text();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择文本文件！");
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "警告", "无法打开文件！");
        return;
    }
    
    QString content = file.readAll();
    file.close();
    
    // 使用后端API进行编码（直接使用UTF-8版本）
    std::string utf8Content = content.toUtf8().toStdString();
    std::string encoded = backend_api::encodeTextUtf8(utf8Content);
    if (encoded.empty()) {
        QMessageBox::warning(this, "警告", "编码失败！");
        return;
    }
    
    currentEncodedText = encoded;
    textResultEdit->setPlainText(QString::fromUtf8(encoded.c_str()));
    QMessageBox::information(this, "提示", "文本文件编码成功！");
}

void MainWindow::onExportTextHufClicked()
{
    if (currentEncodedText.empty()) {
        QMessageBox::warning(this, "警告", "没有可导出的编码结果！");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出为HUF文件", "", "HUF Files (*.huf);;All Files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "警告", "无法创建文件！");
        return;
    }
    
    file.write(QString::fromStdString(currentEncodedText).toUtf8());
    file.close();
    
    QMessageBox::information(this, "提示", "HUF文件导出成功！");
}

// 图片编码相关槽函数实现
void MainWindow::onBrowseImageClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择图片文件", "", "Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        imagePathEdit->setText(filePath);
        currentImagePath = filePath;
        
        // 显示图片预览
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            // 缩放图片以适应预览区域
            QPixmap scaledPixmap = pixmap.scaled(imagePreviewLabel->size(), 
                                                Qt::KeepAspectRatio, 
                                                Qt::SmoothTransformation);
            imagePreviewLabel->setPixmap(scaledPixmap);
            currentImage = pixmap;
        }
    }
}

void MainWindow::onEncodeImageClicked()
{
    if (currentImage.isNull()) {
        QMessageBox::warning(this, "警告", "请先选择图片！");
        return;
    }
    
    // 将QPixmap转换为字节数据
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    currentImage.save(&buffer, "PNG");
    buffer.close();

    // 使用后端API进行编码
    std::vector<uint8_t> imageBytes(imageData.begin(), imageData.end());
    std::string encoded = backend_api::encodeImage(imageBytes);
    if (encoded.empty()) {
        QMessageBox::warning(this, "警告", "图片编码失败！");
        return;
    }
   currentEncodedImage = encoded;
    imageResultEdit->setPlainText(QString("图片编码成功！") +
                                 QString("图片路径：") + currentImagePath + "\n" +
                                 QString("图片大小：") + QString::number(currentImage.width()) + "x" + QString::number(currentImage.height()) + "\n" +
                                 QString("原始数据大小：") + QString::number(imageBytes.size()) + " 字节\n" +
                                 QString("编码后大小：") + QString::number(encoded.size()) + " 字符\n" +
                                 QString("压缩率：") + QString::number((double)encoded.size() / imageBytes.size() * 100, 'f', 2) + "%");

    QMessageBox::information(this, "提示", "图片编码成功！");
}

void MainWindow::onExportImageHufClicked()
{
    if (currentEncodedImage.empty()) {
        QMessageBox::warning(this, "警告", "没有可导出的编码结果！");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "导出为HUF文件", "", "HUF Files (*.huf);;All Files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "警告", "无法创建文件！");
        return;
    }

    file.write(QString::fromStdString(currentEncodedImage).toUtf8());
    file.close();

    QMessageBox::information(this, "提示", "HUF文件导出成功！");
}

// 预留扩展接口实现
void MainWindow::onAdvancedFeatureClicked()
{
    // 预留高级功能接口
    QMessageBox::information(this, "提示", "高级功能接口，后续可扩展！");
}

void MainWindow::onSettingsClicked()
{
    // 预留设置接口
    QMessageBox::information(this, "提示", "设置接口，后续可扩展！");
}