#include "I18n.h"
#include "WorkspaceCatalog.h"

#include <QCoreApplication>
#include <QFile>
#include <QSet>
#include <iostream>

namespace {
bool require(bool condition, const QString& message) {
    if (!condition) std::cerr << "FAIL: " << message.toStdString() << '\n';
    return condition;
}
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    bool ok = require(WorkspaceCatalog::all().size() == 8, QStringLiteral("eight workspaces"));
    QSet<QString> ids;
    for (const auto language : {AppLanguage::German, AppLanguage::English}) {
        I18n::instance().setLanguage(language);
        for (const auto& workspace : WorkspaceCatalog::all()) {
            ok &= require(!ids.contains(workspace.stableId) || language == AppLanguage::English,
                QStringLiteral("unique workspace ID: ") + workspace.stableId);
            ids.insert(workspace.stableId);
            ok &= require(!I18n::instance().text(workspace.labelKey).startsWith(QLatin1Char('[')), workspace.labelKey);
            for (const auto& command : workspace.commands) {
                ok &= require(QFile::exists(QStringLiteral(":/icons/v2/light/%1").arg(command.iconName)), command.iconName);
                ok &= require(!I18n::instance().text(command.labelKey).startsWith(QLatin1Char('[')), command.labelKey);
                ok &= require(!I18n::instance().text(command.descriptionKey).startsWith(QLatin1Char('[')), command.descriptionKey);
            }
        }
    }
    for (const auto& workspace : WorkspaceCatalog::all())
        ok &= require(workspaceFromStableId(workspace.stableId) == workspace.id, workspace.stableId);
    return ok ? 0 : 1;
}
