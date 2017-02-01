#include <QFile>
#include <QJsonDocument>
#include <QRegularExpression>

#include "varianttreemodel.h"

VariantTreeModel::VariantTreeModel(QObject* parent) :
    QAbstractItemModel(parent)
{
    m_rootItem = VariantTreeItem::load(m_variantTree);
}

VariantTreeModel::~VariantTreeModel()
{ }

bool VariantTreeModel::load(const QString& fileName)
{
    QFile file(fileName);
    bool success = false;
    if (file.open(QIODevice::ReadOnly)) {
        success = load(&file);
        file.close();
    } else
        success = false;

    return success;
}

bool VariantTreeModel::load(QIODevice* device)
{
    return loadJson(device->readAll());
}

bool VariantTreeModel::loadJson(const QByteArray& json)
{
    QJsonDocument jdoc;
    jdoc = QJsonDocument::fromJson(json);

    if (!jdoc.isNull()) {
        beginResetModel(); {
            VariantTreeItem::destroy(m_rootItem);

            m_variantTree = jdoc.toVariant();
            m_rootItem = VariantTreeItem::load(m_variantTree);
        } endResetModel();
        return true;
    }
    return false;
}

bool VariantTreeModel::loadVariantTree(const QVariant& v)
{
    beginResetModel(); {
        VariantTreeItem::destroy(m_rootItem);

        m_variantTree = v;
        m_rootItem = VariantTreeItem::load(m_variantTree);
    } endResetModel();
    return true;
}

void VariantTreeModel::destroy()
{
    beginResetModel(); {
        VariantTreeItem::destroy(m_rootItem);

        m_variantTree.clear();
        m_rootItem = VariantTreeItem::load(m_variantTree);
    } endResetModel();
}

Qt::ItemFlags VariantTreeModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (!index.isValid())
        return flags;

    int column = index.column();
    VariantTreeItem* item = castItemFromIndex(index);

    if (!item->hasParent())
        return flags;

    VariantTreeItem* parentItem = item->parent();

    switch (column) {
    case KeyColumn: {
        if (parentItem->isObject())
            flags |= Qt::ItemIsEditable;
        break;
    }
    case ValueColumn:
        if (item->isPlain())
            flags |= Qt::ItemIsEditable;
        break;
    case TypeColumn:
        flags |= Qt::ItemIsEditable;
        break;
    default:
        break;
    }

    return flags;
}

QVariant VariantTreeModel::data(const QModelIndex& index, int role) const
{
    QVariant value;

    if (!index.isValid())
        return value;

    int column = index.column();
    VariantTreeItem* item = castItemFromIndex(index);

    if (!item->hasParent())
        return value;

    VariantTreeItem* parentItem = item->parent();

    switch (role) {
    case Qt::DisplayRole: {
        switch (column) {
        case KeyColumn: {
            if (parentItem->isArray())
                value = QString("[array item]");
            else
                value = item->key();
            break;
        }
        case ValueColumn: {
            if (item->isPlain())
                value = item->value();
            break;
        }
        case TypeColumn: {
            QString typeName = item->typeName();
            value = QString("[%1]").arg(typeName);
            break;
        }
        default:
            break;
        }
        break;
    }
    case Qt::EditRole: {
        switch (column) {
        case KeyColumn: {
            if (parentItem->isObject())
                value = item->key();
            break;
        }
        case ValueColumn: {
            if (item->isPlain())
                value = item->value();
            break;
        }
        case TypeColumn: {
            value = item->valueType();
            break;
        }
        default:
            break;
        }
        break;
    }
    case UrlRole: {
        break;
    }
    default:
        break;
    }

    return value;
}

bool VariantTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int row = index.row();
    int column = index.column();
    VariantTreeItem* item = castItemFromIndex(index);

    if (!item->hasParent())
        return false;

    QModelIndex parent = index.parent();

    switch (role) {
    case Qt::EditRole: {
        switch (column) {
        case KeyColumn: {
            QVariant::Type valueType = value.type();
            if (valueType == QVariant::String) {
                QString key = value.toString();
                return setChildKey(row, key, parent);
            }

            break;
        }
        case ValueColumn: {
            if (item->isPlain()) {
                QVariant::Type valueType = value.type();
                if (valueType != QVariant::List && valueType != QVariant::Map) {
                    item->setValue(value);
                    return true;
                }
            }
            break;
        }
        case TypeColumn: {
            bool ok;
            QVariant::Type toType = (QVariant::Type)value.toUInt(&ok);
            if (!ok)
                return false;

            QVariant::Type fromType = item->valueType();
            if (fromType == toType)
                return true;

            QModelIndex idx = parent.child(row, 0);

            if (item->isArray()) {
                if (toType != QVariant::Map) {
                    beginRemoveRows(idx, 0, item->childCount() - 1);
                    item->convertTo(toType, true);
                    endRemoveRows();

                    return true;
                }
            } else if (item->isObject()) {
                if (toType != QVariant::List) {
                    beginRemoveRows(idx, 0, item->childCount() - 1);
                    item->convertTo(toType, true);
                    endRemoveRows();

                    return true;
                }
            }

            item->convertTo(toType, true);
            return true;
        }
        default:
            break;
        }
    }
    default:
        break;
    }

    return false;
}

QVariant VariantTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant value;

    switch (orientation) {
    case Qt::Horizontal: {
        switch (role) {
        case Qt::DisplayRole: {
            switch (section) {
            case KeyColumn: {
                value = "key";
                break;
            }
            case ValueColumn: {
                value = "value";
                break;
            }
            case TypeColumn: {
                value = "type";
                break;
            }
            default:
                break;
            }
        }
        default:
            break;
        }

        break;
    }
    default:
        value = Base::headerData(section, Qt::Vertical, role);
    }

    return value;
}

QModelIndex VariantTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    VariantTreeItem *parentItem = This::item(parent);
    VariantTreeItem* item = parentItem->child(row);
    QModelIndex idx = createIndex(row, column, item == nullptr ? Q_NULLPTR : item);
    return idx;
}

QModelIndex VariantTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    VariantTreeItem* item = castItemFromIndex(index);

    if (!item->hasParent())
        return QModelIndex();

    VariantTreeItem* parentItem = item->parent();

    if (parentItem == m_rootItem) {
        return QModelIndex();
        //return createIndex(-1, -1, parentItem);
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int VariantTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    VariantTreeItem* parentItem = This::item(parent);
    return parentItem->childCount();
}

int VariantTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 3;
}

QMimeData* VariantTreeModel::mimeData(const QModelIndexList& indexes) const
{
    //TODO: implement drag&drop

    //if (indexes.count() > 0)
    //    return Base::mimeData(indexes);
    QModelIndex idx = indexes.at(0);
    VariantTreeItem* item = castItemFromIndex(idx);

    Q_UNUSED(item)

    QString url;
    url += QString::number(idx.row()) + "/";
    while (idx.isValid()) {
        idx = idx.parent();
        url += QString::number(idx.row()) + "/";
    }

    return Base::mimeData(indexes);
}

bool VariantTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    return Base::dropMimeData(data, action, row, column, parent);
}

Qt::DropActions VariantTreeModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

bool VariantTreeModel::insertRows(int row, int count, const QModelIndex& parent)
{
    return insertRows(row, count, QVariant(), parent);
}

bool VariantTreeModel::insertRows(int row, int count, const QVariant& value, const QModelIndex& parent)
{
    int column = parent.column();
    if (column > 0)
        return false;

    VariantTreeItem* item = This::item(parent);

    if (row < 0)
        return false;
    if (row > item->childCount())
        return false;

    if (count < 0)
        return false;

    if (item->isArray()) {
        if (count > 0) {
            beginInsertRows(parent, row, row + count - 1);
            for (int i = 0; i < count; i++)
                item->insertChild(row + i, value);
            endInsertRows();
        }

        return true;
    } else if (item->isObject()) {
        int attempt = 0;
        bool insertOk;
        QString key;

        if (row > 0) {
            key = item->childKey(row - 1);
            key.remove(QRegularExpression("\\d+$"));
        }

        auto func = [this, &parent, &insertOk](int to) {
            if (to < 0) {
                insertOk = false;
            } else {
                beginInsertRows(parent, to, to);
                insertOk = true;
            }

            return insertOk;
        };

        while (count) {
            QString name;
            if (row > 0)
                name = key + QString::number(attempt);
            else
                name = QString::number(attempt);

            item->insertChild(name, func, value);

            if (insertOk) {
                endInsertRows();
                count--;
            }

            attempt++;
            if (attempt < 0)
                break;
        }

        return true;
    } else
        return false;

    return Base::insertRows(row, count, parent);
}

bool VariantTreeModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
    if (count < 0)
        return false;

    int sourceParentColumn = sourceParent.column();
    if (sourceParentColumn > 0)
        return false;

    int destinationParentColumn = destinationParent.column();
    if (destinationParentColumn > 0)
        return false;

    VariantTreeItem* srcParentItem = This::item(sourceParent);
    VariantTreeItem* dstParentItem = This::item(destinationParent);

    if (sourceRow < 0)
        return false;
    if (sourceRow >= srcParentItem->childCount())
        return false;

    if (destinationChild < 0)
        return false;
    if (destinationChild > dstParentItem->childCount())
        return false;

    bool mvOk;

    if (dstParentItem->isArray()) {
        if (count > 0) {
            mvOk = beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
            if (!mvOk)
                return false;

            int correctRow = sourceParent == destinationParent ? destinationChild > sourceRow ? 1 : 0 : 0;
            for (int i = 0; i < count; i++)
                srcParentItem->moveChild(sourceRow, dstParentItem, destinationChild + i - correctRow);
            endMoveRows();
        }

        return true;
    } else if (dstParentItem->isObject()) {
        auto func = [this, &sourceParent, &sourceRow, &count, &destinationParent, &mvOk](int to) {
            mvOk = to < 0 ? false : beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, to);
            return mvOk;
        };

        if (srcParentItem->isArray()) {
            while (count) {
                srcParentItem->moveChild(sourceRow, dstParentItem, QString::number(destinationChild), func);

                if (mvOk) {
                    endMoveRows();
                    count--;
                }

                destinationChild++;
                if (destinationChild < 0)
                    break;
            }

            return true;
        } else if (srcParentItem->isObject()) {
            for (int i = 0; i < count; i++) {
                QString key;

                if (destinationChild < 0)
                    key = srcParentItem->childKey(sourceRow);
                else
                    key = dstParentItem->childKey(destinationChild);

                key.remove(QRegularExpression("\\d+$"));

                int attempt = 0;
                do {
                    QString name;
                    if (attempt > 0)
                        name = key + QString::number(attempt);
                    else
                        name = key;

                    srcParentItem->moveChild(sourceRow, dstParentItem, name, func);

                    if (mvOk)
                        endMoveRows();
                    else {
                        attempt++;
                        if (attempt < 0)
                            break;
                    }
                } while (!mvOk);

                destinationChild++;
                if (destinationChild < 0)
                    break;
            }

            return true;
        } else
            return false;
    } else
        return false;

    return Base::moveRows(sourceParent, sourceRow, count, destinationParent, destinationChild);
}

bool VariantTreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    int column = parent.column();
    if (column > 0)
        return false;

    VariantTreeItem* item = This::item(parent);

    if (count < 0)
        return false;

    if (row < 0 || row >= item->childCount())
        return false;

    if (item->isArray() || item->isObject()) {
        if (count > 0) {
            beginRemoveRows(parent, row, row + count - 1);
            for (int i = 0; i < count; i++) {
                item->removeChild(row + i);
            }
            endRemoveRows();
        }

        return true;
    } else
        return false;

    return Base::removeRows(row, count, parent);
}

bool VariantTreeModel::setChildKey(int row, const QString& key, const QModelIndex& parent)
{
    VariantTreeItem* item = This::item(parent);

    if (!item->isObject())
        return false;

    bool mvOk;
    item->setChildKey(key, [this, &parent, row, &mvOk](int to) {
        if (to < 0) {
            mvOk = false;
        } else {
            mvOk = beginMoveRows(parent, row, row, parent, to);
            return true;
        }
        return mvOk;
    }, row);

    if (mvOk)
        endMoveRows();
    else
        return false;

    return true;
}

void VariantTreeModel::setValue(const QVariant& value, const QModelIndex& index)
{
    VariantTreeItem* item = static_cast<VariantTreeItem*>(index.internalPointer());

    // TODO:
    //item->setValue(value);

    Q_UNUSED(item);
    Q_UNUSED(value);
}

VariantTreeItem* VariantTreeModel::item(const QModelIndex& index) const
{
    if (!index.isValid())
        return m_rootItem;

    return castItemFromIndex(index);
}
