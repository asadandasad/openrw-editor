#ifndef WORLD_OUTLINER_H
#define WORLD_OUTLINER_H

#include "types.h"
#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QHeaderView>

class Entity;
class SceneManager;

// World outliner widget for displaying scene hierarchy
class WorldOutliner : public QWidget {
    Q_OBJECT
    
public:
    explicit WorldOutliner(QWidget* parent = nullptr);
    
    void refreshHierarchy();
    void selectEntity(EntityId id);
    void selectEntities(const QVector<EntityId>& ids);
    void clearSelection();
    
    void setShowLayers(bool show);
    bool isShowLayers() const;
    
    void setShowComponents(bool show);
    bool isShowComponents() const;
    
signals:
    void entitySelected(EntityId id);
    void entitiesSelected(const QVector<EntityId>& ids);
    void entityRenamed(EntityId id, const QString& newName);
    void entityParentChanged(EntityId id, EntityId newParentId);
    void layerVisibilityChanged(const QString& layer, bool visible);
    void layerLockChanged(const QString& layer, bool locked);
    void contextMenuRequested(EntityId id, const QPoint& position);
    
private slots:
    void onSceneChanged();
    void onSelectionChanged(const QVector<EntityId>& selectedIds);
    void onEntityCreated(Entity* entity);
    void onEntityDestroyed(EntityId id);
    void onLayerCreated(const QString& name);
    void onLayerDeleted(const QString& name);
    void onLayerVisibilityChanged(const QString& name, bool visible);
    void onLayerLockChanged(const QString& name, bool locked);
    
    void onItemSelectionChanged();
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onCustomContextMenuRequested(const QPoint& position);
    void onSearchTextChanged(const QString& text);
    void onFilterChanged(int index);
    void onExpandAllClicked();
    void onCollapseAllClicked();
    
    // Context menu actions
    void onCreateEntityAction();
    void onDeleteEntityAction();
    void onDuplicateEntityAction();
    void onRenameEntityAction();
    void onFocusEntityAction();
    void onCreateLayerAction();
    void onDeleteLayerAction();
    void onRenameLayerAction();
    
private:
    // UI setup
    void setupUI();
    void setupToolbar();
    void setupTreeWidget();
    void setupContextMenu();
    
    // Hierarchy management
    void buildHierarchy();
    void buildLayerHierarchy();
    void buildEntityHierarchy();
    void addEntityToTree(Entity* entity, QTreeWidgetItem* parent = nullptr);
    void addLayerToTree(const QString& layerName);
    void addComponentsToEntity(QTreeWidgetItem* entityItem, Entity* entity);
    
    // Tree item management
    QTreeWidgetItem* findEntityItem(EntityId id) const;
    QTreeWidgetItem* findLayerItem(const QString& layerName) const;
    EntityId getEntityIdFromItem(QTreeWidgetItem* item) const;
    QString getLayerNameFromItem(QTreeWidgetItem* item) const;
    
    // Filtering and searching
    void applyFilters();
    bool matchesFilter(QTreeWidgetItem* item) const;
    bool matchesSearch(QTreeWidgetItem* item, const QString& searchText) const;
    void setItemVisibility(QTreeWidgetItem* item, bool visible);
    
    // Utility
    QIcon getEntityIcon(Entity* entity) const;
    QIcon getComponentIcon(ComponentType type) const;
    QIcon getLayerIcon() const;
    QString getEntityDisplayName(Entity* entity) const;
    QString getComponentDisplayName(ComponentType type) const;
    
    // UI components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    
    // Toolbar
    QLineEdit* m_searchEdit;
    QComboBox* m_filterCombo;
    QPushButton* m_expandAllButton;
    QPushButton* m_collapseAllButton;
    QLabel* m_statusLabel;
    
    // Tree widget
    QTreeWidget* m_treeWidget;
    
    // Context menu
    QMenu* m_contextMenu;
    QMenu* m_entityContextMenu;
    QMenu* m_layerContextMenu;
    
    // Actions
    QAction* m_createEntityAction;
    QAction* m_deleteEntityAction;
    QAction* m_duplicateEntityAction;
    QAction* m_renameEntityAction;
    QAction* m_focusEntityAction;
    QAction* m_createLayerAction;
    QAction* m_deleteLayerAction;
    QAction* m_renameLayerAction;
    
    // Data
    SceneManager* m_sceneManager;
    bool m_showLayers;
    bool m_showComponents;
    bool m_updatingSelection;
    
    // Tree item types
    enum ItemType {
        EntityItem,
        LayerItem,
        ComponentItem
    };
    
    // Custom data roles
    enum DataRole {
        ItemTypeRole = Qt::UserRole + 1,
        EntityIdRole,
        LayerNameRole,
        ComponentTypeRole
    };
};

// Custom tree widget item for entities
class EntityTreeItem : public QTreeWidgetItem {
public:
    EntityTreeItem(Entity* entity, QTreeWidget* parent = nullptr);
    EntityTreeItem(Entity* entity, QTreeWidgetItem* parent = nullptr);
    
    Entity* getEntity() const;
    EntityId getEntityId() const;
    
    void updateFromEntity();
    
private:
    void setupItem();
    
    Entity* m_entity;
};

// Custom tree widget item for layers
class LayerTreeItem : public QTreeWidgetItem {
public:
    LayerTreeItem(const QString& layerName, QTreeWidget* parent = nullptr);
    
    QString getLayerName() const;
    
    void setVisible(bool visible);
    void setLocked(bool locked);
    
private:
    void setupItem();
    
    QString m_layerName;
};

// Custom tree widget item for components
class ComponentTreeItem : public QTreeWidgetItem {
public:
    ComponentTreeItem(ComponentType type, QTreeWidgetItem* parent = nullptr);
    
    ComponentType getComponentType() const;
    
private:
    void setupItem();
    
    ComponentType m_componentType;
};

#endif // WORLD_OUTLINER_H

