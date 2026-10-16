#pragma once

#include "AppTypes.h"

#include <QJsonObject>
#include <QString>
#include <QVector>

struct Feature {
    QString id;
    QString type;
    QString name;
    QJsonObject parameters;

    QJsonObject toJson() const;
    static Feature fromJson(const QJsonObject& object);
    bool operator==(const Feature&) const = default;
};

class Document {
public:
    static constexpr int SchemaVersion = 2;

    QString name() const;
    void setName(const QString& name);
    QString documentType() const;
    void setDocumentType(const QString& type);
    WorkspaceId activeWorkspace() const;
    void setActiveWorkspace(WorkspaceId workspace, bool markModified = false);
    QString units() const;
    void setUnits(const QString& units);
    const QVector<Feature>& features() const;
    void addFeature(const Feature& feature);
    void removeLastFeature();
    bool isModified() const;
    void markSaved();

    QJsonObject toJson() const;
    static Document fromJson(const QJsonObject& object);
    bool save(const QString& path, QString* error = nullptr);
    static bool load(const QString& path, Document* target, QString* error = nullptr);

private:
    QString m_name = QStringLiteral("Part1");
    QString m_documentType = QStringLiteral("part");
    WorkspaceId m_activeWorkspace = WorkspaceId::Part;
    QString m_units = QStringLiteral("mm");
    QVector<Feature> m_features;
    bool m_modified = false;
};
