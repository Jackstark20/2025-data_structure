#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QTableView>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QTimer>
#include <QFile>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QFont>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QProcess>
#include <QDir>
#include <QSortFilterProxyModel>
#include <QTextStream>
#include <QSlider>
#include <unordered_map>
#include <vector>
#include <algorithm>

// 使用 Qt Charts 的类在全局命名空间可直接使用 QChart 等类型

using StringFreqMap = std::unordered_map<std::string, size_t>;

class StreamFreqWindow : public QMainWindow {
    Q_OBJECT
public:
    StreamFreqWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("流式字符串频率统计 (STL unordered_map)");
        resize(900, 600);

        QWidget *central = new QWidget(this);
        setCentralWidget(central);
        auto *mainLay = new QVBoxLayout(central);
        mainLay->setContentsMargins(12,12,12,12);

        // top controls
        auto *topLay = new QHBoxLayout();
        pathEdit = new QLineEdit();
        pathEdit->setPlaceholderText("选择要统计的文本文件...");
        QPushButton *btnBrowse = new QPushButton("浏览");
        QPushButton *btnStart = new QPushButton("开始");
        QPushButton *btnStop = new QPushButton("停止");
        btnStop->setEnabled(false);
        topLay->addWidget(pathEdit);
        topLay->addWidget(btnBrowse);
        topLay->addWidget(btnStart);
        topLay->addWidget(btnStop);
        mainLay->addLayout(topLay);

        // speed control slider
        auto *speedLay = new QHBoxLayout();
        QLabel *speedLabel = new QLabel("速度：");
        speedSlider = new QSlider(Qt::Horizontal);
        speedSlider->setRange(1,100);
        speedSlider->setValue(10);
        speedValueLabel = new QLabel("50");
        speedLay->addWidget(speedLabel);
        speedLay->addWidget(speedSlider, 1);
        speedLay->addWidget(speedValueLabel);
        mainLay->addLayout(speedLay);

        // status line
        statusLabel = new QLabel("就绪");
        statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        mainLay->addWidget(statusLabel);

        // split area: left table+chart, right sample preview
        auto *splitLay = new QHBoxLayout();

        // left area contains table on top and chart below
        auto *leftLay = new QVBoxLayout();

        // table view
        tableModel = new QStandardItemModel(0,2,this);
        tableModel->setHeaderData(0, Qt::Horizontal, "字符串");
        tableModel->setHeaderData(1, Qt::Horizontal, "出现次数");
        tableView = new QTableView();
        tableView->setModel(tableModel);
        tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        leftLay->addWidget(tableView, 1);

        // chart area (Qt Charts)
        chart = new QChart();
        chart->setTitle("Top 10 词频分布");
        // enable animations for smooth transitions
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setAnimationDuration(animationDurationMs);
        chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        leftLay->addWidget(chartView, 1);

        splitLay->addLayout(leftLay, 3);

        // right: simple preview and progress
        auto *rightLay = new QVBoxLayout();
        QLabel *previewLabel = new QLabel("最近读取文本片段：");
        previewEdit = new QPlainTextEdit();
        previewEdit->setReadOnly(true);
        // allow larger previews (remove block limit)
        previewEdit->setMaximumBlockCount(0);
        progress = new QProgressBar();
        progress->setRange(0,100);
        progress->setValue(0);
        rightLay->addWidget(previewLabel);
        rightLay->addWidget(previewEdit);
        rightLay->addWidget(progress);
        splitLay->addLayout(rightLay,2);

        mainLay->addLayout(splitLay);

        // connections
        connect(btnBrowse, &QPushButton::clicked, this, &StreamFreqWindow::onBrowse);
        connect(btnStart, &QPushButton::clicked, this, &StreamFreqWindow::onStart);
        connect(btnStop, &QPushButton::clicked, this, &StreamFreqWindow::onStop);
        connect(speedSlider, &QSlider::valueChanged, this, &StreamFreqWindow::onSpeedChanged);

        // timer: use single-shot reads so we can pause between small batches
        readTimer = new QTimer(this);
        readTimer->setSingleShot(true);
        connect(readTimer, &QTimer::timeout, this, &StreamFreqWindow::readChunk);
        pauseTimer = new QTimer(this);
        pauseTimer->setSingleShot(true);
        connect(pauseTimer, &QTimer::timeout, this, &StreamFreqWindow::resumeRead);

        // update UI timer
        uiTimer = new QTimer(this);
        uiTimer->setInterval(500); // 500ms 更新展示（更频繁，更流畅）
        connect(uiTimer, &QTimer::timeout, this, &StreamFreqWindow::updateTable);

        // defaults
        chunkSize = 4096;
        totalBytes = 0;
        fileSize = 0;

        uiTimer->start();
        // apply initial slider setting
        onSpeedChanged(speedSlider->value());
    }

private slots:
    void onBrowse() {
        QString fn = QFileDialog::getOpenFileName(this, "选择文本文件", "", "Text Files (*.txt);;All Files (*.*)");
        if (!fn.isEmpty()) pathEdit->setText(fn);
    }

    void resumeRead() {
        if (file.isOpen()) readChunk();
    }

    void onSpeedChanged(int v) {
        // show value
        if (speedValueLabel) speedValueLabel->setText(QString::number(v));
        // map slider value to speed factor (50 -> 1.0)
        qreal factor = v / 50.0;
        // adjust read batch word count
        readWordsPerStep = std::max(1, (int)qRound(30 * factor));
        // shorten pause as factor increases
        pauseDurationMs = std::clamp((int)qRound(300 / factor), 50, 2000);
        // adjust animation duration (faster when factor larger)
        animationDurationMs = std::clamp((int)qRound(900 / factor), 100, 2000);
        if (chart) chart->setAnimationDuration(animationDurationMs);
        // ui refresh interval
        // ensure UI/chart update interval is not faster than 1s
        int uiInterval = std::max(1000, (int)qRound(2000 / factor));
        if (uiTimer) uiTimer->setInterval(uiInterval);
    }

    void onStart() {
        QString fn = pathEdit->text();
        if (fn.isEmpty()) { QMessageBox::warning(this, "警告", "请选择文件"); return; }
        file.setFileName(fn);
        if (!file.open(QIODevice::ReadOnly)) { QMessageBox::warning(this, "警告", "无法打开文件"); return; }
        if (textStream) { delete textStream; textStream = nullptr; }
        textStream = new QTextStream(&file);
        totalBytes = 0;
        fileSize = file.size();
        leftover.clear();
        freqMap.clear();
        // clear previous preview when starting a new run
        initialPreview.clear();
        previewEdit->clear();
        statusLabel->setText("读取中...");
        progress->setValue(0);
        readChunk();
        uiTimer->start();
        // enable/disable buttons (using findChildren below)
        // simple enable/disable via find
        auto btns = findChildren<QPushButton*>();
        for (QPushButton *b : btns) {
            if (b->text() == "开始") b->setEnabled(false);
            if (b->text() == "停止") b->setEnabled(true);
        }
    }

    void onStop() {
        readTimer->stop();
        file.close();
        if (textStream) { delete textStream; textStream = nullptr; }
        statusLabel->setText("已停止");
        auto btns = findChildren<QPushButton*>();
        for (QPushButton *b : btns) {
            if (b->text() == "开始") b->setEnabled(true);
            if (b->text() == "停止") b->setEnabled(false);
        }
    }

    void readChunk() {
        if (!file.isOpen()) { readTimer->stop(); return; }
        if (!textStream) textStream = new QTextStream(&file);
        // accumulate until we have enough words
        QString acc;
        QRegularExpression reTokens("(\\p{Han}+)|(\\w+)");
        int tokenCount = 0;
        while (tokenCount < readWordsPerStep) {
            QString piece = textStream->read((qint64)readBatchCharSize);
            if (piece.isNull() || piece.isEmpty()) break;
            acc += piece;
            // count tokens in acc
            auto itc = reTokens.globalMatch(acc);
            tokenCount = 0;
            while (itc.hasNext()) { itc.next(); tokenCount++; if (tokenCount >= readWordsPerStep) break; }
            if (tokenCount >= readWordsPerStep) break;
            // if piece smaller than batch, probably EOF soon - continue loop to break
        }
        QString chunk = acc;
        if (chunk.isNull() || chunk.isEmpty()) {
            // finished - process any leftover characters before closing
            QString proc = leftover;
            leftover.clear();
            if (!proc.isEmpty()) {
                QString text = proc;
                // preserve beginning preview (keep up to previewMaxChars)
                if (initialPreview.size() < previewMaxChars) {
                    int need = previewMaxChars - initialPreview.size();
                    initialPreview += text.left(need);
                    previewEdit->setPlainText(initialPreview);
                }
                QRegularExpression re("(\\p{Han}+)|(\\w+)");
                auto it = re.globalMatch(text);
                while (it.hasNext()) {
                    QRegularExpressionMatch m = it.next();
                    QString token;
                    if (!m.captured(1).isEmpty()) {
                        QString chineseSeq = m.captured(1);
                        QStringList segs = segmentChinese(chineseSeq);
                        for (const QString &s : segs) {
                            if (s.isEmpty()) continue;
                            QByteArray ba = s.toUtf8();
                            std::string key(ba.constData(), (size_t)ba.size());
                            freqMap[key]++;
                        }
                        continue;
                    } else if (!m.captured(2).isEmpty()) {
                        token = m.captured(2);
                    }
                    if (token.isEmpty()) continue;
                    QByteArray ba = token.toUtf8();
                    std::string key(ba.constData(), (size_t)ba.size());
                    freqMap[key]++;
                }
            }
            // finished
            readTimer->stop();
            file.close();
            if (textStream) { delete textStream; textStream = nullptr; }
            statusLabel->setText("读取完成");
            progress->setValue(100);
            updateTable();
            return;
        }

        // account for bytes read for progress (approximate)
        totalBytes += chunk.toUtf8().size();
        if (fileSize>0) progress->setValue((int)(100.0 * totalBytes / fileSize));

        // Append to initial preview from raw chunk so right preview updates quickly
        if (initialPreview.size() < previewMaxChars) {
            int need = previewMaxChars - initialPreview.size();
            if (need > 0 && !chunk.isEmpty()) {
                initialPreview += chunk.left(need);
                previewEdit->setPlainText(initialPreview);
            }
        }

        // append chunk to leftover (QString) and process up to last ASCII punctuation/space boundary
        leftover.append(chunk);
        int lastSep = -1;
        for (int i = leftover.size()-1; i >= 0 && i >= leftover.size()-64; --i) {
            QChar c = leftover.at(i);
            if (c.isSpace() || c == ',' || c == '.' || c == ';' || c == ':' || c == '!' || c == '?' || c == '(' || c == ')' || c == '"' || c == '\'') { lastSep = i; break; }
        }
        QString proc;
        if (lastSep>=0) { proc = leftover.left(lastSep+1); leftover = leftover.mid(lastSep+1); }
        else if (leftover.size() > 65536) { // too large, process all to avoid memory blow
            proc = leftover;
            leftover.clear();
        } else {
            proc.clear();
        }

        if (!proc.isEmpty()) {
            QString text = proc;
            if (initialPreview.size() < previewMaxChars) {
                int need = previewMaxChars - initialPreview.size();
                initialPreview += text.left(need);
                previewEdit->setPlainText(initialPreview);
            }
            QRegularExpression re("(\\p{Han}+)|(\\w+)");
            auto it = re.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                QString token;
                if (!m.captured(1).isEmpty()) {
                    QString chineseSeq = m.captured(1);
                    QStringList segs = segmentChinese(chineseSeq);
                    for (const QString &s : segs) {
                        if (s.isEmpty()) continue;
                        QByteArray ba = s.toUtf8();
                        std::string key(ba.constData(), (size_t)ba.size());
                        freqMap[key]++;
                    }
                    continue;
                } else if (!m.captured(2).isEmpty()) {
                    token = m.captured(2);
                }
                if (token.isEmpty()) continue;
                QByteArray ba = token.toUtf8();
                std::string key(ba.constData(), (size_t)ba.size());
                freqMap[key]++;
            }
        }

        // update display after this small batch
        updateTable();

        // pause reading for a bit before resuming
        pauseTimer->start(pauseDurationMs);
    }

    // Try to segment Chinese text using external jieba_segment.py (if python + jieba available).
    // Falls back to single-character tokens when helper is not available.
    QStringList segmentChinese(const QString &text) {
        QStringList out;
        // helper path next to executable
        QString helper = QCoreApplication::applicationDirPath() + QDir::separator() + "jieba_segment.py";
        if (QFile::exists(helper)) {
            QProcess p;
            QString python = "python"; // assume python in PATH
            QStringList args;
            args << helper;
            p.start(python, args);
            if (!p.waitForStarted(3000)) return fallbackCharSplit(text);
            QByteArray in = text.toUtf8();
            p.write(in);
            p.closeWriteChannel();
            if (!p.waitForFinished(5000)) return fallbackCharSplit(text);
            if (p.exitCode() != 0) return fallbackCharSplit(text);
            QByteArray outBytes = p.readAllStandardOutput();
            QString outStr = QString::fromUtf8(outBytes).trimmed();
            if (outStr.isEmpty()) return fallbackCharSplit(text);
            for (const QString &tok : outStr.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)) {
                // skip pure-punctuation tokens (中文标点/其他标点)
                bool allPunct = true;
                for (QChar c : tok) {
                    QChar::Category cat = c.category();
                    if (!(cat == QChar::Punctuation_Connector || cat == QChar::Punctuation_Dash ||
                          cat == QChar::Punctuation_Open || cat == QChar::Punctuation_Close ||
                          cat == QChar::Punctuation_InitialQuote || cat == QChar::Punctuation_FinalQuote ||
                          cat == QChar::Punctuation_Other)) { allPunct = false; break; }
                }
                if (tok.isEmpty() || allPunct) continue;
                out << tok;
            }
            return out;
        }
        return fallbackCharSplit(text);
    }

    QStringList fallbackCharSplit(const QString &text) {
        QStringList lst;
        for (QChar c : text) {
            if (c.isSpace()) continue;
            QChar::Category cat = c.category();
            bool isPunct = (cat == QChar::Punctuation_Connector || cat == QChar::Punctuation_Dash ||
                            cat == QChar::Punctuation_Open || cat == QChar::Punctuation_Close ||
                            cat == QChar::Punctuation_InitialQuote || cat == QChar::Punctuation_FinalQuote ||
                            cat == QChar::Punctuation_Other);
            if (isPunct) continue;
            lst << QString(c);
        }
        return lst;
    }

    void updateTable() {
        // produce vector of pairs and sort by freq
        std::vector<std::pair<std::string,size_t>> vec;
        vec.reserve(freqMap.size());
        for (auto &p : freqMap) vec.emplace_back(p.first, p.second);
        std::sort(vec.begin(), vec.end(), [](auto &a, auto &b){ return a.second > b.second; });

        // show top 200
        int showN = std::min((size_t)200, vec.size());
        tableModel->removeRows(0, tableModel->rowCount());
        tableModel->setRowCount(showN);
        for (int i=0;i<showN;i++) {
            const auto &p = vec[i];
            QString s = QString::fromUtf8(p.first.data(), (int)p.first.size());
            tableModel->setItem(i, 0, new QStandardItem(s));
            tableModel->setItem(i, 1, new QStandardItem(QString::number((long long)p.second)));
        }

        // Update chart with top N (chartTopN)
        int topN = std::min((size_t)chartTopN, vec.size());
        if (topN > 0) {
            QStringList categories;
            QVector<qreal> values;
            values.reserve(topN);
            for (int i=0;i<topN;i++) {
                const auto &p = vec[i];
                values.push_back((qreal)p.second);
                categories << QString::fromUtf8(p.first.data(), (int)p.first.size());
            }

            // avoid redundant chart updates when nothing changed
            bool same = (prevCategories == categories && prevValues == values);
            if (!same) {
                // Create live series first time
                if (!liveSeries) {
                    liveSeries = new QBarSeries();
                    liveSet = new QBarSet("频次");
                    for (qreal v : values) *liveSet << v;
                    liveSeries->append(liveSet);
                    chart->addSeries(liveSeries);
                    // remove old axes
                    auto oldAxes = chart->axes();
                    for (QAbstractAxis *a : oldAxes) {
                        chart->removeAxis(a);
                        delete a;
                    }
                    axisX = new QBarCategoryAxis();
                    axisX->append(categories);
                    chart->addAxis(axisX, Qt::AlignBottom);
                    liveSeries->attachAxis(axisX);
                    axisY = new QValueAxis();
                    axisY->setLabelFormat("%d");
                    axisY->setTitleText("频次");
                    chart->addAxis(axisY, Qt::AlignLeft);
                    liveSeries->attachAxis(axisY);
                    // set initial target max and range
                    qreal maxv = *std::max_element(values.begin(), values.end());
                    axisYTargetMax = qMax<qreal>(1.0, maxv * 1.5);
                    axisY->setRange(0, axisYTargetMax);
                    chart->legend()->setVisible(false);
                } else {
                    // update values in place to trigger Qt's series animations
                    int curCount = liveSet->count();
                    if (curCount != topN) {
                        // resize set
                        while (liveSet->count()>0) liveSet->remove(0, liveSet->count());
                        for (qreal v : values) *liveSet << v;
                    } else {
                        for (int i=0;i<topN;i++) liveSet->replace(i, values[i]);
                    }
                    if (axisX) { axisX->clear(); axisX->append(categories); }
                    if (axisY) {
                        qreal maxv = *std::max_element(values.begin(), values.end());
                        // set a target max a bit higher and interpolate current axis max toward it
                        axisYTargetMax = qMax<qreal>(1.0, maxv * 1.5);
                        qreal curMax = axisY->max();
                        qreal newMax = curMax + (axisYTargetMax - curMax) * axisYSmoothFactor;
                        axisY->setRange(0, qMax<qreal>(1.0, newMax));
                    }
                }
                prevCategories = categories;
                prevValues = values;
            }
        }
    }

private:
    QLineEdit *pathEdit;
    QLabel *statusLabel;
    QPlainTextEdit *previewEdit;
    QTableView *tableView;
    QStandardItemModel *tableModel;
    QProgressBar *progress;
    QChart *chart;
    QChartView *chartView;
    QSlider *speedSlider;
    QLabel *speedValueLabel;

    // live chart objects for smooth updates (Bar Chart Race)
    QBarSeries *liveSeries = nullptr;
    QBarSet *liveSet = nullptr;
    QBarCategoryAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;
    int chartTopN = 10;
    int animationDurationMs = 900;
    qreal axisYTargetMax = 1.0;
    qreal axisYSmoothFactor = 0.25; // interpolation factor for smooth Y-axis stretching
    // previous chart snapshot to avoid redundant updates
    QVector<qreal> prevValues;
    QStringList prevCategories;

    QTimer *readTimer;
    QTimer *pauseTimer;
    QTimer *uiTimer;
    QTextStream *textStream = nullptr;
    QFile file;
    QString leftover;
    size_t chunkSize;
    int readWordsPerStep = 30; // read 30 words per batch
    int readBatchCharSize = 256; // read this many chars per small read while accumulating words
    int pauseDurationMs = 300; // pause 300ms between batches (shorter)
    qint64 totalBytes;
    qint64 fileSize;
    QString initialPreview;
    int previewMaxChars = 20000;

    StringFreqMap freqMap;
};

#include "main.moc"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    StreamFreqWindow w;
    w.show();
    return app.exec();
}
