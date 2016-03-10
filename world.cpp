#include "stdafx.h"
#include "world.h"
#include <stdio.h>
#define atx(X) ox + (X) * ex
#define aty(Y) oy + (Y) * ey


float gravityScale = 5.0f;
Vector2 gravity( 0, 10.0f * gravityScale );
float fixed_dt = 1.0/60.;								// avance del tiempo constante
World _world;
char status_bar[255];

World::World()
{
	m_bodyCount = 0;
	m_constraintCount = 0;
	strcpy(status_bar,"");
	time = 0;
}

World::~World()
{
	Clear();
}

void World::Clear()
{
	for(int i = 0; i < m_bodyCount; ++i)
	{
		delete bodies[i];
		bodies[i] = NULL;
	}
	m_bodyCount = 0;

	for(int i = 0; i < m_constraintCount ; ++i)
	{
		delete constraints[i];
		constraints[i] = NULL;
	}
	m_constraintCount = 0;

}

void World::Update(float elapsed_time)
{
	// Generate new collision info
	m_contactCount = 0;
	for(int i = 0; i < m_bodyCount-1; ++i)
	{
		Body *A = bodies[i];

		for(int j = i + 1; j < m_bodyCount; ++j)
		{
			Body *B = bodies[j];
			// si ambos cuerpos son estaticos, no pueden colisionar
			if(A->inverseMass == 0 && B->inverseMass== 0)
				continue;

			// si Ambos cuerpos forman un joint tampoco pueden colisionar, aunque si lo pueden hacer en la vida real 
			// el solver no es estable con este tipo de configuracion 
			//if(is_joint(A,B))
				//continue;

			ContactSolver C;
			if(C.BodyCollide(elapsed_time , A,B))
			{
				contacts[m_contactCount++] = C;
			}
		}
	}


	// Integrar fuerzas primer parte
	for(int i = 0; i < m_bodyCount; ++i)
		bodies[i]->IntegrateForces( elapsed_time/2. );

	// Initialize collision: Precalculo la info de contactos
	for(int i=0;i<m_contactCount;++i)
		contacts[i].PreCalc();

	// Solve collisions
	for(int j = 0; j < 5; ++j)
	{
		for(int i = 0; i < m_contactCount; ++i)
			contacts[i].ApplyImpulse(j);
	}

		
	
	// Integrar velocidades
	for(int i = 0; i < m_bodyCount; ++i)
		bodies[i]->IntegrateVelocity(  elapsed_time );

	// Integrar fuerzas segunda parte
	for(int i = 0; i < m_bodyCount; ++i)
		bodies[i]->IntegrateForces( elapsed_time/2. );


	// Correct positions
	for(int i = 0; i < m_contactCount; ++i)
	{
		contacts[i].PositionalCorrection( );
	}

	// Limpiar todas las fuerzas, pues ya fueron integradas
	for(int i = 0; i < m_bodyCount; ++i)
	{
		bodies[i]->force = Vector2(0,0);
		bodies[i]->torque = 0;
	}

	// Solve constrains
	//for(int j = 0; j < 3; ++j)
	for(int i = 0; i < m_constraintCount; ++i)
		constraints[i]->Solve();
		

	time += elapsed_time;



}

void World::Render( HDC hdc,int ox,int oy,float ex,float ey)
{
	HFONT hfont = CreateFont(12,0,0,0,0,0,0,0,0,0,0,0,0,"Tahoma");
	HFONT hfontOld = (HFONT)SelectObject(hdc,hfont);

	for(int i = 0; i < m_bodyCount; ++i)
	{
		bodies[i]->Render(hdc,ox,oy,ex,ey);
		if(false)
		{
			// info de la posicion
			Vector2 pos = bodies[i]->position;
			SetTextAlign(hdc,TA_CENTER);
			char saux[255];
			sprintf(saux,"%.2f",pos.y);
			TextOutA(hdc, atx(pos.x) ,aty(pos.y),saux,strlen(saux));
		}
	}


	HPEN hpen = CreatePen(PS_SOLID,1,RGB(64,0,0));
	HPEN hpenOld = (HPEN)SelectObject(hdc,hpen);

	HBRUSH hbrush = CreateSolidBrush(RGB(0,0,0));
	HBRUSH hbrushOld = (HBRUSH)SelectObject(hdc,hbrush);


	for(int i = 0; i < m_contactCount; ++i)
	{
		for(int j = 0; j < contacts[i].contact_count; ++j)
		{
			Vector2 c = contacts[i].contacts[j];
			Vector2 n = contacts[i].normal;
			Ellipse(hdc,atx(c.x)-5,aty(c.y)-5,atx(c.x)+5,aty(c.y)+5);
			MoveToEx(hdc , atx(c.x) ,aty(c.y),NULL);
			c = c + n;
			LineTo(hdc , atx(c.x) ,aty(c.y));
		}
	}


	HPEN hpen2 = CreatePen(PS_SOLID,2,RGB(192,192,255));
	SelectObject(hdc,hpen2);

	for(int i = 0; i < m_constraintCount; ++i)
	{
		switch(constraints[i]->visible)
		{
			case 1:
				{
					Vector2 c = constraints[i]->A->position + constraints[i]->A->shape->matWorld*constraints[i]->ptA;
					MoveToEx(hdc , atx(c.x) ,aty(c.y),NULL);
					c = constraints[i]->B->position +  constraints[i]->B->shape->matWorld*constraints[i]->ptB;
					LineTo(hdc , atx(c.x) ,aty(c.y));
				}
				break;
			case 2:
				{
					Vector2 p0 = constraints[i]->A->position + constraints[i]->A->shape->matWorld*constraints[i]->ptA;
					Vector2 p1 = constraints[i]->B->position +  constraints[i]->B->shape->matWorld*constraints[i]->ptB;
					Vector2 dir_v = p1-p0;
					dir_v.Normalize();
					Vector2 dir_w = dir_v.normal();
					float rA = ((GeoCircle *)constraints[i]->A->shape)->radius;
					float rB = ((GeoCircle *)constraints[i]->B->shape)->radius;
					Vector2 Q[4];
					Q[0] = p0 + dir_w*rA;
					Q[1] = p1 + dir_w*rB;
					Q[2] = p1 - dir_w*rB;
					Q[3] = p0 - dir_w*rA;
					POINT pt[5];
					for(int t=0;t<4;++t)
					{
						pt[t].x = atx(Q[t].x);
						pt[t].y = aty(Q[t].y);
					}
					pt[4] = pt[0];
					Polygon(hdc,pt,5);
				}
				break;
		}
	}


	/*
	TextOutA(hdc,10,10,status_bar,strlen(status_bar));
	char buffer[255];
	float dW = bodies[0]->orient - bodies[1]->orient;
	sprintf(buffer,"%10.1f",dW);
	TextOutA(hdc,10,20,buffer,strlen(buffer));
	*/

	SelectObject(hdc,hpenOld);
	DeleteObject(hpen);
	DeleteObject(hpen2);

	SelectObject(hdc,hfontOld);
	DeleteObject(hfont);

	SelectObject(hdc,hbrushOld);
	DeleteObject(hbrush);

}

Body *World::AddCircle( float x, float y , float r)
{
	Body *b = bodies[m_bodyCount++] = new Body();
	b->CreateCircle(r,Vector2(x,y));
	return b;
}


Body *World::AddBox( float x, float y , float w, float h)
{
	Body *b = bodies[m_bodyCount++] = new Body();
	b->CreateBox(w,h,Vector2(x,y));
	return b;
}


Body *World::AddTri( float ax, float ay , float bx, float by ,float cx, float cy )
{
	Body *b = bodies[m_bodyCount++] = new Body();
	b->CreateTri(ax,ay,bx,by,cx,cy);
	return b;

}

Body *World::AddPoly( Vector2 *pt,int cant_pt)
{
	Body *b = bodies[m_bodyCount++] = new Body();
	b->CreatePoly(pt,cant_pt);
	return b;
}


Constraint *World::AddConstraint(Body *A,Body *B,float dist)
{
	Constraint *c = constraints[m_constraintCount++] = new Constraint;
	c->A = A;
	c->B = B;
	c->dist = dist;
	// x defecto el pto se toma en el centro de gravedad
	c->ptA = c->ptB = Vector2(0,0);
	c->joint = false;
	c->angle_constraint = false;
	c->exact_distance = false;
	c->visible = 1;
	return c;
}


Constraint *World::AddDistanceConstraint(Body *A,Vector2 ptA , Body *B,Vector2 ptB)
{

	Constraint *c = constraints[m_constraintCount++] = new Constraint;
	c->A = A;
	c->B = B;
	c->ptA = ptA;
	c->ptB = ptB;
	Vector2 rA = A->position + A->shape->matWorld*ptA;
	Vector2 rB = B->position + B->shape->matWorld*ptB;
	c->dist = (rA - rB).Length();
	c->joint = false;
	c->angle_constraint = false;
	c->exact_distance = true;
	c->visible = 1;
	return c;

}

Constraint *World::AddJointConstraint(Body *A,Body *B)
{
	Constraint *c = constraints[m_constraintCount++] = new Constraint;
	c->A = A;
	c->B = B;
	// x defecto el pivote se toma en el centro de gravedad
	c->ptA = c->ptB = Vector2(0,0);
	// y la distancia exacta que tienen
	c->dist = (A->position - B->position).Length();
	c->joint = true;
	c->exact_distance = true;
	c->angle_constraint = false;
	c->visible = 2;
	return c;
}


Constraint *World::AddAngleConstraint(Body *A,Body *B,Body *C)
{
	Constraint *c = constraints[m_constraintCount++] = new Constraint;
	c->A = A;
	c->B = B;
	c->C = C;
	c->joint = false;
	c->exact_distance = false;
	c->angle_constraint = true;

	// determino el angulo (cos angulo)
	Vector2 L = A->position-B->position;
	Vector2 R = C->position-B->position;
	L.Normalize();
	R.Normalize();
	c->angle = acos(dot(L,R));

	return c;
}


// devuelve si el pto esta cercano al centro de gravedad de un cuerpo
int World::PointNearBodyCM(Vector2 pt, float dE)
{
	int rta = -1;
	float min_dist = dE;
	for(int i = 0;i<m_bodyCount;++i)
	if(bodies[i]->inverseMass!=0)		// solo se aplica a objetos dinamicos
	{
		float dist = (pt-bodies[i]->position).Length();
		if(dist<min_dist)
		{
			rta = i;
			min_dist = dist;
		}
	}

	return rta;
}

// devuelve true si el cuerpo A y el B forman un joint
bool World::is_joint(Body *A,Body *B)
{
	bool rta = false;
	int i=0;
	while(i<m_constraintCount && !rta)
	{
		if((constraints[i]->A==A && constraints[i]->B==B) || (constraints[i]->A==B && constraints[i]->B==A))
			rta = true;
		else
			++i;
	}
	return rta;
}
