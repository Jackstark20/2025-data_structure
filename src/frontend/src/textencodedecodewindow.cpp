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
    setGeometry(100, 100, 1400, 600);  // 调整窗口尺寸
    
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
    encodeGroupBox = new QGroupBox("文本编码");
    QVBoxLayout *encodeLayout = new QVBoxLayout(encodeGroupBox);
    encodeLayout->setSpacing(10);
    
    // 文本输入区域
    QLabel *textInputLabel = new QLabel("输入要编码的文本：");
    textInputEdit = new QTextEdit();
    textInputEdit->setPlaceholderText("请输入文本...");
    textInputEdit->setMaximumHeight(80);  // 限制高度
    
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
    encodeResultEdit->setMaximumHeight(150);  // 调整高度
    
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
    decodeLayout->setSpacing(10);
    
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
    decodeResultEdit->setMaximumHeight(150);  // 调整高度
    
    saveDecodedTextButton = new QPushButton("保存解码结果");
    connect(saveDecodedTextButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onSaveDecodedTextClicked);
    
    // 将解码部分的组件添加到布局
    decodeLayout->addWidget(encodedFileLabel);
    decodeLayout->addLayout(encodedFileLayout);
    decodeLayout->addWidget(decodeResultLabel);
    decodeLayout->addWidget(decodeResultEdit);
    decodeLayout->addWidget(saveDecodedTextButton);

    // 将编码和解码部分添加到水平布局
    encodeDecodeLayout->addWidget(encodeGroupBox);
    encodeDecodeLayout->addWidget(decodeGroupBox);
    
    // 设置编码和解码区域的拉伸比例
    encodeDecodeLayout->setStretchFactor(encodeGroupBox, 1);
    encodeDecodeLayout->setStretchFactor(decodeGroupBox, 1);

    // 将水平布局添加到主布局
    mainLayout->addLayout(encodeDecodeLayout);
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
}

void TextEncodeDecodeWindow::onBrowseTextFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择文本文件", "", "Text Files (*.txt);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        textFilePathEdit->setText(filePath);
    }
}

// 修改为直接读取整个文件
void TextEncodeDecodeWindow::onEncodeTextFileClicked()
{
    // 使用之前浏览控件选择的文件路径
    QString filePath = textFilePathEdit->text();
    if (filePath.isEmpty()) {
        // 如果没有选择文件，则弹出对话框
        filePath = QFileDialog::getOpenFileName(this, tr("Open Text File"), "", tr("Text Files (*.txt);;All Files (*)"));
        if (filePath.isEmpty()) {
            return;
        }
        // 更新浏览控件显示的路径
        textFilePathEdit->setText(filePath);
    }
    
    // 直接读取整个文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file!"));
        return;
    }
    
    QByteArray content = file.readAll();
    file.close();
    
    // 使用后端API进行编码
    std::string utf8Text = content.toStdString();
    std::string encoded = backend_api::encodeTextUtf8(utf8Text);
    if (encoded.empty()) {
        QMessageBox::warning(this, "警告", "编码失败！");
        return;
    }
    
    currentEncodedText = encoded;
    encodeResultEdit->setPlainText(QString::fromUtf8(encoded.c_str()));
    
    QMessageBox::information(this, tr("Success"), tr("File encoded successfully!"));
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
    
    QByteArray content = QString::fromStdString(currentEncodedText).toUtf8();
    qint64 bytesWritten = file.write(content);
    file.close();
    
    if (bytesWritten == content.size()) {
        QMessageBox::information(this, "提示", "HUF文件导出成功！");
    } else {
        QMessageBox::warning(this, "警告", "HUF文件导出不完整！");
    }
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
    
    // 使用后端API进行解码
    std::string encoded = content.toStdString();
    std::string decoded = backend_api::decodeTextUtf8(encoded);
    if (decoded.empty()) {
        QMessageBox::warning(this, "警告", "解码失败！");
        return;
    }
    
    decodeResultEdit->setPlainText(QString::fromUtf8(decoded.c_str()));
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