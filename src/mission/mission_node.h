#ifndef MISSION_NODE_H
#define MISSION_NODE_H

#include "types.h"
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QVariant>
#include <QVector>
#include <QMap>
#include <QColor>
#include <QFont>

// Mission node types for visual scripting
enum class MissionNodeType {
    // Flow control
    Start,
    End,
    Sequence,
    Branch,
    Loop,
    Delay,
    
    // Triggers
    EnterZone,
    ExitZone,
    Timer,
    PlayerAction,
    VehicleAction,
    NPCAction,
    
    // Actions
    SpawnEntity,
    DestroyEntity,
    MoveEntity,
    PlayAnimation,
    PlaySound,
    ShowMessage,
    SetObjective,
    CompleteObjective,
    
    // Conditions
    CheckDistance,
    CheckHealth,
    CheckVehicle,
    CheckWeapon,
    CheckTime,
    CheckVariable,
    
    // Variables
    SetVariable,
    GetVariable,
    MathOperation,
    
    // AI/NPC
    SetNPCBehavior,
    NPCFollowPlayer,
    NPCAttackTarget,
    NPCPatrol,
    
    // Camera
    SetCameraTarget,
    CameraFade,
    Cutscene,
    
    // Custom
    CustomScript
};

// Pin types for node connections
enum class PinType {
    Execution,  // Flow control
    Boolean,    // True/false
    Integer,    // Whole numbers
    Float,      // Decimal numbers
    String,     // Text
    Vector3D,   // 3D position/direction
    Entity,     // Entity reference
    Trigger     // Trigger zone reference
};

// Pin direction
enum class PinDirection {
    Input,
    Output
};

// Pin definition
struct NodePin {
    QString name;
    PinType type;
    PinDirection direction;
    QVariant defaultValue;
    bool isConnected;
    QString description;
    
    NodePin(const QString& n = "", PinType t = PinType::Execution, PinDirection d = PinDirection::Input)
        : name(n), type(t), direction(d), isConnected(false) {}
};

// Forward declarations
class MissionConnection;
class MissionEditor;

// Base class for mission nodes
class MissionNode : public QGraphicsItem {
public:
    explicit MissionNode(MissionNodeType type, QGraphicsItem* parent = nullptr);
    virtual ~MissionNode();
    
    // Node properties
    MissionNodeType getNodeType() const;
    QString getNodeName() const;
    void setNodeName(const QString& name);
    
    QString getDescription() const;
    void setDescription(const QString& description);
    
    QColor getNodeColor() const;
    void setNodeColor(const QColor& color);
    
    // Pin management
    void addInputPin(const QString& name, PinType type, const QVariant& defaultValue = QVariant());
    void addOutputPin(const QString& name, PinType type);
    void removePin(const QString& name);
    
    QVector<NodePin> getInputPins() const;
    QVector<NodePin> getOutputPins() const;
    NodePin* getPin(const QString& name);
    
    QPointF getPinPosition(const QString& pinName) const;
    QString getPinAtPosition(const QPointF& position) const;
    
    // Connections
    void addConnection(MissionConnection* connection);
    void removeConnection(MissionConnection* connection);
    QVector<MissionConnection*> getConnections() const;
    
    // Properties
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
    QMap<QString, QVariant> getAllProperties() const;
    
    // Validation
    virtual bool isValid() const;
    virtual QStringList getValidationErrors() const;
    
    // Execution
    virtual bool canExecute() const;
    virtual void execute();
    
    // Serialization
    virtual QVariantMap serialize() const;
    virtual void deserialize(const QVariantMap& data);
    
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;
    
    // Mouse events
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    
    // Selection
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    
    // Constants
    static const qreal NodeWidth;
    static const qreal NodeHeight;
    static const qreal PinRadius;
    static const qreal PinSpacing;
    
protected:
    virtual void setupPins();
    virtual void updateLayout();
    virtual void paintNode(QPainter* painter, const QStyleOptionGraphicsItem* option);
    virtual void paintPins(QPainter* painter);
    virtual void paintConnections(QPainter* painter);
    
    QColor getTypeColor(MissionNodeType type) const;
    QColor getPinColor(PinType type) const;
    QString getTypeName(MissionNodeType type) const;
    
private:
    void calculateSize();
    QRectF getPinRect(const QString& pinName) const;
    
    MissionNodeType m_nodeType;
    QString m_nodeName;
    QString m_description;
    QColor m_nodeColor;
    
    QVector<NodePin> m_inputPins;
    QVector<NodePin> m_outputPins;
    QVector<MissionConnection*> m_connections;
    QMap<QString, QVariant> m_properties;
    
    QRectF m_boundingRect;
    QFont m_font;
    QFont m_pinFont;
    
    bool m_isExecuting;
    bool m_hasError;
};

// Connection between two nodes
class MissionConnection : public QGraphicsItem {
public:
    MissionConnection(MissionNode* fromNode, const QString& fromPin, 
                     MissionNode* toNode, const QString& toPin,
                     QGraphicsItem* parent = nullptr);
    virtual ~MissionConnection();
    
    // Connection properties
    MissionNode* getFromNode() const;
    MissionNode* getToNode() const;
    QString getFromPin() const;
    QString getToPin() const;
    
    PinType getConnectionType() const;
    bool isValid() const;
    
    // Visual properties
    void setColor(const QColor& color);
    QColor getColor() const;
    
    void setWidth(qreal width);
    qreal getWidth() const;
    
    // QGraphicsItem interface
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;
    
    // Selection
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    
    // Update connection path
    void updatePath();
    
private:
    QPainterPath createConnectionPath() const;
    QColor getConnectionColor() const;
    
    MissionNode* m_fromNode;
    MissionNode* m_toNode;
    QString m_fromPin;
    QString m_toPin;
    
    QColor m_color;
    qreal m_width;
    QPainterPath m_path;
};

// Specific node implementations

// Start node - entry point for mission
class StartNode : public MissionNode {
public:
    StartNode(QGraphicsItem* parent = nullptr);
    
protected:
    void setupPins() override;
};

// End node - exit point for mission
class EndNode : public MissionNode {
public:
    EndNode(QGraphicsItem* parent = nullptr);
    
protected:
    void setupPins() override;
};

// Sequence node - executes outputs in order
class SequenceNode : public MissionNode {
public:
    SequenceNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

// Branch node - conditional execution
class BranchNode : public MissionNode {
public:
    BranchNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

// Trigger zone node - area-based trigger
class TriggerZoneNode : public MissionNode {
public:
    TriggerZoneNode(QGraphicsItem* parent = nullptr);
    
    bool canExecute() const override;
    void execute() override;
    
protected:
    void setupPins() override;
};

// Spawn entity node - creates game objects
class SpawnEntityNode : public MissionNode {
public:
    SpawnEntityNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

// Set objective node - mission objective management
class SetObjectiveNode : public MissionNode {
public:
    SetObjectiveNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

// Show message node - display text to player
class ShowMessageNode : public MissionNode {
public:
    ShowMessageNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

// Delay node - time-based delay
class DelayNode : public MissionNode {
public:
    DelayNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

// Variable nodes for data manipulation
class SetVariableNode : public MissionNode {
public:
    SetVariableNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

class GetVariableNode : public MissionNode {
public:
    GetVariableNode(QGraphicsItem* parent = nullptr);
    
    void execute() override;
    
protected:
    void setupPins() override;
};

// Factory function for creating nodes
MissionNode* createMissionNode(MissionNodeType type, QGraphicsItem* parent = nullptr);

#endif // MISSION_NODE_H

