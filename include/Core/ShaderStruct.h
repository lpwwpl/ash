#pragma once
#ifndef SHADERSTRUCT_H
#define SHADERSTRUCT_H

#include <QVector4D>
struct ShaderAxisInfo { // struct size: 64
 //                         // base align  // aligned offset
    float hostModelMat[16];    // 64          // 0
};// shaderAxisInfo;

struct ShaderCameraInfo { // struct size: 144
    //                         // base align  // aligned offset
    float projMat[16];         // 64          // 0
    float viewMat[16];         // 64          // 64
    QVector4D cameraPos;       // 16          // 128
};// shaderCameraInfo;

struct ShaderAmbientLight { // struct size: 16
    //                         // base align  // aligned offset
    QVector4D color;           // 16          // 0
};

struct ShaderDirectionalLight { // struct size: 32
    //                         // base align  // aligned offset
    QVector4D color;           // 16          // 0
    QVector4D direction;       // 16          // 16
};

struct ShaderPointLight { // struct size: 48
    //                           // base align  // aligned offset
    QVector4D color;             // 16          // 0
    QVector4D pos;               // 16          // 16
    QVector4D attenuation;       // 16          // 32
};

struct ShaderSpotLight { // struct size: 80
    //                           // base align  // aligned offset
    QVector4D color;             // 16          // 0
    QVector4D pos;               // 16          // 16
    QVector4D direction;         // 16          // 32
    QVector4D attenuation;       // 16          // 48
    QVector4D cutOff;            // 16          // 64
};

struct ShaderlightInfo { // struct size: 1424
    //                                          // base align  // aligned offset
    int ambientLightNum;                        // 4           // 0
    int directionalLightNum;                    // 4           // 4
    int pointLightNum;                          // 4           // 8
    int spotLightNum;                           // 4           // 12
    ShaderAmbientLight ambientLight[8];         // 16          // 16
    ShaderDirectionalLight directionalLight[8]; // 32          // 144
    ShaderPointLight pointLight[8];             // 48          // 400
    ShaderSpotLight spotLight[8];               // 80          // 784
};
#endif