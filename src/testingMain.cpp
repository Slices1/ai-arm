#include <iostream>
#include <cstring> // For strcpy
#include <cmath> // For trig functions
#include <algorithm> // for std::max and std::min
using namespace std;

class Vec2 {
    public: 
        float x, y;
    
    Vec2(float a = 0.0, float b = 0.0) {
        x = a;
        y = b;
    }
    
    // Scalar Vector product using operator*
    public: Vec2 operator*(float other) {
        return Vec2(x * other, y * other);
    }

    // Scalar Vector quotient using operator/
    public: Vec2 operator/(float other) {
        return Vec2(x / other, y / other);
    }

    // Dot product using operator*
    float operator*(const Vec2& other) const {
        return (x * other.x) + (y * other.y);
    }

    // Vec2 addition using operator+
    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    // Vec2 += addition using operator+=
    Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Vec2 subtraction using operator-
    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }
};

float LengthOfVector(Vec2 myVector) {
    return sqrt(myVector.x*myVector.x + myVector.y*myVector.y);
}

Vec2 Normalise(Vec2 myVector) {
    return myVector/LengthOfVector(myVector);
}

Vec2 Perpendicular(Vec2 myVector) { // 90 degree anti-clockwise rotation
    return Vec2(-myVector.y, myVector.x);
}

typedef struct StaticCollider { // must be such that left side of collider is the exterior
    Vec2 startPos;
    Vec2 endPos;
    Vec2 direction;
    StaticCollider(Vec2 startPos1, Vec2 endPos1) {
        startPos = startPos1;
        endPos = endPos1;
        direction = Normalise(endPos - startPos);
    } // constructor
} StaticCollider;

// void incrementBy10(int *input) {
//     *input += 10;
// }
 
// void increment(StaticCollider colliders[]) {
//     for (StaticCollider collider : colliders) {
//         cout << "collider dir x = " << collider.direction.x << endl;
//     }
//     colliders[0].direction.x = 100.;
// }

int main() {
    StaticCollider colliders[2] = {
        StaticCollider(Vec2(1,1),Vec2(2,2)),
        StaticCollider(Vec2(0,0),Vec2(2,3))
    };
    // cout << a << endl << "incrementing" << endl;
    // increment(colliders); 
    // cout << a << endl;
    // for (StaticCollider collider : colliders) {
        // cout << "collider dir x = " << collider.direction.x << endl;
// 
// 
    // }

    float angle = 360.0f; // degrees
    float limit = 181.0f;
    cout << "New angle: " << angle % limit <<  endl;
    return 0;
}
