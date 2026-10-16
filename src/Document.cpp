#include "Document.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <stdexcept>

QJsonObject Feature::toJson() const {
    return {{QStringLiteral("id"), id}, {QStringLiteral("type"), type},
            {QStringLiteral("name"), name}, {QStringLiteral("parameters"), parameters}};
}

Feature Feature::fromJson(const QJsonObject& object) {
    return {object.value(QStringLiteral("id")).toString(),
            object.value(QStringLiteral("type")).toString(),
            object.value(QStringLiteral("name")).toString(),
            object.value(QStringLiteral("parameters")).toObject()};
}

QString Document::name() const { return m_name; }
void Document::setName(const QString& value) {
    const QString cleaned = value.trimmed();
    if (!cleaned.isEmpty() && cleaned != m_name) { m_name = cleaned; m_modified = true; }
}
QString Document::documentType() const { return m_documentType; }
void Document::setDocumentType(const QString& value) {
    if (!value.isEmpty() && value != m_documentType) { m_documentType = value; m_modified = true; }
}
WorkspaceId Document::activeWorkspace() const { return m_activeWorkspace; }
void Document::setActiveWorkspace(WorkspaceId value, bool dirty) {
    if (value != m_activeWorkspace) { m_activeWorkspace = value; m_modified = m_modified || dirty; }
}
QString Document::units() const { return m_units; }
void Document::setUnits(const QString& value) {
    if (!value.isEmpty() && value != m_units) { m_units = value; m_modified = true; }
}
const QVector<Feature>& Document::features() const { return m_features; }
void Document::addFeature(const Feature& value) { m_features.push_back(value); m_modified = true; }
void Document::removeLastFeature() { if (!m_features.isEmpty()) { m_features.removeLast(); m_modified = true; } }
bool Document::isModified() const { return m_modified; }
void Document::markSaved() { m_modified = false; }

QJsonObject Document::toJson() const {
    QJsonArray features;
    for (const auto& feature : m_features) features.append(feature.toJson());
    return {{QStringLiteral("schemaVersion"), SchemaVersion},
            {QStringLiteral("application"), QStringLiteral("10derCAE")},
            {QStringLiteral("name"), m_name},
            {QStringLiteral("documentType"), m_documentType},
            {QStringLiteral("activeWorkspace"), workspaceStableId(m_activeWorkspace)},
            {QStringLiteral("units"), m_units},
            {QStringLiteral("features"), features}};
}

Document Document::fromJson(const QJsonObject& object) {
    const int version = object.value(QStringLiteral("schemaVersion")).toInt(-1);
    if (version != SchemaVersion)
        throw std::runtime_error(QStringLiteral("Unsupported .10cae schema version %1 (expected %2).")
                                     .arg(version).arg(SchemaVersion).toStdString());
    if (object.value(QStringLiteral("application")).toString() != QStringLiteral("10derCAE"))
        throw std::runtime_error("The file is not a 10derCAE document.");
    Document result;
    result.m_name = object.value(QStringLiteral("name")).toString(QStringLiteral("Part1"));
    result.m_documentType = object.value(QStringLiteral("documentType")).toString(QStringLiteral("part"));
    result.m_activeWorkspace = workspaceFromStableId(object.value(QStringLiteral("activeWorkspace")).toString());
    result.m_units = object.value(QStringLiteral("units")).toString(QStringLiteral("mm"));
    for (const auto& value : object.value(QStringLiteral("features")).toArray())
        result.m_features.push_back(Feature::fromJson(value.toObject()));
    return result;
}

bool Document::save(const QString& path, QString* error) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { if (error) *error = file.errorString(); return false; }
    if (file.write(QJsonDocument(toJson()).toJson(QJsonDocument::Indented)) < 0 || !file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    markSaved();
    return true;
}

bool Document::load(const QString& path, Document* target, QString* error) {
    if (!target) { if (error) *error = QStringLiteral("No target document supplied."); return false; }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { if (error) *error = file.errorString(); return false; }
    QJsonParseError parseError;
    const auto json = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !json.isObject()) {
        if (error) *error = parseError.errorString();
        return false;
    }
    try { *target = fromJson(json.object()); }
    catch (const std::exception& exception) { if (error) *error = QString::fromUtf8(exception.what()); return false; }
    return true;
}
