#include "property_inspector.h"
#include "entity_system.h"
#include "scene_manager.h"
#include "math_utils.h"
#include <QDebug>
#include <QApplication>
#include <QStyle>

PropertyInspector::PropertyInspector(QWidget* parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_entityGroup(nullptr)
    , m_entityNameEdit(nullptr)
    , m_entityIdLabel(nullptr)
    , m_componentsLayout(nullptr)
    , m_addComponentGroup(nullptr)
    , m_componentTypeCombo(nullptr)
    , m_addComponentButton(nullptr)
    , m_currentEntity(nullptr)
    , m_sceneManager(&SceneManager::instance())
    , m_updatingProperties(false)
{
    setupUI();
    
    // Connect to scene manager
    connect(m_sceneManager, &SceneManager::selectionChanged, this, &PropertyInspector::onSelectionChanged);
}

void PropertyInspector::setSelectedEntity(Entity* entity) {
    if (m_currentEntity == entity) {
        return;
    }
    
    disconnectPropertySignals();
    m_currentEntity = entity;
    
    if (m_currentEntity) {
        connectPropertySignals();
        refreshProperties();
    } else {
        clearSelection();
    }
}

void PropertyInspector::clearSelection() {
    disconnectPropertySignals();
    m_currentEntity = nullptr;
    
    // Clear entity section
    if (m_entityNameEdit) {
        m_entityNameEdit->clear();
    }
    if (m_entityIdLabel) {
        m_entityIdLabel->setText("ID: -");
    }
    
    // Clear component sections
    clearLayout(m_componentsLayout);
    m_componentGroups.clear();
    m_componentWidgets.clear();
    
    // Disable add component section
    if (m_addComponentGroup) {
        m_addComponentGroup->setEnabled(false);
    }
}

void PropertyInspector::refreshProperties() {
    if (!m_currentEntity) {
        clearSelection();
        return;
    }
    
    m_updatingProperties = true;
    
    // Update entity section
    if (m_entityNameEdit) {
        m_entityNameEdit->setText(m_currentEntity->getName());
    }
    if (m_entityIdLabel) {
        m_entityIdLabel->setText(QString("ID: %1").arg(m_currentEntity->getId()));
    }
    
    // Clear and recreate component sections
    clearLayout(m_componentsLayout);
    m_componentGroups.clear();
    m_componentWidgets.clear();
    
    createComponentSections();
    
    // Enable add component section
    if (m_addComponentGroup) {
        m_addComponentGroup->setEnabled(true);
    }
    
    m_updatingProperties = false;
}

void PropertyInspector::onSelectionChanged(const QVector<EntityId>& selectedIds) {
    if (selectedIds.isEmpty()) {
        setSelectedEntity(nullptr);
    } else {
        // Show properties for the first selected entity
        Entity* entity = m_sceneManager->getEntity(selectedIds.first());
        setSelectedEntity(entity);
    }
}

void PropertyInspector::onEntityPropertyChanged() {
    if (m_updatingProperties || !m_currentEntity) {
        return;
    }
    
    QLineEdit* nameEdit = qobject_cast<QLineEdit*>(sender());
    if (nameEdit == m_entityNameEdit) {
        m_currentEntity->setName(nameEdit->text());
        emit propertyChanged(m_currentEntity->getId(), "name", nameEdit->text());
    }
}

void PropertyInspector::onComponentPropertyChanged() {
    if (m_updatingProperties || !m_currentEntity) {
        return;
    }
    
    // Property changes are handled by individual component widgets
    // This is called when any component property changes
    emit propertyChanged(m_currentEntity->getId(), "component", QVariant());
}

void PropertyInspector::onAddComponentClicked() {
    if (!m_currentEntity || !m_componentTypeCombo) {
        return;
    }
    
    int index = m_componentTypeCombo->currentIndex();
    if (index < 0) {
        return;
    }
    
    ComponentType type = static_cast<ComponentType>(m_componentTypeCombo->currentData().toInt());
    
    // Check if component already exists
    switch (type) {
        case ComponentType::Transform:
            if (m_currentEntity->hasComponent<TransformComponent>()) return;
            m_currentEntity->addComponent<TransformComponent>();
            break;
        case ComponentType::Mesh:
            if (m_currentEntity->hasComponent<MeshComponent>()) return;
            m_currentEntity->addComponent<MeshComponent>();
            break;
        case ComponentType::Light:
            if (m_currentEntity->hasComponent<LightComponent>()) return;
            m_currentEntity->addComponent<LightComponent>();
            break;
        case ComponentType::Script:
            if (m_currentEntity->hasComponent<ScriptComponent>()) return;
            m_currentEntity->addComponent<ScriptComponent>();
            break;
        default:
            return;
    }
    
    emit componentAdded(m_currentEntity->getId(), type);
    refreshProperties();
}

void PropertyInspector::onRemoveComponentClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button || !m_currentEntity) {
        return;
    }
    
    ComponentType type = static_cast<ComponentType>(button->property("componentType").toInt());
    
    // Don't allow removing Transform component
    if (type == ComponentType::Transform) {
        return;
    }
    
    switch (type) {
        case ComponentType::Mesh:
            m_currentEntity->removeComponent<MeshComponent>();
            break;
        case ComponentType::Light:
            m_currentEntity->removeComponent<LightComponent>();
            break;
        case ComponentType::Script:
            m_currentEntity->removeComponent<ScriptComponent>();
            break;
        default:
            return;
    }
    
    emit componentRemoved(m_currentEntity->getId(), type);
    refreshProperties();
}

void PropertyInspector::setupUI() {
    setMinimumWidth(300);
    
    // Create main layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    
    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Create content widget
    m_contentWidget = new QWidget();
    m_mainLayout = new QVBoxLayout(m_contentWidget);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(10);
    
    // Create sections
    createEntitySection();
    
    // Components layout
    m_componentsLayout = new QVBoxLayout();
    m_mainLayout->addLayout(m_componentsLayout);
    
    createAddComponentSection();
    
    // Add stretch to push everything to the top
    m_mainLayout->addStretch();
    
    m_scrollArea->setWidget(m_contentWidget);
    layout->addWidget(m_scrollArea);
    
    // Initially disabled
    setEnabled(false);
}

void PropertyInspector::createEntitySection() {
    m_entityGroup = new QGroupBox("Entity", m_contentWidget);
    QVBoxLayout* entityLayout = new QVBoxLayout(m_entityGroup);
    
    // Entity ID (read-only)
    m_entityIdLabel = new QLabel("ID: -");
    m_entityIdLabel->setStyleSheet("color: gray; font-size: 10px;");
    entityLayout->addWidget(m_entityIdLabel);
    
    // Entity name
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Name:"));
    m_entityNameEdit = new QLineEdit();
    connect(m_entityNameEdit, &QLineEdit::textChanged, this, &PropertyInspector::onEntityPropertyChanged);
    nameLayout->addWidget(m_entityNameEdit);
    entityLayout->addLayout(nameLayout);
    
    m_mainLayout->addWidget(m_entityGroup);
}

void PropertyInspector::createComponentSections() {
    if (!m_currentEntity) {
        return;
    }
    
    QVector<Component*> components = m_currentEntity->getAllComponents();
    
    for (Component* component : components) {
        if (!component) continue;
        
        ComponentType type = component->getType();
        QString typeName = componentTypeToString(type);
        
        QGroupBox* group = new QGroupBox(typeName, m_contentWidget);
        QVBoxLayout* groupLayout = new QVBoxLayout(group);
        
        // Add remove button (except for Transform)
        if (type != ComponentType::Transform) {
            QHBoxLayout* headerLayout = new QHBoxLayout();
            headerLayout->addStretch();
            
            QPushButton* removeButton = new QPushButton("Remove");
            removeButton->setMaximumWidth(60);
            removeButton->setProperty("componentType", static_cast<int>(type));
            connect(removeButton, &QPushButton::clicked, this, &PropertyInspector::onRemoveComponentClicked);
            headerLayout->addWidget(removeButton);
            
            groupLayout->addLayout(headerLayout);
        }
        
        // Create component-specific UI
        QWidget* componentWidget = nullptr;
        switch (type) {
            case ComponentType::Transform:
                componentWidget = createTransformComponentUI(static_cast<TransformComponent*>(component));
                break;
            case ComponentType::Mesh:
                componentWidget = createMeshComponentUI(static_cast<MeshComponent*>(component));
                break;
            case ComponentType::Light:
                componentWidget = createLightComponentUI(static_cast<LightComponent*>(component));
                break;
            case ComponentType::Script:
                componentWidget = createScriptComponentUI(static_cast<ScriptComponent*>(component));
                break;
            default:
                break;
        }
        
        if (componentWidget) {
            groupLayout->addWidget(componentWidget);
            m_componentWidgets[type] = componentWidget;
        }
        
        m_componentGroups[type] = group;
        m_componentsLayout->addWidget(group);
    }
}

void PropertyInspector::createAddComponentSection() {
    m_addComponentGroup = new QGroupBox("Add Component", m_contentWidget);
    QHBoxLayout* addLayout = new QHBoxLayout(m_addComponentGroup);
    
    m_componentTypeCombo = new QComboBox();
    m_componentTypeCombo->addItem("Mesh", static_cast<int>(ComponentType::Mesh));
    m_componentTypeCombo->addItem("Light", static_cast<int>(ComponentType::Light));
    m_componentTypeCombo->addItem("Script", static_cast<int>(ComponentType::Script));
    
    m_addComponentButton = new QPushButton("Add");
    connect(m_addComponentButton, &QPushButton::clicked, this, &PropertyInspector::onAddComponentClicked);
    
    addLayout->addWidget(m_componentTypeCombo);
    addLayout->addWidget(m_addComponentButton);
    
    m_mainLayout->addWidget(m_addComponentGroup);
}

QWidget* PropertyInspector::createTransformComponentUI(TransformComponent* component) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Position
    QWidget* posWidget = createVector3DProperty("Position", component->transform.position,
        [this, component](const QVector3D& value) {
            component->transform.position = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(posWidget);
    
    // Rotation (as Euler angles)
    QWidget* rotWidget = createQuaternionProperty("Rotation", component->transform.rotation,
        [this, component](const QQuaternion& value) {
            component->transform.rotation = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(rotWidget);
    
    // Scale
    QWidget* scaleWidget = createVector3DProperty("Scale", component->transform.scale,
        [this, component](const QVector3D& value) {
            component->transform.scale = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(scaleWidget);
    
    return widget;
}

QWidget* PropertyInspector::createMeshComponentUI(MeshComponent* component) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Mesh path
    QWidget* meshWidget = createFileProperty("Mesh", component->meshPath, "DFF Files (*.dff)",
        [this, component](const QString& value) {
            component->meshPath = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(meshWidget);
    
    // Material path
    QWidget* materialWidget = createFileProperty("Material", component->materialPath, "TXD Files (*.txd)",
        [this, component](const QString& value) {
            component->materialPath = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(materialWidget);
    
    // Visible
    QWidget* visibleWidget = createBoolProperty("Visible", component->isVisible,
        [this, component](bool value) {
            component->isVisible = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(visibleWidget);
    
    return widget;
}

QWidget* PropertyInspector::createLightComponentUI(LightComponent* component) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Light type
    QStringList lightTypes = {"Directional", "Point", "Spot"};
    QWidget* typeWidget = createEnumProperty("Type", static_cast<int>(component->lightType), lightTypes,
        [this, component](int value) {
            component->lightType = static_cast<LightComponent::LightType>(value);
            onComponentPropertyChanged();
        });
    layout->addWidget(typeWidget);
    
    // Color
    QWidget* colorWidget = createColorProperty("Color", component->color,
        [this, component](const QVector3D& value) {
            component->color = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(colorWidget);
    
    // Intensity
    QWidget* intensityWidget = createFloatProperty("Intensity", component->intensity, 0.0f, 10.0f,
        [this, component](float value) {
            component->intensity = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(intensityWidget);
    
    // Range
    QWidget* rangeWidget = createFloatProperty("Range", component->range, 0.1f, 100.0f,
        [this, component](float value) {
            component->range = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(rangeWidget);
    
    // Cast shadows
    QWidget* shadowsWidget = createBoolProperty("Cast Shadows", component->castShadows,
        [this, component](bool value) {
            component->castShadows = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(shadowsWidget);
    
    return widget;
}

QWidget* PropertyInspector::createScriptComponentUI(ScriptComponent* component) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Script path
    QWidget* scriptWidget = createFileProperty("Script", component->scriptPath, "Script Files (*.lua *.as)",
        [this, component](const QString& value) {
            component->scriptPath = value;
            onComponentPropertyChanged();
        });
    layout->addWidget(scriptWidget);
    
    return widget;
}

QWidget* PropertyInspector::createVector3DProperty(const QString& name, const QVector3D& value,
                                                  std::function<void(const QVector3D&)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    label->setMinimumWidth(80);
    layout->addWidget(label);
    
    Vector3DWidget* vector3DWidget = new Vector3DWidget(value);
    if (callback) {
        connect(vector3DWidget, &Vector3DWidget::valueChanged, callback);
    }
    layout->addWidget(vector3DWidget);
    
    return widget;
}

QWidget* PropertyInspector::createQuaternionProperty(const QString& name, const QQuaternion& value,
                                                    std::function<void(const QQuaternion&)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    label->setMinimumWidth(80);
    layout->addWidget(label);
    
    QuaternionWidget* quaternionWidget = new QuaternionWidget(value);
    if (callback) {
        connect(quaternionWidget, &QuaternionWidget::valueChanged, callback);
    }
    layout->addWidget(quaternionWidget);
    
    return widget;
}

QWidget* PropertyInspector::createFloatProperty(const QString& name, float value, float min, float max,
                                               std::function<void(float)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    label->setMinimumWidth(80);
    layout->addWidget(label);
    
    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setRange(min, max);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(3);
    spinBox->setValue(value);
    
    if (callback) {
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [callback](double value) { callback(static_cast<float>(value)); });
    }
    
    layout->addWidget(spinBox);
    
    return widget;
}

QWidget* PropertyInspector::createIntProperty(const QString& name, int value, int min, int max,
                                             std::function<void(int)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    label->setMinimumWidth(80);
    layout->addWidget(label);
    
    QSpinBox* spinBox = new QSpinBox();
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    
    if (callback) {
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), callback);
    }
    
    layout->addWidget(spinBox);
    
    return widget;
}

QWidget* PropertyInspector::createBoolProperty(const QString& name, bool value,
                                              std::function<void(bool)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QCheckBox* checkBox = new QCheckBox(name);
    checkBox->setChecked(value);
    
    if (callback) {
        connect(checkBox, &QCheckBox::toggled, callback);
    }
    
    layout->addWidget(checkBox);
    layout->addStretch();
    
    return widget;
}

QWidget* PropertyInspector::createStringProperty(const QString& name, const QString& value,
                                                std::function<void(const QString&)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    label->setMinimumWidth(80);
    layout->addWidget(label);
    
    QLineEdit* lineEdit = new QLineEdit(value);
    
    if (callback) {
        connect(lineEdit, &QLineEdit::textChanged, callback);
    }
    
    layout->addWidget(lineEdit);
    
    return widget;
}

QWidget* PropertyInspector::createFileProperty(const QString& name, const QString& value, const QString& filter,
                                              std::function<void(const QString&)> callback) {
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    layout->addWidget(label);
    
    QHBoxLayout* fileLayout = new QHBoxLayout();
    QLineEdit* lineEdit = new QLineEdit(value);
    QPushButton* browseButton = new QPushButton("...");
    browseButton->setMaximumWidth(30);
    
    fileLayout->addWidget(lineEdit);
    fileLayout->addWidget(browseButton);
    layout->addLayout(fileLayout);
    
    if (callback) {
        connect(lineEdit, &QLineEdit::textChanged, callback);
        connect(browseButton, &QPushButton::clicked, [lineEdit, filter, callback]() {
            QString fileName = QFileDialog::getOpenFileName(lineEdit, "Select File", lineEdit->text(), filter);
            if (!fileName.isEmpty()) {
                lineEdit->setText(fileName);
                if (callback) callback(fileName);
            }
        });
    }
    
    return widget;
}

QWidget* PropertyInspector::createColorProperty(const QString& name, const QVector3D& color,
                                               std::function<void(const QVector3D&)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    label->setMinimumWidth(80);
    layout->addWidget(label);
    
    ColorWidget* colorWidget = new ColorWidget(color);
    if (callback) {
        connect(colorWidget, &ColorWidget::colorChanged, callback);
    }
    layout->addWidget(colorWidget);
    
    return widget;
}

QWidget* PropertyInspector::createEnumProperty(const QString& name, int value, const QStringList& options,
                                              std::function<void(int)> callback) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* label = new QLabel(name + ":");
    label->setMinimumWidth(80);
    layout->addWidget(label);
    
    QComboBox* comboBox = new QComboBox();
    comboBox->addItems(options);
    comboBox->setCurrentIndex(value);
    
    if (callback) {
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), callback);
    }
    
    layout->addWidget(comboBox);
    
    return widget;
}

void PropertyInspector::clearLayout(QLayout* layout) {
    if (!layout) return;
    
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
        }
        delete item;
    }
}

void PropertyInspector::connectPropertySignals() {
    if (m_currentEntity) {
        connect(m_currentEntity, &Entity::nameChanged, this, &PropertyInspector::refreshProperties);
        connect(m_currentEntity, &Entity::componentAdded, this, &PropertyInspector::refreshProperties);
        connect(m_currentEntity, &Entity::componentRemoved, this, &PropertyInspector::refreshProperties);
    }
}

void PropertyInspector::disconnectPropertySignals() {
    if (m_currentEntity) {
        disconnect(m_currentEntity, nullptr, this, nullptr);
    }
}

QString PropertyInspector::componentTypeToString(ComponentType type) const {
    switch (type) {
        case ComponentType::Transform: return "Transform";
        case ComponentType::Mesh: return "Mesh";
        case ComponentType::Light: return "Light";
        case ComponentType::Script: return "Script";
        case ComponentType::Trigger: return "Trigger";
        case ComponentType::Vehicle: return "Vehicle";
        case ComponentType::NPC: return "NPC";
        case ComponentType::Prop: return "Prop";
        case ComponentType::Sound: return "Sound";
        default: return "Unknown";
    }
}

QIcon PropertyInspector::componentTypeToIcon(ComponentType type) const {
    // Return appropriate icons for each component type
    // For now, use standard icons
    switch (type) {
        case ComponentType::Transform:
            return QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
        case ComponentType::Mesh:
            return QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
        case ComponentType::Light:
            return QApplication::style()->standardIcon(QStyle::SP_DialogYesButton);
        case ComponentType::Script:
            return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
        default:
            return QIcon();
    }
}

// Vector3DWidget implementation
Vector3DWidget::Vector3DWidget(const QVector3D& value, QWidget* parent)
    : QWidget(parent), m_updating(false) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    
    // X component
    layout->addWidget(new QLabel("X:"));
    m_xSpinBox = new QDoubleSpinBox();
    m_xSpinBox->setRange(-10000.0, 10000.0);
    m_xSpinBox->setSingleStep(0.1);
    m_xSpinBox->setDecimals(3);
    m_xSpinBox->setValue(value.x());
    connect(m_xSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector3DWidget::onValueChanged);
    layout->addWidget(m_xSpinBox);
    
    // Y component
    layout->addWidget(new QLabel("Y:"));
    m_ySpinBox = new QDoubleSpinBox();
    m_ySpinBox->setRange(-10000.0, 10000.0);
    m_ySpinBox->setSingleStep(0.1);
    m_ySpinBox->setDecimals(3);
    m_ySpinBox->setValue(value.y());
    connect(m_ySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector3DWidget::onValueChanged);
    layout->addWidget(m_ySpinBox);
    
    // Z component
    layout->addWidget(new QLabel("Z:"));
    m_zSpinBox = new QDoubleSpinBox();
    m_zSpinBox->setRange(-10000.0, 10000.0);
    m_zSpinBox->setSingleStep(0.1);
    m_zSpinBox->setDecimals(3);
    m_zSpinBox->setValue(value.z());
    connect(m_zSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Vector3DWidget::onValueChanged);
    layout->addWidget(m_zSpinBox);
}

QVector3D Vector3DWidget::getValue() const {
    return QVector3D(m_xSpinBox->value(), m_ySpinBox->value(), m_zSpinBox->value());
}

void Vector3DWidget::setValue(const QVector3D& value) {
    if (m_updating) return;
    
    m_updating = true;
    m_xSpinBox->setValue(value.x());
    m_ySpinBox->setValue(value.y());
    m_zSpinBox->setValue(value.z());
    m_updating = false;
}

void Vector3DWidget::onValueChanged() {
    if (!m_updating) {
        emit valueChanged(getValue());
    }
}

// QuaternionWidget implementation
QuaternionWidget::QuaternionWidget(const QQuaternion& value, QWidget* parent)
    : QWidget(parent), m_updating(false) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    
    // Convert quaternion to Euler angles
    QVector3D euler = MathUtils::quaternionToEuler(value);
    
    // Pitch component
    layout->addWidget(new QLabel("P:"));
    m_pitchSpinBox = new QDoubleSpinBox();
    m_pitchSpinBox->setRange(-180.0, 180.0);
    m_pitchSpinBox->setSingleStep(1.0);
    m_pitchSpinBox->setDecimals(1);
    m_pitchSpinBox->setValue(euler.x());
    connect(m_pitchSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QuaternionWidget::onValueChanged);
    layout->addWidget(m_pitchSpinBox);
    
    // Yaw component
    layout->addWidget(new QLabel("Y:"));
    m_yawSpinBox = new QDoubleSpinBox();
    m_yawSpinBox->setRange(-180.0, 180.0);
    m_yawSpinBox->setSingleStep(1.0);
    m_yawSpinBox->setDecimals(1);
    m_yawSpinBox->setValue(euler.y());
    connect(m_yawSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QuaternionWidget::onValueChanged);
    layout->addWidget(m_yawSpinBox);
    
    // Roll component
    layout->addWidget(new QLabel("R:"));
    m_rollSpinBox = new QDoubleSpinBox();
    m_rollSpinBox->setRange(-180.0, 180.0);
    m_rollSpinBox->setSingleStep(1.0);
    m_rollSpinBox->setDecimals(1);
    m_rollSpinBox->setValue(euler.z());
    connect(m_rollSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QuaternionWidget::onValueChanged);
    layout->addWidget(m_rollSpinBox);
}

QQuaternion QuaternionWidget::getValue() const {
    return MathUtils::eulerToQuaternion(m_pitchSpinBox->value(), m_yawSpinBox->value(), m_rollSpinBox->value());
}

void QuaternionWidget::setValue(const QQuaternion& value) {
    if (m_updating) return;
    
    m_updating = true;
    QVector3D euler = MathUtils::quaternionToEuler(value);
    m_pitchSpinBox->setValue(euler.x());
    m_yawSpinBox->setValue(euler.y());
    m_rollSpinBox->setValue(euler.z());
    m_updating = false;
}

void QuaternionWidget::onValueChanged() {
    if (!m_updating) {
        emit valueChanged(getValue());
    }
}

// ColorWidget implementation
ColorWidget::ColorWidget(const QVector3D& color, QWidget* parent)
    : QWidget(parent), m_color(color), m_updating(false) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    
    // Color button
    m_colorButton = new QPushButton();
    m_colorButton->setMaximumSize(30, 20);
    updateColorButton();
    connect(m_colorButton, &QPushButton::clicked, this, &ColorWidget::onColorButtonClicked);
    layout->addWidget(m_colorButton);
    
    // RGB spin boxes
    layout->addWidget(new QLabel("R:"));
    m_rSpinBox = new QDoubleSpinBox();
    m_rSpinBox->setRange(0.0, 1.0);
    m_rSpinBox->setSingleStep(0.01);
    m_rSpinBox->setDecimals(3);
    m_rSpinBox->setValue(color.x());
    connect(m_rSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorWidget::onSpinBoxChanged);
    layout->addWidget(m_rSpinBox);
    
    layout->addWidget(new QLabel("G:"));
    m_gSpinBox = new QDoubleSpinBox();
    m_gSpinBox->setRange(0.0, 1.0);
    m_gSpinBox->setSingleStep(0.01);
    m_gSpinBox->setDecimals(3);
    m_gSpinBox->setValue(color.y());
    connect(m_gSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorWidget::onSpinBoxChanged);
    layout->addWidget(m_gSpinBox);
    
    layout->addWidget(new QLabel("B:"));
    m_bSpinBox = new QDoubleSpinBox();
    m_bSpinBox->setRange(0.0, 1.0);
    m_bSpinBox->setSingleStep(0.01);
    m_bSpinBox->setDecimals(3);
    m_bSpinBox->setValue(color.z());
    connect(m_bSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ColorWidget::onSpinBoxChanged);
    layout->addWidget(m_bSpinBox);
}

QVector3D ColorWidget::getColor() const {
    return m_color;
}

void ColorWidget::setColor(const QVector3D& color) {
    if (m_updating) return;
    
    m_color = color;
    updateColorButton();
    updateSpinBoxes();
}

void ColorWidget::onColorButtonClicked() {
    QColor qcolor = QColor::fromRgbF(m_color.x(), m_color.y(), m_color.z());
    QColor newColor = QColorDialog::getColor(qcolor, this);
    
    if (newColor.isValid()) {
        m_color = QVector3D(newColor.redF(), newColor.greenF(), newColor.blueF());
        updateColorButton();
        updateSpinBoxes();
        emit colorChanged(m_color);
    }
}

void ColorWidget::onSpinBoxChanged() {
    if (!m_updating) {
        m_color = QVector3D(m_rSpinBox->value(), m_gSpinBox->value(), m_bSpinBox->value());
        updateColorButton();
        emit colorChanged(m_color);
    }
}

void ColorWidget::updateColorButton() {
    QColor qcolor = QColor::fromRgbF(m_color.x(), m_color.y(), m_color.z());
    QString style = QString("background-color: %1; border: 1px solid black;").arg(qcolor.name());
    m_colorButton->setStyleSheet(style);
}

void ColorWidget::updateSpinBoxes() {
    m_updating = true;
    m_rSpinBox->setValue(m_color.x());
    m_gSpinBox->setValue(m_color.y());
    m_bSpinBox->setValue(m_color.z());
    m_updating = false;
}

#include "property_inspector.moc"

