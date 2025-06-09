// jsonhighlighter.cpp
#include "jsonhighlighter.h"
#include <QColor>
#include <QTextCharFormat>
#include <QVector>

JsonHighlighter::JsonHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\btrue\\b" << "\\bfalse\\b" << "\\bnull\\b";

    for (const QString &pattern : std::as_const(keywordPatterns)) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);  // ✅ 使用 QRegularExpression
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    quotationFormat.setForeground(Qt::red);
    HighlightingRule stringRule;
    stringRule.pattern = QRegularExpression("\"([^\"\\\\]|\\\\.)*\"");  // ✅ 正则字符串匹配
    stringRule.format = quotationFormat;
    highlightingRules.append(stringRule);

    singleLineCommentFormat.setForeground(Qt::gray);
    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("//[^\n]*");  // ✅
    singleLineCommentRule.format = singleLineCommentFormat;
    highlightingRules.append(singleLineCommentRule);

    multiLineCommentFormat.setForeground(Qt::gray);
    commentStartExpression = QRegularExpression("/\\*");  // ✅
    commentEndExpression = QRegularExpression("\\*/");    // ✅
}

void JsonHighlighter::highlightBlock(const QString &text)
{    
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = commentStartExpression.match(text).capturedStart();
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength = 0;

        if (endIndex == -1) {
            commentLength = text.length() - startIndex;
            setCurrentBlockState(1);
        } else {
            commentLength = endMatch.capturedEnd() - startIndex;
        }

        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.match(text, startIndex + commentLength).capturedStart();
    }
}
