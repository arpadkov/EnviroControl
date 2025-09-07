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
                ListElement { data: "East"; direction: 0; label_offset_x: -10; label_offset_y: -22 }
                ListElement { data: "South"; direction: -90; label_offset_x: 17; label_offset_y: -10 }
                ListElement { data: "West"; direction: 180; label_offset_x: 10; label_offset_y: -22 }
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

            // Container for the value text labels
            Item {
                id: textContainer
                anchors.fill: parent

                // Add Text labels to the segments
                Instantiator {
                    model: sunRaysModel
                    delegate: Text {
                        property real value: {
                            if (model.data === "East") {
                                return sunPlotData.sunEast;
                            } else if (model.data === "West") {
                                return sunPlotData.sunWest;
                            } else {
                                return sunPlotData.sunSouth;
                            }
                        }
                        
                        text: (value * 99).toFixed(0)
                        color: "black"
                        font.pixelSize: 20
                        font.bold: true

                        // Position the text in the middle of each segment
                        property real textRadius: sunPlot.max_radius * 0.55 // Adjust this value to change the distance from the center
                        property var textPosition: sunPlot.caartesianCoordsFromPolar(model.direction, textRadius)
                        x: textPosition.x - width / 2 + model.label_offset_x
                        y: textPosition.y - height / 2 + model.label_offset_y
                    }

                    onObjectAdded: function(index, obj) {
                        sunRays.data.push(obj)
                    }
                    onObjectRemoved: function(index, obj) {
                        sunRays.data.splice(sunRays.data.indexOf(obj), 1)
                    }

                }
            }

                        // New section for the directional arrows and labels
            Shape {
                id: directionalArrowsAndLabels
                anchors.fill: parent

                Instantiator {
                    model: sunRaysModel
                    delegate: Item { // An Item to group the line, arrowhead, and text
                        property real lineLength: sunPlot.max_radius
                        property real labelOffset: 0 // Distance of the label from the arrowhead

                        property var endPoint: sunPlot.caartesianCoordsFromPolar(model.direction, lineLength)
                        property var labelPoint: sunPlot.caartesianCoordsFromPolar(model.direction, lineLength + labelOffset)

                        // Arrow Line (ShapePath within this Item's coordinate system)
                        Shape {
                            ShapePath {
                                strokeColor: "black"
                                strokeWidth: 2

                                //PathLine { x: 100; y: 100 }
                                
                                PathMove { x: sunPlot.centerX; y: sunPlot.centerY } // Start from the center
                                PathLine { x: endPoint.x; y: endPoint.y } // End at the outer radius
                            }
                        }

                        // Arrowhead (a small Shape positioned at the end of the line)
                        Shape {
                            x: endPoint.x // Position at the end of the line
                            y: endPoint.y
                            width: 20
                            height: 20
                            antialiasing: true
                            
                            // Manually correct arrowhed rotation (because i am dumm)
                            property real additional_rotation: {
                                if (model.data === "East") {
                                    return 90;
                                } else if (model.data === "West") {
                                    return -90;
                                } else {
                                    return 180; // South
                                }
                            }

                            rotation: additional_rotation
                            transformOrigin: Item.TopLeft    

                            ShapePath {
                                fillColor: "black"
                                PathMove { x: 0; y: -10 } // Pointy top
                                PathLine { x: 10; y: 10 } // Bottom right
                                PathLine { x: -10; y: 10 } // Bottom left
                                PathLine { x: 0; y: -10 } // Close path
                            }
                        }

                        // Directional Label Text (E, W, S)
                        Text {
                            text: model.data.charAt(0) // Get the first letter (E, S, W)
                            color: "black"
                            font.pixelSize: 25
                            font.bold: true
                            // Position it based on the labelPoint
                            x: labelPoint.x - width / 2 + model.label_offset_x
                            y: labelPoint.y - height / 2 + model.label_offset_y
                            //x: labelPoint.x + label_offset.x
                            //y: labelPoint.y + label_offset.y
                        }
                    }

                    onObjectAdded: function(index, obj) {
                        obj.parent = directionalArrowsAndLabels
                    }
                }
            }  // End of

        }
    }
}