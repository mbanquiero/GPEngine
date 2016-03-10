//constraint

#pragma once

#include "stdafx.h"
#include "gpe_math.h"
#include "math.h"
#include "body.h"


class Constraint
{
public:
	Body *A;
	Body *B;
	Body *C;
	float dist;
	float angle;
	Vector2 ptA;
	Vector2 ptB;
	Vector2 ptC;
	bool joint;
	bool exact_distance;
	bool angle_constraint;
	char visible;
	void Solve();
	void SolveAngleConstraint();
};

