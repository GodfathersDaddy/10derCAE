#pragma once

#include "AppTypes.h"
#include "Document.h"

#include <QMainWindow>
#include <memory>

class KernelShape;
class OcctViewport;
enum class AppLanguage;
class QCloseEvent;
class QDockWidget;
class QLabel;
class QStackedWidget;
class QTreeWidget;
class RibbonWidget;
class SketchCanvas;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    WorkspaceId currentWorkspace() const;
    void setWorkspace(WorkspaceId workspace);
    void setLanguage(AppLanguage language);
    void triggerCommand(CommandId command);
    bool dumpViewport(const QString& path) const;
    bool captureWindow(const QString& path) const;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void buildDocks();
    void buildPages();
    QWidget* createStartPage();
    QWidget* createDrawingPage();
    QWidget* createSystemPage();
    void connectRibbon();
    void applyStyle();
    void retranslate();
    void refreshUi();
    void refreshBrowser();
    void refreshProperties();
    void updateWindowTitle();
    void changeLanguage(AppLanguage language);
    void newDocument(WorkspaceId workspace = WorkspaceId::Part);
    void openDocument();
    bool saveDocument(bool choosePath = false);
    void exportShape(const QString& format);
    void createRectangleSketch();
    void beginSketch();
    void finishSketch();
    void applySketchConstraint();
    void applySketchDimension();
    void extrudeSketch();
    void applyEdgeFeature(bool fillet);
    void rebuildModel();
    void commitFeature(const Feature& feature);
    void undo();
    void redo();
    bool confirmDiscard();
    void restoreSettings();
    void saveSettings();

    Document m_document;
    QString m_path;
    QVector<Document> m_undo;
    QVector<Document> m_redo;
    WorkspaceId m_workspace = WorkspaceId::Start;
    RibbonWidget* m_ribbon = nullptr;
    QDockWidget* m_browserDock = nullptr;
    QDockWidget* m_propertiesDock = nullptr;
    QTreeWidget* m_browser = nullptr;
    QTreeWidget* m_properties = nullptr;
    QStackedWidget* m_pages = nullptr;
    QWidget* m_startPage = nullptr;
    QWidget* m_drawingPage = nullptr;
    QWidget* m_systemPage = nullptr;
    OcctViewport* m_viewport = nullptr;
    SketchCanvas* m_sketchCanvas = nullptr;
    QLabel* m_statusMessage = nullptr;
    QLabel* m_statusWorkspace = nullptr;
    QLabel* m_statusUnits = nullptr;
    std::shared_ptr<KernelShape> m_modelShape;
    QString m_activeSketchPlane = QStringLiteral("XY");
};
