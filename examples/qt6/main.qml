import QtQuick

Rectangle {
	width: 600
	height: 300
	focus: true

	Rectangle {
		y: 63
		width: parent ? parent.width : 0
		height: 1
		color: "red"

		Timer {
			property bool toggle
			interval: 1000
			repeat: true
			onTriggered: { toggle = !toggle; parent.color = (toggle ? "blue" : "red") }
			running: true
		}
	}
}
