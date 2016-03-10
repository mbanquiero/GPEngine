#include "stdafx.h"
#include "constraint.h"
#include "body.h"
#include "world.h"
#include "gpe_math.h"
#include <stdio.h>


void Constraint::Solve()
{
	if(angle_constraint)
	{
		SolveAngleConstraint();
		return;
	}

	Vector2 rA = A->shape->matWorld*ptA;
	Vector2 rB = B->shape->matWorld*ptB;
	Vector2 vr = B->position + rB - A->position - rA;
	float dreal = vr.Length();
	if(exact_distance)
	{
		if(fabs(dreal-dist)<0.01)
			return;
	}
	else
	{
		if(dreal<dist)
			return;
	}
	
	// no se cumple la restriccion de distancia
	Vector2 n = vr * (1/dreal);		// direccion de A hacia B normalizada
	float D = dreal - dist;			// distancia que tengo que corregir 
	float dt = fixed_dt;			// en cierta cantidad de tiempo

	// Para ello genero una fuerza ficticia en direccion hacia el objeto B, que compense la velocidad relativa de A
	// y lo haga dirigirse hacia B 
	// tomo solo la parte de la velocidad de A sobre la linea de fuerza n
	// uso la velocidad relativa entre ambos cuerpos
	float j;
	Vector2 velRel = A->velocity - B->velocity;
	j = D/dt -dot(n,velRel);
	j*=0.3;

	// distribuyo el impulso de forma proporcional a las masas de ambos cuerpos
	float t;
	if(A->inverseMass==0)
		t = 0;
	else
	if(B->inverseMass==0)
		t = 1;
	else
		t = A->mass / (A->mass + B->mass);

	float jA = j * t;
	float jB = j * (1-t);


	// Primero corrijo el Objeto A
	if(A->inverseMass!=0)
		A->velocity += n*jA;
	if(A->inverseInertia!=0)
		A->SetOrient( A->orient + A->inverseInertia * cross(rA,n*jA));

	// Ahorra corrijo el B
	if(B->inverseMass!=0)
		B->velocity -= n*jB;
	if(B->inverseInertia!=0)
		B->SetOrient( B->orient + B->inverseInertia * cross(rB,-n*jB));

	if(joint)
	{
		float limit = 3.1415/8.0;
		float dW = A->orient-B->orient;
		if(fabs(dW)>limit)
		{
			float desf = (fabs(dW)-limit) *.8;
			if(A->orient<B->orient)
				desf *= -1;

			if(A->inverseInertia!=0 )
				A->SetOrient( A->orient - desf/2);
			if(B->inverseInertia!=0 )
				B->SetOrient( B->orient + desf/2);

		}
	}


}

void Constraint::SolveAngleConstraint()
{
	// determino el angulo (cos angulo)
	Vector2 L = A->position-B->position;
	Vector2 R = C->position-B->position;
	L.Normalize();
	R.Normalize();
	float angle_actual = acos(dot(L,R));

	float limit = 3.1415/8.0;
	float dW = angle_actual - angle;
	if(fabs(dW)>limit)
	{
		L.Rotate(-dW/2);
		R.Rotate(dW/2);
		angle_actual = acos(dot(L,R));
		if(fabs(angle_actual - angle)>fabs(dW))
		{
			// era para el otro lado
			L.Rotate(dW);
			R.Rotate(-dW);
			angle_actual = acos(dot(L,R));
		}
		dW = angle_actual - angle;
		Vector2 posA = B->position + L*(A->position-B->position).Length();
		A->velocity = (posA-A->position) / fixed_dt * 0.2;

		Vector2 posC = B->position + R*(C->position-B->position).Length();
		C->velocity = (posC-C->position) / fixed_dt * 0.2;
	}

}
	

/*
void Constraint::SolveJointConstraint()
{
	Vector2 rA = A->shape->matWorld*ptA;
	Vector2 rB = B->shape->matWorld*ptB;
	Vector2 vr = B->position + rB - A->position - rA;
	float dreal = vr.Length();

	if(joint)
	{
		// joint constrain: 
		if(dreal<dist)
			return;

	}
	else
	{
		// distance constraing
	}
	if(dreal<dist)
		return;

	// no se cumple la restriccion de distancia
	Vector2 n = vr * (1/dreal);		// direccion de A hacia B normalizada
	float D = dreal - dist;			// distancia que tengo que corregir 
	float dt = fixed_dt;			// en cierta cantidad de tiempo

	// Para ello genero una fuerza ficticia en direccion hacia el objeto B, que compense la velocidad relativa de A
	// y lo haga dirigirse hacia B 
	// tomo solo la parte de la velocidad de A sobre la linea de fuerza n
	// uso la velocidad relativa entre ambos cuerpos

	// experimento, anulo la velocidad si es un joint
	float j;
	if(joint)
	{
		A->angularVelocity = B->angularVelocity = 0;
		A->velocity*=0.6;
		B->velocity*=0.6;
		j = D/dt;
	}
	else
	{
		Vector2 velRel = A->velocity - B->velocity;
		j = D/dt -dot(n,velRel);
		j*=0.3;
	}

	// distribuyo el impulso de forma proporcional a las masas de ambos cuerpos
	float t;
	if(joint)
	{
		t = rA.y>rB.y ? 0.75 : 0.25;
	}
	else
	{
		if(A->inverseMass==0)
			t = 0;
		else
		if(B->inverseMass==0)
			t = 1;
		else
			t = A->mass / (A->mass + B->mass);
	}

	float jA = j * t;
	float jB = j * (1-t);


	// Primero corrijo el Objeto A
	if(A->inverseMass!=0)
		A->velocity += n*jA;
	if(joint)
		jA *= 0.05;
	if(A->inverseInertia!=0)
		A->SetOrient( A->orient + A->inverseInertia * cross(rA,n*jA));
	
	// Ahorra corrijo el B
	if(B->inverseMass!=0)
		B->velocity -= n*jB;
	if(joint)
		jB *= 0.05;
	if(B->inverseInertia!=0)
		B->SetOrient( B->orient + B->inverseInertia * cross(rB,-n*jB));


	if(joint)
	{
		float limit = 3.1415/8.0;
		float dW = A->orient-B->orient;
		if(fabs(dW)>limit)
		{
			float desf = (fabs(dW)-limit) *.3;
			if(A->orient<B->orient)
				desf *= -1;

			if(A->inverseInertia!=0 )
				A->SetOrient( A->orient - desf/2);
			if(B->inverseInertia!=0 )
				B->SetOrient( B->orient + desf/2);

		}
	}
}
*/
