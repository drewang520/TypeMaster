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
    app.setApplicationName(QStringLiteral("TypeMaster"));  //名称
    app.setApplicationVersion(QStringLiteral("1.0"));      // 版本
    app.setOrganizationName(QStringLiteral("TypeMaster"));  // 组织名称
    
    // ── Load translation ──────────────────────────────────────────────────
    //用于加载.qm翻译文件(Qt编译后的翻译文件)
    QTranslator translator; 
    // 获取系统的 UI 语言列表，例如 ["zh_CN", "en_US"]，按照用户的系统设置排序
    const QStringList uiLanguages = QLocale::system().uiLanguages();

    // Try to load zh_CN first (project default), then fall back to system locale
    bool loaded = translator.load(QStringLiteral(":/i18n/typemaster_zh_CN.qm"));
    if (!loaded) {
        for (const QString &locale : uiLanguages) {
            const QString baseName = QStringLiteral("typemaster_") + QLocale(locale).name();
            if (translator.load(QStringLiteral(":/i18n/") + baseName)) {
                loaded = true;
                break;
            }
        }
    }
    if (loaded) {
        app.installTranslator(&translator);
        // 将翻译器安装到应用程序中，使得应用程序能够根据加载的翻译文件显示对应语言的界面文本
    }
    
    MainWindow w;
    w.show();
    return app.exec();
}
