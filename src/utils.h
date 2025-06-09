#ifndef UTILS_H
#define UTILS_H


#include <QString>
namespace Utils {

QString readFile(const QString &filePath);
QString curDateTime();
QString dateToStr(const QString &format);

/**
     * @brief 隔符字符串转驼峰
     * @param str
     * @param symbol
     * @return
     */
QString toCamelCase(const QString &str, const char symbol);

/**
     * @brief 驼峰字符串转间隔符字符串
     * @param str
     * @param symbol
     * @return
     */
QString toUnderlineCase(const QString &str, const char symbol);

/**
     * @brief url解析
     * @param szUrl
     * @param szSchema
     * @param szHost
     * @param port
     * @param szPath
     * @return
     */
int parseUrl(const QString &szUrl, QString &szSchema, QString &szHost, QString &port, QString &szPath);



/**
     * @brief 字符串是否可转为json类型
     * @param jsonStr
     * @return
     */
int toJsonType(const QString &jsonStr);
}
#endif // UTILS_H
