#ifndef JSONDELEGATE_H
#define JSONDELEGATE_H

#include <QStyledItemDelegate>

#include "varianttreeitem.h"

class JsonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    using Base = QStyledItemDelegate;
    using This = JsonDelegate;

public:
    JsonDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

private:
    QStringList m_lst;
};

#endif // JSONDELEGATE_H
