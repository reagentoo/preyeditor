#include <QFileDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeView>
#include <QTextStream>

#include <QJsonDocument>

#include "varianttreewidget.h"

#include "jsondelegate.h"
#include "yamldelegate.h"

VariantTreeWidget::VariantTreeWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* lt = new QVBoxLayout;

    QHBoxLayout* btnLt = new QHBoxLayout;
    lt->addLayout(btnLt);

    QPushButton* btnOpen = new QPushButton("Open", this);
    QPushButton* btnSaveAs = new QPushButton("Save as", this);
    QPushButton* btnClose = new QPushButton("Close", this);

    QPushButton* btnAdd = new QPushButton("Add", this);
    QPushButton* btnInsert = new QPushButton("Insert", this);
    QPushButton* btnDelete = new QPushButton("Delete", this);
    //QPushButton* btnChange = new QPushButton("Change", this);

    QPushButton* btnDown = new QPushButton("Down", this);
    QPushButton* btnUp = new QPushButton("Up", this);

    btnLt->addWidget(btnOpen);
    btnLt->addWidget(btnSaveAs);
    btnLt->addWidget(btnClose);

    btnLt->addWidget(btnAdd);
    btnLt->addWidget(btnInsert);
    btnLt->addWidget(btnDelete);
    //btnLt->addWidget(btnChange);

    btnLt->addWidget(btnDown);
    btnLt->addWidget(btnUp);

    btnOpen->setIcon(QIcon::fromTheme("document-open"));
    btnSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
    btnClose->setIcon(QIcon::fromTheme("document-close"));

    btnAdd->setIcon(QIcon::fromTheme("list-add"));
    btnInsert->setIcon(QIcon::fromTheme("edit-table-insert-row-below"));
    btnDelete->setIcon(QIcon::fromTheme("list-remove"));

    btnDown->setIcon(QIcon::fromTheme("go-down-search"));
    btnUp->setIcon(QIcon::fromTheme("go-up-search"));

    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    VariantTreeModel* jmod = new VariantTreeModel(this);
    m_jmod = jmod;

    QTreeView* jview = new QTreeView;
    m_jview = jview;
    jview->setModel(jmod);

    jview->setColumnWidth(0, 300);
    jview->setColumnWidth(1, 200);
    jview->setColumnWidth(2, 200);

    jview->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //jview->setEditTriggers(QAbstractItemView::SelectedClicked);
    jview->setDragEnabled(true);
    jview->setAcceptDrops(true);
    jview->setDropIndicatorShown(true);

    jview->setItemDelegate(new JsonDelegate(m_jview));
    lt->addWidget(jview);

    setLayout(lt);
    lt->setMargin(0);
    btnLt->setMargin(0);

    connect(jmod, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)), SLOT(rowMoved()));

    connect(btnOpen, SIGNAL(clicked(bool)), SLOT(btnOpen_clicked()));
    connect(btnSaveAs, SIGNAL(clicked(bool)), SLOT(btnSaveAs_clicked()));
    connect(btnClose, SIGNAL(clicked(bool)), SLOT(btnClose_clicked()));

    connect(btnAdd, SIGNAL(clicked(bool)), SLOT(btnAdd_clicked()));
    connect(btnInsert, SIGNAL(clicked(bool)), SLOT(btnInsert_clicked()));
    connect(btnDelete, SIGNAL(clicked(bool)), SLOT(btnDelete_clicked()));
    //connect(btnChange, SIGNAL(clicked(bool)), SLOT(btnChange_clicked()));

    connect(btnDown, SIGNAL(clicked(bool)), SLOT(btnDown_clicked()));
    connect(btnUp, SIGNAL(clicked(bool)), SLOT(btnUp_clicked()));
}

void VariantTreeWidget::rowMoved()
{
    QTextStream(stdout) << "moved" << endl;
}

void VariantTreeWidget::btnOpen_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);

    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    if (fileNames.size() > 0) {
        m_jmod->load(fileNames.first());
    }
}

void VariantTreeWidget::btnSaveAs_clicked()
{
    QString fn = QFileDialog::getSaveFileName(this, "Save As");

    if (fn.size() > 0) {
        QJsonDocument jdoc = QJsonDocument::fromVariant(m_jmod->variantTree());

        QFile f(fn);
        f.open( QIODevice::WriteOnly );
        f.write(jdoc.toJson());
        f.close();
    }
}

void VariantTreeWidget::btnClose_clicked()
{
    m_jmod->destroy();
}

void VariantTreeWidget::btnAdd_clicked()
{
    VariantTreeItem* item;
    QModelIndex idx = m_jview->currentIndex();

    if (idx.isValid()) {
        int column = idx.column();
        if (column > 0) {
            int row = idx.row();
            idx = m_jmod->index(row, 0, idx.parent());
        }
        item = VariantTreeModel::castItemFromIndex(idx);
    } else
        item = m_jmod->rootItem();

    if (item->isArray()) {
        int row = item->childCount();
        m_jmod->insertRow(row, idx);
    } else if (item->isObject())
        m_jmod->insertRow(0, idx);
}

void VariantTreeWidget::btnInsert_clicked()
{
    int row = -1;
    QModelIndex idx = m_jview->currentIndex();

    if (idx.isValid()) {
        row = idx.row();
        idx = idx.parent();
    }

    const VariantTreeItem* item = m_jmod->item(idx);

    if (row > 0)
        m_jmod->insertRow(row, idx);
    else if (item->isArray()) {
        row = item->childCount();
        m_jmod->insertRow(row, idx);
    } else
        m_jmod->insertRow(0, idx);
}

void VariantTreeWidget::btnDelete_clicked()
{
    QModelIndex idx = m_jview->currentIndex();

    if (!idx.isValid())
        return;

    int row = idx.row();
    QModelIndex parent = idx.parent();
    m_jmod->removeRow(row, parent);
}

void VariantTreeWidget::btnChange_clicked()
{
    QModelIndex dst = m_jmod->index(0,0, QModelIndex());
    QModelIndex idx = m_jview->currentIndex();
    QModelIndex parent = idx.parent();

    m_jmod->moveRow(parent, idx.row(), dst, 1);
}

void VariantTreeWidget::btnDown_clicked()
{
    QModelIndex idx = m_jview->currentIndex();

    if (!idx.isValid())
        return;

    QModelIndex parent = idx.parent();

    VariantTreeItem* item = VariantTreeModel::castItemFromIndex(idx);
    VariantTreeItem* parentItem = item->parent();

    if (!parentItem->isArray())
        return;

    int row = idx.row();
    m_jmod->moveRow(parent, row, parent, row + 2);
}

void VariantTreeWidget::btnUp_clicked()
{
    QModelIndex idx = m_jview->currentIndex();

    if (!idx.isValid())
        return;

    QModelIndex parent = idx.parent();

    VariantTreeItem* item = VariantTreeModel::castItemFromIndex(idx);
    VariantTreeItem* parentItem = item->parent();

    if (!parentItem->isArray())
        return;

    int row = idx.row();
    m_jmod->moveRow(parent, row, parent, row - 1);
}
