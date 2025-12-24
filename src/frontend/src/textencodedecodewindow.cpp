#include "textencodedecodewindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QPolygonF>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QWheelEvent>
#include <algorithm>
#include "backend_api.h"

// 自定义缩放视图类
class ZoomView : public QGraphicsView {
public:
    ZoomView(QWidget *parent = nullptr) : QGraphicsView(parent) {
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    }
protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier) {
            const double scaleFactor = 1.15;
            if (event->angleDelta().y() > 0) scale(scaleFactor, scaleFactor);
            else scale(1.0/scaleFactor, 1.0/scaleFactor);
            event->accept();
        } else {
            QGraphicsView::wheelEvent(event);
        }
    }
};

// HuffmanBuilder类成员函数实现
HuffmanBuilder::HuffmanBuilder(const QVector<QPair<QString,int>>& items)
    : m_items(items)
{
}

QVector<HuffStep> HuffmanBuilder::buildSteps()
{
    QVector<HuffStep> steps;
    // create initial nodes
    QVector<QSharedPointer<HuffNode>> pool;
    int nextId = 0;
    for (const auto &p : m_items) {
        auto node = QSharedPointer<HuffNode>::create(p.second, p.first, nextId++);
        pool.append(node);
    }

    // use simple greedy algorithm
    while (pool.size() > 1) {
        // sort ascending by weight, tie-break by id
        std::sort(pool.begin(), pool.end(), [](const QSharedPointer<HuffNode>& n1, const QSharedPointer<HuffNode>& n2){
            if (n1->weight != n2->weight) return n1->weight < n2->weight;
            return n1->id < n2->id;
        });

        auto a = pool.takeFirst();
        auto b = pool.takeFirst();
        auto parent = QSharedPointer<HuffNode>::create(a->weight + b->weight, QString(), nextId++);
        parent->left = a;
        parent->right = b;

        HuffStep s;
        s.a = a;
        s.b = b;
        s.parent = parent;
        steps.append(s);

        pool.append(parent);
    }

    return steps;
}

TextEncodeDecodeWindow::TextEncodeDecodeWindow(QWidget *parent)
    : QMainWindow(parent) {
    initUI();
}

TextEncodeDecodeWindow::~TextEncodeDecodeWindow() {
}

void TextEncodeDecodeWindow::initUI() {
    setWindowTitle("文本编码解码");
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
    QLabel *encodedFileLabel = new QLabel("选择要解码的.huf文件：");
    encodedFilePathEdit = new QLineEdit();
    encodedFilePathEdit->setPlaceholderText(".huf文件路径");
    browseEncodedFileButton = new QPushButton("浏览");
    connect(browseEncodedFileButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onBrowseEncodedFileClicked);
    
    encodedFileLayout->addWidget(encodedFilePathEdit);
    encodedFileLayout->addWidget(browseEncodedFileButton);
    
    // 解码结果展示区域
    QLabel *decodeResultLabel = new QLabel("解码结果：");
    decodeResultEdit = new QPlainTextEdit();
    decodeResultEdit->setReadOnly(true);
    decodeResultEdit->setMaximumHeight(100);  // 限制高度
    
    // 解码按钮
    decodeTextButton = new QPushButton("解码");
    connect(decodeTextButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onDecodeTextClicked);
    
    // 保存解码结果按钮
    saveDecodedTextButton = new QPushButton("保存解码结果");
    connect(saveDecodedTextButton, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onSaveDecodedTextClicked);
    
    // 将解码部分的组件添加到布局
    decodeLayout->addWidget(encodedFileLabel);
    decodeLayout->addLayout(encodedFileLayout);
    decodeLayout->addWidget(decodeResultLabel);
    decodeLayout->addWidget(decodeResultEdit);
    decodeLayout->addWidget(decodeTextButton);
    decodeLayout->addWidget(saveDecodedTextButton);
    
    // 将编码和解码部分添加到主布局
    encodeDecodeLayout->addWidget(encodeGroupBox, 1);
    encodeDecodeLayout->addWidget(decodeGroupBox, 1);
    mainLayout->addLayout(encodeDecodeLayout);
    
    // ==================== 哈夫曼树可视化部分 ====================
    initHuffmanVisualization();
    
    // 布局完成
    mainLayout->addStretch(1);
}

void TextEncodeDecodeWindow::initHuffmanVisualization() {
    // 创建哈夫曼树可视化区域
    QGroupBox *huffmanGroupBox = new QGroupBox("哈夫曼树可视化");
    QVBoxLayout *huffmanLayout = new QVBoxLayout(huffmanGroupBox);
    
    // 创建图形视图和场景
    m_huffmanView = new ZoomView(this);
    m_huffmanScene = new QGraphicsScene(this);
    m_huffmanScene->setSceneRect(-1000, -1000, 2000, 2000);
    m_huffmanView->setScene(m_huffmanScene);
    
    // 创建控制按钮
    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_stepBtn = new QPushButton("Step");
    m_playBtn = new QPushButton("Play");
    m_resetBtn = new QPushButton("Reset");
    m_huffmanTimer = new QTimer(this);
    
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(10, 200);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setToolTip("缩放");
    
    controlLayout->addWidget(m_stepBtn);
    controlLayout->addWidget(m_playBtn);
    controlLayout->addWidget(m_resetBtn);
    controlLayout->addStretch(1);
    controlLayout->addWidget(new QLabel("缩放："));
    controlLayout->addWidget(m_zoomSlider);
    
    // 连接信号槽
    connect(m_stepBtn, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onHuffmanStep);
    connect(m_playBtn, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onHuffmanPlayPause);
    connect(m_resetBtn, &QPushButton::clicked, this, &TextEncodeDecodeWindow::onHuffmanReset);
    connect(m_huffmanTimer, &QTimer::timeout, this, &TextEncodeDecodeWindow::autoplayHuffmanStep);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &TextEncodeDecodeWindow::onHuffmanZoomChanged);
    
    // 将组件添加到布局
    huffmanLayout->addLayout(controlLayout);
    huffmanLayout->addWidget(m_huffmanView);
    
    // 将哈夫曼树可视化区域添加到主布局
    QVBoxLayout *mainLayout = static_cast<QVBoxLayout*>(centralWidget()->layout());
    mainLayout->addWidget(huffmanGroupBox);
}

void TextEncodeDecodeWindow::onEncodeTextClicked() {
    QString text = textInputEdit->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要编码的文本！");
        return;
    }
    
    // 统计字符频率
    m_characterFrequency.clear();
    for (QChar c : text) {
        m_characterFrequency[c]++;
    }
    
    // 调用后端API进行编码
    std::string encodedData = backend_api::encodeTextUtf8(text.toStdString());
if (!encodedData.empty()) {
    encodeResultEdit->setPlainText(QString::fromStdString(encodedData));
    currentEncodedText = encodedData;
    
    // 计算压缩率
    double originalSize = text.toUtf8().size();
    
    // 分离编码表和实际编码数据（查找最后一个|字符作为分隔符）
    size_t separatorPos = encodedData.rfind('|');
    double encodedSize = encodedData.size();
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

    QString resultText = QString("文本编码成功！") +
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

    QMessageBox::information(this, "编码结果", resultText);
    
    // 生成哈夫曼树可视化
    prepareHuffmanNodes();
} else {
    QMessageBox::warning(this, "警告", "编码失败！");
}
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
        QMessageBox::warning(this, "警告", "请先选择要编码的文本文件！");
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "警告", "无法打开文件！");
        return;
    }
    
    // 读取整个文件
    QByteArray content = file.readAll();
    file.close();
    
    // 将文件内容转换为字符串
    QString text = QString::fromUtf8(content);
    
    // 统计字符频率
    m_characterFrequency.clear();
    for (QChar c : text) {
        m_characterFrequency[c]++;
    }
    
    // 调用后端API进行编码
    std::string encodedData = backend_api::encodeTextUtf8(text.toStdString());
if (!encodedData.empty()) {
    encodeResultEdit->setPlainText(QString::fromStdString(encodedData));
    currentEncodedText = encodedData;
    
    // 计算压缩率
    double originalSize = content.size();
    
    // 分离编码表和实际编码数据（查找最后一个|字符作为分隔符）
    size_t separatorPos = encodedData.rfind('|');
    double encodedSize = encodedData.size();
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

    QString resultText = QString("文本文件编码成功！") +
                     QString("文件路径：") + filePath + "\n" +
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

    QMessageBox::information(this, "编码结果", resultText);
    
    // 生成哈夫曼树可视化
    prepareHuffmanNodes();
} else {
    QMessageBox::warning(this, "警告", "编码失败！");
}
}

void TextEncodeDecodeWindow::onExportTextHufClicked() {
    if (currentEncodedText.empty()) {
        QMessageBox::warning(this, "警告", "没有可导出的编码结果！");
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出为.huf文件", "", "Huffman Encoded Files (*.huf);;All Files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "警告", "无法创建文件！");
        return;
    }
    
    // 写入编码结果（实际应用中需要包含解码信息）
    file.write(currentEncodedText.c_str(), currentEncodedText.size());
    file.close();
    
    QMessageBox::information(this, "提示", "导出成功！");
}

void TextEncodeDecodeWindow::onBrowseEncodedFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择.huf文件", "", "Huffman Encoded Files (*.huf);;All Files (*.*)");
    if (!filePath.isEmpty()) {
        encodedFilePathEdit->setText(filePath);
    }
}

void TextEncodeDecodeWindow::onDecodeTextClicked() {
    QString filePath = encodedFilePathEdit->text();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要解码的.huf文件！");
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "警告", "无法打开文件！");
        return;
    }
    
    QByteArray content = file.readAll();
    file.close();
    
    // 调用后端API进行解码
    std::string decodedData = backend_api::decodeTextUtf8(std::string(content.constData(), content.size()));
if (!decodedData.empty()) {
    decodeResultEdit->setPlainText(QString::fromStdString(decodedData));
} else {
    QMessageBox::warning(this, "警告", "解码失败！");
}
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

// ==================== 哈夫曼树可视化相关函数 ====================

void TextEncodeDecodeWindow::prepareHuffmanNodes()
{
    // 停止当前播放
    m_huffmanTimer->stop();
    m_playBtn->setText(tr("Play"));
    
    // 清理场景
    m_huffmanScene->clear();
    m_huffmanItemMap.clear();
    m_huffmanEdgeItems.clear();
    m_activeAnims.clear();
    
    // 转换字符频率为哈夫曼树所需的格式
    m_huffmanItems.clear();
    for (auto it = m_characterFrequency.constBegin(); it != m_characterFrequency.constEnd(); ++it) {
        QString label;
        if (it.key() == '\n') {
            label = "\\n";
        } else if (it.key() == '\t') {
            label = "\\t";
        } else if (it.key() == ' ') {
            label = "(空格)";
        } else {
            label = it.key();
        }
        m_huffmanItems.append(qMakePair(label, it.value()));
    }
    
    // 如果没有字符，使用默认示例数据
    if (m_huffmanItems.isEmpty()) {
        m_huffmanItems = {{"A",45},{"B",13},{"C",12},{"D",16},{"E",9},{"F",5}};
    }
    
    // 创建初始节点
    for (int i = 0; i < m_huffmanItems.size(); ++i) {
        QString label = m_huffmanItems[i].first;
        int weight = m_huffmanItems[i].second;
        
        QGraphicsItemGroup *group = new QGraphicsItemGroup();
        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(-30, -30, 60, 60);
        ellipse->setBrush(QBrush(QColor(100, 150, 240)));
        ellipse->setPen(QPen(Qt::black));
        
        QGraphicsTextItem *labelText = new QGraphicsTextItem(label);
        labelText->setDefaultTextColor(Qt::white);
        labelText->setPos(-10, -10);
        
        QGraphicsTextItem *weightText = new QGraphicsTextItem(QString::number(weight));
        weightText->setDefaultTextColor(Qt::black);
        weightText->setPos(-10, 10);
        
        group->addToGroup(ellipse);
        group->addToGroup(labelText);
        group->addToGroup(weightText);
        m_huffmanScene->addItem(group);
        
        group->setData(0, i);
        group->setToolTip(QString("%1 : %2").arg(label).arg(weight));
        m_huffmanItemMap.insert(i, group);
    }
    
    // 构建哈夫曼树
    HuffmanBuilder builder(m_huffmanItems);
    m_huffmanSteps = builder.buildSteps();
    m_currentHuffmanStep = 0;
    
    // 初始布局
    layoutAndAnimateHuffman(0);
}

void TextEncodeDecodeWindow::onHuffmanStep()
{
    if (m_currentHuffmanStep >= m_huffmanSteps.size()) return;
    performHuffmanStep(m_huffmanSteps[m_currentHuffmanStep]);
    m_currentHuffmanStep++;
}

void TextEncodeDecodeWindow::onHuffmanPlayPause()
{
    if (m_huffmanTimer->isActive()) {
        m_huffmanTimer->stop();
        m_playBtn->setText(tr("Play"));
    } else {
        if (m_currentHuffmanStep >= m_huffmanSteps.size()) {
            m_currentHuffmanStep = 0;
            prepareHuffmanNodes();
        }
        m_huffmanTimer->start(1000);
        m_playBtn->setText(tr("Pause"));
    }
}

void TextEncodeDecodeWindow::onHuffmanReset()
{
    m_huffmanTimer->stop();
    m_playBtn->setText(tr("Play"));
    m_currentHuffmanStep = 0;
    prepareHuffmanNodes();
}

void TextEncodeDecodeWindow::autoplayHuffmanStep()
{
    if (m_currentHuffmanStep >= m_huffmanSteps.size()) {
        m_huffmanTimer->stop();
        m_playBtn->setText(tr("Play"));
        return;
    }
    performHuffmanStep(m_huffmanSteps[m_currentHuffmanStep]);
    m_currentHuffmanStep++;
}

void TextEncodeDecodeWindow::onHuffmanZoomChanged(int value)
{
    if (!m_huffmanView) return;
    double factor = double(value) / 100.0;
    m_huffmanView->resetTransform();
    m_huffmanView->scale(factor, factor);
}

void TextEncodeDecodeWindow::performHuffmanStep(const HuffStep &step)
{
    // 找到对应的图形项
    int idA = step.a->id;
    int idB = step.b->id;
    QGraphicsItem *itemA = m_huffmanItemMap.value(idA, nullptr);
    QGraphicsItem *itemB = m_huffmanItemMap.value(idB, nullptr);
    
    // 高亮选中的节点
    if (itemA) {
        auto group = static_cast<QGraphicsItemGroup*>(itemA);
        foreach(QGraphicsItem *child, group->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(240, 120, 100)));
            }
        }
    }
    
    if (itemB) {
        auto group = static_cast<QGraphicsItemGroup*>(itemB);
        foreach(QGraphicsItem *child, group->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(240, 120, 100)));
            }
        }
    }
    
    // 创建父节点
    int parentId = step.parent->id;
    QGraphicsItemGroup *parentGroup = new QGraphicsItemGroup();
    QGraphicsEllipseItem *parentEllipse = new QGraphicsEllipseItem(-30, -30, 60, 60);
    
    QLinearGradient gradient(-30, -30, 30, 30);
    gradient.setColorAt(0, QColor(140, 220, 180));
    gradient.setColorAt(1, QColor(80, 180, 120));
    parentEllipse->setBrush(gradient);
    parentEllipse->setPen(QPen(Qt::black));
    
    QGraphicsTextItem *parentWeightText = new QGraphicsTextItem(QString::number(step.parent->weight));
    parentWeightText->setDefaultTextColor(Qt::black);
    parentWeightText->setPos(-10, 0);
    
    parentGroup->addToGroup(parentEllipse);
    parentGroup->addToGroup(parentWeightText);
    m_huffmanScene->addItem(parentGroup);
    
    parentGroup->setData(0, parentId);
    parentGroup->setToolTip(QString("Weight: %1").arg(step.parent->weight));
    m_huffmanItemMap.insert(parentId, parentGroup);
    
    // 变暗已合并的节点
    if (itemA) {
        auto group = static_cast<QGraphicsItemGroup*>(itemA);
        foreach(QGraphicsItem *child, group->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(180, 180, 180)));
            }
        }
    }
    
    if (itemB) {
        auto group = static_cast<QGraphicsItemGroup*>(itemB);
        foreach(QGraphicsItem *child, group->childItems()) {
            if (auto e = qgraphicsitem_cast<QGraphicsEllipseItem*>(child)) {
                e->setBrush(QBrush(QColor(180, 180, 180)));
            }
        }
    }
    
    // 更新布局
    layoutAndAnimateHuffman(m_currentHuffmanStep + 1);
}

// 修改layoutAndAnimateHuffman函数，使用QParallelAnimationGroup并在动画完成后绘制边
void TextEncodeDecodeWindow::layoutAndAnimateHuffman(int stepsDone)
{
    // 构建轻量级布局节点
    struct LNode { 
        int id; 
        int weight; 
        LNode *left{nullptr}, *right{nullptr}; 
        double x{0}, y{0}, width{0}; 
    };
    
    QVector<LNode*> forest;
    QMap<int, LNode*> nodeMap;
    
    // 创建初始叶子节点
    for (int i = 0; i < m_huffmanItems.size(); ++i) {
        LNode *ln = new LNode();
        ln->id = i;
        ln->weight = m_huffmanItems[i].second;
        ln->width = 60;
        nodeMap.insert(i, ln);
        forest.append(ln);
    }
    
    // 应用合并步骤
    for (int i = 0; i < stepsDone && i < m_huffmanSteps.size(); ++i) {
        const HuffStep &s = m_huffmanSteps[i];
        int aId = s.a->id;
        int bId = s.b->id;
        int pId = s.parent->id;
        
        LNode *a = nodeMap.value(aId, nullptr);
        LNode *b = nodeMap.value(bId, nullptr);
        LNode *p = new LNode();
        p->id = pId;
        p->weight = s.parent->weight;
        p->left = a;
        p->right = b;
        
        // 从森林中移除a和b
        for (int j = 0; j < forest.size(); ++j) {
            if (forest[j] == a || forest[j] == b) {
                forest.removeAt(j);
                --j;
            }
        }
        
        forest.append(p);
        nodeMap.insert(pId, p);
    }
    
    // 计算子树宽度
    std::function<double(LNode*)> computeWidth = [&](LNode* n)->double {
        if (!n) return 0;
        if (!n->left && !n->right) {
            n->width = 60;
            return n->width;
        }
        double leftW = computeWidth(n->left);
        double rightW = computeWidth(n->right);
        double spacing = 20;
        n->width = leftW + rightW + spacing;
        return n->width;
    };
    
    // 计算深度
    std::function<int(LNode*)> computeDepth = [&](LNode* n)->int {
        if (!n) return 0;
        if (!n->left && !n->right) return 0;
        return 1 + qMax(computeDepth(n->left), computeDepth(n->right));
    };
    
    // 分配位置，调整布局方向：根节点在上，叶子节点在下
    std::function<void(LNode*, double, int, double)> assignPos = [&](LNode* n, double left, int depth, double levelHeight) {
        if (!n) return;
        if (!n->left && !n->right) {
            n->x = left + n->width / 2;
            n->y = 100 + depth * levelHeight;  // 调整y坐标计算方式
            return;
        }
        
        double spacing = 20;
        double leftStart = left;
        assignPos(n->left, leftStart, depth + 1, levelHeight);
        assignPos(n->right, leftStart + (n->left ? n->left->width : 0) + spacing, depth + 1, levelHeight);
        
        // 父节点位置在子节点中间
        double lx = n->left ? n->left->x : 0;
        double rx = n->right ? n->right->x : lx;
        n->x = (lx + rx) / 2.0;
        n->y = 100 + depth * levelHeight;  // 调整y坐标计算方式
    };
    
    // 计算宽度
    for (LNode* r : forest) {
        computeWidth(r);
    }
    
    // 分配位置
    double totalWidth = 0;
    for (LNode* r : forest) {
        totalWidth += r->width + 20;
    }
    if (totalWidth < 400) totalWidth = 400;
    
    // 计算场景中心和起始位置，使哈夫曼树居中显示
    double sceneCenterX = 0;  // 场景的实际中心坐标是0，因为sceneRect设置为(-1000, -1000, 2000, 2000)
    double left = sceneCenterX - totalWidth / 2;
    double levelHeight = 100.0;
    for (LNode* r : forest) {
        assignPos(r, left, 0, levelHeight);
        left += r->width + 20;
    }
    
    // 动画
    QVector<QAbstractAnimation*> anims;
    int maxWeight = 1;
    for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it) {
        if (it.value()) {
            maxWeight = qMax(maxWeight, it.value()->weight);
        }
    }
    
    // 使用QParallelAnimationGroup管理动画
    QParallelAnimationGroup *parallelGroup = new QParallelAnimationGroup();
    
    for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it) {
        int id = it.key();
        LNode* ln = it.value();
        QGraphicsItem *g = m_huffmanItemMap.value(id, nullptr);
        if (!g || !ln) continue;
        
        // 创建位置动画
        QVariantAnimation *anim = new QVariantAnimation();
        anim->setDuration(500);
        anim->setStartValue(g->pos());
        anim->setEndValue(QPointF(ln->x - g->boundingRect().center().x(), ln->y - g->boundingRect().center().y()));
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &QVariantAnimation::valueChanged, [g](const QVariant& value) {
            g->setPos(value.toPointF());
        });
        parallelGroup->addAnimation(anim);
    }
    
    // 在动画完成后绘制边
    connect(parallelGroup, &QParallelAnimationGroup::finished, this, [this, parallelGroup, stepsDone]() {
        // 更新边
        clearHuffmanEdges();
        drawHuffmanEdges(stepsDone);
        
        // 清理动画组
        parallelGroup->deleteLater();
    });
    
    // 启动动画
    parallelGroup->start();
    
    // 清理临时节点
    for (LNode* r : forest) {
        delete r; // 递归删除所有节点
    }
    
    // 更新动画列表
    m_activeAnims.clear();
    m_activeAnims.append(parallelGroup);
}

void TextEncodeDecodeWindow::clearHuffmanEdges()
{
    for (QGraphicsItem* item : m_huffmanEdgeItems) {
        m_huffmanScene->removeItem(item);
        delete item;
    }
    m_huffmanEdgeItems.clear();
}

void TextEncodeDecodeWindow::drawHuffmanEdges(int stepsDone)
{
    // 清除现有的边
    for (QGraphicsItem* edge : m_huffmanEdgeItems) {
        m_huffmanScene->removeItem(edge);
        delete edge;
    }
    m_huffmanEdgeItems.clear();

    // 绘制当前步骤的边
    for (int i = 0; i < stepsDone && i < m_huffmanSteps.size(); ++i) {
        const HuffStep& step = m_huffmanSteps[i];
        int parentId = step.parent->id;
        int leftId = step.a->id;
        int rightId = step.b->id;

        QGraphicsItem* parentItem = m_huffmanItemMap.value(parentId);
        QGraphicsItem* leftItem = m_huffmanItemMap.value(leftId);
        QGraphicsItem* rightItem = m_huffmanItemMap.value(rightId);

        if (parentItem && leftItem) {
            QPointF parentPos = parentItem->pos();
            QPointF leftPos = leftItem->pos();

            // 计算贝塞尔曲线的控制点
            QPointF controlParent = parentPos + QPointF(-30, 40);
            QPointF controlLeft = leftPos + QPointF(30, -40);

            // 创建贝塞尔曲线路径
            QPainterPath path(parentPos);
            path.cubicTo(controlParent, controlLeft, leftPos);

            // 绘制曲线
            QGraphicsPathItem* edgeItem = new QGraphicsPathItem(path);
            QPen pen(Qt::black, 2);
            edgeItem->setPen(pen);
            edgeItem->setZValue(-1);
            m_huffmanScene->addItem(edgeItem);
            m_huffmanEdgeItems.append(edgeItem);

            // 绘制箭头
            QPointF dir = (leftPos - controlLeft);
            double length = std::hypot(dir.x(), dir.y());
            if (length > 0.1) {
                // 手动归一化向量
                QPointF normalizedDir = dir / length;
                
                QPointF arrowPoint1 = leftPos - normalizedDir * 12 - QPointF(-normalizedDir.y() * 6, normalizedDir.x() * 6);
                QPointF arrowPoint2 = leftPos - normalizedDir * 12 - QPointF(normalizedDir.y() * 6, -normalizedDir.x() * 6);

                QPolygonF arrowHead;
                arrowHead << leftPos << arrowPoint1 << arrowPoint2;

                QGraphicsPolygonItem* arrowItem = new QGraphicsPolygonItem(arrowHead);
                arrowItem->setBrush(Qt::black);
                arrowItem->setZValue(-1);
                m_huffmanScene->addItem(arrowItem);
                m_huffmanEdgeItems.append(arrowItem);
            }

            // 绘制0/1标签
            QPointF midPoint = (controlParent + controlLeft) / 2;
            QGraphicsTextItem* labelItem = new QGraphicsTextItem("0");
            labelItem->setDefaultTextColor(Qt::red);
            labelItem->setPos(midPoint.x() - 5, midPoint.y() - 10);
            m_huffmanScene->addItem(labelItem);
            m_huffmanEdgeItems.append(labelItem);
        }

        if (parentItem && rightItem) {
            QPointF parentPos = parentItem->pos();
            QPointF rightPos = rightItem->pos();

            // 计算贝塞尔曲线的控制点
            QPointF controlParent = parentPos + QPointF(30, 40);
            QPointF controlRight = rightPos + QPointF(-30, -40);

            // 创建贝塞尔曲线路径
            QPainterPath path(parentPos);
            path.cubicTo(controlParent, controlRight, rightPos);

            // 绘制曲线
            QGraphicsPathItem* edgeItem = new QGraphicsPathItem(path);
            QPen pen(Qt::black, 2);
            edgeItem->setPen(pen);
            edgeItem->setZValue(-1);
            m_huffmanScene->addItem(edgeItem);
            m_huffmanEdgeItems.append(edgeItem);

            // 绘制箭头
            QPointF dir = (rightPos - controlRight);
            double length = std::hypot(dir.x(), dir.y());
            if (length > 0.1) {
                // 手动归一化向量
                QPointF normalizedDir = dir / length;
                
                QPointF arrowPoint1 = rightPos - normalizedDir * 12 - QPointF(-normalizedDir.y() * 6, normalizedDir.x() * 6);
                QPointF arrowPoint2 = rightPos - normalizedDir * 12 - QPointF(normalizedDir.y() * 6, -normalizedDir.x() * 6);

                QPolygonF arrowHead;
                arrowHead << rightPos << arrowPoint1 << arrowPoint2;

                QGraphicsPolygonItem* arrowItem = new QGraphicsPolygonItem(arrowHead);
                arrowItem->setBrush(Qt::black);
                arrowItem->setZValue(-1);
                m_huffmanScene->addItem(arrowItem);
                m_huffmanEdgeItems.append(arrowItem);
            }

            // 绘制0/1标签
            QPointF midPoint = (controlParent + controlRight) / 2;
            QGraphicsTextItem* labelItem = new QGraphicsTextItem("1");
            labelItem->setDefaultTextColor(Qt::red);
            labelItem->setPos(midPoint.x() - 5, midPoint.y() - 10);
            m_huffmanScene->addItem(labelItem);
            m_huffmanEdgeItems.append(labelItem);
        }
    }
}