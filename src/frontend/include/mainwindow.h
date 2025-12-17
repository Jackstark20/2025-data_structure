#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QByteArray>
#include <QBuffer>

// 包含后端API头文件
#include "../backend/include/backend_api.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文本编码相关槽函数
    void onEncodeTextClicked();
    void onBrowseTextFileClicked();
    void onEncodeTextFileClicked();
    void onExportTextHufClicked();
    
    // 图片编码相关槽函数
    void onBrowseImageClicked();
    void onEncodeImageClicked();
    void onExportImageHufClicked();

private:
    // 初始化UI组件
    void initUI();
    
    // 文本编码部分UI组件
    QGroupBox *textGroupBox;
    QLabel *textInputLabel;
    QTextEdit *textInputEdit;
    QPushButton *encodeTextButton;
    
    QLabel *textFileLabel;
    QLineEdit *textFilePathEdit;
    QPushButton *browseTextFileButton;
    QPushButton *encodeTextFileButton;
    
    QLabel *textResultLabel;
    QPlainTextEdit *textResultEdit;
    QPushButton *exportTextHufButton;
    
    // 图片编码部分UI组件
    QGroupBox *imageGroupBox;
    QLabel *imagePathLabel;
    QLineEdit *imagePathEdit;
    QPushButton *browseImageButton;
    QPushButton *encodeImageButton;
    
    QLabel *imagePreviewLabel;
    QLabel *imageResultLabel;
    QPlainTextEdit *imageResultEdit;
    QPushButton *exportImageHufButton;
    
    // 存储当前处理的数据
    std::string currentEncodedText;
    std::string currentEncodedImage;
    QPixmap currentImage;
    QString currentImagePath;
    
    // 预留扩展接口
    virtual void onAdvancedFeatureClicked();
    virtual void onSettingsClicked();
};

#endif // MAINWINDOW_H