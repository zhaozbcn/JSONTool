// codeeditor.h
#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
class QPainter;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;

namespace Ui {

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void updateLineNumberArea(const QRect &, int dy);

signals:
    void toggleFold(int blockNumber);  // 点击折叠图标时触发

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);

    void highlightCurrentLine();

private:
    QWidget *lineNumberArea;
    QPixmap iconClosed, iconOpened;
};

} // namespace Ui

#endif // CODEEDITOR_H
