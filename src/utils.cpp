#include "utils.h"

#include <QDateTime>
#include <QFile>
#include <QJsonParseError>

namespace Utils {


QString readFile(const QString &filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug()<<"文件打开失败";
        return "";
    }
    QString content = file.readAll();
    file.close();
    return content;
}

QString curDateTime() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

QString dateToStr(const QString &format) {
    return QDateTime::currentDateTime().toString(format);
}

int upperLower(int ch) {
    if (((ch >= 97) && (ch <= 122)) || ((ch >= 65) && (ch <= 90)))
        return ch ^ 32;
    else
        return ch;
}

QString toCamelCase(const QString &str, const char symbol) {
    QStringList parts = str.toLower().split(symbol, Qt::KeepEmptyParts);
    for (int i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            parts[i].replace(0, 1, parts[i][0].toUpper());
        }
    }
    return parts.join("");
}

QString toUnderlineCase(const QString &str, const char symbol) {
    QString newStr;
    for (int i = 0; i < str.size(); ++i) {
        if (!str.isEmpty() && str[i].isUpper()) {
            newStr.append(symbol);
        }
        newStr.append(str[i]);
    }
    return newStr.toUpper();
}

int  parseUrl(const QString &szUrl, QString &szSchema, QString &szHost, QString &port, QString &szPath) {
    if (!szUrl.isEmpty()) {
        QString tmpUrl = szUrl;
        if (tmpUrl.contains("://")) {
            QStringList urlSplit = tmpUrl.split("://");
            szSchema.append(urlSplit[0]);
            tmpUrl = urlSplit[1];
        }
        qsizetype colonIdx=  tmpUrl.indexOf(":");
        qsizetype oindx= tmpUrl.indexOf("/");
        qsizetype widx= tmpUrl.indexOf("?");
        qsizetype hostLen = tmpUrl.length();
        qsizetype portStart = 0;
        qsizetype portLen = 0;

        if (colonIdx>-1 && (oindx<0 || colonIdx < oindx) ) {
            hostLen= colonIdx;
            portStart = colonIdx+ 1;
            portLen = oindx-colonIdx -1;
        } else if (colonIdx>-1 && (widx<0 || colonIdx < widx)) {
            hostLen= colonIdx;
            portStart = colonIdx+ 1;
            portLen = widx-colonIdx-1;
        } else if (colonIdx>-1) {
            hostLen= colonIdx;
            portStart = colonIdx+ 1;
            portLen = tmpUrl.length()-colonIdx-1;
        } else if (oindx>-1) {
            hostLen= oindx;
        } else if (widx>-1) {
            hostLen= widx;
        }
        szHost.append(tmpUrl.mid(0, hostLen));
        port.append(tmpUrl.mid(portStart, portLen));
        qsizetype pathStart = hostLen;
        if (portLen> 0) {
            pathStart+=portLen + 1;
        }
        szPath.append(tmpUrl.mid(pathStart));
    }


    return 0;
}


}
