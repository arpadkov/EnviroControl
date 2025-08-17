import QtQuick 2.15
import QtQuick.Shapes 1.15
import QtQuick.Layouts 1.15

Item {
    id: rootItem
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        Item {
            id: sunPlot
            Layout.preferredWidth: Math.min(parent.width, parent.height)
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            property real centerX: sunPlot.width / 2
            property real centerY: sunPlot.height / 2

            // Define the coordinate calculation function
            function getCoordinates(angleInDegrees, radius) {
                var angleInRadians = angleInDegrees * Math.PI / 180;
                var x = sunPlot.centerX + radius * Math.cos(angleInRadians);
                var y = sunPlot.centerY - radius * Math.sin(angleInRadians);
                return { x: x, y: y };
            }

            ListModel {
                id: semiCircleModel
                ListElement { ratio: 1}
                ListElement { ratio: 0.75}
                ListElement { ratio: 0.5}
                ListElement { ratio: 0.25}
            }
        
            Shape {
                id: coordinateSystem
                anchors.fill: parent
        
                Instantiator {
                    model: semiCircleModel
                    delegate: ShapePath {
                        strokeColor: "black"
                        strokeWidth: 1

                        property real radius: sunPlot.centerX * model.ratio

                        property var startPoint: sunPlot.getCoordinates(45, radius)
                        property var endPoint: sunPlot.getCoordinates(135, radius)
                         
        
                        PathMove { x: startPoint.x; y: startPoint.y }
                        PathArc {
                            x: endPoint.x;
                            y: endPoint.y;
                            radiusX: radius;
                            radiusY: radius;
                            useLargeArc: true
                            direction: PathArc.Clockwise
                        }
                    }
        
                    onObjectAdded: function(index, obj) {
                        coordinateSystem.data.push(obj)
                    }
                    onObjectRemoved: function(index, obj) {
                        coordinateSystem.data.splice(coordinateSystem.data.indexOf(obj), 1)
                    }
                }
            } // End of Shape coordinateSystem

            Shape {
                id: sunPathSouth
                anchors.fill: parent

                ShapePath {
                    id: sunPathShape
                    fillColor: "#90FFFF00"
                    strokeWidth: 1

                    property real radius: sunPlot.centerX * 0.9

                    property var startPoint: sunPlot.getCoordinates(-50, radius)
                    property var endPoint: sunPlot.getCoordinates(-130, radius)
                    
                    PathMove { x: sunPlot.centerX; y: sunPlot.centerY }
                    PathLine { x: sunPathShape.startPoint.x; y: sunPathShape.startPoint.y }
                    PathArc {
                        x: sunPathShape.endPoint.x;
                        y: sunPathShape.endPoint.y;
                        radiusX: sunPathShape.radius;
                        radiusY: sunPathShape.radius;
                        useLargeArc: false
                        direction: PathArc.Clockwise
                    }
                }
            } // End of Shape sunPath
        }
    }
}