import QtQuick 2.15

Item {
    // configurable background: default transparent, set to "white" to force white background
    property color backgroundColor: "white"

    // background rectangle so the control can be made white or transparent
    Rectangle {
        anchors.fill: parent
        color: backgroundColor
    }
    width: 200
    height: 200

    Image {
        id: windWheel
        source: "qrc:/WeatherStation/icons/wind_wheel.svg"
        anchors.centerIn: parent
        width: 150
        height: 150

        // The rotation property is driven by a continuous animation
        rotation: 0
        SequentialAnimation on rotation {
            loops: Animation.Infinite
            
            // This NumberAnimation runs from 0 to 360 degrees
            NumberAnimation {
                to: 360
                duration: 20000 / (windWheelData.windSpeed + 1)
                easing.type: Easing.Linear
            }
        }
    }

    // Wind speed label in a rounded box (meters per second)
    Rectangle {
        id: windSpeedBox
        anchors.top: windWheel.bottom
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        color: "#22000000" // subtle translucent background
        radius: 10
        border.width: 0

        // subtle shadow behind the rounded box
        Rectangle {
            anchors.fill: parent
            x: 1
            y: 1
            color: "#70000000"
            radius: parent.radius
            z: -1
        }

        // text showing wind speed
        Text {
            id: windSpeedLabel
            text: windWheelData.windSpeed.toFixed(1) + " m/s"
            anchors.centerIn: parent
            font.pixelSize: 30
            font.bold: true
            color: "white"
        }

        // size the box based on the text with some padding
        width: windSpeedLabel.paintedWidth + 24
        height: windSpeedLabel.paintedHeight + 12
    }
}