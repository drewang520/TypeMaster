#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // 设置应用程序元数据，这些信息可以在系统的应用程序菜单、关于对话框等地方显示
    // QStringLiteral 宏 用于创建不可变的字符串字面量，将字符串常量直接存储为 QString 数据，避免了运行时的内存分配，提高性能    
    app.setApplicationName(QStringLiteral("KeyVerse"));  //名称
    app.setApplicationVersion(QStringLiteral("1.0"));      // 版本
    app.setOrganizationName(QStringLiteral("KeyVerse"));  // 组织名称
    MainWindow w;
    w.show();
    return app.exec();
}
