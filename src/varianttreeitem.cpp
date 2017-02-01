#include <QDateTime>
#include <QUrl>
#include <QUuid>

#include "varianttreeitem.h"
#include "varianttreemodel.h"

VariantTreeItem::VariantTreeItem(QVariant& value, VariantTreeItem* parent) :
    m_valuePtr(&value),
    m_parent(parent)
{
    Q_ASSERT(checkValue(value));
    initChilds();
}

VariantTreeItem::VariantTreeItem(const QString& key, QVariant& value, VariantTreeItem* parent) :
    m_key(key),
    m_valuePtr(&value),
    m_parent(parent)
{
    Q_ASSERT(checkValue(value));
    initChilds();
}

VariantTreeItem::~VariantTreeItem()
{
    destroyChilds();
}

void VariantTreeItem::destroyChilds()
{
    for (auto item : m_childs) {
        delete item;
    }
    m_childs.clear();
}

void VariantTreeItem::initChilds()
{
    if (m_valuePtr->type() == QVariant::List) {
        QVariantList& arr = *array();
        for (QVariant& v : arr) {
            VariantTreeItem* child = new VariantTreeItem(v, this);
            m_childs.append(child);
        }
    } else if (m_valuePtr->type() == QVariant::Map) {
        QVariantMap& obj = *object();
        auto it = obj.begin();
        auto itEnd = obj.end();
        while (it != itEnd) {
            QString key = it.key();
            QVariant& v = it.value();
            VariantTreeItem* child = new VariantTreeItem(key, v, this);
            m_childs.append(child);
            it++;
        }
    }
}

// internal object functions
// @@@@@@@@@@@@@@@@@@@@@@@@@

int VariantTreeItem::findChildPos(const QString& key) const
{
    Q_ASSERT(isObject());

    auto itBegin = m_childs.begin();
    auto itEnd = m_childs.end();
    auto cnd = [&key](VariantTreeItem* item) {
        return key == item->key();
    };
    auto it = std::find_if(itBegin, itEnd, cnd);
    return it - itBegin;
}

int VariantTreeItem::findNewChildPos(const QString& key) const
{
    Q_ASSERT(isObject());

    auto itBegin = m_childs.begin();
    auto itEnd = m_childs.end();
    auto cnd = [&key](VariantTreeItem* item) {
        return key <= item->key();
    };
    auto it = std::find_if(itBegin, itEnd, cnd);
    return it - itBegin;
}

VariantTreeItem* VariantTreeItem::load(QVariant& value, VariantTreeItem* parent)
{
    VariantTreeItem* item = new VariantTreeItem(value, parent);
    //item->initChilds();
    return item;
}

void VariantTreeItem::destroy(VariantTreeItem* parent)
{
    delete parent;
}

// array functions
// @@@@@@@@@@@@@@@
void VariantTreeItem::insertChild(int row, const QVariant& value)
{
    Q_ASSERT(isArray());
    QVariantList& arr = *array();

    arr.insert(row, value);
    VariantTreeItem* item = new VariantTreeItem(arr[row], this);
    m_childs.insert(row, item);
}

void VariantTreeItem::moveChild(int from, int to)
{
    Q_ASSERT(isArray());

    m_childs.move(from, to);
    array()->move(from, to);
}

// object functions
// @@@@@@@@@@@@@@@@
void VariantTreeItem::insertChild(const QString& key, std::function<bool(int)> func, const QVariant& value)
{
    Q_ASSERT(isObject());
    QVariantMap& obj = *object();

    int to = findNewChildPos(key);
    if (to < childCount()) {
        if (key == childKey(to)) {
            func(-1);

            /* replace feature */
            // auto it = obj.find(key);
            // *it = value;
            // m_childs[to]->setValue(value);

            return;
        }
    }

    if (!func(to))
        return;

    auto it = obj.insert(key, value);
    VariantTreeItem* item = new VariantTreeItem(*it, this);
    item->m_key = key;

    m_childs.insert(to, item);

    return;
}

void VariantTreeItem::removeChild(const QString& key, std::function<bool(int)> func)
{
    Q_ASSERT(isObject());
    QVariantMap& obj = *object();

    int row = findChildPos(key);
    if (row < childCount()) {
        if (!func(row))
            return;

        delete m_childs[row];
        m_childs.removeAt(row);
        obj.remove(key);
    } else {
        func(-1);
        return;
    }

    return;
}

void VariantTreeItem::setChildKey(const QString& key, std::function<bool(int)> func, int row)
{
    Q_ASSERT(isObject());
    QVariantMap& obj = *object();

    if (key == childKey(row)) {
        func(row);
        return;
    }

    int to = findNewChildPos(key);
    if (to < childCount()) {
        if (key == childKey(to)) {
            func(-1);
            return;
        }
    }

    if (!func(to))
        return;

    VariantTreeItem* childItem = m_childs[row];
    QVariant& val = obj[key];

    auto oldValueIt = obj.find(childItem->m_key);

    val = std::move(*oldValueIt);
    obj.erase(oldValueIt);

    /* replace feature */
    // m_childs.replace(to, m_childs[row]);
    // m_childs.removeAt(row);

    m_childs.move(row, to > row ? (to - 1) : to);

    childItem->m_key = key;
    childItem->m_valuePtr = &val;

    return;
}

// moving functions
// @@@@@@@@@@@@@@@@

void VariantTreeItem::moveChild(int row, VariantTreeItem* destinationParent, int destinationChild)
{
    Q_ASSERT(!isPlain());
    Q_ASSERT(destinationParent->isArray());

    if (this == destinationParent) {
        moveChild(row, destinationChild);
        return;
    }

    QVariantList& destinationArr = *destinationParent->array();

    VariantTreeItem* child = m_childs[row];
    m_childs.removeAt(row);

    child->m_parent = destinationParent;

    destinationParent->m_childs.insert(destinationChild, child);
    destinationArr.insert(destinationChild, QVariant());
    QVariant& v = destinationArr[destinationChild];

    v = std::move(*child->m_valuePtr);
    child->m_valuePtr = &v;

    if (isArray()) {
        array()->removeAt(row);
    } else if (isObject()) {
        object()->remove(child->m_key);

        // prepare array item
        child->m_key.clear();
    }
}

void VariantTreeItem::moveChild(int row, VariantTreeItem* destinationParent, const QString& destinationKey, std::function<bool(int)> func)
{
    Q_ASSERT(!isPlain());
    Q_ASSERT(destinationParent->isObject());

    if (this == destinationParent) {
        setChildKey(destinationKey, func, row);
        return;
    }

    QVariantMap& destinationObj = *destinationParent->object();

    int to = destinationParent->findNewChildPos(destinationKey);
    if (to < destinationParent->childCount()) {
        if (destinationKey == destinationParent->childKey(to)) {
            func(-1);
            return;
        }
    }

    if (!func(to))
        return;

    VariantTreeItem* child = m_childs[row];
    m_childs.removeAt(row);

    child->m_parent = destinationParent;
    child->m_key = destinationKey;

    QVariant& v = *destinationObj.insert(destinationKey, QVariant());
    v = std::move(*child->m_valuePtr);
    child->m_valuePtr = &v;

    destinationParent->m_childs.insert(to, child);

    if (isArray()) {
        array()->removeAt(row);
    } else if (isObject()) {
        object()->remove(destinationKey);
    }
}

// array and object functions
// @@@@@@@@@@@@@@@@@@@@@@@@@@

void VariantTreeItem::removeChild(int row)
{
    Q_ASSERT(isArray() || isObject());

    auto itBegin = m_childs.begin();
    auto it = itBegin + row;

    if (isArray()) {
        array()->removeAt(row);
    } else if (isObject()) {
        QString key = (*it)->m_key;
        object()->remove(key);
    }

    delete *it;
    m_childs.erase(it);
}

// array <--> object
// @@@@@@@@@@@@@@@@@

void VariantTreeItem::arrayToObject()
{
    Q_ASSERT(isArray());

    QVariant value = QVariantMap();
    QVariantMap* obj = reinterpret_cast<QVariantMap*>(value.data());

    int count = m_childs.count();
    for (int i = 0; i < count; i++) {
        QString num = QString("Item%1").arg(i);
        auto it = obj->insert(num, QVariant());

        VariantTreeItem* item = m_childs[i];
        *it = std::move(*item->m_valuePtr);
        item->m_valuePtr = &it.value();
        item->m_key = num;
    }

    *m_valuePtr = std::move(value);
}

void VariantTreeItem::objectToArray()
{
    Q_ASSERT(isObject());

    QVariant value = QVariantList();
    QVariantList* arr = reinterpret_cast<QVariantList*>(value.data());

    auto it = m_childs.begin();
    auto itEnd = m_childs.end();

    while (it != itEnd) {
        arr->append(QVariant());
        QVariant& dest = arr->last();

        VariantTreeItem* item = *it;
        dest = std::move(*item->m_valuePtr);
        item->m_valuePtr = &dest;
        item->m_key.clear();

        it++;
    }

    *m_valuePtr = std::move(value);
}

// cleaning function
// @@@@@@@@@@@@@@@@@

void VariantTreeItem::clear()
{
    if (isArray() || isObject())
        destroyChilds();
    m_valuePtr->clear();
}

void VariantTreeItem::clearArray()
{
    Q_ASSERT(isArray());

    destroyChilds();
    array()->clear();
}

void VariantTreeItem::clearObject()
{
    Q_ASSERT(isObject());

    destroyChilds();
    object()->clear();
}

// node functions
// @@@@@@@@@@@@@@

VariantTreeItem* VariantTreeItem::child(int row)
{
    Q_ASSERT(isArray() || isObject());
    return m_childs.value(row, nullptr);
}

const VariantTreeItem* VariantTreeItem::child(int row) const
{
    Q_ASSERT(isArray() || isObject());
    return m_childs.value(row, nullptr);
}

const QString& VariantTreeItem::childKey(int row) const
{
    Q_ASSERT(isObject());
    return child(row)->key();
}

int VariantTreeItem::childCount() const
{
    return m_childs.count();
}

int VariantTreeItem::row() const
{
    Q_ASSERT(hasParent());
    return m_parent->m_childs.indexOf(const_cast<VariantTreeItem*>(this));
}

// value getters
// @@@@@@@@@@@@@

const QString& VariantTreeItem::key() const
{
    //Q_ASSERT(!m_key.isNull());
    return m_key;
}
const QVariantList& VariantTreeItem::arrayValue() const
{
    Q_ASSERT(isArray());
    return *array();
}
const QVariantMap& VariantTreeItem::objectValue() const
{
    Q_ASSERT(isObject());
    return *object();
}

// value setters
// @@@@@@@@@@@@@

void VariantTreeItem::setArray(const QVariantList& arr)
{
    setValue(arr);
}

void VariantTreeItem::setObject(const QVariantMap& obj)
{
    setValue(obj);
}

void VariantTreeItem::setValue(const QVariant& value)
{
    Q_ASSERT(checkValue(value));

    if (isArray() || isObject())
        destroyChilds();

    *m_valuePtr = value;
    initChilds();
}

// node value type getters
// @@@@@@@@@@@@@@@@@@@@@@@

QJsonValue::Type VariantTreeItem::jsonType() const
{
    switch (valueType()) {
    case QVariant::Bool:    return QJsonValue::Bool;
    case QVariant::Double:  return QJsonValue::Double;
    case QVariant::Invalid: return QJsonValue::Null;
    case QVariant::List:    return QJsonValue::Array;
    case QVariant::Map:     return QJsonValue::Object;
    case QVariant::String:  return QJsonValue::String;
    default:
        return QJsonValue::Undefined;
    }
}

QString VariantTreeItem::typeName() const
{
    uint type = valueType();

    switch (type) {
    case QVariant::Bool:        return QString("bool");         // json
    case QVariant::Char:        return QString("char");
    case QVariant::Date:        return QString("date");
    case QVariant::DateTime:    return QString("datetime");
    case QVariant::Double:      return QString("double");       // json
    case QMetaType::Float:      return QString("float");
    case QVariant::Invalid:     return QString("invalid");      // json
    case QVariant::Int:         return QString("int");
    case QVariant::List:        return QString("array");        // json
    case QVariant::LongLong:    return QString("longlong");
    case QVariant::Map:         return QString("object");       // json
    case QVariant::String:      return QString("string");       // json
    case QVariant::StringList:  return QString("stringlist");
    case QVariant::Time:        return QString("time");
    case QVariant::UInt:        return QString("uint");
    case QVariant::ULongLong:   return QString("ulonglong");
    case QVariant::Url:         return QString("url");
    case QVariant::Uuid:        return QString("uuid");
    default:
        break;
    }

    return QString(":%1:").arg(m_valuePtr->typeName());
}

// value validation
// @@@@@@@@@@@@@@@@

bool VariantTreeItem::checkValue(const QVariant& value)
{
    uint type = value.type();

    switch (type) {
    case QVariant::Bool:        return true; // json
    case QVariant::Char:        return true;
    case QVariant::Date:        return true;
    case QVariant::DateTime:    return true;
    case QVariant::Double:      return true; // json
    case QMetaType::Float:      return true;
    case QVariant::Invalid:     return true; // json
    case QVariant::Int:         return true;
    case QVariant::List:        return true; // json
    case QVariant::LongLong:    return true;
    case QVariant::Map:         return true; // json
    case QVariant::String:      return true; // json
    case QVariant::StringList:  return true;
    case QVariant::Time:        return true;
    case QVariant::UInt:        return true;
    case QVariant::ULongLong:   return true;
    case QVariant::Url:         return true;
    case QVariant::Uuid:        return true;
    default:
        return false;
    }
}

bool VariantTreeItem::convertTo(QVariant::Type to, bool force)
{
    QVariant::Type from = m_valuePtr->type();
    if (from == to)
        return false;

    bool ok;
    QVariant newValue;

    switch ((uint)to) {
    case QVariant::Bool: {
        newValue = m_valuePtr->toBool();
        ok = true;
        break;
    }
    case QVariant::Char: {
        newValue = m_valuePtr->toChar();
        ok = true;
        break;
    }
    case QVariant::Date: {
        newValue = m_valuePtr->toDate();
        ok = true;
        break;
    }
    case QVariant::DateTime: {
        newValue = m_valuePtr->toDateTime();
        ok = true;
        break;
    }
    case QVariant::Double: {
        newValue = m_valuePtr->toDouble(&ok);
        break;
    }
    case QMetaType::Float: {
        newValue = m_valuePtr->toFloat(&ok);
        break;
    }
    case QVariant::Int: {
        newValue = m_valuePtr->toInt(&ok);
        break;
    }
    case QVariant::List: {
        if (isObject()) {
            objectToArray();
            return true;
        } else {
            if (force) {
                newValue = m_valuePtr->toList();
            } else
                return false;
        }
        break;
    }
    case QVariant::LongLong: {
        newValue = m_valuePtr->toLongLong(&ok);
        break;
    }
    case QVariant::Map: {
        if (isArray()) {
            arrayToObject();
            return true;
        } else {
            if (force) {
                newValue = m_valuePtr->toMap();
            } else
                return false;
        }
        break;
    }
    case QVariant::String: {
        newValue = m_valuePtr->toString();
        ok = true;
        break;
    }
    case QVariant::StringList: {
        newValue = m_valuePtr->toStringList();
        ok = true;
        break;
    }
    case QVariant::Time: {
        newValue = m_valuePtr->toTime();
        ok = true;
        break;
    }
    case QVariant::UInt: {
        newValue = m_valuePtr->toUInt(&ok);
        break;
    }
    case QVariant::ULongLong: {
        newValue = m_valuePtr->toULongLong(&ok);
        break;
    }
    case QVariant::Url: {
        newValue = m_valuePtr->toUrl();
        ok = true;
        break;
    }
    case QVariant::Uuid: {
        newValue = m_valuePtr->toUuid();
        ok = true;
        break;
    }
    default: {
        ok = false;
        break;
    }
    }

    if (ok || force) {
        if (isArray() || isObject())
            destroyChilds();

        *m_valuePtr = std::move(newValue);
        initChilds();

        return true;
    }

    return false;
}
