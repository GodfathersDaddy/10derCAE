#include "I18n.h"
#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFont>
#include <QScreen>
#include <QTimer>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("10derCAE"));
    application.setApplicationVersion(QStringLiteral("0.2.0"));
    application.setOrganizationName(QStringLiteral("10der"));
    application.setFont(QFont(QStringLiteral("Segoe UI"), 9));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("10derCAE native CAD/CAE shell"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({QStringLiteral("lang"), QStringLiteral("UI language: de or en"), QStringLiteral("code")});
    parser.addOption({QStringLiteral("workspace"), QStringLiteral("Initial workspace stable ID"), QStringLiteral("id")});
    parser.addOption({QStringLiteral("size"), QStringLiteral("Initial window size, e.g. 1440x920"), QStringLiteral("widthxheight")});
    parser.addOption({QStringLiteral("capture"), QStringLiteral("Save a window screenshot and exit"), QStringLiteral("path")});
    parser.addOption({QStringLiteral("viewport-dump"), QStringLiteral("Save the native OCCT framebuffer"), QStringLiteral("path")});
    parser.process(application);

    MainWindow window;
    if (parser.isSet(QStringLiteral("lang")))
        window.setLanguage(parser.value(QStringLiteral("lang")) == QStringLiteral("en") ? AppLanguage::English : AppLanguage::German);
    if (parser.isSet(QStringLiteral("workspace"))) window.setWorkspace(workspaceFromStableId(parser.value(QStringLiteral("workspace"))));
    if (parser.isSet(QStringLiteral("size"))) {
        const QStringList dimensions = parser.value(QStringLiteral("size")).split(QLatin1Char('x'));
        if (dimensions.size() == 2) window.resize(dimensions.at(0).toInt(), dimensions.at(1).toInt());
    }
    window.show();

    if (parser.isSet(QStringLiteral("capture"))) {
        const QString path = QDir::cleanPath(parser.value(QStringLiteral("capture")));
        QTimer::singleShot(1200, &window, [&window, path] {
            qApp->exit(window.captureWindow(path) ? 0 : 2);
        });
    }
    if (parser.isSet(QStringLiteral("viewport-dump"))) {
        const QString path = QDir::cleanPath(parser.value(QStringLiteral("viewport-dump")));
        QTimer::singleShot(1500, &window, [&window, path] { qApp->exit(window.dumpViewport(path) ? 0 : 3); });
    }
    return application.exec();
}
