import QtQuick 2.15

Item {
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