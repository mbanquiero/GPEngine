#pragma once



#include "stdafx.h"
#include "gpe_math.h"
#include "math.h"
#include "body.h"
#include "contact.h"
#include "constraint.h"

#define MAX_BODIES				1000
#define MAX_CONSTRAINT			1000


extern float gravityScale;
extern Vector2 gravity;
extern float fixed_dt;
extern char status_bar[255];


class World
{
	public:

	// tiempo global
	float time;
	// cuerpos rigidos 
	int m_bodyCount;
	Body *bodies[MAX_BODIES];

	// Contactos entre los cuerpos rigidos
	int m_contactCount;
	ContactSolver contacts[MAX_BODIES*MAX_BODIES*2];

	// restricciones 
	int m_constraintCount;
	Constraint *constraints[MAX_CONSTRAINT];

	World();
	~World();

	void Update(float elapsed_time);
	void Render( HDC hdc,int ox,int oy,float ex,float ey);
	void Clear();

	// interface de creacion de objetos
	Body *AddCircle( float x, float y , float r);
	Body *AddBox( float x, float y , float w,float h);
	Body *AddTri( float ax, float ay , float bx, float by ,float cx, float cy );
	Body *AddPoly( Vector2 *pt,int cant_pt);

	// constraints
	Constraint *AddConstraint(Body *A,Body *B,float dist);
	Constraint *AddJointConstraint(Body *A,Body *B);
	Constraint *AddDistanceConstraint(Body *A,Vector2 ptA , Body *B,Vector2 ptB);
	Constraint *AddAngleConstraint(Body *A,Body *B,Body *C);
	bool is_joint(Body *A,Body *B);			// devuelve true si el cuerpo A y el B forman un joint


	// interaccion c/ usuario
	int PointNearBodyCM(Vector2 pt, float dE);



};

extern World _world;
