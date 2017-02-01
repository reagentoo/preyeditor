#ifndef VARIANTTREEWIDGET_H
#define VARIANTTREEWIDGET_H

#include <QTreeView>
#include <QWidget>

#include "jsondelegate.h"
#include "varianttreemodel.h"
#include "yamldelegate.h"

class VariantTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VariantTreeWidget(QWidget *parent = 0);

signals:
public slots:
    void rowMoved();

    void btnOpen_clicked();
    void btnSaveAs_clicked();
    void btnClose_clicked();

    void btnAdd_clicked();
    void btnInsert_clicked();
    void btnDelete_clicked();
    void btnChange_clicked();

    void btnDown_clicked();
    void btnUp_clicked();

private:
    VariantTreeModel* m_jmod;
    QTreeView* m_jview;

    QAction* m_action;
    QMenu* m_menu;
};

#endif // VARIANTTREEWIDGET_H
