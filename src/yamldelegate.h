#ifndef YAMLDELEGATE_H
#define YAMLDELEGATE_H

#include <QStyledItemDelegate>

#include "varianttreeitem.h"

class YamlDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    using Base = QStyledItemDelegate;
    using This = YamlDelegate;

public:
    YamlDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void destroyEditor(QWidget* editor, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

private:
    QStringList m_lst;
};

#endif // YAMLDELEGATE_H
