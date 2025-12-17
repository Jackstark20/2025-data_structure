#include "textencodedecodewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include "backend_api.h"

TextEncodeDecodeWindow::TextEncodeDecodeWindow(QWidget *parent)
    : QMainWindow(parent) {
    initUI();
}

TextEncodeDecodeWindow::~TextEncodeDecodeWindow() {
}

void TextEncodeDecodeWindow::initUI() {
    setWindowTitle("文本编码解码");
    setGeometry(100, 100, 1200, 800);
    
    // 创建主布局
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // ==================== 编码部分 ====================
    encodeGroupBox = new QGroupBox("文本编码");
    QVBoxLayout *encodeLayout = new QVBoxLayout(encodeGroupBox);
    encodeLayout->setSpacing(15);
    
    // 文本输入区域
    QLabel *textInputLabel = new QLabel("输入要编码的文本：");
    textInputEdit = new QTextEdit();
    textInputEdit->setPlaceholderText("请输入文本...");
    textInputEdit->setMinimumHeight(150);
    
    encodeTextButton = new QPushButton("编码文本");
    connect(encodeTextButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onEncodeTextClicked);
    
    // 文本文件浏览区域
    QHBoxLayout *textFileLayout = new QHBoxLayout();
    QLabel *textFileLabel = new QLabel("或选择本地文本文件：");
    textFilePathEdit = new QLineEdit();
    textFilePathEdit->setPlaceholderText("文本文件路径");
    browseTextFileButton = new QPushButton("浏览");
    connect(browseTextFileButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onBrowseTextFileClicked);
    encodeTextFileButton = new QPushButton("编码文本文件");
    connect(encodeTextFileButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onEncodeTextFileClicked);
    
    textFileLayout->addWidget(textFilePathEdit);
    textFileLayout->addWidget(browseTextFileButton);
    textFileLayout->addWidget(encodeTextFileButton);
    
    // 编码结果展示区域
    QLabel *encodeResultLabel = new QLabel("编码结果：");
    encodeResultEdit = new QPlainTextEdit();
    encodeResultEdit->setReadOnly(true);
    encodeResultEdit->setMinimumHeight(200);
    
    exportTextHufButton = new QPushButton("导出为.huf文件");
    connect(exportTextHufButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onExportTextHufClicked);
    
    // 将编码部分的组件添加到布局
    encodeLayout->addWidget(textInputLabel);
    encodeLayout->addWidget(textInputEdit);
    encodeLayout->addWidget(encodeTextButton);
    encodeLayout->addWidget(textFileLabel);
    encodeLayout->addLayout(textFileLayout);
    encodeLayout->addWidget(encodeResultLabel);
    encodeLayout->addWidget(encodeResultEdit);
    encodeLayout->addWidget(exportTextHufButton);
    
    // ==================== 解码部分 ====================
    decodeGroupBox = new QGroupBox("文本解码");
    QVBoxLayout *decodeLayout = new QVBoxLayout(decodeGroupBox);
    decodeLayout->setSpacing(15);
    
    // 编码文件浏览区域
    QHBoxLayout *encodedFileLayout = new QHBoxLayout();
    QLabel *encodedFileLabel = new QLabel("选择编码文件：");
    encodedFilePathEdit = new QLineEdit();
    encodedFilePathEdit->setPlaceholderText("编码文件路径");
    browseEncodedFileButton = new QPushButton("浏览");
    connect(browseEncodedFileButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onBrowseEncodedFileClicked);
    decodeTextButton = new QPushButton("解码");
    connect(decodeTextButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onDecodeTextClicked);
    
    encodedFileLayout->addWidget(encodedFilePathEdit);
    encodedFileLayout->addWidget(browseEncodedFileButton);
    encodedFileLayout->addWidget(decodeTextButton);
    
    // 解码结果展示区域
    QLabel *decodeResultLabel = new QLabel("解码结果：");
    decodeResultEdit = new QPlainTextEdit();
    decodeResultEdit->setReadOnly(true);
    decodeResultEdit->setMinimumHeight(200);
    
    saveDecodedTextButton = new QPushButton("保存解码结果");
    connect(saveDecodedTextButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onSaveDecodedTextClicked);
    
    // 将解码部分的组件添加到布局
    decodeLayout->addWidget(encodedFileLabel);
    decodeLayout->addLayout(encodedFileLayout);
    decodeLayout->addWidget(decodeResultLabel);
    decodeLayout->addWidget(decodeResultEdit);
    decodeLayout->addWidget(saveDecodedTextButton);
    
    // 将编码和解码部分添加到主布局
    mainLayout->addWidget(encodeGroupBox);
    mainLayout->addWidget(decodeGroupBox);
}

// 编码相关槽函数实现
void TextEncodeDecodeWindow::onEncodeTextClicked() {
    QString text = textInputEdit->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要编码的文本！");
        return;
    }
    
    // 使用后端API进行编码
    std::string utf8Text = text.toUtf8().toStdString();
    std::string encoded = backend_api::encodeTextUtf8(utf8Text);
    if (encoded.empty()) {
        QMessageBox::warning(this, "警告", "编码失败！");
        return;
    }
    
    currentEncodedText = encoded;
    encodeResultEdit->setPlainText(QString::fromUtf8(encoded.c_str()));
    
    // 添加调试信息
    QString debugInfo = QString("编码成功！\n") +
                       QString("原文本长度: %1 字节\n").arg(utf8Text.size()) +
                       QString("编码后长度: %1 字节\n").arg(encoded.size()) +
                       QString("编码前几个字符: ") + QString::fromStdString(encoded.substr(0, 50)) + "...";
    QMessageBox::information(this, "调试信息", debugInfo);
}

void TextEncodeDecodeWindow::onBrowseTextFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择文本文件", "", "Text Files (*.txt);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        textFilePathEdit->setText(filePath);
    }
}

void TextEncodeDecodeWindow::onEncodeTextFileClicked() {
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
    
    // 使用后端API进行编码
    std::string utf8Content = content.toUtf8().toStdString();
    std::string encoded = backend_api::encodeTextUtf8(utf8Content);
    if (encoded.empty()) {
        QMessageBox::warning(this, "警告", "编码失败！");
        return;
    }
    
    currentEncodedText = encoded;
    encodeResultEdit->setPlainText(QString::fromUtf8(encoded.c_str()));
    
    // 添加调试信息
    QString debugInfo = QString("文件编码成功！\n") +
                       QString("原文件内容长度: %1 字节\n").arg(utf8Content.size()) +
                       QString("编码后长度: %1 字节\n").arg(encoded.size()) +
                       QString("编码前几个字符: ") + QString::fromStdString(encoded.substr(0, 50)) + "...";
    QMessageBox::information(this, "调试信息", debugInfo);
}

void TextEncodeDecodeWindow::onExportTextHufClicked() {
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
    if (!file.open(QIODevice::WriteOnly)) { // 移除QIODevice::Text
        QMessageBox::warning(this, "警告", "无法创建文件！");
        return;
    }
    
    qint64 bytesWritten = file.write(QString::fromStdString(currentEncodedText).toUtf8());
    file.close();
    
    // 添加调试信息
    QString debugInfo = QString("HUF文件导出成功！\n") +
                       QString("导出文件路径: %1\n").arg(filePath) +
                       QString("导出文件大小: %1 字节\n").arg(bytesWritten) +
                       QString("编码数据长度: %1 字节").arg(currentEncodedText.size());
    QMessageBox::information(this, "调试信息", debugInfo);
}

// 解码相关槽函数实现
void TextEncodeDecodeWindow::onBrowseEncodedFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择编码文件", "", "HUF Files (*.huf);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        encodedFilePathEdit->setText(filePath);
    }
}

void TextEncodeDecodeWindow::onDecodeTextClicked() {
    QString filePath = encodedFilePathEdit->text();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择编码文件！");
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) { // 移除QIODevice::Text
        QMessageBox::warning(this, "警告", "无法打开文件！");
        return;
    }
    
    QByteArray content = file.readAll();
    file.close();
    
    // 添加调试信息 - 文件读取
    QString debugInfo1 = QString("文件读取成功！\n") +
                        QString("文件路径: %1\n").arg(filePath) +
                        QString("文件大小: %1 字节\n").arg(content.size());
    QMessageBox::information(this, "调试信息1", debugInfo1);
    
    // 使用后端API进行解码
    std::string encoded = content.toStdString();
    
    // 添加调试信息 - 编码数据
    QString debugInfo2 = QString("编码数据信息！\n") +
                        QString("编码数据长度: %1 字节\n").arg(encoded.size()) +
                        QString("编码前几个字符: ") + QString::fromStdString(encoded.substr(0, 50)) + "...";
    QMessageBox::information(this, "调试信息2", debugInfo2);
    
    std::string decoded = backend_api::decodeTextUtf8(encoded);
    if (decoded.empty()) {
        // 检查解码失败的原因
        std::wstring combined = utf8_to_wstring(encoded);
        size_t sep = combined.find(L'|');
        
        QString errorInfo = QString("解码失败！\n") +
                          QString("分隔符位置: %1\n").arg(static_cast<int>(sep));
        
        if (sep == std::wstring::npos) {
            errorInfo += "无法找到分隔符 '|'！\n";
        } else {
            errorInfo += QString("编码表部分长度: %1\n").arg(static_cast<int>(sep));
            errorInfo += QString("编码位部分长度: %1\n").arg(static_cast<int>(combined.size() - sep - 1));
        }
        
        QMessageBox::warning(this, "解码失败详细信息", errorInfo);
        return;
    }
    
    decodeResultEdit->setPlainText(QString::fromUtf8(decoded.c_str()));
    
    // 添加调试信息
    QString debugInfo3 = QString("文本解码成功！\n") +
                        QString("解码后长度: %1 字节\n").arg(decoded.size()) +
                        QString("解码前几个字符: ") + QString::fromStdString(decoded.substr(0, 50)) + "...";
    QMessageBox::information(this, "调试信息3", debugInfo3);
}

void TextEncodeDecodeWindow::onSaveDecodedTextClicked() {
    QString text = decodeResultEdit->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有可保存的解码结果！");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this, "保存解码结果", "", "Text Files (*.txt);;All Files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "警告", "无法创建文件！");
        return;
    }
    
    file.write(text.toUtf8());
    file.close();
    
    QMessageBox::information(this, "提示", "解码结果保存成功！");
}