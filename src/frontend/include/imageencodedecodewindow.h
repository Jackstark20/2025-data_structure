#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QPixmap>

class ImageEncodeDecodeWindow : public QMainWindow {
    Q_OBJECT

public:
    ImageEncodeDecodeWindow(QWidget *parent = nullptr);
    ~ImageEncodeDecodeWindow();

private slots:
    // 编码相关
    void onBrowseImageClicked();
    void onEncodeImageClicked();
    void onExportImageHufClicked();
    
    // 解码相关
    void onBrowsePhufFileClicked();
    void onDecodeImageClicked();
    void onSaveDecodedImageClicked();

private:
    void initUI();
    
    // 编码部分组件
    QGroupBox *encodeGroupBox;
    QLineEdit *imagePathEdit;
    QPushButton *browseImageButton;
    QPushButton *encodeImageButton;
    QLabel *imagePreviewLabel;
    QPlainTextEdit *encodeResultEdit;
    QPushButton *exportImageHufButton;
    
    // 解码部分组件
    QGroupBox *decodeGroupBox;
    QLineEdit *phufFilePathEdit;
    QPushButton *browsePhufFileButton;
    QPushButton *decodeImageButton;
    QLabel *decodedImageLabel;
    QPushButton *saveDecodedImageButton;
    
    // 当前图片和编码结果
    QPixmap currentImage;
    QString currentImagePath;
    std::string currentEncodedImage;
    QPixmap decodedImage;
};