#pragma once

#define PI 3.14159265359

#include <cmath>

inline void panningEqualPower(float panning, float& left, float& right) {
	float x = PI / 4 * (panning + 1.0f); // panning -- -1 means all to the left and 1 to the right
	left = std::cos(x);
	right = std::sin(x);
}