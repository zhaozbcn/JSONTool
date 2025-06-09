#include "jsonmainwindow.h"
#include "utils.h"
#include <QApplication>
#include <QException>
#include <QDebug>
#include <QFile>

#define logDebug    QMessageLogger(__FILE__,__LINE__, QT_MESSAGELOG_FUNC).debug
#define logInfo     QMessageLogger(__FILE__,__LINE__, QT_MESSAGELOG_FUNC).info
#define logWarning  QMessageLogger(__FILE__,__LINE__, QT_MESSAGELOG_FUNC).warning
#define logCritical QMessageLogger(__FILE__,__LINE__, QT_MESSAGELOG_FUNC).critical
#define logFatal    QMessageLogger(__FILE__,__LINE__, QT_MESSAGELOG_FUNC).fatal
static QtMessageHandler g_oldMessageHandler = Q_NULLPTR;
void logFileMessageHander(QtMsgType type, const QMessageLogContext& context, const QString& message);
std::string appRoot;
std::string dataRootDir;
std::string appDbPath;
std::string appIniCfgPath;
std::string appResourcPath;
std::string appLogPath;
using namespace Utils;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    appRoot = QCoreApplication::applicationDirPath().toStdString();
    dataRootDir = appRoot + "/appData/";
    appResourcPath=  appRoot + "/resource/";
    appIniCfgPath = dataRootDir + "/app.ini";
    appLogPath = appRoot + "/app_log" + Utils::dateToStr("yyyy-MM-dd_hhmmss").toStdString() + ".log";
    qSetMessagePattern("%{time yyyy-MM-dd hh:mm:ss.zzz} %{type}"
                       "%{if-debug} %{file} %{function} [%{line}]%{endif}"
                       "%{if-warning} %{file} %{function} [%{line}]%{endif}"
                       "%{if-critical} %{file} %{function} [%{line}]%{endif}"
                       "%{if-fatal} %{file} %{function} [%{line}]%{endif}"
                       " : %{message}");
    g_oldMessageHandler = qInstallMessageHandler(logFileMessageHander);
    JSONMainWindow w;
    w.show();
    return a.exec();
}


//日志消息的处理函数
void logFileMessageHander(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    if (g_oldMessageHandler) {
        g_oldMessageHandler(type, context, message);
    }
    //获取格式化的日志信息
    QString typeStr = qFormatLogMessage(type,context,message);
    //可以根据日志的级别进行过滤
    QString levelText;
    switch (type) {
    case QtDebugMsg:
        levelText = "Debug";
        break;
    case QtInfoMsg:
        levelText = "Info";
        break;
    case QtWarningMsg:
        levelText = "Warning";
        break;
    case QtCriticalMsg:
        levelText = "Critical";
        break;
    case QtFatalMsg:
        levelText = "Fatal";
        break;
    }

#ifdef QT_DEBUG
    // 在调试模式下输出所有日志
    QFile file(appLogPath.c_str());
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&file);
    textStream << typeStr << "\n";
#else
    // 在发布模式下只输出异常日志
    if (type == QtCriticalMsg || type == QtFatalMsg) {
        QFile file(appLogPath.c_str());
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream textStream(&file);
        textStream << typeStr << "\n";
    }
#endif
}

