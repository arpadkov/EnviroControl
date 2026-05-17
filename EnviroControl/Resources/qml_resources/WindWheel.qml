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
}