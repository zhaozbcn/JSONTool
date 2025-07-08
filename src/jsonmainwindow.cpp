#include "jsonmainwindow.h"
#include "./ui_jsonmainwindow.h"
#include "./jsonwidget.h"

#include <QMenu>
#include <QMessageBox>
#include <QTabBar>
#include <QToolButton>

// 添加版本号定义
#ifndef PROJECT_VERSION
#define PROJECT_VERSION "1.0.0"  // 默认值，实际会由CMake覆盖
#endif

JSONMainWindow::JSONMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::JSONMainWindow)
{
    ui->setupUi(this);
    // 设置窗口标题包含版本号
    setWindowTitle(QString("JSON Viewer v%1").arg(PROJECT_VERSION));

    QToolButton *tabLeftCornerButton = new QToolButton(ui->jsonTabWidget->tabBar());
    tabLeftCornerButton->setObjectName("leftAddTabButton");
    tabLeftCornerButton->setText("+");
    QToolButton *tabRightCornerButton = new QToolButton(ui->jsonTabWidget->tabBar());
    tabRightCornerButton->setObjectName("cornerAddReqBtn");
    tabRightCornerButton->setText("+");
    ui->jsonTabWidget->setCornerWidget(tabLeftCornerButton, Qt::TopLeftCorner);
    ui->jsonTabWidget->setCornerWidget(tabRightCornerButton, Qt::TopRightCorner);

    connect(tabLeftCornerButton, &QToolButton::clicked, this, &JSONMainWindow::addJSONWidget);
    connect(tabRightCornerButton, &QToolButton::clicked, this, &JSONMainWindow::addJSONWidget);


    addJSONWidget();

    // 设置窗口图标
    setWindowIcon(QIcon(":/resources/json.png"));

    connect(ui->showNullCheckBox, &QCheckBox::checkStateChanged, this, &JSONMainWindow::showNullCheckBox_checkStateChanged);

    connect(ui->jsonTextWrapCheckBox, &QCheckBox::checkStateChanged, this, &JSONMainWindow::jsonTextWrapCheckBox_checkStateChanged);
    connect(ui->jsonAutoFormatCheckBox, &QCheckBox::checkStateChanged, this, &JSONMainWindow::jsonAutoFormatCheckBox_checkStateChanged);
    connect(ui->formatButton, &QPushButton::clicked, this, &JSONMainWindow::formatButton_clicked);
    // // connect(ui->foldButton, &QPushButton::clicked, this, &JSONMainWindow::on_foldButton_clicked);
    connect(ui->removeSpacesButton, &QPushButton::clicked, this, &JSONMainWindow::removeSpacesButton_clicked);
    connect(ui->removeEscapeButton, &QPushButton::clicked, this, &JSONMainWindow::removeEscapeButton_clicked);
    connect(ui->jsonItemHideCheckBox, &QCheckBox::checkStateChanged, this, &JSONMainWindow::jsonItemHideCheckBox_checkStateChanged);
    connect(ui->searchButton, &QPushButton::clicked, this, &JSONMainWindow::searchButton_clicked);
    connect(ui->nextButton, &QPushButton::clicked, this, &JSONMainWindow::nextButton_clicked);
    connect(ui->previousButton, &QPushButton::clicked, this, &JSONMainWindow::previousButton_clicked);
    connect(ui->expandAllButton, &QPushButton::clicked, this, &JSONMainWindow::expandAllButton_clicked);
    connect(ui->collapseAllButton, &QPushButton::clicked, this, &JSONMainWindow::collapseAllButton_clicked);

    connect(ui->jsonTabWidget, &QTabWidget::tabCloseRequested, this, &JSONMainWindow::close_tab);

    connect(ui->jsonTabWidget->tabBar(), &QTabBar::tabBarDoubleClicked, this,  &JSONMainWindow::editTab);
    
    // 设置标签页支持右键菜单
    ui->jsonTabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->jsonTabWidget->tabBar(), &QTabBar::customContextMenuRequested,
            this, &JSONMainWindow::showTabContextMenu);

    connect(ui->aboutButton, &QToolButton::clicked, this, &JSONMainWindow::aboutButton_clicked);

}

JSONMainWindow::~JSONMainWindow()
{
    delete ui;
}


JSONWidget* JSONMainWindow::currentJSONWidget()
{
    JSONWidget *curJSONWidget =static_cast<JSONWidget*>(ui->jsonTabWidget->currentWidget());
    return curJSONWidget;
}

void JSONMainWindow::addJSONWidget()
{
    JSONWidget* jsonWidget = new JSONWidget(ui->jsonTabWidget);
    jsonWidget->setJsonTextLineWrap(ui->jsonTextWrapCheckBox->isChecked());

    int index = ui->jsonTabWidget->addTab(jsonWidget, QString("New %1").arg(ui->jsonTabWidget->count() +1));
    ui->jsonTabWidget->setCurrentIndex(index);
}

void JSONMainWindow::showNullCheckBox_checkStateChanged(const Qt::CheckState &state)
{
    for (int i = 0; i < ui->jsonTabWidget->count(); ++i) {
        // 获取每个tab中的widget
        QWidget *widget = ui->jsonTabWidget->widget(i);
        // 尝试转换为JSONWidget类型
        JSONWidget *jsonWidget = qobject_cast<JSONWidget*>(widget);
        if (jsonWidget) {
            // 这里可以对每个JSONWidget进行操作
            jsonWidget->showNullValues(state == Qt::Checked);
        }
    }
}


void JSONMainWindow::formatButton_clicked()
{
    currentJSONWidget()->formatJSON();
}

void JSONMainWindow::jsonTextWrapCheckBox_checkStateChanged(const Qt::CheckState &state)
{
    for (int i = 0; i < ui->jsonTabWidget->count(); ++i) {
        QWidget *widget = ui->jsonTabWidget->widget(i);
        JSONWidget *jsonWidget = qobject_cast<JSONWidget*>(widget);
        if (jsonWidget) {
            // 这里可以对每个JSONWidget进行操作
            qDebug() << "Found JSONWidget at tab index:" << i;

            jsonWidget->setJsonTextLineWrap(state == Qt::Checked);
        }
    }
}

void JSONMainWindow::jsonAutoFormatCheckBox_checkStateChanged(const Qt::CheckState &state)
{
    for (int i = 0; i < ui->jsonTabWidget->count(); ++i) {
        // 获取每个tab中的widget
        QWidget *widget = ui->jsonTabWidget->widget(i);
        // 尝试转换为JSONWidget类型
        JSONWidget *jsonWidget = qobject_cast<JSONWidget*>(widget);
        if (jsonWidget) {
            // 这里可以对每个JSONWidget进行操作
            qDebug() << "Found JSONWidget at tab index:" << i;
            jsonWidget->setJsonTextAutoFormat(state == Qt::Checked);
        }
    }
}

void JSONMainWindow::removeSpacesButton_clicked()
{
    currentJSONWidget()->removeSpaces();
}

void JSONMainWindow::removeEscapeButton_clicked()
{
    currentJSONWidget()->removeEscape();
}

void JSONMainWindow::jsonItemHideCheckBox_checkStateChanged(const Qt::CheckState &state)
{
    currentJSONWidget()->hideUnSelectedNode(state == Qt::Checked);
}

void JSONMainWindow::searchButton_clicked()
{
    currentJSONWidget()->searchJSON(ui->searchLineEdit->text().trimmed());
}

void JSONMainWindow::nextButton_clicked()
{
    currentJSONWidget()->searchNext();
}

void JSONMainWindow::previousButton_clicked()
{
    currentJSONWidget()->searchPrevious();
}

void JSONMainWindow::collapseAllButton_clicked()
{
    currentJSONWidget()->collapseAllJSONNode();
}

void JSONMainWindow::expandAllButton_clicked()
{
    currentJSONWidget()->expandAllJSONNode();
}


void JSONMainWindow::editTab(int index)
{
    QString originalText = ui->jsonTabWidget->tabText(index); // 保存原始文本
    QLineEdit *editor = new QLineEdit(ui->jsonTabWidget);
    editor->setText(originalText);
    ui->jsonTabWidget->setTabText(index, "");
    connect(editor, &QLineEdit::editingFinished, [this, index, editor, originalText]() {
        QString newText = editor->text();
        if(newText.isEmpty()) {
            // 如果编辑后为空，恢复原始文本
            ui->jsonTabWidget->setTabText(index, originalText);
        } else {
            ui->jsonTabWidget->setTabText(index, newText);
        }
        ui->jsonTabWidget->tabBar()->setTabButton(index, QTabBar::LeftSide, nullptr);
        editor->deleteLater();
    });
    ui->jsonTabWidget->tabBar()->setTabButton(index, QTabBar::LeftSide, editor);
    editor->setFocus();
}

void JSONMainWindow::close_tab(int index)
{
    QWidget *widget = ui->jsonTabWidget->widget(index);
    if (widget) {
        // 从tabWidget中移除widget
        ui->jsonTabWidget->removeTab(index);
        // 释放内存
        widget->deleteLater();
        if (ui->jsonTabWidget->count() == 0) {
            addJSONWidget();
        }
    }
}


void JSONMainWindow::showTabContextMenu(const QPoint &pos)
{
    int currentIndex = ui->jsonTabWidget->tabBar()->tabAt(pos);
    if(currentIndex < 0) return;

    QMenu menu;
    QAction *closeOther = menu.addAction("关闭其他标签页");
    QAction *closeRight = menu.addAction("关闭右侧标签页");
    QAction *closeLeft = menu.addAction("关闭左侧标签页");

    connect(closeOther, &QAction::triggered, this, &JSONMainWindow::closeOtherTabs);
    connect(closeRight, &QAction::triggered, this, &JSONMainWindow::closeTabsToRight);
    connect(closeLeft, &QAction::triggered, this, &JSONMainWindow::closeTabsToLeft);

    menu.exec(ui->jsonTabWidget->tabBar()->mapToGlobal(pos));
}

void JSONMainWindow::closeOtherTabs()
{
    int currentIndex = ui->jsonTabWidget->currentIndex();
    for(int i = ui->jsonTabWidget->count()-1; i >= 0; --i) {
        if(i != currentIndex) {
            close_tab(i);
        }
    }
}

void JSONMainWindow::closeTabsToRight()
{
    int currentIndex = ui->jsonTabWidget->currentIndex();
    for(int i = ui->jsonTabWidget->count()-1; i > currentIndex; --i) {
        close_tab(i);
    }
}

void JSONMainWindow::closeTabsToLeft()
{
    int currentIndex = ui->jsonTabWidget->currentIndex();
    for(int i = currentIndex-1; i >= 0; --i) {
        close_tab(i);
    }
}

void JSONMainWindow::aboutButton_clicked()
{
    QString message = QString("<p>当前文件版本: %1</p>\n"
                              "<p>原代码路径为: <a href='https://github.com/zhaozbcn/JSONTool'>GitHub 项目地址</a></p>\n"
                              "<p>下载地址: <a href='https://github.com/zhaozbcn/JSONTool/releases/'>GitHub 下载页面</a></p>")
                          .arg(PROJECT_VERSION);

    QMessageBox::about(this, "关于", message);
}
