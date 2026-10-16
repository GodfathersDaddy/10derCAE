#pragma once

#include <QHash>
#include <QString>

enum class AppLanguage { German, English };

class I18n {
public:
    static I18n& instance();
    void setLanguage(AppLanguage language);
    AppLanguage language() const;
    QString text(const QString& key) const;
    QString code() const;

private:
    I18n();
    void load(const QString& code);
    AppLanguage m_language = AppLanguage::German;
    QHash<QString, QString> m_strings;
    QHash<QString, QString> m_fallback;
};

