import QtQuick 2.6
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQml.Models 2.2

ApplicationWindow {
    visible: true
    title: qsTr("Tree View")

    width: 400
    height: 400

    ColumnLayout {
        anchors.fill: parent

        TreeView {
            id: view

            Layout.fillHeight: true
            Layout.fillWidth: true

            model: jmod
            clip: true

            TableViewColumn {
                role: "display"
                title: "Key"
            }

            TableViewColumn {
                role: "display"
                title: "Value"
            }

            TableViewColumn {
                role: "display"
                title: "type"
            }


            itemDelegate: Text {
                color: styleData.textColor
                verticalAlignment: Text.AlignVCenter

                text: {
                    var index = jmod.index(styleData.index.row, styleData.column, styleData.index.parent)
                    var data = jmod.data(index);
                    return (typeof data === 'undefined') ? String() : data
                }
            }
        }

        Button {
            id: button

            width: 100
            height: 40

            text: "clear"
        }
    }
}
