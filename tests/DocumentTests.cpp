#include "Document.h"

#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <iostream>

namespace {
bool require(bool condition, const char* message) {
    if (!condition) std::cerr << "FAIL: " << message << '\n';
    return condition;
}
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    bool ok = true;
    QTemporaryDir directory;
    ok &= require(directory.isValid(), "temporary directory");
    Document original;
    original.setName(QStringLiteral("Prüfteil"));
    original.setDocumentType(QStringLiteral("part"));
    original.setActiveWorkspace(WorkspaceId::Fem, true);
    original.addFeature({QStringLiteral("f1"), QStringLiteral("Extrusion"),
        QStringLiteral("Extrusion 1"), {{QStringLiteral("distance"), 42.0}}});
    const QString path = directory.filePath(QStringLiteral("part.10cae"));
    QString error;
    ok &= require(original.save(path, &error), qPrintable(error));
    ok &= require(!original.isModified(), "save clears modified state");
    Document loaded;
    ok &= require(Document::load(path, &loaded, &error), qPrintable(error));
    ok &= require(loaded.name() == original.name(), "name roundtrip");
    ok &= require(loaded.activeWorkspace() == WorkspaceId::Fem, "workspace roundtrip");
    ok &= require(loaded.features() == original.features(), "feature roundtrip");

    const QString futurePath = directory.filePath(QStringLiteral("future.10cae"));
    QFile future(futurePath);
    ok &= require(future.open(QIODevice::WriteOnly), "open future schema fixture");
    future.write(R"({"schemaVersion":999,"application":"10derCAE","name":"Future"})");
    future.close();
    Document target;
    target.setName(QStringLiteral("Keep me"));
    error.clear();
    ok &= require(!Document::load(futurePath, &target, &error), "reject unknown schema");
    ok &= require(error.contains(QStringLiteral("999")), "schema error is understandable");
    ok &= require(target.name() == QStringLiteral("Keep me"), "failed load keeps target");
    return ok ? 0 : 1;
}
