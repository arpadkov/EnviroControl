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

            property real max_radius: centerX

            // Define the coordinate calculation function
            function caartesianCoordsFromPolar(angleInDegrees, radius) {
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

            ListModel {
                id: sunRaysModel
                ListElement {data: "East"; direction: 0} // East
                ListElement {data: "South"; direction: -90} // South
                ListElement {data: "West"; direction: 180} // West
            }
        
            Shape {
                id: coordinateSystem
                anchors.fill: parent
        
                Instantiator {
                    model: semiCircleModel
                    delegate: ShapePath {
                        strokeColor: "black"
                        strokeWidth: 1

                        property real radius: sunPlot.max_radius * model.ratio

                        property var startPoint: sunPlot.caartesianCoordsFromPolar(45, radius)
                        property var endPoint: sunPlot.caartesianCoordsFromPolar(135, radius)
                         
        
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
                id: sunRays
                anchors.fill: parent

                Instantiator {
                    model: sunRaysModel
                    delegate: ShapePath {
                        fillColor: "#90FFFF00"
                        strokeWidth: 1

                        property real smallRadius: sunPlot.max_radius * 0.1

                        property real largeRadius: {
                            var value;
                            if (model.data === "East") {
                                value = sunPlotData.sunEast;
                            } else if (model.data === "West") {
                                value = sunPlotData.sunWest;
                            } else {
                                value = sunPlotData.sunSouth;
                            }
                            return sunPlot.max_radius * value;
                        }

                        property real polarCoverage: 80 // Degrees (leave some empty space between the rays for 90 deg segments)
                        property real startDirection: model.direction - polarCoverage / 2
                        property real endDirection: model.direction + polarCoverage / 2

                        property var startPointSmallArc: sunPlot.caartesianCoordsFromPolar(startDirection, smallRadius)
                        property var endPointSmallArc: sunPlot.caartesianCoordsFromPolar(endDirection, smallRadius)

                        property var startPointLargeArc: sunPlot.caartesianCoordsFromPolar(startDirection, largeRadius)
                        property var endPointLargeArc: sunPlot.caartesianCoordsFromPolar(endDirection, largeRadius)

                        // Draw the rays as closed shape:
                        // startPointSmallArc -> Line to startPointLargeArc -> Arc to endPointLargeArc -> Line to endPointSmallArc -> Arc back to startPointSmallArc
                        PathMove { x: startPointSmallArc.x; y: startPointSmallArc.y }                    
                        PathLine { x: startPointLargeArc.x; y: startPointLargeArc.y }

                        PathArc {
                            x: endPointLargeArc.x;
                            y: endPointLargeArc.y;
                            radiusX: largeRadius;
                            radiusY: largeRadius;
                            useLargeArc: false
                            direction: PathArc.Counterclockwise
                        }

                        PathLine { x: endPointSmallArc.x; y: endPointSmallArc.y }              

                        PathArc {
                            x: startPointSmallArc.x;
                            y: startPointSmallArc.y;
                            radiusX: smallRadius;
                            radiusY: smallRadius;
                            useLargeArc: false
                            direction: PathArc.Clockwise
                        }

                    }

                    onObjectAdded: function(index, obj) {
                        sunRays.data.push(obj)
                    }
                    onObjectRemoved: function(index, obj) {
                        sunRays.data.splice(sunRays.data.indexOf(obj), 1)
                    }
                }

              
            } // End of Shape sunPath
        }
    }
}