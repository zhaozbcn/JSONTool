#include "jsonwidget.h"
#include "ui_jsonwidget.h"
#include "jsonhighlighter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStyleFactory>
#include <QTextBlock>
#include <QTimer>
#include <qnamespace.h>
#include <QMenu>

JSONWidget::JSONWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::JSONWidget)
{
    ui->setupUi(this);

    // 确保分割器已正确初始化
    ui->mainSplitter->setChildrenCollapsible(false);

    // 设置默认宽度相等
    int totalWidth = this->width();
    int partWidth = totalWidth * 0.1;
    ui->mainSplitter->setSizes(QList<int>() << partWidth * 4 << partWidth* 4 << partWidth *2);
    ui->mainSplitter->setStretchFactor(0, 1);
    ui->mainSplitter->setStretchFactor(1, 1);
    ui->mainSplitter->setStretchFactor(2, 1);

    // 添加树视图列宽设置
    ui->jsonTreeView->header()->setStretchLastSection(false);
    ui->jsonTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->jsonTreeView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->jsonTreeView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->jsonTreeView->setColumnWidth(1, 0);
    ui->jsonTreeView->setStyle(QStyleFactory::create("windows"));

    // 恢复上次保存的宽度
    restoreSplitterState();

    // 实时校验 JSON 输入
     new JsonHighlighter(ui->jsonTextEdit->document());


    // 安装事件过滤器
    ui->jsonTextEdit->installEventFilter(this);

     // 失去焦点的处理
     connect(ui->jsonTextEdit, &Ui::CodeEditor::textChanged, this, [this](){
         jsonTextChanged = true;
     });

    connect(this, &JSONWidget::jsonTextEditFocusLost, this, [this](){
        if (jsonTextChanged && jsonTextAutoFormat) {
            formatJSON();
            jsonTextChanged = false;
        }
    });
    connect(ui->jsonTreeView, &QTreeView::clicked, this, &JSONWidget::jsonTreeView_clicked);
    // 设置树视图支持右键菜单
    ui->jsonTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->jsonTreeView, &QTreeView::customContextMenuRequested, this, &JSONWidget::showContextMenu);



}


void JSONWidget::formatJSON()
{
    QString jsonString = ui->jsonTextEdit->toPlainText();

    clearTreeViewHighlights();

    if (jsonString.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入JSON数据");
        return;
    }

    // 获取UTF-8编码的字节数组
    QByteArray utf8Data = jsonString.toUtf8();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(utf8Data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        // 获取UTF-8字节偏移量
        int errorOffset = parseError.offset;
        qDebug() << "Error at position:" << errorOffset;

        // 计算字符位置（考虑多字节字符）
        int charPosition = QString::fromUtf8(utf8Data.left(errorOffset)).length();
        qDebug() << "charPosition:" << charPosition;

        // 显示错误信息到 status bar
        QString errorMessage = QString("JSON解析错误: %1 (位置: %2)").arg(parseError.errorString()).arg(charPosition);
        // ui->statusBar->showMessage(errorMessage, 5000);

        // 定位并高亮错误位置
        QTextCursor cursor(ui->jsonTextEdit->document());
        cursor.setPosition(charPosition -1);
        ui->jsonTextEdit->setTextCursor(cursor);
        ui->jsonTextEdit->ensureCursorVisible();

        // 高亮错误位置（仅高亮单个字符）
        QList<QTextEdit::ExtraSelection> extraSelections;
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(Qt::red);
        selection.cursor = cursor;
        selection.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1); // 仅选中1个字符
        extraSelections.append(selection);
        ui->jsonTextEdit->setExtraSelections(extraSelections);

        ui->jsonTextEdit->setTextCursor(cursor);
        ui->jsonTextEdit->ensureCursorVisible();
        // 添加这行代码强制立即刷新视图
        ui->jsonTextEdit->viewport()->update();
        // 弹窗提示
        QMessageBox::critical(this, "JSON解析错误", errorMessage);
        return;
    }

    // 如果是合法 JSON，则格式化并显示树状结构
    QString formattedJson = jsonDoc.toJson(QJsonDocument::Indented);
    ui->jsonTextEdit->setPlainText(formattedJson);

    // 在创建新模型前先清除旧模型
    if(ui->jsonTreeView->model()) {
        delete ui->jsonTreeView->model();
    }

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Key", "Value"});

    QStandardItem *rootItem = model->invisibleRootItem();
    qint32 rowNum =1;
    if (jsonDoc.isObject()) {
        processJsonObject(jsonDoc.object(), rootItem, rowNum);
    } else if (jsonDoc.isArray()) {
        processJsonArray(jsonDoc.array(), rootItem,rowNum, "Array");
    }

    ui->jsonTreeView->setModel(model);

    // 添加以下代码确保视图完全更新
    ui->jsonTreeView->expandAll();
    ui->jsonTreeView->resizeColumnToContents(0);
    ui->jsonTreeView->update();
    ui->jsonTreeView->collapseAll();
}

// 处理JSON对象
void JSONWidget::processJsonObject(const QJsonObject &obj, QStandardItem *parent, qint32& rowNum)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        rowNum++;
        QStandardItem *keyItem = new QStandardItem(it.key());
        keyItem->setData(QVariant::fromValue(it.value().toVariant()), Qt::UserRole);
        keyItem->setData(rowNum, Qt::UserRole + 1);

        if (it.value().isObject()) {
            // 对象类型只显示key
            keyItem->setIcon(QIcon(":/resources/object.gif"));
            parent->appendRow(keyItem);
            processJsonObject(it.value().toObject(), keyItem, rowNum);
        } else if (it.value().isArray()) {
            // 数组类型只显示key
            keyItem->setIcon(QIcon(":/resources/array.gif"));
            parent->appendRow(keyItem);
            processJsonArray(it.value().toArray(), keyItem,rowNum, it.key());
        } else {
            // 基本数据类型显示为"key:value"
            QString displayText = it.key() + ": ";
            if (it.value().isNull()) {
                displayText += "null";
                keyItem->setIcon(QIcon(":/resources/null.gif"));
            } else if (it.value().isBool()) {
                displayText += it.value().toBool() ? "true" : "false";
                keyItem->setIcon(QIcon(":/resources/bool.gif"));
            } else if (it.value().isDouble()) {
                // 修改这里，使用'g'格式避免科学计数法
                displayText += it.value().toVariant().toString();
                keyItem->setIcon(QIcon(":/resources/number.gif"));
            } else if (it.value().isString()) {
                // 修改这里，为字符串值添加双引号
                displayText += "\"" + it.value().toString() + "\"";
                keyItem->setIcon(QIcon(":/resources/string.gif"));
            } else {
                displayText += it.value().toString();
                keyItem->setIcon(QIcon(":/resources/defult.gif"));
            }
            keyItem->setText(displayText);
            parent->appendRow(keyItem);
        }
    }
    rowNum++;

    // 在设置模型后添加这行代码来调整列宽
    ui->jsonTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->jsonTreeView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->jsonTreeView->setColumnWidth(1, 0);  // 隐藏第二列
}

// 处理JSON数组
void JSONWidget::processJsonArray(const QJsonArray &array, QStandardItem *parent, qint32& rowNum, const QString &name)
{
    int index = 0;
    for (const QJsonValue &value : array) {
        rowNum++;
        QStandardItem *keyItem = new QStandardItem(QString("[%1]").arg(index++));
        keyItem->setData(QVariant::fromValue(value.toVariant()), Qt::UserRole);
        keyItem->setData(rowNum, Qt::UserRole + 1);
        if (value.isObject()) {
            // 对象类型只显示索引
            keyItem->setIcon(QIcon(":/resources/object.gif"));
            parent->appendRow(keyItem);
            processJsonObject(value.toObject(), keyItem, rowNum);
        } else if (value.isArray()) {
            // 数组类型只显示索引
            keyItem->setIcon(QIcon(":/resources/array.gif"));
            parent->appendRow(keyItem);
            processJsonArray(value.toArray(), keyItem, rowNum, QString("[%1]").arg(index-1));
        } else {
            // 基本数据类型显示为"[index]:value"
            QString displayText = QString("[%1]: ").arg(index-1);
            if (value.isNull()) {
                displayText += "null";
                keyItem->setIcon(QIcon(":/resources/null.gif"));
            } else if (value.isBool()) {
                displayText += value.toBool() ? "true" : "false";
                keyItem->setIcon(QIcon(":/resources/bool.gif"));
            } else if (value.isDouble()) {
                // 修改这里，使用'g'格式避免科学计数法
                displayText += QString::number(value.toDouble(), 'g', 15);
                keyItem->setIcon(QIcon(":/resources/number.gif"));
            } else if (value.isString()) {
                // 修改这里，为字符串值添加双引号
                displayText += "\"" + value.toString() + "\"";
                keyItem->setIcon(QIcon(":/resources/string.gif"));
            } else {
                displayText += value.toString();
                keyItem->setIcon(QIcon(":/resources/defult.gif"));
            }
            keyItem->setText(displayText);
            parent->appendRow(keyItem);
        }
    }
    rowNum++;
}


void JSONWidget::setJsonTextAutoFormat(bool newJsonTextAutoFormat)
{
    jsonTextAutoFormat = newJsonTextAutoFormat;
}

void JSONWidget::removeSpaces()
{
    QString jsonString = ui->jsonTextEdit->toPlainText();

    if (jsonString.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入JSON数据");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "错误",QString("JSON解析错误: %1 (位置: %2)").arg(parseError.errorString()).arg(parseError.offset));
        return;
    }

    // 转换为紧凑格式的JSON字符串
    QString compactJson = jsonDoc.toJson(QJsonDocument::Compact);
    ui->jsonTextEdit->setPlainText(compactJson);
}



void JSONWidget::removeEscape()
{
    QString jsonString = ui->jsonTextEdit->toPlainText();

    if (jsonString.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入JSON数据");
        return;
    }

    // 移除转义字符
    QString unescapedJson = jsonString;
    unescapedJson.replace("\\\"", "\"")  // 移除字符串引号转义
        .replace("\\n", "\n")    // 移除换行符转义
        .replace("\\t", "\t")    // 移除制表符转义
        .replace("\\r", "\r")    // 移除回车符转义
        .replace("\\\\", "\\");  // 移除反斜杠转义

    // 验证JSON格式
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(unescapedJson.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "错误", QString("JSON解析错误: %1 (位置: %2)").arg(parseError.errorString()).arg(parseError.offset) );
        return;
    }

    ui->jsonTextEdit->setPlainText(unescapedJson);
}

void JSONWidget::hideUnSelectedNode(bool hide)
{
    jsonTreeHideUnSelectedNode = hide;
    searchJSON(searchText);
}

void JSONWidget::searchJSON(const QString& searchText)
{
    this->searchText = searchText;
    resetTreeViewVisibility(); // 清除之前所有隐藏状态

    // 重置匹配状态
    clearTreeViewHighlights();
    if (searchText.isEmpty()) return;

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->jsonTreeView->model());
    if (!model) return;

    // 递归搜索匹配项
    searchTreeItems(model->invisibleRootItem(), searchText);

    if (!matchedItems.isEmpty()) {
        currentMatchIndex = 0;
        ui->jsonTreeView->setCurrentIndex(matchedItems.first()->index());
        ui->jsonTreeView->scrollTo(matchedItems.first()->index());
    } else {
        QMessageBox::information(this, "提示", "未找到匹配项");
    }
}

void JSONWidget::searchNext()
{

    if (matchedItems.isEmpty()) return;

    if (currentMatchIndex < matchedItems.size() - 1) {
        currentMatchIndex++;
        QStandardItem* item = matchedItems.at(currentMatchIndex);
        item->setBackground(Qt::green); // 高亮当前项
        ui->jsonTreeView->setCurrentIndex(item->index());
        ui->jsonTreeView->scrollTo(item->index());
    } else {
        QMessageBox::information(this, "提示", "已是最后一个匹配项");
    }
}

void JSONWidget::searchPrevious()
{
    if (matchedItems.isEmpty()) return;


    if (currentMatchIndex > 0) {
        currentMatchIndex--;
        QStandardItem* item = matchedItems.at(currentMatchIndex);
        item->setBackground(Qt::red); // 高亮当前项
        ui->jsonTreeView->setCurrentIndex(item->index());
        ui->jsonTreeView->scrollTo(item->index());
    } else {
        QMessageBox::information(this, "提示", "已是第一个匹配项");
    }
}

void JSONWidget::expandAllJSONNode()
{
    ui->jsonTreeView->expandAll();
}

void JSONWidget::collapseAllJSONNode()
{
    ui->jsonTreeView->collapseAll();
}

void JSONWidget::jsonTreeView_clicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->jsonTreeView->model());
    if (!model) return;

    QStandardItem *item = model->itemFromIndex(index);
    if (!item || item->text().isEmpty()) {
        return;
    }

    // 准备表格
    ui->propertiesTableWidget->clear();
    ui->propertiesTableWidget->setRowCount(0);
    ui->propertiesTableWidget->setColumnCount(2);
    ui->propertiesTableWidget->setHorizontalHeaderLabels({"Key", "Value"});

    // 获取当前项的用户数据（存储原始JSON值）
    QVariant userData = item->data(Qt::UserRole);
    if (userData.isValid()) {
        QJsonValue jsonValue = userData.toJsonValue();

        int row = ui->propertiesTableWidget->rowCount();
        if (item->rowCount() > 0) {
            for (int i = 0; i < item->rowCount(); ++i) {
                QStandardItem *child = item->child(i);
                if (!child) continue;

                int row = ui->propertiesTableWidget->rowCount();
                ui->propertiesTableWidget->insertRow(row);

                // 第一列显示属性名
                QTableWidgetItem *keyItem = new QTableWidgetItem(child->text().split(":").at(0));
                ui->propertiesTableWidget->setItem(row, 0, keyItem);

                // 第二列显示属性值 - 修改这里
                QVariant childData = child->data(Qt::UserRole);
                if (childData.isValid()) {
                    QJsonValue jsonValue = childData.toJsonValue();
                    QTableWidgetItem *valueItem = nullptr;
                    if (jsonValue.isObject()) {
                        valueItem = new QTableWidgetItem("Object");
                    } else if (jsonValue.isArray()) {
                        valueItem = new QTableWidgetItem("Array");
                    } else {
                        valueItem = new QTableWidgetItem(jsonValue.toVariant().toString());
                    }
                    ui->propertiesTableWidget->setItem(row, 1, valueItem);
                }
            }
        } else {
            ui->propertiesTableWidget->insertRow(row);
            QTableWidgetItem *keyItem = new QTableWidgetItem(item->text().split(":").at(0));
            ui->propertiesTableWidget->setItem(row, 0, keyItem);
            QTableWidgetItem *valueItem = nullptr;
            if (jsonValue.isObject()) {
                valueItem = new QTableWidgetItem("Object");
            } else if (jsonValue.isArray()) {
                valueItem = new QTableWidgetItem("Array");
            } else {
                valueItem = new QTableWidgetItem(jsonValue.toVariant().toString());
            }
            ui->propertiesTableWidget->setItem(row, 1, valueItem);
        }


    }
    // 调整列宽
    ui->propertiesTableWidget->resizeColumnsToContents();
    // 添加这行代码设置最小列宽
    ui->propertiesTableWidget->horizontalHeader()->setMinimumSectionSize(80);


    // 修改这部分代码，确保正确设置选中状态
    ui->jsonTreeView->setCurrentIndex(index);  // 添加这行确保选中状态

    // 获取当前项对应的JSON路径
    // 获取当前项对应的JSON路径
    QStringList path;
    QModelIndex current = index;
    while (current.isValid()) {
        QString key = model->data(current).toString().split(":").at(0).trimmed();
        path.prepend(key);
        current = current.parent();
    }

    // 在jsonTextEdit中查找并高亮对应文本
    highlightJsonText(item->text().split(":").at(0).trimmed(), (qint32)item->data(Qt::UserRole+1).value<qint32>());

}


void JSONWidget::highlightJsonText(const QString& key, const qint32& rowNum)
{
    QTextDocument *doc = ui->jsonTextEdit->document();

    // 清除之前的高亮
    QList<QTextEdit::ExtraSelection> extraSelections;
    ui->jsonTextEdit->setExtraSelections(extraSelections);

    // 获取指定行号的文本块
    QTextBlock block = doc->findBlockByNumber(rowNum-1);
    if (!block.isValid()) return;

    QString lineText = block.text();
    int keyPos = lineText.indexOf(key);
    if (keyPos == -1) return;

    // 创建高亮选择
    QTextCursor cursor(block);
    cursor.setPosition(block.position() + keyPos);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, key.length());

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(Qt::green);
    selection.cursor = cursor;
    extraSelections.append(selection);

    // 应用高亮
    ui->jsonTextEdit->setExtraSelections(extraSelections);
    ui->jsonTextEdit->setTextCursor(cursor);
}

void JSONWidget::resetTreeViewVisibility()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->jsonTreeView->model());
    if (!model) return;

    for (int i = 0; i < model->invisibleRootItem()->rowCount(); ++i) {
        QStandardItem *item = model->invisibleRootItem()->child(i);
        resetItemVisibility(item);
    }
}

void JSONWidget::resetItemVisibility(QStandardItem *item)
{
    if (!item) return;

    setTreeViewItemHidden(item->index(), false); // 显示该项

    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem *child = item->child(i);
        resetItemVisibility(child);
    }
}

void JSONWidget::setTreeViewItemHidden(const QModelIndex &index, bool hidden)
{
    if (!index.isValid()) return;
    ui->jsonTreeView->setRowHidden(index.row(), index.parent(), hidden);
}

bool JSONWidget::searchTreeItems(QStandardItem *parent, const QString &searchText)
{
    if (!parent) return false;

    bool hasMatch = false;
    for (int i = 0; i < parent->rowCount(); ++i) {
        QStandardItem* item = parent->child(i);
        if (!item) continue;

        bool childHasMatch = false;
        if (item->rowCount() > 0) {
            childHasMatch = searchTreeItems(item, searchText);
        }
        bool currentMatch = item->text().contains(searchText, Qt::CaseInsensitive);
        if (currentMatch || childHasMatch) {
            if (currentMatch) {
                matchedItems.append(item);
                item->setBackground(Qt::red);
            }
            hasMatch = true;
            // 确保匹配节点及其父节点可见
            setTreeViewItemHidden(item->index(), false); // 显示该行
        } else if (jsonTreeHideUnSelectedNode) {
            // 标记不匹配的节点为隐藏
            setTreeViewItemHidden(item->index(), true);
        }
    }
    return hasMatch;
}

void JSONWidget::clearTreeViewHighlights()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->jsonTreeView->model());
    if (!model) return;

    // 清除所有匹配项的高亮
    for (QStandardItem* item : matchedItems) {
        item->setBackground(QBrush());
    }
    matchedItems.clear();
    currentMatchIndex = -1;
}

bool JSONWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->jsonTextEdit && event->type() == QEvent::FocusOut) {
        emit jsonTextEditFocusLost();
    }
    return QWidget::eventFilter(watched, event);
}

void JSONWidget::setJsonTextLineWrap(bool newJsonTextLineWrap)
{
    jsonTextLineWrap = newJsonTextLineWrap;
    if (jsonTextLineWrap) {
        ui->jsonTextEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        ui->jsonTextEdit->setWordWrapMode(QTextOption::WrapMode::WordWrap);
    } else {
        ui->jsonTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        ui->jsonTextEdit->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    }
}


// 恢复分割器状态
void JSONWidget::restoreSplitterState()
{
    QSettings settings;
    QVariant savedSizes = settings.value("splitterSizes");

    if (savedSizes.isValid() && savedSizes.canConvert<QVariantList>()) {
        QVariantList sizes = savedSizes.value<QVariantList>();
        QList<int> intSizes;
        for (const QVariant &v : sizes) {
            bool ok;
            int value = v.toInt(&ok);
            if (!ok) return; // 类型错误，放弃恢复
            intSizes.append(value);
        }
        ui->mainSplitter->setSizes(intSizes);
    }
}

// 保存分割器状态（例如在 closeEvent 中调用）
void JSONWidget::saveSplitterState()
{
    QSettings settings;

    // 将 QList<int> 转换为 QVariantList
    QVariantList sizes;
    for (int size : ui->mainSplitter->sizes()) {
        sizes.append(size);
    }

    settings.setValue("splitterSizes", sizes);
}

JSONWidget::~JSONWidget()
{
    delete ui;
}


void JSONWidget::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->jsonTreeView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu;
    QAction *expandAction = menu.addAction("展开下级节点");
    QAction *collapseAction = menu.addAction("收起节点");
    QAction *expandAllAction = menu.addAction("展开所有子节点");
    QAction *collapseAllAction = menu.addAction("收起所有子节点");

    connect(expandAction, &QAction::triggered, this, &JSONWidget::expandCurrentNode);
    connect(collapseAction, &QAction::triggered, this, &JSONWidget::collapseCurrentNode);
    connect(expandAllAction, &QAction::triggered, this, &JSONWidget::expandAllChildren);
    connect(collapseAllAction, &QAction::triggered, this, &JSONWidget::collapseAllChildren);

    menu.exec(ui->jsonTreeView->viewport()->mapToGlobal(pos));
}

void JSONWidget::expandCurrentNode()
{
    QModelIndex index = ui->jsonTreeView->currentIndex();
    if (index.isValid()) {
        ui->jsonTreeView->expand(index);
    }
}

void JSONWidget::collapseCurrentNode()
{
    QModelIndex index = ui->jsonTreeView->currentIndex();
    if (index.isValid()) {
        ui->jsonTreeView->collapse(index);
    }
}

void JSONWidget::expandAllChildren()
{
    QModelIndex index = ui->jsonTreeView->currentIndex();
    if (index.isValid()) {
        ui->jsonTreeView->expandRecursively(index);
    }
}

void JSONWidget::collapseAllChildren()
{
    QModelIndex index = ui->jsonTreeView->currentIndex();
    if (index.isValid()) {
        collapseChildren(index);
    }
}

void JSONWidget::clear_data()
{


}

void JSONWidget::collapseChildren(const QModelIndex &index)
{
    QAbstractItemModel *model = ui->jsonTreeView->model();
    int rowCount = model->rowCount(index);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex child = model->index(i, 0, index);
        collapseChildren(child);
    }
    ui->jsonTreeView->collapse(index);
}
