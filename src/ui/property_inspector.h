#ifndef PROPERTY_INSPECTOR_H
#define PROPERTY_INSPECTOR_H

#include "types.h"
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QColorDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QFileDialog>
#include <QVector3D>
#include <QQuaternion>

class Entity;
class Component;
class SceneManager;

// Property inspector widget for editing entity and component properties
class PropertyInspector : public QWidget {
    Q_OBJECT
    
public:
    explicit PropertyInspector(QWidget* parent = nullptr);
    
    void setSelectedEntity(Entity* entity);
    void clearSelection();
    void refreshProperties();
    
signals:
    void propertyChanged(EntityId entityId, const QString& propertyName, const QVariant& value);
    void componentAdded(EntityId entityId, ComponentType type);
    void componentRemoved(EntityId entityId, ComponentType type);
    
private slots:
    void onSelectionChanged(const QVector<EntityId>& selectedIds);
    void onEntityPropertyChanged();
    void onComponentPropertyChanged();
    void onAddComponentClicked();
    void onRemoveComponentClicked();
    
private:
    // UI creation
    void setupUI();
    void createEntitySection();
    void createComponentSections();
    void createAddComponentSection();
    
    // Component UI creation
    QWidget* createTransformComponentUI(class TransformComponent* component);
    QWidget* createMeshComponentUI(class MeshComponent* component);
    QWidget* createLightComponentUI(class LightComponent* component);
    QWidget* createScriptComponentUI(class ScriptComponent* component);
    
    // Property widgets
    QWidget* createVector3DProperty(const QString& name, const QVector3D& value, 
                                   std::function<void(const QVector3D&)> callback);
    QWidget* createQuaternionProperty(const QString& name, const QQuaternion& value,
                                     std::function<void(const QQuaternion&)> callback);
    QWidget* createFloatProperty(const QString& name, float value, float min = -1000.0f, float max = 1000.0f,
                                std::function<void(float)> callback = nullptr);
    QWidget* createIntProperty(const QString& name, int value, int min = -1000, int max = 1000,
                              std::function<void(int)> callback = nullptr);
    QWidget* createBoolProperty(const QString& name, bool value,
                               std::function<void(bool)> callback = nullptr);
    QWidget* createStringProperty(const QString& name, const QString& value,
                                 std::function<void(const QString&)> callback = nullptr);
    QWidget* createFileProperty(const QString& name, const QString& value, const QString& filter,
                               std::function<void(const QString&)> callback = nullptr);
    QWidget* createColorProperty(const QString& name, const QVector3D& color,
                                std::function<void(const QVector3D&)> callback = nullptr);
    QWidget* createEnumProperty(const QString& name, int value, const QStringList& options,
                               std::function<void(int)> callback = nullptr);
    
    // Utility
    void clearLayout(QLayout* layout);
    void connectPropertySignals();
    void disconnectPropertySignals();
    QString componentTypeToString(ComponentType type) const;
    QIcon componentTypeToIcon(ComponentType type) const;
    
    // UI components
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_mainLayout;
    
    // Entity section
    QGroupBox* m_entityGroup;
    QLineEdit* m_entityNameEdit;
    QLabel* m_entityIdLabel;
    
    // Component sections
    QVBoxLayout* m_componentsLayout;
    QMap<ComponentType, QGroupBox*> m_componentGroups;
    QMap<ComponentType, QWidget*> m_componentWidgets;
    
    // Add component section
    QGroupBox* m_addComponentGroup;
    QComboBox* m_componentTypeCombo;
    QPushButton* m_addComponentButton;
    
    // Current state
    Entity* m_currentEntity;
    SceneManager* m_sceneManager;
    
    // Property change tracking
    bool m_updatingProperties;
    QMap<QWidget*, std::function<void()>> m_propertyCallbacks;
};

// Custom widget for Vector3D editing
class Vector3DWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit Vector3DWidget(const QVector3D& value = QVector3D(), QWidget* parent = nullptr);
    
    QVector3D getValue() const;
    void setValue(const QVector3D& value);
    
signals:
    void valueChanged(const QVector3D& value);
    
private slots:
    void onValueChanged();
    
private:
    QDoubleSpinBox* m_xSpinBox;
    QDoubleSpinBox* m_ySpinBox;
    QDoubleSpinBox* m_zSpinBox;
    bool m_updating;
};

// Custom widget for Quaternion editing (as Euler angles)
class QuaternionWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit QuaternionWidget(const QQuaternion& value = QQuaternion(), QWidget* parent = nullptr);
    
    QQuaternion getValue() const;
    void setValue(const QQuaternion& value);
    
signals:
    void valueChanged(const QQuaternion& value);
    
private slots:
    void onValueChanged();
    
private:
    QDoubleSpinBox* m_pitchSpinBox;
    QDoubleSpinBox* m_yawSpinBox;
    QDoubleSpinBox* m_rollSpinBox;
    bool m_updating;
};

// Custom widget for color editing
class ColorWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ColorWidget(const QVector3D& color = QVector3D(1, 1, 1), QWidget* parent = nullptr);
    
    QVector3D getColor() const;
    void setColor(const QVector3D& color);
    
signals:
    void colorChanged(const QVector3D& color);
    
private slots:
    void onColorButtonClicked();
    void onSpinBoxChanged();
    
private:
    void updateColorButton();
    void updateSpinBoxes();
    
    QPushButton* m_colorButton;
    QDoubleSpinBox* m_rSpinBox;
    QDoubleSpinBox* m_gSpinBox;
    QDoubleSpinBox* m_bSpinBox;
    QVector3D m_color;
    bool m_updating;
};

#endif // PROPERTY_INSPECTOR_H

