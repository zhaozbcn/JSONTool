#ifndef JSONWIDGET_H
#define JSONWIDGET_H

#include "codeeditor.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class CodeEditor;
class JSONWidget;
}
QT_END_NAMESPACE

class JSONWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JSONWidget(QWidget *parent = nullptr);
    ~JSONWidget();

    /**
     * @brief 格式化json文本
     */
    void formatJSON();
    bool getJsonTextLineWrap() const;
    void setJsonTextLineWrap(bool newJsonTextLineWrap);

    /**
     * @brief 自动格式化
     * @param newJsonTextAutoFormat
     */
    void setJsonTextAutoFormat(bool newJsonTextAutoFormat);

    /**
     * @brief 删除文本空格
     */
    void removeSpaces();
    /**
     * @brief 删除文本转义字符
     */
    void removeEscape();

    /**
     * @brief 隐藏空值
     * @param hide
     */
    void showNullValues(bool show);

    /**
     * @brief 删除文本转义字符
     */
    void hideUnSelectedNode(bool hide);
    /**
     * @brief 查询
     */
    void searchJSON(const QString&  searchText);

    void searchNext();

    void searchPrevious();

    void expandAllJSONNode();

    void collapseAllJSONNode();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void jsonTextEditFocusLost();

private slots:
    void jsonTreeView_clicked(const QModelIndex &index);
    void showContextMenu(const QPoint &pos);
    void expandCurrentNode();
    void collapseCurrentNode();
    void expandAllChildren();
    void collapseAllChildren();



private:
    Ui::JSONWidget *ui;
    QList<QStandardItem*> matchedItems; // 存储匹配的节点
    int currentMatchIndex = -1; // 当前选中的匹配项索引
    bool jsonTextChanged = false;

    /**
     * @brief 隐藏控制
     */
    bool showNullValueFlag = false;
    /**
     * @brief 文本换行
     */
    bool jsonTextLineWrap = false;
    /**
     * @brief 自动格式化
     */
    bool jsonTextAutoFormat = false;

    /**
     * @brief 隐藏未选中的节点
     */
    bool jsonTreeHideUnSelectedNode = false;

    QString searchText;

    void clear_data();

    void showNullValueItem(QStandardItem *parent, bool isRoot);

    void restoreSplitterState();
    void saveSplitterState();
    void processJsonArray(const QJsonArray &array, QStandardItem *parent, qint32 &rowNum, const QString &name);
    void processJsonObject(const QJsonObject &obj, QStandardItem *parent, qint32 &rowNum);
    void highlightJsonText(const QString &key, const qint32 &rowNum);

    void resetTreeViewVisibility();

    void resetItemVisibility(QStandardItem *item);

    void setTreeViewItemHidden(const QModelIndex &index, bool hidden);

    bool searchTreeItems(QStandardItem* parent, const QString& searchText);

    void clearTreeViewHighlights();


    void collapseChildren(const QModelIndex &index);
};

#endif // JSONWIDGET_H
