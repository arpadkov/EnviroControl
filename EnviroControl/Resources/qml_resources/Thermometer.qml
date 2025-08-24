import QtQuick 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: mainLayout
    spacing: 5 // Add some space between the text and the image

    Text {
        id: tempLabel
        text: thermometerData.temperature.toFixed(1) + " \u00B0C"
        Layout.alignment: Qt.AlignHCenter
        font.pixelSize: 30
        font.bold: true
        color: "white" // The color of the actual text

        // The manual drop shadow
        Rectangle {
            id: shadow
            anchors.fill: parent
            anchors.margins: -4
            anchors.horizontalCenterOffset: 1 // Offset to the right
            anchors.verticalCenterOffset: 1 // Offset to the bottom
            color: "#70000000" // Semi-transparent black
            radius: 2
            z: -1 // Place it behind the text
        }

    }


    Image {
        id: thermometerImage
        source: "qrc:/WeatherStation/icons/thermometer_base.svg"
        Layout.alignment: Qt.AlignHCenter
        fillMode: Image.PreserveAspectFit

        Layout.preferredWidth: 120 // Example: a fixed width of 80 pixels
        Layout.preferredHeight: 200 // Example: a fixed height of 200 pixels

        // ### Configurable Properties ###
        property real liquidRelativeWidth: 0.18
        property real liquidOffsetX: 0.415
        property real liquidBaseRelativeY: 0.765 // This is the y-position of the liquid's base, relative to the image's height
        property real liquidTopRelativeY: 0.1
        property real minTemp: -20
        property real maxTemp: 50

        property real bulb_ratio: 2
        property real bulb_overlap: 0.2

        // ### Internal Calculations ###
        property real tempRange: maxTemp - minTemp
        property real currentTempProportion: (thermometerData.temperature - minTemp) / tempRange
        property real liquidColumnMaxHeight: thermometerImage.height * (liquidBaseRelativeY - liquidTopRelativeY)

        // The dynamic liquid color
        property color coldColor: "#6495ED"
        property color midColor: "#F4A460"
        property color hotColor: "#B22222"
        property real minColorTemp: -5
        property real maxColorTemp: 30
        property real midColorTempSwitch: 20
        property color liquidColor: {
            var r, g, b;
            if (thermometerData.temperature < midColorTempSwitch) {
                // Interpolate between coldColor and midColor
                var coldToMidNorm = Math.max(0, Math.min(1, (thermometerData.temperature - minColorTemp) / (midColorTempSwitch - minColorTemp)));
                r = (1 - coldToMidNorm) * Qt.color(coldColor).r + coldToMidNorm * Qt.color(midColor).r;
                g = (1 - coldToMidNorm) * Qt.color(coldColor).g + coldToMidNorm * Qt.color(midColor).g;
                b = (1 - coldToMidNorm) * Qt.color(coldColor).b + coldToMidNorm * Qt.color(midColor).b;
            } else {
                // Interpolate between midColor and hotColor
                var midToHotNorm = Math.max(0, Math.min(1, (thermometerData.temperature - midColorTempSwitch) / (maxColorTemp - midColorTempSwitch)));
                r = (1 - midToHotNorm) * Qt.color(midColor).r + midToHotNorm * Qt.color(hotColor).r;
                g = (1 - midToHotNorm) * Qt.color(midColor).g + midToHotNorm * Qt.color(hotColor).g;
                b = (1 - midToHotNorm) * Qt.color(midColor).b + midToHotNorm * Qt.color(hotColor).b;
            }
            return Qt.rgba(r, g, b, 1);
        }

        // Final calculated properties
        property real liquidHeight: liquidColumnMaxHeight * currentTempProportion
        property real liquidY: thermometerImage.height * liquidBaseRelativeY - liquidHeight

        Rectangle {
            id: liquid
            color: thermometerImage.liquidColor
            border.width: 0

            width: parent.width * thermometerImage.liquidRelativeWidth
            x: parent.width * thermometerImage.liquidOffsetX
            height: thermometerImage.liquidHeight
            y: thermometerImage.liquidY
        }

        Rectangle {
            id: liquidBulb
            color: thermometerImage.liquidColor
            border.width: 0

            radius: width / 2
            
            // Position the circle just below the liquid
            x: liquid.x + liquid.width / 2 - width / 2
            y: liquid.y + liquid.height - (radius * thermometerImage.bulb_overlap)

            width: liquid.width * thermometerImage.bulb_ratio
            height: liquid.width * thermometerImage.bulb_ratio
        }
    }
}
