#include <QApplication>
#include <QDesktopWidget>

#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "varianttreemodel.h"
#include "varianttreewidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VariantTreeWidget jw;
    jw.show();
    jw.setMinimumSize(800,600);

    QRect desktopRect = QApplication::desktop()->availableGeometry(0); // &jw
    QPoint center = desktopRect.center();
    jw.move(center.x() - jw.width() * 0.5, center.y() - jw.height() * 0.5);


    //TODO: rewrite to QtQuick2
/*
    VariantTreeModel jmod;
    jmod.load("test.json");

    QQmlApplicationEngine engine;
    qmlRegisterUncreatableType<VariantTreeModel>("", 1, 0, "JModel", "Cannot create a JsonModel instance.");
    engine.rootContext()->setContextProperty("jmod", &jmod);
    engine.load(QUrl(QStringLiteral("qrc:///editor.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;
*/
    return app.exec();
}
