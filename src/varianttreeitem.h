#ifndef VARIANTTREEITEM_H
#define VARIANTTREEITEM_H

#include <functional>

#include <QVariant>
#include <QJsonValue>

class JsonModel;

class VariantTreeItem
{
    using This = VariantTreeItem;

    VariantTreeItem(QVariant& value, VariantTreeItem* parent = nullptr);
    VariantTreeItem(const QString& key, QVariant& value, VariantTreeItem* parent = nullptr);
    ~VariantTreeItem();

    inline void destroyChilds();
    inline void initChilds();

    // internal object functions
    int findChildPos(const QString& key) const;
    int findNewChildPos(const QString& key) const;

public:
    static VariantTreeItem* load(QVariant& value, VariantTreeItem* parent = nullptr);
    static void destroy(VariantTreeItem* parent);

    // array functions
    void insertChild(int row, const QVariant& value = QVariant());
    void moveChild(int from, int to);

    // object functions
    void insertChild(const QString& key, std::function<bool(int)> func, const QVariant& value = QVariant());
    void removeChild(const QString& key, std::function<bool(int)> func);
    void setChildKey(const QString& key, std::function<bool(int)> func, int row);

    // moving functions
    void moveChild(int row, VariantTreeItem* destinationParent, int destinationChild);
    void moveChild(int row, VariantTreeItem* destinationParent, const QString& destinationKey, std::function<bool(int)> func);

    // array and object functions
    void removeChild(int row);

    // array <--> object
    void arrayToObject();
    void objectToArray();

    // cleaning function
    void clear();
    void clearArray();
    void clearObject();

    // node functions
    VariantTreeItem* parent()
    { return m_parent; }
    VariantTreeItem* child(int row);
    const VariantTreeItem* child(int row) const;
    const QString& childKey(int row) const;
    int childCount() const;
    int row() const;

    // value getters
    const QString& key() const;
    const QVariant& value() const
    { return *m_valuePtr; }
    const QVariantList& arrayValue() const;
    const QVariantMap& objectValue() const;

    // value setters
    void setArray(const QVariantList& arr = QVariantList());
    void setObject(const QVariantMap& obj = QVariantMap());
    void setValue(const QVariant& value);

    // node value type getters
    QVariant::Type valueType() const
    { return m_valuePtr->type(); }
    QJsonValue::Type jsonType() const;
    QString typeName() const;

    // node state getters
    inline bool hasParent() const
    { return m_parent != nullptr; }
    inline bool isRoot() const
    { return m_parent == nullptr; }
    inline bool isPlain() const
    { return !isArray() && !isObject(); }

    // node value type getters
    inline bool isArray() const
    { return valueType() == QVariant::List; }
    inline bool isBool() const
    { return valueType() == QVariant::Bool; }
    inline bool isDouble() const
    { return valueType() == QVariant::Double; }
    inline bool isNull() const
    { return valueType() == QVariant::Invalid; }
    inline bool isObject() const
    { return valueType() == QVariant::Map; }
    inline bool isString() const
    { return valueType() == QVariant::String; }

    // value validation
    static bool checkValue(const QVariant& value);

    bool convertTo(QVariant::Type type, bool force = false);

private:
    inline QVariantList* array()
    { return reinterpret_cast<QVariantList*>(m_valuePtr->data()); }
    inline QVariantMap* object()
    { return reinterpret_cast<QVariantMap*>(m_valuePtr->data()); }

    inline const QVariantList* array() const
    { return reinterpret_cast<QVariantList*>(m_valuePtr->data()); }
    inline const QVariantMap* object() const
    { return reinterpret_cast<QVariantMap*>(m_valuePtr->data()); }

    QString m_key;
    QVariant* m_valuePtr;

    VariantTreeItem* m_parent;
    QList<VariantTreeItem*> m_childs;
};

#endif // VARIANTTREEITEM_H
