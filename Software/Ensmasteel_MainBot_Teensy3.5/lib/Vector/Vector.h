/**   Ensmasteel Library - Vector modelisation
 * 
 * author : Arthur FINDELAIR - EnsmaSteel, github.com/ArthurFDLR
 * date : October 2019
*/

#ifndef VECTOR_H_
#define VECTOR_H_

#include "Arduino.h"
#include "Logger.h"

float normalizeAngle(float angle);

class Vector
{
public:
    float _x, _y;
    Vector operator+(const Vector &other);
    void operator+=(const Vector &other);
    Vector operator-(const Vector &other);
    float operator%(const Vector &other); //Produit scalaire
    Vector operator*(const float scalaire); //Produit par un scalaire (homotetie)
    bool operator==(Vector const &other);
    float norm();
    float angle();
    float distanceWith(Vector &other);
    void print(const String& prefix="",bool info=false);
    void toTelemetry(const String& prefix="");
    Vector(float x = 0.0, float y = 0.0);
};

Vector directeur(float theta);

class VectorE : public Vector
{
public:
    float _theta;
    void normalizeTheta();
    void print(const String& prefix="",bool info=false);
    void toTelemetry(const String& prefix="");
    bool operator==(VectorE const &other);
    VectorE(float x = 0.0, float y = 0.0, float theta = 0.0);
};



class Cinetique : public VectorE
{
public:
    float _v;
    float _w;
    void print(const String& prefix="",bool info=false);
    void toTelemetry(const String& prefix="");
    bool operator==(Cinetique const &other);
    Cinetique(float x = 0.0, float y = 0.0, float theta = 0.0, float v = 0.0, float w = 0.0);
};

#endif
