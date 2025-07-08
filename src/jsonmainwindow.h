#ifndef JSONMAINWINDOW_H
#define JSONMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class JSONMainWindow;
}
QT_END_NAMESPACE


class JSONWidget;

class JSONMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    JSONMainWindow(QWidget *parent = nullptr);
    ~JSONMainWindow();

private:
    Ui::JSONMainWindow *ui;


    /**
     * @brief 当前JSONWidget
     * @return
     */
    JSONWidget *currentJSONWidget();

    /**
     * @brief 添加jsonView
     */
    void addJSONWidget();
private slots:


    /**
     * @brief 显示空值
     */
    void showNullCheckBox_checkStateChanged(const Qt::CheckState &state);


    /**
     * @brief 格式化文本
     */
    void formatButton_clicked();

    /**
     * @brief 文本换行
     */
    void jsonTextWrapCheckBox_checkStateChanged(const Qt::CheckState &state);

    /**
     * @brief 自动格式化
     */
    void jsonAutoFormatCheckBox_checkStateChanged(const Qt::CheckState &state);

    /**
     * @brief 删除空格
     */
    void removeSpacesButton_clicked();

    /**
     * @brief 去除转义字符
     */
    void removeEscapeButton_clicked();


    /**
     * @brief 隐藏未选中节点
     * @param state
     */
    void jsonItemHideCheckBox_checkStateChanged(const Qt::CheckState &state);
    /**
     * @brief 查询
     */
    void searchButton_clicked();

    void nextButton_clicked();

    void previousButton_clicked();

    void collapseAllButton_clicked();
    void expandAllButton_clicked();
    void editTab(int index);
    void close_tab(int index);
    void showTabContextMenu(const QPoint &pos);
    void closeOtherTabs();
    void closeTabsToRight();
    void closeTabsToLeft();

    void aboutButton_clicked();
};
#endif // JSONMAINWINDOW_H
