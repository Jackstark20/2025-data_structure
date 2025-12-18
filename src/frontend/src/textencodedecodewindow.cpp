#include "textencodedecodewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include "backend_api.h"

TextEncodeDecodeWindow::TextEncodeDecodeWindow(QWidget *parent)
    : QMainWindow(parent),
      m_readChunkSize(1024) {  // 初始化：每次读取1024字节
    initUI();
}

TextEncodeDecodeWindow::~TextEncodeDecodeWindow() {
}

void TextEncodeDecodeWindow::initUI() {
    setWindowTitle("文本编码解码");
    setGeometry(100, 100, 1400, 900);  // 增大窗口尺寸
    
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
    encodeResultEdit->setMaximumHeight(100);  // 限制高度
    
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
    decodeResultEdit->setMaximumHeight(100);  // 限制高度
    
    saveDecodedTextButton = new QPushButton("保存解码结果");
    connect(saveDecodedTextButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onSaveDecodedTextClicked);
    
    // 将解码部分的组件添加到布局
    decodeLayout->addWidget(encodedFileLabel);
    decodeLayout->addLayout(encodedFileLayout);
    decodeLayout->addWidget(decodeResultLabel);
    decodeLayout->addWidget(decodeResultEdit);
    decodeLayout->addWidget(saveDecodedTextButton);
    
    // 初始化字符频率图表
    m_frequencyChart = new CharacterFrequencyChart(this);
    if (m_frequencyChart) {
        m_frequencyChart->setRefreshInterval(200); // 设置200ms刷新一次
    }

    // 将编码和解码部分添加到水平布局
    encodeDecodeLayout->addWidget(encodeGroupBox);
    encodeDecodeLayout->addWidget(decodeGroupBox);
    
    // 设置编码和解码区域的拉伸比例（减小编码解码区域）
    encodeDecodeLayout->setStretchFactor(encodeGroupBox, 1);
    encodeDecodeLayout->setStretchFactor(decodeGroupBox, 1);

    // 将水平布局添加到主布局
    mainLayout->addLayout(encodeDecodeLayout);
    
    // 添加分割线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);
    
    // 添加字符频率统计标签
    QLabel *chartLabel = new QLabel("字符频率统计");
    chartLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(chartLabel);
    
    // 设置字符频率图表的拉伸比例（增大图表区域）
    mainLayout->addWidget(m_frequencyChart, 3);  // 拉伸因子为3，占据更多空间
    
    // 初始化读取定时器
    connect(&m_readTimer, &QTimer::timeout, this, &TextEncodeDecodeWindow::readFileChunk);
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

// 修改文件读取函数，实现流式读取
void TextEncodeDecodeWindow::onEncodeTextFileClicked()
{
    // 使用之前浏览控件选择的文件路径，而不是重新弹出对话框
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
    
    // 重置频率数据和累积内容
    m_characterFrequency.clear();
    m_accumulatedContent.clear();
    m_frequencyChart->reset();

    // 打开文件
    m_currentFile.setFileName(filePath);
    if (!m_currentFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file!"));
        return;
    }

    // 开始流式读取
    m_readTimer.start(50); // 每50ms读取一次
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

// 读取文件块并进行编码
void TextEncodeDecodeWindow::readFileChunk()
{
    if (!m_currentFile.isOpen()) {
        m_readTimer.stop();
        return;
    }
    
    // 检查图表是否有效
    if (!m_frequencyChart) {
        m_readTimer.stop();
        m_currentFile.close();
        QMessageBox::warning(this, tr("Error"), tr("Frequency chart is not initialized!"));
        return;
    }
    
    // 读取指定大小的字符
    QByteArray chunk = m_currentFile.read(m_readChunkSize);
    if (chunk.isEmpty() || chunk.size() == 0) {
        // 文件读取完成
        m_readTimer.stop();
        m_currentFile.close();

        // 使用累积的内容进行编码
        encodeAccumulatedContent();

        QMessageBox::information(this, tr("Success"), tr("File read complete!"));
        return;
    }

    // 累积文件内容
    m_accumulatedContent.append(chunk);

    // 更新字符频率 - 安全处理 UTF-8（Qt6 方式）
    QString text = QString::fromUtf8(chunk);

    // 如果 UTF-8 解码失败（返回空字符串但 chunk 不为空），尝试 Latin1
    // 这通常发生在文件不是 UTF-8 编码时
    if (text.isEmpty() && !chunk.isEmpty()) {
        text = QString::fromLatin1(chunk.constData(), chunk.size());
    }

    // 统计字符频率
    for (const QChar &ch : text) {
        if (ch.isPrint() || ch == '\n' || ch == '\t' || ch == '\r') {
            m_characterFrequency[ch]++;
        }
    }

    // 更新图表
    m_frequencyChart->updateFrequency(m_characterFrequency);
}

// 新增：编码累积的文件内容
void TextEncodeDecodeWindow::encodeAccumulatedContent()
{
    // 使用累积的内容进行编码
    std::string utf8Text = m_accumulatedContent.toStdString();
    std::string encoded = backend_api::encodeTextUtf8(utf8Text);
    if (encoded.empty()) {
        QMessageBox::warning(this, "警告", "编码失败！");
        return;
    }

    currentEncodedText = encoded;
    encodeResultEdit->setPlainText(QString::fromUtf8(encoded.c_str()));
}
