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
    setGeometry(100, 100, 1600, 900);  // 增大窗口尺寸
    
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
    
    // 图片浏览区域
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
    
    // 图片预览区域
    imagePreviewLabel = new QLabel("图片预览：");
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setMinimumHeight(300);
    imagePreviewLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
    
    // 图片编码结果区域
    QLabel *imageResultLabel = new QLabel("编码结果摘要：");
    encodeResultEdit = new QPlainTextEdit();
    encodeResultEdit->setReadOnly(true);
    encodeResultEdit->setMinimumHeight(120);
    
    exportImageHufButton = new QPushButton("导出为.phuf文件");
    connect(exportImageHufButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onExportImageHufClicked);
    
    // 将图片编码部分的组件添加到布局
    encodeLayout->addWidget(imagePathLabel);
    encodeLayout->addLayout(imagePathLayout);
    encodeLayout->addWidget(imagePreviewLabel);
    encodeLayout->addWidget(imageResultLabel);
    encodeLayout->addWidget(encodeResultEdit);
    encodeLayout->addWidget(exportImageHufButton);
    
    // ==================== 解码部分 ====================
    decodeGroupBox = new QGroupBox("图片解码");
    QVBoxLayout *decodeLayout = new QVBoxLayout(decodeGroupBox);
    decodeLayout->setSpacing(10);
    
    // 编码文件浏览区域
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
    
    // 解码图片预览区域
    decodedImageLabel = new QLabel("解码图片预览：");
    decodedImageLabel->setAlignment(Qt::AlignCenter);
    decodedImageLabel->setMinimumHeight(300);
    decodedImageLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f0f0f0;");
    
    // 保存解码图片按钮
    saveDecodedImageButton = new QPushButton("保存解码图片");
    connect(saveDecodedImageButton, &QPushButton::clicked, this, &ImageEncodeDecodeWindow::onSaveDecodedImageClicked);
    
    // 将图片解码部分的组件添加到布局
    decodeLayout->addWidget(phufPathLabel);
    decodeLayout->addLayout(phufPathLayout);
    decodeLayout->addWidget(decodedImageLabel);
    decodeLayout->addWidget(saveDecodedImageButton);
    
    // 将编码和解码部分添加到水平布局
    encodeDecodeLayout->addWidget(encodeGroupBox, 1);
    encodeDecodeLayout->addWidget(decodeGroupBox, 1);
    
    // 将水平布局添加到主布局
    mainLayout->addLayout(encodeDecodeLayout);
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
    encodeResultEdit->setPlainText(QString("图片编码成功！") +
                                 QString("图片路径：") + currentImagePath + "\n" +
                                 QString("图片大小：") + QString::number(currentImage.width()) + "x" + QString::number(currentImage.height()) + "\n" +
                                 QString("原始数据大小：") + QString::number(imageBytes.size()) + " 字节\n" +
                                 QString("编码后大小：") + QString::number(encoded.size()) + " 字符\n" +
                                 QString("压缩率：") + QString::number((double)encoded.size() / imageBytes.size() * 100, 'f', 2) + "%");

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
    if (!file.open(QIODevice::WriteOnly)) { // 移除QIODevice::Text
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
    if (!file.open(QIODevice::ReadOnly)) { // 移除QIODevice::Text
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