#ifndef VARIANTTREEMODEL_H
#define VARIANTTREEMODEL_H

#include <QAbstractItemModel>
#include <QJsonValue>

#include "varianttreeitem.h"

class QIODevice;

class VariantTreeModel : public QAbstractItemModel
{
    Q_OBJECT

    using Base = QAbstractItemModel;
    using This = VariantTreeModel;

public:
    enum AdditionalRoles {
        UrlRole = Qt::UserRole
    };

    enum Columns {
        KeyColumn = 0,
        ValueColumn = 1,
        TypeColumn = 2
    };

    explicit VariantTreeModel(QObject* parent = Q_NULLPTR);
    ~VariantTreeModel();

    bool load(const QString& fileName);
    bool load(QIODevice* device);
    bool loadJson(const QByteArray& json);
    bool loadVariantTree(const QVariant& v);
    void destroy();

    Qt::ItemFlags flags(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QMimeData* mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);

    void setIcon(const QJsonValue::Type& type, const QIcon& icon);

    Qt::DropActions supportedDropActions() const;

    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool insertRows(int row, int count, const QVariant& value, const QModelIndex& parent = QModelIndex());

    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild);
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

    bool setChildKey(int row, const QString& key, const QModelIndex& parent = QModelIndex());
    void setValue(const QVariant& value, const QModelIndex& index);

    VariantTreeItem* item(const QModelIndex& index) const;

    const QVariant& variantTree() const
    { return m_variantTree; }
    VariantTreeItem* rootItem() const
    { return m_rootItem; }

    static VariantTreeItem* castItemFromIndex(const QModelIndex& index)
    { return static_cast<VariantTreeItem*>(index.internalPointer()); }
private:
    QVariant m_variantTree;
    VariantTreeItem* m_rootItem;
};

#endif // VARIANTTREEMODEL_H
