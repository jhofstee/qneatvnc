import QtQuick

ListView {
	width: 600;
	height: 300

	model: ObjectModel {
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
