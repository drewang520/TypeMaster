#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H  

#include <QTranslator>
#include <QApplication>

class LanguageManager
{
public:
    static LanguageManager &getInstance() 
    {
        static LanguageManager mgr;
        return mgr;
    }

    // locale: "zh_CN" / "en"
    void switchLanguage(const QString &locale) 
    {
        // 先卸载旧翻译器
        if (m_installed) 
        {
            qApp->removeTranslator(&m_translator);
            m_installed = false;
        }

        // 加载新 .qm 文件
        if (m_translator.load(QStringLiteral(":/i18n/typemaster_") + locale + ".qm")) 
        {
            m_installed = true;
            m_currentLocale = locale;
            qApp->installTranslator(&m_translator);
        }
    }

    QString currentLocale() const { return m_currentLocale; }

private:
    LanguageManager()
    {
        // Try to load zh_CN first (project default), then fall back to system locale
        bool loaded = m_translator.load(QStringLiteral(":/i18n/typemaster_zh_CN.qm"));
        if (!loaded) 
        {
            // 获取系统的 UI 语言列表，例如 ["zh_CN", "en_US"]，按照用户的系统设置排序
            const QStringList uiLanguages = QLocale::system().uiLanguages();
            for (const QString &locale : uiLanguages) 
            {
                const QString baseName = QStringLiteral("typemaster_") + QLocale(locale).name();
                if (m_translator.load(QStringLiteral(":/i18n/") + baseName)) 
                {
                    loaded = true;
                    m_currentLocale = locale;
                    m_installed = true;
                    break;
                }
            }
        }
        else 
        {
            m_installed = true; // 默认加载成功，设置当前语言为 zh_CN
            m_currentLocale = QStringLiteral("zh_CN");  //  设置当前语言为 zh_CN
        }

        // 将翻译器安装到应用程序中，使得应用程序能够根据加载的翻译文件显示对应语言的界面文本
        qApp->installTranslator(&m_translator);
    }

    LanguageManager(const LanguageManager &) = delete;
    LanguageManager &operator=(const LanguageManager &) = delete;

private:
    QTranslator m_translator;
    bool        m_installed{false};
    QString     m_currentLocale{QStringLiteral("zh_CN")};
};

#endif // LANGUAGEMANAGER_H