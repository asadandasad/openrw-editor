#ifndef MISSION_EDITOR_H
#define MISSION_EDITOR_H

#include "types.h"
#include "mission_node.h"
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QSplitter>
#include <QListWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QGroupBox>
#include <QPushButton>

class MissionNode;
class MissionConnection;
class SceneManager;

// Mission editor widget for visual scripting and mission design
class MissionEditor : public QWidget {
    Q_OBJECT
    
public:
    explicit MissionEditor(QWidget* parent = nullptr);
    
    void newMission();
    bool loadMission(const QString& filePath);
    bool saveMission(const QString& filePath);
    
    void setCurrentMission(const QString& missionId);
    QString getCurrentMission() const;
    
    void addNode(MissionNodeType type, const QPointF& position);
    void removeNode(MissionNode* node);
    void connectNodes(MissionNode* from, MissionNode* to, const QString& outputPin, const QString& inputPin);
    void disconnectNodes(MissionConnection* connection);
    
    void validateMission();
    void testMission();
    
signals:
    void missionChanged();
    void missionValidated(bool isValid, const QStringList& errors);
    void nodeSelected(MissionNode* node);
    void nodeDeselected(MissionNode* node);
    void connectionSelected(MissionConnection* connection);
    
private slots:
    void onNodePaletteItemClicked(QListWidgetItem* item);
    void onNodeSelected();
    void onNodeDeselected();
    void onConnectionSelected();
    void onSceneSelectionChanged();
    void onNodePropertyChanged();
    void onAddObjectiveClicked();
    void onRemoveObjectiveClicked();
    void onTestMissionClicked();
    void onValidateMissionClicked();
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onFitToView();
    
private:
    // UI setup
    void setupUI();
    void setupToolBar();
    void setupNodePalette();
    void setupGraphicsView();
    void setupPropertiesPanel();
    void setupObjectivesPanel();
    
    // Node management
    void populateNodePalette();
    MissionNode* createNode(MissionNodeType type);
    void updateNodeProperties(MissionNode* node);
    void clearNodeProperties();
    
    // Mission management
    void clearMission();
    void updateMissionInfo();
    
    // Serialization
    QVariantMap serializeMission() const;
    void deserializeMission(const QVariantMap& data);
    
    // Validation
    QStringList validateMissionLogic() const;
    bool hasValidStartNode() const;
    bool hasValidEndNode() const;
    bool areAllNodesConnected() const;
    bool hasCircularDependencies() const;
    
    // UI components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    
    // Toolbar
    QToolBar* m_toolBar;
    QAction* m_newMissionAction;
    QAction* m_loadMissionAction;
    QAction* m_saveMissionAction;
    QAction* m_validateAction;
    QAction* m_testAction;
    QAction* m_zoomInAction;
    QAction* m_zoomOutAction;
    QAction* m_zoomResetAction;
    QAction* m_fitToViewAction;
    QComboBox* m_missionCombo;
    
    // Node palette
    QGroupBox* m_nodePaletteGroup;
    QListWidget* m_nodePalette;
    
    // Graphics view
    QGraphicsView* m_graphicsView;
    QGraphicsScene* m_graphicsScene;
    
    // Properties panel
    QGroupBox* m_propertiesGroup;
    QWidget* m_propertiesWidget;
    QVBoxLayout* m_propertiesLayout;
    
    // Objectives panel
    QGroupBox* m_objectivesGroup;
    QTreeWidget* m_objectivesTree;
    QPushButton* m_addObjectiveButton;
    QPushButton* m_removeObjectiveButton;
    
    // Mission info panel
    QGroupBox* m_missionInfoGroup;
    QLineEdit* m_missionNameEdit;
    QTextEdit* m_missionDescriptionEdit;
    
    // Data
    SceneManager* m_sceneManager;
    QString m_currentMissionId;
    QString m_currentMissionFile;
    QVector<MissionNode*> m_nodes;
    QVector<MissionConnection*> m_connections;
    MissionNode* m_selectedNode;
    MissionConnection* m_selectedConnection;
    
    // Node creation state
    bool m_isCreatingConnection;
    MissionNode* m_connectionStartNode;
    QString m_connectionStartPin;
    
    // Mission data
    struct MissionInfo {
        QString id;
        QString name;
        QString description;
        QVector<MissionObjective> objectives;
        QVariantMap properties;
    };
    
    MissionInfo m_missionInfo;
    QMap<QString, MissionInfo> m_missions;
};

// Custom graphics view for mission editor
class MissionGraphicsView : public QGraphicsView {
    Q_OBJECT
    
public:
    explicit MissionGraphicsView(QWidget* parent = nullptr);
    
    void setMissionEditor(MissionEditor* editor);
    
signals:
    void nodeDropRequested(MissionNodeType type, const QPointF& position);
    void backgroundClicked(const QPointF& position);
    
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    
private:
    void showContextMenu(const QPoint& position);
    
    MissionEditor* m_missionEditor;
    bool m_isPanning;
    QPoint m_lastPanPoint;
};

// Node palette item for drag and drop
class NodePaletteItem : public QListWidgetItem {
public:
    NodePaletteItem(MissionNodeType type, const QString& name, const QString& description, QListWidget* parent = nullptr);
    
    MissionNodeType getNodeType() const;
    QString getDescription() const;
    
private:
    MissionNodeType m_nodeType;
    QString m_description;
};

#endif // MISSION_EDITOR_H

