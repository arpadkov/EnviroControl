import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Item {
    id: rootItem
    width: 100
    height: 100

    // ### Configurable Properties ###
    property real liquidRelativeWidth: 0.5
    property real liquidOffsetX: 0.25
    property real liquidBaseRelativeY: 0.9 // This is the y-position of the liquid's base, relative to the image's height
    property real liquidTopRelativeY: 0.1
    property color liquidColor: "red"
    property real minTemp: -20
    property real maxTemp: 50
    property string thermometerSource: "qrc:/WeatherStation/icons/thermometer_base.svg"

    // ### Internal Calculations ###
    property real tempRange: maxTemp - minTemp
    property real currentTempProportion: (thermometerData.temperature - minTemp) / tempRange
    property real liquidColumnMaxHeight: thermometerImage.height * (liquidBaseRelativeY - liquidTopRelativeY)

    // Final calculated properties
    property real liquidHeight: liquidColumnMaxHeight * currentTempProportion
    property real liquidY: thermometerImage.height * liquidBaseRelativeY - liquidHeight


    Image {
        id: thermometerImage
        source: rootItem.thermometerSource
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit

        Rectangle {
            id: liquid
            color: rootItem.liquidColor
            border.color: "darkred"
            border.width: 1

            width: parent.width * rootItem.liquidRelativeWidth
            x: parent.width * rootItem.liquidOffsetX
            height: rootItem.liquidHeight
            y: rootItem.liquidY
        }
    }
}