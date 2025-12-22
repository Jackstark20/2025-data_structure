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
};