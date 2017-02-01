#include <QComboBox>
#include <QLineEdit>

#include "yamldelegate.h"
#include "varianttreemodel.h"

YamlDelegate::YamlDelegate(QObject* parent)
    : Base(parent)
{
    m_lst << "[array]"
          << "[bool]"
          << "[double]"
          << "[float]"
          << "[int]"
          << "[longlong]"
          << "[object]"
          << "[string]"
          << "[uint]"
          << "[ulonglong]"
          << "[undefined]";
}

QWidget* YamlDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

void YamlDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    Base::destroyEditor(editor, index);
}


void YamlDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (!index.isValid())
        return;

    int column = index.column();
    VariantTreeItem* item = static_cast<VariantTreeItem*>(index.internalPointer());

    switch (column) {
    case VariantTreeModel::TypeColumn: {
        QComboBox* cmb = qobject_cast<QComboBox*>(editor);
        uint itemType = item->valueType();

        int cmbIndex;
        switch (itemType) {
        case QVariant::Bool:
            cmbIndex = 1;
            break;
        case QVariant::Double:
            cmbIndex = 2;
            break;
        case QMetaType::Float:
            cmbIndex = 3;
            break;
        case QVariant::Int:
            cmbIndex = 4;
            break;
        case QVariant::Invalid:
            cmbIndex = 10;
            break;
        case QVariant::LongLong:
            cmbIndex = 5;
            break;
        case QVariant::List:
            cmbIndex = 0;
            break;
        case QVariant::Map:
            cmbIndex = 6;
            break;
        case QVariant::String:
            cmbIndex = 7;
            break;
        case QVariant::UInt:
            cmbIndex = 8;
            break;
        case QVariant::ULongLong:
            cmbIndex = 9;
            break;
        default:
            cmbIndex = 10;
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

void YamlDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    int column = index.column();
    VariantTreeItem* item = static_cast<VariantTreeItem*>(index.internalPointer());

    switch (column) {
    case VariantTreeModel::ValueColumn: {
        bool ok;
        QLineEdit* lineEdit;
        QVariant value;

        uint itemType = item->valueType();
        switch (itemType) {
        case QMetaType::Float: {
            lineEdit = qobject_cast<QLineEdit*>(editor);
            if (lineEdit) {
                value = lineEdit->text();
                float f = value.toFloat(&ok);
                if (ok) {
                    model->setData(index, f);
                    return;
                }
            }
        }
        case QVariant::LongLong: {
            lineEdit = qobject_cast<QLineEdit*>(editor);
            if (lineEdit) {
                value = lineEdit->text();
                qlonglong ll = value.toLongLong(&ok);
                if (ok) {
                    model->setData(index, ll);
                    return;
                }
            }
        }
        case QVariant::ULongLong: {
            lineEdit = qobject_cast<QLineEdit*>(editor);
            if (lineEdit) {
                value = lineEdit->text();
                qulonglong ull = value.toULongLong(&ok);
                if (ok) {
                    model->setData(index, ull);
                    return;
                }
            }
        }
        default:
            break;
        }

        break;
    }
    case VariantTreeModel::TypeColumn: {
        QComboBox* cmb = qobject_cast<QComboBox*>(editor);
        uint itemType;

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
            itemType = QMetaType::Float;
            break;
        case 4:
            itemType = QVariant::Int;
            break;
        case 5:
            itemType = QVariant::LongLong;
            break;
        case 6:
            itemType = QVariant::Map;
            break;
        case 7:
            itemType = QVariant::String;
            break;
        case 8:
            itemType = QVariant::UInt;
            break;
        case 9:
            itemType = QVariant::ULongLong;
            break;
        case 10:
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
