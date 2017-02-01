#include <QComboBox>

#include "jsondelegate.h"
#include "varianttreemodel.h"

JsonDelegate::JsonDelegate(QObject* parent)
    : Base(parent)
{
    m_lst << "[array]"
          << "[bool]"
          << "[double]"
          << "[object]"
          << "[string]"
          << "[undefined]";
}

QWidget* JsonDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int column = index.column();

    switch (column) {
    case VariantTreeModel::TypeColumn: {
        QComboBox* cmb = new QComboBox(parent);
        cmb->addItems(m_lst);
        return cmb;
    }
    default:
        break;
    }

    return Base::createEditor(parent, option, index);
}

void JsonDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    Base::destroyEditor(editor, index);
}


void JsonDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    int column = index.column();
    VariantTreeItem* item = static_cast<VariantTreeItem*>(index.internalPointer());

    switch (column) {
    case VariantTreeModel::TypeColumn: {
        QComboBox* cmb = qobject_cast<QComboBox*>(editor);
        QVariant::Type itemType = item->valueType();

        int cmbIndex;
        switch (itemType) {
        case QVariant::Bool:
            cmbIndex = 1;
            break;
        case QVariant::Double:
            cmbIndex = 2;
            break;
        case QVariant::Invalid:
            cmbIndex = 5;
            break;
        case QVariant::List:
            cmbIndex = 0;
            break;
        case QVariant::Map:
            cmbIndex = 3;
            break;
        case QVariant::String:
            cmbIndex = 4;
            break;
        default:
            cmbIndex = 5;
            break;
        }

        cmb->setCurrentIndex(cmbIndex);

        return;
    }
    default:
        break;
    }

    Base::setEditorData(editor, index);
}

void JsonDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    int column = index.column();

    switch (column) {
    case VariantTreeModel::TypeColumn: {
        QComboBox* cmb = qobject_cast<QComboBox*>(editor);
        QVariant::Type itemType;

        int cmbIndex = cmb->currentIndex();
        switch (cmbIndex) {
        case 0:
            itemType = QVariant::List;
            break;
        case 1:
            itemType = QVariant::Bool;
            break;
        case 2:
            itemType = QVariant::Double;
            break;
        case 3:
            itemType = QVariant::Map;
            break;
        case 4:
            itemType = QVariant::String;
            break;
        case 5:
            itemType = QVariant::Invalid;
            break;
        default:
            itemType = QVariant::Invalid;
            break;
        }

        model->setData(index, (uint)itemType);

        return;
    }
    default:
        break;
    }

    Base::setModelData(editor, model, index);
}
