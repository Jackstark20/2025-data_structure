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
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QAbstractAnimation>
#include <QSlider>
#include <QTimer>
#include <QMap>
#include <QPair>
#include <QSharedPointer>
#include <functional>

// Huffman树相关结构体和类
struct HuffNode {
    int weight;
    QString label; // for leaves
    QSharedPointer<HuffNode> left;
    QSharedPointer<HuffNode> right;
    int id;
    HuffNode(int w = 0, const QString &l = QString(), int _id = -1)
        : weight(w), label(l), left(nullptr), right(nullptr), id(_id) {}
};

struct HuffStep {
    QSharedPointer<HuffNode> a;
    QSharedPointer<HuffNode> b;
    QSharedPointer<HuffNode> parent;
};

class HuffmanBuilder {
public:
    // input: vector of (label, weight)
    explicit HuffmanBuilder(const QVector<QPair<QString,int>>& items);

    // generate steps of combining two smallest nodes until one root remains
    QVector<HuffStep> buildSteps();

private:
    QVector<QPair<QString,int>> m_items;
};

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
    
    // 哈夫曼树可视化相关
    void onHuffmanStep();
    void onHuffmanPlayPause();
    void onHuffmanReset();
    void onHuffmanZoomChanged(int value);
    void autoplayHuffmanStep();

private:
    void initUI();
    void initHuffmanVisualization();
    void prepareHuffmanNodes();
    void performHuffmanStep(const HuffStep &step);
    void layoutAndAnimateHuffman(int stepsDone);
    void clearHuffmanEdges();
    void drawHuffmanEdges(int stepsDone);
    
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
    
    // 哈夫曼树可视化相关
    QGraphicsView *m_huffmanView{nullptr};
    QGraphicsScene *m_huffmanScene{nullptr};
    QPushButton *m_stepBtn{nullptr};
    QPushButton *m_playBtn{nullptr};
    QPushButton *m_resetBtn{nullptr};
    QTimer *m_huffmanTimer{nullptr};
    QSlider *m_zoomSlider{nullptr};
    
    QVector<QPair<QString, int>> m_huffmanItems;
    QVector<HuffStep> m_huffmanSteps;
    int m_currentHuffmanStep{0};
    QMap<int, QGraphicsItem*> m_huffmanItemMap;
    QVector<QGraphicsItem*> m_huffmanEdgeItems;
    QVector<QAbstractAnimation*> m_activeAnims;
    
    // 字符频率统计
    QMap<QChar, int> m_characterFrequency;
};