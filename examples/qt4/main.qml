import QtQuick 1.1

ListView {
    width: 600;
    height: 300

    model: VisualItemModel {
        Rectangle {
            width:  parent.width
            height: 100
            color: "red"
        }

        Rectangle {
            width:  parent.width
            height: 100
            color: "white"
        }

        Rectangle {
            width:  parent.width
            height: 100
            color: "blue"
        }
    }
}
