// codeeditor.cpp
#include "codeeditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QPixmap>
#include <QIcon>
#include <QScrollBar>

class LineNumberArea : public QWidget {
public:
    LineNumberArea(Ui::CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    Ui::CodeEditor *codeEditor;
};

namespace Ui {

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent), lineNumberArea(new LineNumberArea(this))
{
    setFont(QFont("Consolas", 10));

    connect(document(), &QTextDocument::blockCountChanged, this, [this](int newBlockCount) {
        if (newBlockCount)
            lineNumberArea->setFixedHeight(document()->blockCount() * fontMetrics().height());
    });

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    iconClosed = QIcon(":/icons/fold_closed").pixmap(16, 16);
    iconOpened = QIcon(":/icons/fold_opened").pixmap(16, 16);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, document()->blockCount());  // 使用document()->blockCount()获取实际行数
    while (max >= 10) { max /= 10; ++digits; }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 20;
    return space;
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingGeometry(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);  // 显示实际行号
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                            Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingGeometry(block).height());
        ++blockNumber;
    }
}

void CodeEditor::mousePressEvent(QMouseEvent *e)
{
    // 调用父类实现以确保基本功能正常工作
    QPlainTextEdit::mousePressEvent(e);
    
    // 获取鼠标点击位置的光标
    QTextCursor newCursor = cursorForPosition(e->pos());
    
    // 如果是双击或三击，保留默认选择行为
    if (e->type() == QEvent::MouseButtonDblClick) {
        return;
    }
    
    // 设置新的光标位置
    setTextCursor(newCursor);
    
    // 触发折叠/展开动作
    int blockNumber = newCursor.blockNumber();
    emit toggleFold(blockNumber);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    // 调用父类实现处理按键
    QPlainTextEdit::keyPressEvent(e);
    
    // 确保光标可见
    ensureCursorVisible();
}
} // namespace Ui
