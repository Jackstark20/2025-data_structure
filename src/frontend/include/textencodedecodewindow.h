#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QFile>
#include <QTimer>
#include "characterfrequencychart.h"

class TextEncodeDecodeWindow : public QMainWindow {
    Q_OBJECT

public:
    TextEncodeDecodeWindow(QWidget *parent = nullptr);
    ~TextEncodeDecodeWindow();

private slots:
    // 编码相关
    void onEncodeTextClicked();
    void onBrowseTextFileClicked();
    void onEncodeTextFileClicked();
    void onExportTextHufClicked();
    
    // 解码相关
    void onBrowseEncodedFileClicked();
    void onDecodeTextClicked();
    void onSaveDecodedTextClicked();

    void readFileChunk();
    void encodeAccumulatedContent();

private:
    void initUI();
    
    // 编码部分组件
    QGroupBox *encodeGroupBox;
    QTextEdit *textInputEdit;
    QPushButton *encodeTextButton;
    QLineEdit *textFilePathEdit;
    QPushButton *browseTextFileButton;
    QPushButton *encodeTextFileButton;
    QPlainTextEdit *encodeResultEdit;
    QPushButton *exportTextHufButton;
    
    // 解码部分组件
    QGroupBox *decodeGroupBox;
    QLineEdit *encodedFilePathEdit;
    QPushButton *browseEncodedFileButton;
    QPlainTextEdit *decodeResultEdit;
    QPushButton *decodeTextButton;
    QPushButton *saveDecodedTextButton;
    
    // 当前编码结果
    std::string currentEncodedText;

    // 添加图表组件
    CharacterFrequencyChart *m_frequencyChart;
    
    // 字符频率映射
    QMap<QChar, int> m_characterFrequency;
    
    // 读取文件时的临时数据
    QFile m_currentFile;
    QTimer m_readTimer;
    int m_readChunkSize; // 每次读取的字符数
    QByteArray m_accumulatedContent; // 累积文件内容
};