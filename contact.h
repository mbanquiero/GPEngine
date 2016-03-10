#pragma once

#include "stdafx.h"
#include "gpe_math.h"
#include "math.h"
#include "body.h"

// contactos entre 2 cuerpos

class ContactSolver
{
	public:

		Body *A;
		Body *B;
		float penetration;			// Cuanto penetra el Cuerpo A dentro del B durante el contacto
		Vector2 normal;				// From A to B
		Vector2 contacts[2];		// Puntos de contacto. 
		float dt;					// elapsed time
		int contact_count;			// Cantidad de puntos de contacto
		float e;					// Mixed restitution
		float df;					// Mixed dynamic friction
		float sf;					// Mixed static friction

	BOOL BodyCollide(float elapsed_time, Body *pA , Body *pB);                 // Determina si hay contacto entre el cuerpo A y el Cuerpo B
	void PreCalc( );								// Precalculations for impulse solving
	void ApplyImpulse(int nro_iteraccion);							// Solve impulse and apply
	void PositionalCorrection( );					// Naive correction of positional penetration
	void InfiniteMassCorrection( );

};

