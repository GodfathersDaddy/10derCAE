#include "I18n.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
QHash<QString, QString> loadCatalog(const QString& code) {
    QFile file(QStringLiteral(":/i18n/%1.json").arg(code));
    QHash<QString, QString> result;
    if (!file.open(QIODevice::ReadOnly)) return result;
    const QJsonObject object = QJsonDocument::fromJson(file.readAll()).object();
    for (auto it = object.begin(); it != object.end(); ++it) result.insert(it.key(), it.value().toString());
    return result;
}
}

I18n& I18n::instance() {
    static I18n value;
    return value;
}

I18n::I18n() {
    Q_INIT_RESOURCE(tendercae_resources);
    m_fallback = loadCatalog(QStringLiteral("en"));
    load(QStringLiteral("de"));
}
void I18n::setLanguage(AppLanguage language) { m_language = language; load(code()); }
AppLanguage I18n::language() const { return m_language; }
QString I18n::code() const { return m_language == AppLanguage::German ? QStringLiteral("de") : QStringLiteral("en"); }
void I18n::load(const QString& code) { m_strings = loadCatalog(code); }

QString I18n::text(const QString& key) const {
    if (const auto it = m_strings.constFind(key); it != m_strings.constEnd()) return *it;
    if (const auto it = m_fallback.constFind(key); it != m_fallback.constEnd()) return *it;
    if (key.startsWith(QStringLiteral("tip."))) {
        if (const auto it = m_strings.constFind(QStringLiteral("tip.default")); it != m_strings.constEnd()) return *it;
    }
    return QStringLiteral("[%1]").arg(key);
}

