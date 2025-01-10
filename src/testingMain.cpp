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

class Date {
    public:
        int day, month, year;
        Date(int day1, int month1, int year1) {
            day = day1;
            month = month1;
            year = year1;
        }
    int daysSince1970() {
        int days = 0;
        for (int i = 1970; i < year; i++) {
            days += 365;
            if (i % 4 == 0) {
                days += 1;
            }
        }
        for (int i = 1; i < month; i++) {
            if (i == 2) {
                days += 28;
                if (year % 4 == 0) {
                    days += 1;
                }
            } else if (i == 4 || i == 6 || i == 9 || i == 11) {
                days += 30;
            } else {
                days += 31;
            }
        }
        days += day;
        return days;
    }
};

int calculate_days_between_dates(Date date1, Date date2) {
    const int oneDay = 24 * 60 * 60 * 1000; // hours*minutes*seconds*milliseconds
    const int firstDate = date1.daysSince1970();
    const int secondDate = date2.daysSince1970();
    const int diffDays = (firstDate - secondDate);
    return diffDays;
}



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

    // float angle = 360.0f; // degrees
    // float limit = 181.0f;
    // cout << "New angle: " << angle % limit <<  endl;
    cout << calculate_days_between_dates(Date(2,5,1986), Date(25,12,2024)) << endl;
    return 0;
}
