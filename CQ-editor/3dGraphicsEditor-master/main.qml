import QtQuick 2.11
import QtQuick.Controls 2.4
import Qt.labs.platform 1.0 as Lab
import QtQuick.Layouts 1.11

ApplicationWindow {
    id: window1;
    property var typeofobj;
    property bool editing: false;
    property bool selectingofObj: false;
    property int indexofObj : 0;
    property int first;
    property var objects : [];
    visible: true
    width: 640
    height: 480
    title: qsTr("3d structure Editor")
    ListModel {
        id: objectsModel
    }
    Component {
        id: objectsDelegate
        Item {
            width: window1.width*0.2; height: 40
            Column {
                Text { text: '<b>Name:</b> ' + name }
                Text { text: '<b>Number:</b> ' + number }
                Text { text: '<b>Type:</b> ' + type }
            }
            Menu{
                id: objectPopup
                width: window1.width*0.2;
                MenuItem {
                    text: "Edit..."
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            editing = true;
                            popup.open();
                            objectPopup.close();
                        }
                    }
                }
                MenuItem {
                    text: "Delete..."
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            imageCube.deleteObject(index)
                            objectsModel.remove(index,1);
                            objectsList.currentIndex = -1;
                        }
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    objectsList.currentIndex = index
                    if(selectingofObj)
                    {
                        console.log("selecting of object", first, index);
                        imageCube.doBooleanMinus(first,index,dialogOperation.typeofoperation);
                        selectingofObj = false;
                        if(first> index){
                            objectsModel.remove(first,1);
                            objectsModel.remove(index,1);
                        }
                        else {
                            objectsModel.remove(index,1);
                            objectsModel.remove(first,1);
                        }
                        objectsModel.append({"number": first.toString(), "name": "MergedObj", "type": "4"});
                        objectsList.currentIndex = -1;
                    } else if(mouse.button & Qt.RightButton) {
                        objectPopup.open();
                        objectPopup.x = this.x;
                        objectPopup.y = this.y+30;
                    }
                }
            }
        }
    }
    RowLayout{
        anchors.left: parent.left
        Button{
            text: "View"
            id: viewButton
            onClicked: {
                viewMenu.open()
                viewMenu.y = viewButton.y+ viewButton.height;
            }
            Menu{
                id: viewMenu
                width: viewButton.width;
                MenuItem {
                    id: gridMenuItem
                    text: "Grid"
                    checked: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (gridMenuItem.checked)  {
                                gridMenuItem.checked=false;
                                imageCube.grid(0);
                            } else {
                                gridMenuItem.checked=true;
                                imageCube.grid(1);
                            }
                        }
                    }
                    indicator: Item {
                        anchors.verticalCenter: parent.verticalCenter
                        implicitWidth: 40
                        implicitHeight: 40
                        Rectangle {
                            width: 26
                            height: 26
                            anchors.centerIn: parent
                            border.color: "gray"
                            radius: 3
                            Rectangle {
                                width: 14
                                height: 14
                                anchors.centerIn: parent
                                visible: gridMenuItem.checked
                                color: "gray"
                                radius: 2
                            }
                        }
                    }
                }
            }
        }
        Button{
            text: "Create Object"
            id: createOBJbutton
            onClicked: {
                createobjPopup.open()
                createobjPopup.y = createOBJbutton.y+ createOBJbutton.height;
            }
            Menu{
                id: createobjPopup
                width: createOBJbutton.width;
                MenuItem {
                    text: "Cube"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            typeofobj = 1;
                            popup.open()
                            createobjPopup.close()
                        }
                    }
                }
                MenuItem {
                    text: "Sphere"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            typeofobj = 2;
                            popup.open()
                            createobjPopup.close()
                        }
                    }
                }
                MenuItem {
                    text: "Cylinder"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            typeofobj = 3;
                            popup.open()
                            createobjPopup.close()
                        }
                    }
                }
            }
        }
        Button{
            text: "Boolean Operation"
            id: booleanButton
            onClicked: {
                booleanPopup.open()
               //booleanPopup.x = booleanButton.x;
                booleanPopup.y = booleanButton.y+ booleanButton.height;
            }
            Menu{
                id: booleanPopup
                width: booleanButton.width;
                MenuItem {
                    text: "Minus"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            booleanPopup.close()
                            dialogOperation.open()
                            dialogOperation.typeofoperation = 1
                        }
                    }
                }
                MenuItem {
                    text: "Plus"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            booleanPopup.close()
                            dialogOperation.open()
                            dialogOperation.typeofoperation = 2
                        }
                    }
                }
                MenuItem {
                    text: "Intersect"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            booleanPopup.close()
                            dialogOperation.open()
                            dialogOperation.typeofoperation = 3
                        }
                    }
                }
            }
        }

    }
    Dialog {
        id: dialogOperation
        property var typeofoperation
        x: window1.width/2-40
        y: window1.height/2-40
        title: "Select second object"
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 200
        height: 100
        onAccepted:{
            first = objectsList.currentIndex;
            selectingofObj = true;
           console.log("Ok clicked")
        }
        onRejected: console.log("Cancel clicked")
    }
    Rectangle {
        width: parent.width*0.2; height: parent.height
        border.color: "black"
        border.width: 2
           id: objectsPop
           anchors.top: parent.top
           anchors.right: parent.right
          // modal: true
           focus: true
          // closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
           ListView {
                 id: objectsList
                 anchors.fill: parent
                 model: objectsModel
                 delegate: objectsDelegate
                 highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
                 focus: true

             }
    }
    Popup {
           id: popup
           x: window1.width/2 - this.width/2
           y: window1.height/2 - this.height/2
           width: 280
           height: 300
           modal: true
           focus: true
           closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
           onClosed: {
               editing = false;
           }

           Column{
               anchors.fill: parent;
               RowLayout {
                   width: parent.width*0.8
                   Text{
                       width: parent.width /4
                       Layout.fillWidth: true
                       text:(typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? 'xMin' :
                            (typeofobj === 2 || (editing && objectsModel.get(objectsList.currentIndex).type === "2")) ? 'Радиус' :
                            (typeofobj === 3 || (editing && objectsModel.get(objectsList.currentIndex).type === "3")) ? 'Верх.радиус' :
                            '';
                   }
                   Text{
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? 'xMax' :
                                                 (typeofobj === 2 || (editing && objectsModel.get(objectsList.currentIndex).type === "2")) ? 'Верт.сегм' :
                                                 (typeofobj === 3 || (editing && objectsModel.get(objectsList.currentIndex).type === "3")) ? 'Ниж.радиус' :
                                                 '';
                   }
               }
               RowLayout {
                   width: parent.width*0.8
                   TextField  {
                       id: param1
                       width: parent.width /4
                       Layout.fillWidth: true
                       text:
                           if(editing){
                             imageCube.getValue(objectsList.currentIndex,1);
                           }
                            else{
                                (typeofobj === 1) ? '-40' :
                                (typeofobj === 2) ? '50' :
                                (typeofobj === 3) ? '50' :
                                '';
                            }
                       cursorVisible: false
                   }
                   TextField  {
                       id: param2
                       width: parent.width /4
                       Layout.fillWidth: true
                       text:  (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? '40' :
                                                  (typeofobj === 2|| (editing && objectsModel.get(objectsList.currentIndex).type === "2")) ? '10' :
                                                  (typeofobj === 3 || (editing && objectsModel.get(objectsList.currentIndex).type === "3")) ? '50' :
                                                  '';
                       cursorVisible: false
                   }
               }
               RowLayout {
                   width: parent.width*0.8
                   Text{
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? 'yMin' :
                                                 (typeofobj === 2 || (editing && objectsModel.get(objectsList.currentIndex).type === "2")) ? 'Гор. сегм' :
                                                 (typeofobj === 3 || (editing && objectsModel.get(objectsList.currentIndex).type === "3")) ? 'Рад. сегм' :
                                                 '';
                   }
                   Text{
                       width: parent.width /4
                       Layout.fillWidth: true
                       text:
                           (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? "yMax" : "xPos";
                   }
               }
               RowLayout {
                   width: parent.width*0.8
                   spacing: 5
                   TextField  {
                       id: param3
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? "-40" : "10";
                       cursorVisible: false
                   }
                   TextField  {
                       id: param4
                       width: parent.width/4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? "40" : "0";
                       cursorVisible: false
                   }
               }
               RowLayout {
                   width: parent.width*0.8
                   Text{
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? "zMin" : "yPos";
                   }
                   Text{
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? "zMax" : "zPos";
                   }
               }
               RowLayout {
                   width: parent.width*0.8
                   TextField  {
                       id: param5
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? "-40" : "0";
                       cursorVisible: false
                   }
                   TextField  {
                       id: param6
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: (typeofobj === 1 || (editing && objectsModel.get(objectsList.currentIndex).type === "1")) ? "40" : "0";
                       cursorVisible: false
                   }
               }
               RowLayout {
                   width: parent.width*0.8
                   TextField  {
                       id: param7
                       width: parent.width /4
                       Layout.fillWidth: true
                       text: "40";
                       cursorVisible: false
                       visible: typeofobj === 3 ? true : false;
                   }
               }
                RowLayout {
                   Button{
                       onClicked: {
                            if(editing){
                                imageCube.editObject(objectsList.currentIndex,param1.text, param2.text, param3.text, param4.text, param5.text, param6.text, param7.text,objectsModel.get(objectsList.currentIndex).type)
                                editing=false;
                            } else{
                                if(typeofobj === 1){
                                    indexofObj++;
                                    imageCube.createCube(param1.text, param2.text, param3.text, param4.text, param5.text, param6.text);
                                    objectsModel.append({"number": indexofObj.toString(), "name": "Cube", "type": "1"});
                                    //objects.push() LOOK HERE
                                } else if (typeofobj === 2){
                                    indexofObj++;
                                    objectsModel.append({"number": indexofObj.toString(), "name": "Sphere", "type": "2"})
                                    imageCube.createSphere(param1.text, param2.text, param3.text, param4.text, param5.text, param6.text);
                                } else if (typeofobj === 3){
                                    indexofObj++;
                                    objectsModel.append({"number": indexofObj.toString(), "name": "Cylinder", "type": "3"})
                                    imageCube.createCylinder(param1.text, param2.text, param3.text, param4.text, param5.text, param6.text, param7.text);
                                }
                            }
                            popup.close();
                       }
                       text: editing ? "Save" : "Create"
                   }
                   Button{
                        text: "Cancel"
                       onClicked: {
                                popup.close();
                                editing=false;
                           }
                   }
                }
           }
       }


    ImageCube {
        id: imageCube
        width: parent.width*0.8
        height: parent.height * 0.8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.bottomMargin: 30
        //anchors.horizontalCenter: parent.horizontalCenter
        aa: 10*5
        bb: 10*5
        dd: 10*5
        t: 1*5
        w: 1*5
        l: 1*5
        xx: 1*5
        yy: 1*5
        //! [0]
        angleOffset: -180 / 8.0
    }
}
