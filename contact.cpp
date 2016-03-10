
#include "stdafx.h"
#include "body.h"
#include "world.h"
#include "collision.h"
#include "contact.h"


// Resuelve la colision entre 2 cuerpos, devuelve TRUE si hay contacto
BOOL ContactSolver::BodyCollide(float elapsed_time, Body *pA , Body *pB )
{
	dt = elapsed_time;
	A = pA;
	B = pB;
	contact_count = 0;

	if(pA->shape->GetType()==GPE_CIRCLE && pB->shape->GetType()==GPE_CIRCLE)
		CircleCircle( A, B ,this);
	else
	if(pA->shape->GetType()==GPE_CIRCLE && pB->shape->GetType()==GPE_POLYGON)
		CirclePolygon( A, B ,this);
	else
	if(pA->shape->GetType()==GPE_POLYGON && pB->shape->GetType()==GPE_CIRCLE)
	{
		A = pB;
		B = pA;
		CirclePolygon( A, B ,this);
	}
	else
	if(pA->shape->GetType()==GPE_POLYGON && pB->shape->GetType()==GPE_POLYGON)
		PolygonPolygon(A, B ,this);


	return contact_count > 0 ?  TRUE : FALSE;
}

// El cuerpo A y el B entraron en colision, precomputa ciertos parametros para resolver luego el impulso
// La idea es modificar la velocidad de los cuerpos para que se alejan en direccion a la normal donde hicieron contacto
// De esta forma se evitara que sigan colisionando
void ContactSolver::PreCalc()
{
	// El coeficiente de restitucion indica que tan elastico es el choque, o cuanta energia se pierde por el impacto
	// usualmente se toma el minimo entre los 2 cuerpos que entran en colision
	e = min( A->restitution, B->restitution );

	// Calculate static and dynamic friction
	sf = sqrt( A->staticFriction * B->staticFriction );
	df = sqrt( A->dynamicFriction * B->dynamicFriction );

	for(int i = 0; i < contact_count; ++i)
	{
		// radios desde el pto de contacto hasta el centro de masa 
		Vector2 ra = contacts[i] - A->position;
		Vector2 rb = contacts[i] - B->position;

		Vector2 rv = B->velocity + cross( B->angularVelocity, rb ) - A->velocity - cross( A->angularVelocity, ra );

		// Verifico si se trata de "resting contact" 
		// Se llama resting contact a los contactos en equilibrio estatico, por ejemplo un libro sobre la mesa
		// la idea es que si la unica fuerza que acelera el objeto es la de gravedad
		// la colision se tiene que hacer sin ninguna restitucion, para evitar que el cuerpo rebote indefinidamente
		if(rv.LengthSq( ) < (gravity * dt).LengthSq() + EPSILON)
			e = 0.0f;
	}
}

void ContactSolver::ApplyImpulse(int nro_iteraccion)
{
	// si ambos objetos son esticos, corrijo la posicion directamente
	if(A->inverseMass  + B->inverseMass < EPSILON)
	{
		InfiniteMassCorrection( );
		return;
	}

	for(int ii = 0; ii < contact_count; ++ii)
	{
		//int i = nro_iteraccion%2 ? ii : contact_count - ii -1;
		int i = ii;
		// distancia desde el pto de contacto hasta el CM
		Vector2 ra = contacts[i] - A->position;
		Vector2 rb = contacts[i] - B->position;

		// Computa la velocidad relativa. Es como si uno de los cuerpos estuviera fijo, y el otro se mueve con
		// la velocidad total del sistema. Es un cambio de referencia. 
		Vector2 rv = B->velocity + cross( B->angularVelocity, rb ) - A->velocity - cross( A->angularVelocity, ra );

		// se llama velocidad del contacto al componente de la velocidad sobre la normal del mismo. 
		// Es la parte de la misma que interviene en la restitucion.
		float contactVel = dot( rv, normal );

		// Luego, si es velocidad es positiva los objetos ya se estan separando, no tengo que hacer nada
		if(contactVel > 0)
			return;

		// Para corregir la orientacion del cuerpo, preciso trabajar con el momento de la fuerza con respecto al punto
		// Eso indica que parte de la fuerza produce una acelaracion angular, o interviene en la rotacion.,
		float raCrossN = cross( ra, normal );				// momento de A
		float rbCrossN = cross( rb, normal );				// momento de B
		float invMassSum = A->inverseMass + B->inverseMass 
			+ raCrossN*raCrossN * A->inverseInertia + rbCrossN*rbCrossN * B->inverseInertia;

		// Ecuacion para el impuslo escalar
		e = 0;
		float j = -(1.0f + e) * contactVel;
		j /= invMassSum;
		j /= (float)contact_count;

		// Aplico el impulso pp dicho
		Vector2 impulse = normal * j;
		A->ApplyImpulse( -impulse, ra );
		B->ApplyImpulse(  impulse, rb );

		// Computo la fuerz de friccion, la fuerza es tangente la direccion de colision, es decir la direccion de colision
		// es la normal, y la friccion es la tangente en el choque. 
		rv = B->velocity + cross( B->angularVelocity, rb ) -A->velocity - cross( A->angularVelocity, ra );
		Vector2 t = rv - (normal * dot( rv, normal ));
		if(t.LengthSq()==0)
			return;

		t.Normalize( );
		// j tangent magnitude
		float jt = -dot( rv, t );
		jt /= invMassSum;
		jt /= (float)contact_count;

		// si la frccion es muy pequeña no la tengo en cuenta para no introducir inestabilidad en el sistmea
		if(fabs( jt)<0.01)
			return;

		// Ley de Coulumb's 
		Vector2 tangentImpulse;
		if(fabs( jt ) < j * sf)
			tangentImpulse = t * jt;
		else
			tangentImpulse = t * -j * df;

		// Aplico el impulso de la friccion
		A->ApplyImpulse( -tangentImpulse, ra );
		B->ApplyImpulse(  tangentImpulse, rb );

		// paso al siguiente contacto
	}
}

void ContactSolver::PositionalCorrection()
{
	float k_slop = 0.05f; // Penetration allowance
	float percent = 0.4f; // Penetration percentage to correct
	float K = (max( penetration - k_slop, 0.0f ) / (A->inverseMass + B->inverseMass)) * percent;
	Vector2 correction =  normal * K;
	A->position -= correction * A->inverseMass;
	B->position += correction * B->inverseMass;
}

void ContactSolver::InfiniteMassCorrection()
{
	A->velocity = B->velocity = Vector2( 0, 0 );
}
