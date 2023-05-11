/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCanvas3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

Qt.include("three.js")
Qt.include("csg.js")
Qt.include("ThreeCSG.js")
Qt.include("delaunator.js")
Qt.include("perlin.js")
var camera, scene, renderer, controls/*, raycaster, mouse*/;
var cube, result, axesHelper, gridHelper1, gridHelper2,gridHelper3,canvas1;
var pointLight;
var PrevX=40, PrevY=90/*, INTERSECTED, SELECTED*/;
var buttonpressed,leftbuttonpressed/*, offset = new THREE.Vector3()*/;
var sceneObjects = []/*, plane*/;
var radious = 600, theta = 45, onMouseDownTheta = 45, phi = 60, onMouseDownPhi = 60, onMouseDownPosition;
function initializeGL(canvas, eventSource) {
    //raycaster = new THREE.Raycaster();
   // mouse = new THREE.Vector2();
    canvas1 = canvas;
    camera = new THREE.PerspectiveCamera(40, canvas.width / canvas.height, 1, 2000);
    camera.position.z = radious * Math.cos( theta * Math.PI / 360 ) * Math.cos( phi * Math.PI / 360 );
    camera.position.y = radious * Math.sin( phi * Math.PI / 360 );
    camera.position.x = radious * Math.sin( theta * Math.PI / 360 ) * Math.cos( phi * Math.PI / 360 );
    //controls = new THREE.OrbitControls( camera );
   // controls.update();
    scene = new THREE.Scene();
    onMouseDownPosition = new THREE.Vector2();
    var faceMaterial = new THREE.MeshBasicMaterial({
       color: 0x000000,
       wireframe: true,
     });

    var faceMaterial2 = new THREE.MeshBasicMaterial({
       color: 0x00FF00,
     });
    var size = 100;
    var divisions = 10;

    gridHelper1 = new THREE.GridHelper( size, divisions );
    scene.add( gridHelper1 );
    gridHelper2 = new THREE.GridHelper( size, divisions );
    gridHelper2.rotation.x = Math.PI / 2;
    scene.add( gridHelper2 );
    gridHelper3 = new THREE.GridHelper( size, divisions );
    gridHelper3.rotation.z = Math.PI / 2;
    scene.add( gridHelper3 );
    axesHelper = new THREE.AxisHelper( 25 );
    axesHelper.translateX(150);
    scene.add( axesHelper );

    var cube1 = new THREE.Mesh(new THREE.BoxGeometry(canvas.aa,canvas.bb,canvas.dd),new THREE.MeshLambertMaterial({color: 0xFF0000}));
    var cube1Csg	= THREE.CSG.toCSG(cube1);
    var cube3 = new THREE.Mesh(new THREE.BoxGeometry(canvas.aa-canvas.t,canvas.bb-canvas.t,canvas.dd-canvas.t),new THREE.MeshLambertMaterial({color: 0xFF0000}));
    var cube3Csg	= THREE.CSG.toCSG(cube3);
    var cube2 = new THREE.Mesh(new THREE.BoxGeometry(canvas.l,canvas.w,canvas.t),new THREE.MeshLambertMaterial({color: 0xFF0000}));
    cube2.translateX(-canvas.aa/2+canvas.l/2+canvas.xx);
    cube2.translateY(-canvas.bb/2+canvas.w/2+canvas.yy);
    cube2.translateZ(canvas.dd/2);
    var cube2Csg	= THREE.CSG.toCSG(cube2);
    var resultCsg	= (cube1Csg.subtract(cube3Csg)).subtract(cube2Csg);
    var resultGeo	= THREE.CSG.fromCSG( resultCsg );
    cube = new THREE.Mesh( resultGeo,  new THREE.MeshLambertMaterial({color: 0xFF0000}) );
   // scene.add( cube );
    camera.lookAt(scene.position);

    // Lights
    //! [6]
    scene.add(new THREE.AmbientLight(0x444444));
    var directionalLight = new THREE.DirectionalLight(0xffffff, 1.0);
    directionalLight.position.y = 130;
    directionalLight.position.z = 700;
    directionalLight.position.x = Math.tan(canvas.angleOffset) * directionalLight.position.z;
    directionalLight.position.normalize();
    scene.add(directionalLight);
    //! [6]

    //! [4]
    renderer = new THREE.Canvas3DRenderer(
                { canvas: canvas, antialias: true, devicePixelRatio: canvas.devicePixelRatio });
    renderer.setPixelRatio(canvas.devicePixelRatio);
    renderer.setSize(canvas.width, canvas.height);
    setBackgroundColor(canvas.backgroundColor);
    eventSource.mouseMove.connect(onDocumentMouseMove);
    eventSource.mouseDown.connect(onDocumentMouseDown);
    eventSource.mouseUp.connect(onDocumentMouseUp);
    eventSource.mouseWheel.connect(onDocumentMouseWheel);
    //! [4]
}

function setBackgroundColor(backgroundColor) {
    var str = ""+backgroundColor;
    var color = parseInt(str.substring(1), 16);
    if (renderer)
        renderer.setClearColor(color);
}

function resizeGL(canvas) {
    if (camera === undefined) return;
    camera.aspect = canvas.width / canvas.height;
    camera.updateProjectionMatrix();
    renderer.setPixelRatio(canvas.devicePixelRatio);
    renderer.setSize(canvas.width, canvas.height);
}
function onDocumentMouseWheel(x,y) {
  camera.zoom += (y/120)/8;
  // console.log(y);
    camera.updateProjectionMatrix();
}
function onDocumentMouseDown(x,y, buttons) {
    console.log(buttons);
    if(buttons===2){
        buttonpressed = true;
        onMouseDownTheta = theta;
        onMouseDownPhi = phi;
        onMouseDownPosition.x = x;
        onMouseDownPosition.y = y;
    } else {
        leftbuttonpressed = true;
        /*raycaster.setFromCamera( mouse, camera );
        var intersects = raycaster.intersectObjects( scene.children );
        if ( intersects.length > 0 ) {
            SELECTED = intersects[ 0 ].object;
            intersects = raycaster.intersectObject( plane );

            if ( intersects.length > 0 ) {
                offset.copy( intersects[ 0 ].point ).sub( plane.position );
            }

        }*/
    }
}
function onDocumentMouseUp(x,y) {
    buttonpressed = false;
    leftbuttonpressed = false;
    onMouseDownPosition.x = x - onMouseDownPosition.x;
    onMouseDownPosition.y = y - onMouseDownPosition.y;

  /*  if ( INTERSECTED ) {
        plane.position.copy( INTERSECTED.position );
        SELECTED = null;
    }*/
}
function onDocumentMouseMove(x,y) {

    var delta=PrevY-y;
    var delta2=PrevX-x;

   if(buttonpressed){
  /*  cube.rotation.y += (-1*delta)/70;
    axesHelper.rotation.y += (-1*delta)/70;
    gridHelper.rotation.y += (-1*delta)/70;
        cube.rotation.x += (-1*delta2)/70;
        axesHelper.rotation.x += (-1*delta2)/70;
        gridHelper.rotation.x += (-1*delta2)/70;
    } */


   //camera.position.y += (-1*delta)*1.5;
       theta = - ( ( x - onMouseDownPosition.x ) * 0.5 ) + onMouseDownTheta;
       phi = ( ( y - onMouseDownPosition.y ) * 0.5 ) + onMouseDownPhi;

       phi = Math.min( 180, Math.max( 0, phi ) );

       camera.position.x = radious * Math.sin( theta * Math.PI / 360 ) * Math.cos( phi * Math.PI / 360 );
       camera.position.y = radious * Math.sin( phi * Math.PI / 360 ) ;
       camera.position.z = radious * Math.cos( theta * Math.PI / 360 ) * Math.cos( phi * Math.PI / 360 );
       camera.updateMatrix();
       // console.log(camera.position.x,camera.position.y, camera.position.z );
      camera.lookAt( scene.position );

   }
   //mouse.x = ( x / canvas1.width ) * 2 - 1;
   //mouse.y = - ( y / canvas1.height ) * 2 + 1;

   /* var intersects = raycaster.intersectObjects( scene.children );
    if (intersects.length > 0) {
        for ( var i = 0; i < intersects.length; i++ ) {
           // console.log( intersects[ i ] );
         //   intersects[ i ] .position.copy(
            //  intersects[ i ].object.material.color.set( 0xff0000 );
        }
    }*/


/*    raycaster.setFromCamera( mouse, camera );
    if(leftbuttonpressed){

        if ( SELECTED ) {
            var intersects = raycaster.intersectObject( plane );

            if ( intersects.length > 0 ) {
                SELECTED.position.copy( intersects[ 0 ].point.sub( offset ) );
            }
            return;
        }

        var intersects = raycaster.intersectObjects( scene.children );

        if ( intersects.length > 0 ) {

            if ( INTERSECTED != intersects[ 0 ].object ) {

                if ( INTERSECTED ) INTERSECTED.material.color.setHex( INTERSECTED.currentHex );
                INTERSECTED = intersects[ 0 ].object;
                INTERSECTED.currentHex = INTERSECTED.material.color.getHex();
                plane.position.copy( INTERSECTED.position );
                plane.lookAt( camera.position );
            }

        } else {
            if ( INTERSECTED ) INTERSECTED.material.color.setHex( INTERSECTED.currentHex );
            INTERSECTED = null;
        }
    } */
   // console.log(mouse.x, mouse.y)
   // camera.position.y += ( -y/40 - camera.position.y ) * .2;
    PrevY=y;
    PrevX=x;
}
function getRandomColor() {
  var letters = '0123456789ABCDEF';
  var color = '#';
  for (var i = 0; i < 6; i++) {
    color += letters[Math.floor(Math.random() * 16)];
  }
  return color;
}

function deleteObject(index)
{
    scene.remove(sceneObjects[index]);
    sceneObjects.splice(index,1);
}

function doBooleanMinus(f,s, type)
{

  //  console.log(group.children.length);
   // console.log(f,s);
    var obj1Csg	= THREE.CSG.toCSG(sceneObjects[f]) ;
    var obj2Csg	= THREE.CSG.toCSG(sceneObjects[s]) ;
    var result;
    if(type===1){
        result	= obj1Csg.subtract(obj2Csg) ;
    }
    else if(type===2){
        result	= obj1Csg.union(obj2Csg) ;
    }
    else if(type===3){
        result	= obj1Csg.intersect(obj2Csg) ;
    }
    var resultGeo	= THREE.CSG.fromCSG( result );
    var cubean = new THREE.Mesh( resultGeo,  new THREE.MeshLambertMaterial({color: 0xFF0000, wireframe: true}));
    sceneObjects.push(cubean);
    scene.remove(sceneObjects[f]);
    scene.remove(sceneObjects[s]);
    if(f >s){
        sceneObjects.splice(f,1);
        sceneObjects.splice(s,1);
    } else {
        sceneObjects.splice(s,1);
        sceneObjects.splice(f,1);
    }
    scene.add( cubean );
}
function grid(g)
{
    if(g===0){
        gridHelper1.visible  = false;
        gridHelper2.visible  = false;
        gridHelper3.visible  = false;
    } else{
        gridHelper1.visible = true;
        gridHelper2.visible = true;
        gridHelper3.visible = true;
    }
}
function editObject(index,p1,p2,p3,p4,p5,p6,p7,type){
    scene.remove(sceneObjects[index]);
    if(type==="1") {
        sceneObjects[index] = new THREE.Mesh(new THREE.BoxGeometry(Math.abs(p2-p1),Math.abs(p4-p3),Math.abs(p5-p6)),new THREE.MeshLambertMaterial({color: getRandomColor()}));
        sceneObjects[index].translateX((Math.abs(p1) > Math.abs(p2)) ? p1-((Math.abs(p2-p1))/2) : p2-((Math.abs(p2-p1))/2));
        sceneObjects[index].translateY((Math.abs(p3) > Math.abs(p4)) ? p3-((Math.abs(p4-p3))/2) : p4-((Math.abs(p4-p3))/2));
        sceneObjects[index].translateZ((Math.abs(p5) > Math.abs(p6)) ? p5-((Math.abs(p5-p6))/2) : p6-((Math.abs(p5-p6))/2));
    } else if (type==="2"){
        sceneObjects[index] = new THREE.Mesh(new THREE.SphereGeometry(p1,p2,p3),new THREE.MeshLambertMaterial({color: getRandomColor()}));
        sceneObjects[index].translateX(p4);
        sceneObjects[index].translateY(p5);
        sceneObjects[index].translateZ(p6);
    } else if (type==="3"){
        sceneObjects[index]  = new THREE.Mesh(new THREE.CylinderGeometry(p1, p2, p7, p3),new THREE.MeshLambertMaterial({color: getRandomColor()}));
        sceneObjects[index].translateX(p4);
        sceneObjects[index].translateY(p5);
        sceneObjects[index].translateZ(p6);
    }
    console.log(index,p1,p2,p3,p4,p5,p6,p7,type);
    scene.add( sceneObjects[index] );
}
function getValue(index,value){
    return "sceneObjects[index]"
}

function createCube(xMin, xMax, yMin, yMax, zMin, zMax)
{
   // console.log(x,y,z,xpos,ypos,zpos);
    var geometry = new THREE.BoxGeometry(Math.abs(xMax-xMin),Math.abs(yMax-yMin),Math.abs(zMax-zMin));
    var bufferGeometry = new THREE.BufferGeometry().fromGeometry(geometry);
    var cubs = new THREE.Mesh(geometry,new THREE.MeshLambertMaterial({color: getRandomColor()}));
    cubs.translateX((Math.abs(xMin) > Math.abs(xMax)) ? xMin-((Math.abs(xMax-xMin))/2) : xMax-((Math.abs(xMax-xMin))/2));
    cubs.translateY((Math.abs(yMin) > Math.abs(yMax)) ? yMin-((Math.abs(yMax-yMin))/2) : yMax-((Math.abs(yMax-yMin))/2));
    cubs.translateZ((Math.abs(zMin) > Math.abs(zMax)) ? zMin-((Math.abs(zMax-zMin))/2) : zMax-((Math.abs(zMax-zMin))/2));
   // group.add(cubs);
    sceneObjects.push(cubs);

  //  var points3d = [];
 //   (geometry.vertices).forEach(function(element) {
  //     points3d.push([element.x,  element.z]);
 //   });
  //  var indexDelaunay = Delaunator.from(points3d);
  //  var meshIndex = []; // delaunay index => three.js index
  //  for (var  i = 0; i < indexDelaunay.triangles.length; i++){
  //    meshIndex.push(indexDelaunay.triangles[i]);
 //   }
 //   bufferGeometry.setIndex(meshIndex); // add three.js index to the existing geometry
  //  bufferGeometry.computeVertexNormals();
    scene.add(cubs);
    console.log("aza");
    var size = { x: 200, z: 200 };
    var pointsCount = 1000;
    //var points3d = [];
   /* for (var i = 0; i < pointsCount; i++) {
      var x = THREE.Math.randFloatSpread(size.x);
      var z = THREE.Math.randFloatSpread(size.z);
      var y = noise.perlin2(x / size.x * 5, z / size.z * 5) * 50;
      points3d.push(new THREE.Vector3(x, y, z));
    }
*/

     var points3d = [];
     (geometry.vertices).forEach(function(element) {
         points3d.push(new THREE.Vector3(element.x, element.y, element.z));
       //  points3d.push([element.x,element.y,  element.z]);
      });
    var geom = new THREE.BufferGeometry().setFromPoints(points3d);
     console.log("aza");
    //var geom = new THREE.BufferGeometry().setFromObject(bufferGeometry);
    var cloud = new THREE.Points(
      bufferGeometry,
      new THREE.PointsMaterial({ color: "white", size: 10 })
    );
    //scene.add(cloud);

  //  var points13d = [];
    // triangulate x, z
    var indexDelaunay = Delaunator.from(
      points3d.map(function(num){
        return [num.x, num.z];
      })
    );
    var meshIndex = []; // delaunay index => three.js index
    for (var i = 0; i < indexDelaunay.triangles.length; i++){
      meshIndex.push(indexDelaunay.triangles[i]);
    //    console.log(indexDelaunay.triangles[i]);
   }
    indexDelaunay = Delaunator.from(
          points3d.map(function(num){
            return [num.y, num.z];
          })
        );
    for (i = 0; i < indexDelaunay.triangles.length; i++){
      meshIndex.push(indexDelaunay.triangles[i]);
    //    console.log(indexDelaunay.triangles[i]);
   }
    console.log("aza");
 //   var geom1 = geom;
     geom.setIndex(new THREE.BufferAttribute( new Uint16Array( meshIndex ), 1 )); // add three.js index to the existing geometry
     geom.computeVertexNormals();
    var planeMaterial = new THREE.MeshLambertMaterial({ color: "black", wireframe: true })
   var mesh = new THREE.Mesh(geom,planeMaterial);
  //  scene.add(mesh);
 // var planeGeometry = new THREE.PlaneGeometry(60,40,1,1);

  //  var plane = new THREE.Mesh(planeGeometry,planeMaterial);
 //  scene.add(plane);
}

function createCylinder(radTop, radBot, radSegm, x, y, z, height)
{
  //  console.log(x,y,z,xpos,ypos,zpos);
    var geometry = new THREE.CylinderGeometry(radTop, radBot, height, radSegm)
    var bufferGeometry = new THREE.BufferGeometry().fromGeometry(geometry);
    var cylinder  = new THREE.Mesh(bufferGeometry,new THREE.MeshLambertMaterial({color: getRandomColor()}));

    cylinder.translateX(x);
    cylinder.translateY(y);
    cylinder.translateZ(z);
    sceneObjects.push(cylinder);
    scene.add( cylinder );

    var points3d = [];
    (geometry.vertices).forEach(function(element) {
        points3d.push(new THREE.Vector3(element.x, element.y, element.z));
      //  points3d.push([element.x,element.y,  element.z]);
     });
    var geom = new THREE.BufferGeometry().setFromPoints(points3d);
    var indexDelaunay = Delaunator.from(
      points3d.map(function(num){
        return [num.x, num.z];
      })
    );
    var meshIndex = []; // delaunay index => three.js index
    for (var i = 0; i < indexDelaunay.triangles.length; i++){
      meshIndex.push(indexDelaunay.triangles[i]);
    //    console.log(indexDelaunay.triangles[i]);
   }
    console.log("aza");
 //   var geom1 = geom;
     geom.setIndex(new THREE.BufferAttribute( new Uint16Array( meshIndex ), 1 )); // add three.js index to the existing geometry
     geom.computeVertexNormals();
    var planeMaterial = new THREE.MeshLambertMaterial({ color: "black", wireframe: true })
   var mesh = new THREE.Mesh(geom,planeMaterial);
   // scene.add(mesh);
}
function createSphere(rad, verSegm, horSegm, x, y, z)
{
    var geometry = new THREE.SphereGeometry(rad,verSegm,horSegm);
    var bufferGeometry = new THREE.BufferGeometry().fromGeometry(geometry);
  //  console.log(x,y,z,xpos,ypos,zpos);
    var sphere = new THREE.Mesh(geometry,new THREE.MeshLambertMaterial({color: getRandomColor()}));

    sphere.translateX(x);
    sphere.translateY(y);
    sphere.translateZ(z);
    sceneObjects.push(sphere);
    scene.add( sphere );

    var points3d = [];
    (geometry.vertices).forEach(function(element) {
        points3d.push(new THREE.Vector3(element.x, element.y, element.z));
      //  points3d.push([element.x,element.y,  element.z]);
     });
    var geom = new THREE.BufferGeometry().setFromPoints(points3d);
     console.log("aza");
    var indexDelaunay = Delaunator.from(
      points3d.map(function(num){
        return [num.x, num.z];
      })
    );
    var meshIndex = []; // delaunay index => three.js index
    for (var i = 0; i < indexDelaunay.triangles.length; i++){
      meshIndex.push(indexDelaunay.triangles[i]);
    //    console.log(indexDelaunay.triangles[i]);
   }
    console.log("aza");
 //   var geom1 = geom;
     geom.setIndex(new THREE.BufferAttribute( new Uint16Array( meshIndex ), 1 )); // add three.js index to the existing geometry
     geom.computeVertexNormals();
    var planeMaterial = new THREE.MeshLambertMaterial({ color: "black", wireframe: true })
   var mesh = new THREE.Mesh(geom,planeMaterial);
  //  scene.add(mesh);

}
function animate() {
 // resize();
 // cube.rotation.x += 0.01;
 // cube.rotation.y += 0.03;
 //   axesHelper.rotation.y += 0.03;
 //   gridHelper.rotation.y += 0.03;
  //cube.rotation.z += 0.01;
   // controls.update();
    renderer.render(scene, camera);
}
function resize() {
  //var width = renderer.domElement.clientWidth;
 // var height = renderer.domElement.clientHeight;
  //if (renderer.domElement.width !== width || renderer.domElement.height !== height) {
    renderer.setSize(width, height, false);
    camera.aspect = width / height;
    camera.updateProjectionMatrix();
 // }
}
function resizeCube(canvas){
  //console.log(xSize/cube.geometry.parameters.width);
    while(scene.children.length > 0){
        scene.remove(scene.children[0]);
    }
    axesHelper = new THREE.AxisHelper( 25 );
    axesHelper.translateX(canvas.aa*0.8);
    scene.add( axesHelper );
    gridHelper = new THREE.GridHelper( 100, 10 );
    scene.add( gridHelper );
    gridHelper.translateY(-canvas.bb/2);
    var cube1 = new THREE.Mesh(new THREE.BoxGeometry(canvas.aa,canvas.bb,canvas.dd),new THREE.MeshLambertMaterial({color: 0xFF0000}));
    var cube1Csg	= THREE.CSG.toCSG(cube1);
    var cube3 = new THREE.Mesh(new THREE.BoxGeometry(canvas.aa-canvas.t,canvas.bb-canvas.t,canvas.dd-canvas.t),new THREE.MeshLambertMaterial({color: 0xFF0000}));
    var cube3Csg	= THREE.CSG.toCSG(cube3);
    var cube2 = new THREE.Mesh(new THREE.BoxGeometry(canvas.l,canvas.w,canvas.t),new THREE.MeshLambertMaterial({color: 0xFF0000}));
    cube2.translateX(-canvas.aa/2+canvas.l/2+canvas.xx);
    cube2.translateY(-canvas.bb/2+canvas.w/2+canvas.yy);
    cube2.translateZ(canvas.dd/2);
    var cube2Csg	= THREE.CSG.toCSG(cube2);
    var resultCsg	= (cube1Csg.subtract(cube3Csg)).subtract(cube2Csg);
    var resultGeo	= THREE.CSG.fromCSG( resultCsg );
    cube = new THREE.Mesh( resultGeo,  new THREE.MeshLambertMaterial({color: 0xFF0000}));
    scene.add( cube );
    camera.lookAt(cube.position);
 /* var scaleFactorX = xSize / cube.geometry.parameters.width;
  var scaleFactorY = ySize / cube.geometry.parameters.height;
  var scaleFactorZ = zSize / cube.geometry.parameters.depth;
  cube.scale.set( scaleFactorX, scaleFactorY, scaleFactorZ );
  */
    // Lights
    //! [6]
    scene.add(new THREE.AmbientLight(0x444444));
    var directionalLight = new THREE.DirectionalLight(0xffffff, 1.0);
    directionalLight.position.y = 130;
    directionalLight.position.z = 700;
    directionalLight.position.x = Math.tan(canvas.angleOffset) * directionalLight.position.z;
    directionalLight.position.normalize();
    scene.add(directionalLight);
   // setBackgroundColor(cube.backgroundColor);
    //! [6]
}

//! [5]
function paintGL(canvas) {
    cube.rotation.x = canvas.xRotation * Math.PI / 180;
    cube.rotation.y = canvas.yRotation * Math.PI / 180;
    cube.rotation.z = canvas.zRotation * Math.PI / 180;
    renderer.render(scene, camera);
}
//! [5]
