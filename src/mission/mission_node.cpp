#include "mission_node.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QFontMetrics>
#include <QtMath>
#include <QDebug>

// Constants
const qreal MissionNode::NodeWidth = 150.0;
const qreal MissionNode::NodeHeight = 80.0;
const qreal MissionNode::PinRadius = 6.0;
const qreal MissionNode::PinSpacing = 20.0;

MissionNode::MissionNode(MissionNodeType type, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_nodeType(type)
    , m_nodeName(getTypeName(type))
    , m_nodeColor(getTypeColor(type))
    , m_isExecuting(false)
    , m_hasError(false)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    m_font = QFont("Arial", 10, QFont::Bold);
    m_pinFont = QFont("Arial", 8);
    
    setupPins();
    calculateSize();
}

MissionNode::~MissionNode() {
    // Clean up connections
    for (MissionConnection* connection : m_connections) {
        if (connection->scene()) {
            connection->scene()->removeItem(connection);
        }
        delete connection;
    }
}

MissionNodeType MissionNode::getNodeType() const {
    return m_nodeType;
}

QString MissionNode::getNodeName() const {
    return m_nodeName;
}

void MissionNode::setNodeName(const QString& name) {
    if (m_nodeName != name) {
        m_nodeName = name;
        calculateSize();
        update();
    }
}

QString MissionNode::getDescription() const {
    return m_description;
}

void MissionNode::setDescription(const QString& description) {
    m_description = description;
}

QColor MissionNode::getNodeColor() const {
    return m_nodeColor;
}

void MissionNode::setNodeColor(const QColor& color) {
    if (m_nodeColor != color) {
        m_nodeColor = color;
        update();
    }
}

void MissionNode::addInputPin(const QString& name, PinType type, const QVariant& defaultValue) {
    NodePin pin(name, type, PinDirection::Input);
    pin.defaultValue = defaultValue;
    m_inputPins.append(pin);
    calculateSize();
    update();
}

void MissionNode::addOutputPin(const QString& name, PinType type) {
    NodePin pin(name, type, PinDirection::Output);
    m_outputPins.append(pin);
    calculateSize();
    update();
}

void MissionNode::removePin(const QString& name) {
    // Remove from input pins
    for (int i = 0; i < m_inputPins.size(); ++i) {
        if (m_inputPins[i].name == name) {
            m_inputPins.removeAt(i);
            calculateSize();
            update();
            return;
        }
    }
    
    // Remove from output pins
    for (int i = 0; i < m_outputPins.size(); ++i) {
        if (m_outputPins[i].name == name) {
            m_outputPins.removeAt(i);
            calculateSize();
            update();
            return;
        }
    }
}

QVector<NodePin> MissionNode::getInputPins() const {
    return m_inputPins;
}

QVector<NodePin> MissionNode::getOutputPins() const {
    return m_outputPins;
}

NodePin* MissionNode::getPin(const QString& name) {
    // Search input pins
    for (auto& pin : m_inputPins) {
        if (pin.name == name) {
            return &pin;
        }
    }
    
    // Search output pins
    for (auto& pin : m_outputPins) {
        if (pin.name == name) {
            return &pin;
        }
    }
    
    return nullptr;
}

QPointF MissionNode::getPinPosition(const QString& pinName) const {
    QRectF rect = boundingRect();
    
    // Check input pins
    for (int i = 0; i < m_inputPins.size(); ++i) {
        if (m_inputPins[i].name == pinName) {
            qreal y = rect.top() + 30 + i * PinSpacing;
            return QPointF(rect.left(), y);
        }
    }
    
    // Check output pins
    for (int i = 0; i < m_outputPins.size(); ++i) {
        if (m_outputPins[i].name == pinName) {
            qreal y = rect.top() + 30 + i * PinSpacing;
            return QPointF(rect.right(), y);
        }
    }
    
    return QPointF();
}

QString MissionNode::getPinAtPosition(const QPointF& position) const {
    QPointF localPos = mapFromScene(position);
    
    // Check input pins
    for (int i = 0; i < m_inputPins.size(); ++i) {
        QRectF pinRect = getPinRect(m_inputPins[i].name);
        if (pinRect.contains(localPos)) {
            return m_inputPins[i].name;
        }
    }
    
    // Check output pins
    for (int i = 0; i < m_outputPins.size(); ++i) {
        QRectF pinRect = getPinRect(m_outputPins[i].name);
        if (pinRect.contains(localPos)) {
            return m_outputPins[i].name;
        }
    }
    
    return QString();
}

void MissionNode::addConnection(MissionConnection* connection) {
    if (connection && !m_connections.contains(connection)) {
        m_connections.append(connection);
    }
}

void MissionNode::removeConnection(MissionConnection* connection) {
    m_connections.removeAll(connection);
}

QVector<MissionConnection*> MissionNode::getConnections() const {
    return m_connections;
}

void MissionNode::setProperty(const QString& name, const QVariant& value) {
    m_properties[name] = value;
}

QVariant MissionNode::getProperty(const QString& name) const {
    return m_properties.value(name);
}

QMap<QString, QVariant> MissionNode::getAllProperties() const {
    return m_properties;
}

bool MissionNode::isValid() const {
    return getValidationErrors().isEmpty();
}

QStringList MissionNode::getValidationErrors() const {
    QStringList errors;
    
    // Check if required input pins are connected
    for (const auto& pin : m_inputPins) {
        if (pin.type == PinType::Execution && !pin.isConnected) {
            errors.append(QString("Required input pin '%1' is not connected").arg(pin.name));
        }
    }
    
    return errors;
}

bool MissionNode::canExecute() const {
    return isValid() && !m_isExecuting;
}

void MissionNode::execute() {
    if (!canExecute()) {
        return;
    }
    
    m_isExecuting = true;
    // Base implementation - derived classes should override
    m_isExecuting = false;
}

QVariantMap MissionNode::serialize() const {
    QVariantMap data;
    data["type"] = static_cast<int>(m_nodeType);
    data["name"] = m_nodeName;
    data["description"] = m_description;
    data["position"] = QVariant::fromValue(pos());
    data["color"] = m_nodeColor.name();
    data["properties"] = m_properties;
    
    return data;
}

void MissionNode::deserialize(const QVariantMap& data) {
    m_nodeName = data.value("name", getTypeName(m_nodeType)).toString();
    m_description = data.value("description").toString();
    setPos(data.value("position").toPointF());
    m_nodeColor = QColor(data.value("color", getTypeColor(m_nodeType).name()).toString());
    m_properties = data.value("properties").toMap();
    
    calculateSize();
    update();
}

QRectF MissionNode::boundingRect() const {
    return m_boundingRect;
}

void MissionNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(widget)
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    paintNode(painter, option);
    paintPins(painter);
}

QPainterPath MissionNode::shape() const {
    QPainterPath path;
    path.addRoundedRect(boundingRect(), 5, 5);
    return path;
}

void MissionNode::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    // Check if clicking on a pin
    QString pinName = getPinAtPosition(event->scenePos());
    if (!pinName.isEmpty()) {
        // Handle pin click for connection creation
        event->accept();
        return;
    }
    
    QGraphicsItem::mousePressEvent(event);
}

void MissionNode::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseMoveEvent(event);
    
    // Update connections
    for (MissionConnection* connection : m_connections) {
        connection->updatePath();
    }
}

void MissionNode::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseReleaseEvent(event);
}

void MissionNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    // Open node properties dialog
    event->accept();
}

QVariant MissionNode::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        // Update connections
        for (MissionConnection* connection : m_connections) {
            connection->updatePath();
        }
    }
    
    return QGraphicsItem::itemChange(change, value);
}

void MissionNode::setupPins() {
    // Base implementation - derived classes should override
}

void MissionNode::updateLayout() {
    calculateSize();
    update();
}

void MissionNode::paintNode(QPainter* painter, const QStyleOptionGraphicsItem* option) {
    QRectF rect = boundingRect();
    
    // Node background
    QColor bgColor = m_nodeColor;
    if (isSelected()) {
        bgColor = bgColor.lighter(120);
    }
    if (m_isExecuting) {
        bgColor = QColor(255, 255, 0, 100); // Yellow highlight
    }
    if (m_hasError) {
        bgColor = QColor(255, 0, 0, 100); // Red highlight
    }
    
    painter->setBrush(bgColor);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRoundedRect(rect, 5, 5);
    
    // Node title
    painter->setFont(m_font);
    painter->setPen(Qt::white);
    QRectF titleRect = QRectF(rect.left() + 5, rect.top() + 5, rect.width() - 10, 20);
    painter->drawText(titleRect, Qt::AlignCenter, m_nodeName);
    
    // Selection outline
    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::yellow, 3, Qt::DashLine));
        painter->drawRoundedRect(rect, 5, 5);
    }
}

void MissionNode::paintPins(QPainter* painter) {
    painter->setFont(m_pinFont);
    
    // Paint input pins
    for (int i = 0; i < m_inputPins.size(); ++i) {
        const NodePin& pin = m_inputPins[i];
        QPointF pinPos = getPinPosition(pin.name);
        
        // Pin circle
        QColor pinColor = getPinColor(pin.type);
        if (pin.isConnected) {
            painter->setBrush(pinColor);
        } else {
            painter->setBrush(pinColor.darker(150));
        }
        painter->setPen(Qt::black);
        painter->drawEllipse(pinPos, PinRadius, PinRadius);
        
        // Pin label
        painter->setPen(Qt::white);
        QRectF labelRect(pinPos.x() + PinRadius + 5, pinPos.y() - 8, 100, 16);
        painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, pin.name);
    }
    
    // Paint output pins
    for (int i = 0; i < m_outputPins.size(); ++i) {
        const NodePin& pin = m_outputPins[i];
        QPointF pinPos = getPinPosition(pin.name);
        
        // Pin circle
        QColor pinColor = getPinColor(pin.type);
        if (pin.isConnected) {
            painter->setBrush(pinColor);
        } else {
            painter->setBrush(pinColor.darker(150));
        }
        painter->setPen(Qt::black);
        painter->drawEllipse(pinPos, PinRadius, PinRadius);
        
        // Pin label
        painter->setPen(Qt::white);
        QRectF labelRect(pinPos.x() - 105, pinPos.y() - 8, 100, 16);
        painter->drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, pin.name);
    }
}

void MissionNode::paintConnections(QPainter* painter) {
    // Connections are painted by MissionConnection items
}

QColor MissionNode::getTypeColor(MissionNodeType type) const {
    switch (type) {
        case MissionNodeType::Start:
            return QColor(0, 255, 0);
        case MissionNodeType::End:
            return QColor(255, 0, 0);
        case MissionNodeType::Sequence:
        case MissionNodeType::Branch:
        case MissionNodeType::Loop:
            return QColor(100, 100, 255);
        case MissionNodeType::EnterZone:
        case MissionNodeType::ExitZone:
        case MissionNodeType::Timer:
            return QColor(255, 150, 0);
        case MissionNodeType::SpawnEntity:
        case MissionNodeType::DestroyEntity:
        case MissionNodeType::MoveEntity:
            return QColor(150, 255, 150);
        case MissionNodeType::SetObjective:
        case MissionNodeType::CompleteObjective:
            return QColor(255, 255, 0);
        case MissionNodeType::SetVariable:
        case MissionNodeType::GetVariable:
        case MissionNodeType::MathOperation:
            return QColor(255, 100, 255);
        default:
            return QColor(128, 128, 128);
    }
}

QColor MissionNode::getPinColor(PinType type) const {
    switch (type) {
        case PinType::Execution:
            return QColor(255, 255, 255);
        case PinType::Boolean:
            return QColor(255, 0, 0);
        case PinType::Integer:
            return QColor(0, 255, 0);
        case PinType::Float:
            return QColor(0, 255, 255);
        case PinType::String:
            return QColor(255, 0, 255);
        case PinType::Vector3D:
            return QColor(255, 255, 0);
        case PinType::Entity:
            return QColor(100, 150, 255);
        case PinType::Trigger:
            return QColor(255, 150, 100);
        default:
            return QColor(128, 128, 128);
    }
}

QString MissionNode::getTypeName(MissionNodeType type) const {
    switch (type) {
        case MissionNodeType::Start: return "Start";
        case MissionNodeType::End: return "End";
        case MissionNodeType::Sequence: return "Sequence";
        case MissionNodeType::Branch: return "Branch";
        case MissionNodeType::Loop: return "Loop";
        case MissionNodeType::Delay: return "Delay";
        case MissionNodeType::EnterZone: return "Enter Zone";
        case MissionNodeType::ExitZone: return "Exit Zone";
        case MissionNodeType::Timer: return "Timer";
        case MissionNodeType::PlayerAction: return "Player Action";
        case MissionNodeType::VehicleAction: return "Vehicle Action";
        case MissionNodeType::NPCAction: return "NPC Action";
        case MissionNodeType::SpawnEntity: return "Spawn Entity";
        case MissionNodeType::DestroyEntity: return "Destroy Entity";
        case MissionNodeType::MoveEntity: return "Move Entity";
        case MissionNodeType::PlayAnimation: return "Play Animation";
        case MissionNodeType::PlaySound: return "Play Sound";
        case MissionNodeType::ShowMessage: return "Show Message";
        case MissionNodeType::SetObjective: return "Set Objective";
        case MissionNodeType::CompleteObjective: return "Complete Objective";
        case MissionNodeType::CheckDistance: return "Check Distance";
        case MissionNodeType::CheckHealth: return "Check Health";
        case MissionNodeType::CheckVehicle: return "Check Vehicle";
        case MissionNodeType::CheckWeapon: return "Check Weapon";
        case MissionNodeType::CheckTime: return "Check Time";
        case MissionNodeType::CheckVariable: return "Check Variable";
        case MissionNodeType::SetVariable: return "Set Variable";
        case MissionNodeType::GetVariable: return "Get Variable";
        case MissionNodeType::MathOperation: return "Math Operation";
        case MissionNodeType::SetNPCBehavior: return "Set NPC Behavior";
        case MissionNodeType::NPCFollowPlayer: return "NPC Follow Player";
        case MissionNodeType::NPCAttackTarget: return "NPC Attack Target";
        case MissionNodeType::NPCPatrol: return "NPC Patrol";
        case MissionNodeType::SetCameraTarget: return "Set Camera Target";
        case MissionNodeType::CameraFade: return "Camera Fade";
        case MissionNodeType::Cutscene: return "Cutscene";
        case MissionNodeType::CustomScript: return "Custom Script";
        default: return "Unknown";
    }
}

void MissionNode::calculateSize() {
    QFontMetrics fm(m_font);
    QFontMetrics pinFm(m_pinFont);
    
    // Calculate minimum width based on title
    qreal titleWidth = fm.horizontalAdvance(m_nodeName) + 20;
    
    // Calculate minimum width based on pins
    qreal maxPinWidth = 0;
    for (const auto& pin : m_inputPins) {
        qreal pinWidth = pinFm.horizontalAdvance(pin.name) + PinRadius * 2 + 10;
        maxPinWidth = qMax(maxPinWidth, pinWidth);
    }
    for (const auto& pin : m_outputPins) {
        qreal pinWidth = pinFm.horizontalAdvance(pin.name) + PinRadius * 2 + 10;
        maxPinWidth = qMax(maxPinWidth, pinWidth);
    }
    
    qreal width = qMax(qMax(NodeWidth, titleWidth), maxPinWidth);
    
    // Calculate height based on pins
    int maxPins = qMax(m_inputPins.size(), m_outputPins.size());
    qreal height = qMax(NodeHeight, 30 + maxPins * PinSpacing + 10);
    
    m_boundingRect = QRectF(-width/2, -height/2, width, height);
}

QRectF MissionNode::getPinRect(const QString& pinName) const {
    QPointF pinPos = getPinPosition(pinName);
    return QRectF(pinPos.x() - PinRadius, pinPos.y() - PinRadius, PinRadius * 2, PinRadius * 2);
}

// MissionConnection implementation
MissionConnection::MissionConnection(MissionNode* fromNode, const QString& fromPin,
                                   MissionNode* toNode, const QString& toPin,
                                   QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_fromNode(fromNode)
    , m_toNode(toNode)
    , m_fromPin(fromPin)
    , m_toPin(toPin)
    , m_color(Qt::white)
    , m_width(2.0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setZValue(-1); // Draw behind nodes
    
    if (m_fromNode) {
        m_fromNode->addConnection(this);
    }
    if (m_toNode) {
        m_toNode->addConnection(this);
    }
    
    updatePath();
}

MissionConnection::~MissionConnection() {
    if (m_fromNode) {
        m_fromNode->removeConnection(this);
    }
    if (m_toNode) {
        m_toNode->removeConnection(this);
    }
}

MissionNode* MissionConnection::getFromNode() const {
    return m_fromNode;
}

MissionNode* MissionConnection::getToNode() const {
    return m_toNode;
}

QString MissionConnection::getFromPin() const {
    return m_fromPin;
}

QString MissionConnection::getToPin() const {
    return m_toPin;
}

PinType MissionConnection::getConnectionType() const {
    if (m_fromNode) {
        NodePin* pin = m_fromNode->getPin(m_fromPin);
        if (pin) {
            return pin->type;
        }
    }
    return PinType::Execution;
}

bool MissionConnection::isValid() const {
    if (!m_fromNode || !m_toNode) {
        return false;
    }
    
    NodePin* fromPin = m_fromNode->getPin(m_fromPin);
    NodePin* toPin = m_toNode->getPin(m_toPin);
    
    if (!fromPin || !toPin) {
        return false;
    }
    
    // Check pin directions
    if (fromPin->direction != PinDirection::Output || toPin->direction != PinDirection::Input) {
        return false;
    }
    
    // Check pin types match
    if (fromPin->type != toPin->type) {
        return false;
    }
    
    return true;
}

void MissionConnection::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QColor MissionConnection::getColor() const {
    return m_color;
}

void MissionConnection::setWidth(qreal width) {
    if (m_width != width) {
        m_width = width;
        update();
    }
}

qreal MissionConnection::getWidth() const {
    return m_width;
}

QRectF MissionConnection::boundingRect() const {
    if (!m_fromNode || !m_toNode) {
        return QRectF();
    }
    
    QPointF fromPos = m_fromNode->getPinPosition(m_fromPin);
    QPointF toPos = m_toNode->getPinPosition(m_toPin);
    
    qreal minX = qMin(fromPos.x(), toPos.x()) - m_width;
    qreal minY = qMin(fromPos.y(), toPos.y()) - m_width;
    qreal maxX = qMax(fromPos.x(), toPos.x()) + m_width;
    qreal maxY = qMax(fromPos.y(), toPos.y()) + m_width;
    
    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

void MissionConnection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)
    
    if (!isValid()) {
        return;
    }
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    QColor connectionColor = getConnectionColor();
    if (isSelected()) {
        connectionColor = connectionColor.lighter(150);
    }
    
    painter->setPen(QPen(connectionColor, m_width, Qt::SolidLine, Qt::RoundCap));
    painter->drawPath(m_path);
    
    // Draw arrow at the end
    QPointF toPos = m_toNode->getPinPosition(m_toPin);
    QPointF fromPos = m_fromNode->getPinPosition(m_fromPin);
    
    QVector2D direction = QVector2D(toPos - fromPos).normalized();
    QVector2D perpendicular(-direction.y(), direction.x());
    
    qreal arrowSize = 8.0;
    QPointF arrowTip = toPos - direction.toPointF() * MissionNode::PinRadius;
    QPointF arrowLeft = arrowTip - direction.toPointF() * arrowSize + perpendicular.toPointF() * arrowSize * 0.5;
    QPointF arrowRight = arrowTip - direction.toPointF() * arrowSize - perpendicular.toPointF() * arrowSize * 0.5;
    
    painter->setBrush(connectionColor);
    painter->drawPolygon(QPolygonF() << arrowTip << arrowLeft << arrowRight);
}

QPainterPath MissionConnection::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(m_width + 4); // Make selection area slightly larger
    return stroker.createStroke(m_path);
}

QVariant MissionConnection::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemSelectedChange) {
        update();
    }
    return QGraphicsItem::itemChange(change, value);
}

void MissionConnection::updatePath() {
    prepareGeometryChange();
    m_path = createConnectionPath();
    update();
}

QPainterPath MissionConnection::createConnectionPath() const {
    QPainterPath path;
    
    if (!m_fromNode || !m_toNode) {
        return path;
    }
    
    QPointF fromPos = m_fromNode->getPinPosition(m_fromPin);
    QPointF toPos = m_toNode->getPinPosition(m_toPin);
    
    // Create curved connection
    qreal distance = QVector2D(toPos - fromPos).length();
    qreal controlOffset = qMin(distance * 0.5, 100.0);
    
    QPointF control1 = fromPos + QPointF(controlOffset, 0);
    QPointF control2 = toPos - QPointF(controlOffset, 0);
    
    path.moveTo(fromPos);
    path.cubicTo(control1, control2, toPos);
    
    return path;
}

QColor MissionConnection::getConnectionColor() const {
    // Get color based on connection type
    PinType type = getConnectionType();
    switch (type) {
        case PinType::Execution:
            return QColor(255, 255, 255);
        case PinType::Boolean:
            return QColor(255, 100, 100);
        case PinType::Integer:
            return QColor(100, 255, 100);
        case PinType::Float:
            return QColor(100, 255, 255);
        case PinType::String:
            return QColor(255, 100, 255);
        case PinType::Vector3D:
            return QColor(255, 255, 100);
        case PinType::Entity:
            return QColor(150, 200, 255);
        case PinType::Trigger:
            return QColor(255, 200, 150);
        default:
            return QColor(128, 128, 128);
    }
}

// Specific node implementations
StartNode::StartNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::Start, parent) {
}

void StartNode::setupPins() {
    addOutputPin("Execute", PinType::Execution);
}

EndNode::EndNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::End, parent) {
}

void EndNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
}

SequenceNode::SequenceNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::Sequence, parent) {
}

void SequenceNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
    addOutputPin("Then 1", PinType::Execution);
    addOutputPin("Then 2", PinType::Execution);
    addOutputPin("Then 3", PinType::Execution);
}

void SequenceNode::execute() {
    // Execute outputs in sequence
    MissionNode::execute();
}

BranchNode::BranchNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::Branch, parent) {
}

void BranchNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
    addInputPin("Condition", PinType::Boolean);
    addOutputPin("True", PinType::Execution);
    addOutputPin("False", PinType::Execution);
}

void BranchNode::execute() {
    // Execute based on condition
    MissionNode::execute();
}

TriggerZoneNode::TriggerZoneNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::EnterZone, parent) {
}

void TriggerZoneNode::setupPins() {
    addInputPin("Zone", PinType::Trigger);
    addOutputPin("On Enter", PinType::Execution);
    addOutputPin("On Exit", PinType::Execution);
}

bool TriggerZoneNode::canExecute() const {
    // Check if player is in trigger zone
    return MissionNode::canExecute();
}

void TriggerZoneNode::execute() {
    MissionNode::execute();
}

SpawnEntityNode::SpawnEntityNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::SpawnEntity, parent) {
}

void SpawnEntityNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
    addInputPin("Position", PinType::Vector3D);
    addInputPin("Model", PinType::String);
    addOutputPin("Then", PinType::Execution);
    addOutputPin("Entity", PinType::Entity);
}

void SpawnEntityNode::execute() {
    // Spawn entity at specified position
    MissionNode::execute();
}

SetObjectiveNode::SetObjectiveNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::SetObjective, parent) {
}

void SetObjectiveNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
    addInputPin("Objective", PinType::String);
    addInputPin("Description", PinType::String);
    addOutputPin("Then", PinType::Execution);
}

void SetObjectiveNode::execute() {
    // Set mission objective
    MissionNode::execute();
}

ShowMessageNode::ShowMessageNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::ShowMessage, parent) {
}

void ShowMessageNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
    addInputPin("Message", PinType::String);
    addInputPin("Duration", PinType::Float);
    addOutputPin("Then", PinType::Execution);
}

void ShowMessageNode::execute() {
    // Display message to player
    MissionNode::execute();
}

DelayNode::DelayNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::Delay, parent) {
}

void DelayNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
    addInputPin("Duration", PinType::Float);
    addOutputPin("Then", PinType::Execution);
}

void DelayNode::execute() {
    // Wait for specified duration
    MissionNode::execute();
}

SetVariableNode::SetVariableNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::SetVariable, parent) {
}

void SetVariableNode::setupPins() {
    addInputPin("Execute", PinType::Execution);
    addInputPin("Variable", PinType::String);
    addInputPin("Value", PinType::String); // Generic value
    addOutputPin("Then", PinType::Execution);
}

void SetVariableNode::execute() {
    // Set variable value
    MissionNode::execute();
}

GetVariableNode::GetVariableNode(QGraphicsItem* parent)
    : MissionNode(MissionNodeType::GetVariable, parent) {
}

void GetVariableNode::setupPins() {
    addInputPin("Variable", PinType::String);
    addOutputPin("Value", PinType::String); // Generic value
}

void GetVariableNode::execute() {
    // Get variable value
    MissionNode::execute();
}

// Factory function
MissionNode* createMissionNode(MissionNodeType type, QGraphicsItem* parent) {
    switch (type) {
        case MissionNodeType::Start:
            return new StartNode(parent);
        case MissionNodeType::End:
            return new EndNode(parent);
        case MissionNodeType::Sequence:
            return new SequenceNode(parent);
        case MissionNodeType::Branch:
            return new BranchNode(parent);
        case MissionNodeType::EnterZone:
        case MissionNodeType::ExitZone:
            return new TriggerZoneNode(parent);
        case MissionNodeType::SpawnEntity:
            return new SpawnEntityNode(parent);
        case MissionNodeType::SetObjective:
            return new SetObjectiveNode(parent);
        case MissionNodeType::ShowMessage:
            return new ShowMessageNode(parent);
        case MissionNodeType::Delay:
            return new DelayNode(parent);
        case MissionNodeType::SetVariable:
            return new SetVariableNode(parent);
        case MissionNodeType::GetVariable:
            return new GetVariableNode(parent);
        default:
            return new MissionNode(type, parent);
    }
}

