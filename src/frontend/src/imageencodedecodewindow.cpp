#include "imageencodedecodewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include "backend_api.h"

ImageEncodeDecodeWindow::ImageEncodeDecodeWindow(QWidget *parent)
    : QMainWindow(parent) {
    initUI();
}

ImageEncodeDecodeWindow::~ImageEncodeDecodeWindow() {
}

void ImageEncodeDecodeWindow::initUI() {
    setWindowTitle("图片编码解码");
    setGeometry(100, 100, 1600, 900);
    setMinimumSize(1200, 700);  // 设置最小窗口尺寸，实现自适应窗口
    
    // 创建主布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 创建水平布局用于编码和解码部分
    QHBoxLayout *encodeDecodeLayout = new QHBoxLayout();
    encodeDecodeLayout->setSpacing(15);
    
    // ==================== 编码部分 ====================
    encodeGroupBox = new QGroupBox("图片编码");
    QVBoxLayout *encodeLayout = new QVBoxLayout(encodeGroupBox);
    encodeLayout->setSpacing(10);
    
    // 图片浏览区域（调整为更紧凑的布局）
    QLabel *imagePathLabel = new QLabel("选择图片文件：");
    QHBoxLayout *imagePathLayout = new QHBoxLayout();
    imagePathEdit = new QLineEdit();
    imagePathEdit->setPlaceholderText("图片文件路径");
    browseImageButton = new QPushButton("浏览");
    connect(browseImageButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onBrowseImageClicked);
    encodeImageButton = new QPushButton("编码图片");
    connect(encodeImageButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onEncodeImageClicked);
    
    imagePathLayout->addWidget(imagePathEdit);
    imagePathLayout->addWidget(browseImageButton);
    imagePathLayout->addWidget(encodeImageButton);
    
    // 图片预览区域（放大，使用stretch因子）
    imagePreviewLabel = new QLabel("图片预览：");
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f5f5f5;");
    
    // 图片编码结果区域（缩小，减少高度）
    QLabel *imageResultLabel = new QLabel("编码结果摘要：");
    encodeResultEdit = new QPlainTextEdit();
    encodeResultEdit->setReadOnly(true);
    encodeResultEdit->setMinimumHeight(80);  // 缩小编码结果摘要区域
    
    exportImageHufButton = new QPushButton("导出为.phuf文件");
    connect(exportImageHufButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onExportImageHufClicked);
    
    // 将图片编码部分的组件添加到布局，使用stretch因子控制大小
    encodeLayout->addWidget(imagePathLabel);
    encodeLayout->addLayout(imagePathLayout);
    encodeLayout->addWidget(imagePreviewLabel, 3);  // 放大预览区域（stretch=3）
    encodeLayout->addWidget(imageResultLabel);
    encodeLayout->addWidget(encodeResultEdit, 1);   // 缩小编码结果区域（stretch=1）
    encodeLayout->addWidget(exportImageHufButton);
    
    // ==================== 解码部分 ====================
    decodeGroupBox = new QGroupBox("图片解码");
    QVBoxLayout *decodeLayout = new QVBoxLayout(decodeGroupBox);
    decodeLayout->setSpacing(10);
    
    // 编码文件浏览区域（缩小，更紧凑的布局）
    QLabel *phufPathLabel = new QLabel("选择.phuf文件：");
    QHBoxLayout *phufPathLayout = new QHBoxLayout();
    phufFilePathEdit = new QLineEdit();
    phufFilePathEdit->setPlaceholderText("phuf文件路径");
    browsePhufFileButton = new QPushButton("浏览");
    connect(browsePhufFileButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onBrowsePhufFileClicked);
    decodeImageButton = new QPushButton("解码图片");
    connect(decodeImageButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onDecodeImageClicked);
    
    phufPathLayout->addWidget(phufFilePathEdit);
    phufPathLayout->addWidget(browsePhufFileButton);
    phufPathLayout->addWidget(decodeImageButton);
    
    // 解码图片预览区域（放大，使用stretch因子）
    decodedImageLabel = new QLabel("解码图片预览：");
    decodedImageLabel->setAlignment(Qt::AlignCenter);
    decodedImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f5f5f5;");
    
    // 保存解码图片按钮
    saveDecodedImageButton = new QPushButton("保存解码图片");
    connect(saveDecodedImageButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onSaveDecodedImageClicked);
    
    // 将图片解码部分的组件添加到布局，使用stretch因子控制大小
    decodeLayout->addWidget(phufPathLabel);
    decodeLayout->addLayout(phufPathLayout);  // 浏览区域往上放
    decodeLayout->addWidget(decodedImageLabel, 3);  // 放大预览区域（stretch=3）
    decodeLayout->addWidget(saveDecodedImageButton);
    
    // 将编码和解码部分添加到水平布局
    encodeDecodeLayout->addWidget(encodeGroupBox, 1);
    encodeDecodeLayout->addWidget(decodeGroupBox, 1);
    
    // 将水平布局添加到主布局
    mainLayout->addLayout(encodeDecodeLayout);
    
    // 添加美化样式
    applyStyles();
}

// 添加美化样式的函数
void ImageEncodeDecodeWindow::applyStyles() {
    // 设置GroupBox样式
    QString groupBoxStyle = "QGroupBox {"
        "border: 2px solid #4CAF50;"
        "border-radius: 8px;"
        "margin-top: 10px;"
        "padding: 10px;"
        "font-weight: bold;"
        "font-size: 14px;"
    "}"
    "QGroupBox::title {"
        "subcontrol-origin: margin;"
        "left: 10px;"
        "padding: 0 5px 0 5px;"
    "}";
    encodeGroupBox->setStyleSheet(groupBoxStyle);
    decodeGroupBox->setStyleSheet(groupBoxStyle);
    
    // 设置按钮样式
    QString buttonStyle = "QPushButton {"
        "background-color: #4CAF50;"
        "color: white;"
        "border: none;"
        "padding: 8px 16px;"
        "border-radius: 4px;"
        "font-size: 13px;"
    "}"
    "QPushButton:hover {"
        "background-color: #45a049;"
    "}"
    "QPushButton:pressed {"
        "background-color: #3e8e41;"
    "}";
    browseImageButton->setStyleSheet(buttonStyle);
    encodeImageButton->setStyleSheet(buttonStyle);
    exportImageHufButton->setStyleSheet(buttonStyle);
    browsePhufFileButton->setStyleSheet(buttonStyle);
    decodeImageButton->setStyleSheet(buttonStyle);
    saveDecodedImageButton->setStyleSheet(buttonStyle);
    
    // 设置输入框样式
    QString lineEditStyle = "QLineEdit {"
        "padding: 6px;"
        "border: 1px solid #ccc;"
        "border-radius: 4px;"
        "font-size: 13px;"
    "}";
    imagePathEdit->setStyleSheet(lineEditStyle);
    phufFilePathEdit->setStyleSheet(lineEditStyle);
    
    // 设置文本编辑区域样式
    QString textEditStyle = "QPlainTextEdit {"
        "padding: 6px;"
        "border: 1px solid #ccc;"
        "border-radius: 4px;"
        "font-size: 13px;"
        "background-color: #f9f9f9;"
    "}";
    encodeResultEdit->setStyleSheet(textEditStyle);
}

// 编码相关槽函数实现
void ImageEncodeDecodeWindow::onBrowseImageClicked() {
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

void ImageEncodeDecodeWindow::onEncodeImageClicked() {
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
    // 计算压缩率
    double originalSize = imageBytes.size();
    
    // 分离编码表和实际编码数据（查找最后一个|字符作为分隔符）
    size_t separatorPos = encoded.rfind('|');
    double encodedSize = encoded.size();
    double tableSize = 0;
    double bitStringSize = 0;
    double actualEncodedByteSize = 0;
    
    if (separatorPos != std::string::npos) {
        tableSize = separatorPos + 1; // 包括分隔符本身
        bitStringSize = encodedSize - tableSize;
        // 关键修改：将字符数转换为实际二进制位占用的字节数
        // 每个字符代表一个二进制位，所以字节数是字符数除以8
        actualEncodedByteSize = bitStringSize / 8;
    }
    
    // 只使用实际编码数据的大小来计算压缩率
    double compressionRatio = (1 - actualEncodedByteSize / originalSize) * 100;

    QString resultText = QString("图片编码成功！") +
                     QString("图片路径：") + currentImagePath + "\n" +
                     QString("图片大小：") + QString::number(currentImage.width()) + "x" + QString::number(currentImage.height()) + "\n" +
                     QString("原始数据大小：") + QString::number(originalSize) + " 字节\n" +
                     QString("编码后总大小：") + QString::number(encodedSize) + " 字符\n" +
                     QString("其中编码表大小：") + QString::number(tableSize) + " 字符\n" +
                     QString("实际编码数据（字符串）：") + QString::number(bitStringSize) + " 字符\n" +
                     QString("实际编码数据（二进制）：") + QString::number(actualEncodedByteSize) + " 字节\n";

    if (compressionRatio > 0) {
        resultText += QString("压缩率（仅数据）：") + QString::number(compressionRatio, 'f', 2) + "%";
    } else {
        resultText += QString("膨胀率（仅数据）：") + QString::number(-compressionRatio, 'f', 2) + "%";
    }

    encodeResultEdit->setPlainText(resultText);

    QMessageBox::information(this, "提示", "图片编码成功！");
}

void ImageEncodeDecodeWindow::onExportImageHufClicked() {
    if (currentEncodedImage.empty()) {
        QMessageBox::warning(this, "警告", "没有可导出的编码结果！");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "导出为PHUF文件", "", "PHUF Files (*.phuf);;All Files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "警告", "无法创建文件！");
        return;
    }

    file.write(QString::fromStdString(currentEncodedImage).toUtf8());
    file.close();

    QMessageBox::information(this, "提示", "PHUF文件导出成功！");
}

// 解码相关槽函数实现
void ImageEncodeDecodeWindow::onBrowsePhufFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择.phuf文件", "", "PHUF Files (*.phuf);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        phufFilePathEdit->setText(filePath);
    }
}

void ImageEncodeDecodeWindow::onDecodeImageClicked() {
    QString filePath = phufFilePathEdit->text();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择.phuf文件！");
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "警告", "无法打开文件！");
        return;
    }
    
    QByteArray content = file.readAll();
    file.close();
    
    // 使用后端API进行解码
    std::string encoded = content.toStdString();
    std::vector<uint8_t> decodedBytes = backend_api::decodeImage(encoded);
    if (decodedBytes.empty()) {
        QMessageBox::warning(this, "警告", "图片解码失败！");
        return;
    }
    
    // 将字节数据转换为QPixmap
    QByteArray decodedData(reinterpret_cast<const char*>(decodedBytes.data()), decodedBytes.size());
    QPixmap pixmap;
    if (!pixmap.loadFromData(decodedData)) {
        QMessageBox::warning(this, "警告", "无法加载解码后的图片！");
        return;
    }
    
    decodedImage = pixmap;
    
    // 显示解码后的图片
    QPixmap scaledPixmap = pixmap.scaled(decodedImageLabel->size(), 
                                        Qt::KeepAspectRatio, 
                                        Qt::SmoothTransformation);
    decodedImageLabel->setPixmap(scaledPixmap);
    
    QMessageBox::information(this, "提示", "图片解码成功！");
}

void ImageEncodeDecodeWindow::onSaveDecodedImageClicked() {
    if (decodedImage.isNull()) {
        QMessageBox::warning(this, "警告", "没有可保存的解码图片！");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this, "保存解码图片", "", "PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;BMP Files (*.bmp);;All Files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    
    if (decodedImage.save(filePath)) {
        QMessageBox::information(this, "提示", "解码图片保存成功！");
    } else {
        QMessageBox::warning(this, "警告", "保存图片失败！");
    }
}