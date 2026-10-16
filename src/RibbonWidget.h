#pragma once

#include "AppTypes.h"
#include "I18n.h"

#include <QWidget>

class QComboBox;
class QHBoxLayout;
class QTabWidget;
class QToolButton;

class RibbonWidget final : public QWidget {
    Q_OBJECT
public:
    explicit RibbonWidget(QWidget* parent = nullptr);
    void setWorkspace(WorkspaceId workspace);
    WorkspaceId workspace() const;
    void retranslate();

signals:
    void workspaceChanged(WorkspaceId workspace);
    void commandTriggered(CommandId command);
    void newRequested();
    void openRequested();
    void saveRequested();
    void saveAsRequested();
    void exportStepRequested();
    void exportStlRequested();
    void undoRequested();
    void redoRequested();
    void languageChanged(AppLanguage language);

private:
    void build();
    void buildFileMenu();
    QWidget* makeWorkspacePage(const WorkspaceDefinition& definition);
    QToolButton* quickButton(const QString& icon, const QString& tooltip);
    QString iconPath(const QString& name) const;

    QHBoxLayout* m_headerLayout = nullptr;
    QToolButton* m_fileButton = nullptr;
    QToolButton* m_undoButton = nullptr;
    QToolButton* m_redoButton = nullptr;
    QComboBox* m_language = nullptr;
    QTabWidget* m_tabs = nullptr;
};
