#pragma once

#define MAX_VERTEX_COUNT		32
#define GPE_CIRCLE				0
#define GPE_POLYGON				1

#include "stdafx.h"
#include "gpe_math.h"
#include "math.h"

// Conviene desacoplar la "geometria" del objeto, de la clase body pp dicha. 
// De esta forma se puede usar herencia de clases solo la clase geometria que encapsula
// las funciones de computar masa, dibujar, y demas que hacen a la geometria del cuerpo.

extern class Body;

class Geometry
{
	public:
		Matrix2 matWorld;

		// abstractas
		virtual void ComputeMass( float density , float *M , float *I) = 0;
		virtual void SetOrient( float angle);
		virtual void Render( HDC hdc, Body *body,int ox,int oy,float ex,float ey) = 0;
		virtual BYTE GetType( ) = 0;
};


class GeoCircle : public Geometry
{
public:
	float radius;
	GeoCircle( float r );
	void ComputeMass( float density , float *M , float *I);
	void SetOrient( float angle) {};
	void Render( HDC hdc, Body *body,int ox,int oy,float ex,float ey);
	BYTE GetType( ) {return GPE_CIRCLE;};

};


class GeoPolygon : public Geometry
{
public:
	int m_vertexCount;
	Vector2 vertices[MAX_VERTEX_COUNT];
	Vector2 normals[MAX_VERTEX_COUNT];

	GeoPolygon(float w,float h);
	GeoPolygon(Vector2 p0,Vector2 p1,Vector2 p2);
	GeoPolygon( Vector2 *p, int count );

	void ComputeMass( float density , float *M , float *I);
	void Render( HDC hdc, Body *body,int ox,int oy,float ex,float ey);
	BYTE GetType( ) {return GPE_POLYGON;};
	void Close();
	Vector2 GetSupport( const Vector2& dir );

};


class Body
{
	public:
		char name[32];
		Geometry *shape;
		float density;

		Vector2 position;
		Vector2	velocity;
		Vector2 force;

		float angularVelocity;
		float torque;
		float orient;					// orientacion

		float inertia;					// momento de inercia
		float inverseInertia;			// inversa del momento de inercia = 1/inertia
		float mass;						// masa
		float inverseMass;				// inversa de la masa = 1/masa

		float staticFriction;
		float dynamicFriction;
		float restitution;				// coeficiente de restitucion

		float gravity_factor;	

		COLORREF color;
		int id;

		Body();
		~Body();

		void Create(Geometry *p_shape, float p_density=1);
		void CreateCircle(float r,Vector2 pos=Vector2(0,0), float p_density=1);
		void CreateBox(float w,float h,Vector2 pos=Vector2(0,0), float p_density=1);
		void CreateTri(float ax, float ay , float bx, float by ,float cx, float cy, float p_density=1);
		void CreatePoly(Vector2 pt[],int cant_pt, float p_density=1);

		void ApplyForce( const Vector2& f );
		void ApplyImpulse( const Vector2& impulse, const Vector2& contactPoint );
		void SetStatic();
		void SetOrient( float angle);

		void SetMass( float m );
		void SetInertia( float I );


		void IntegrateForces( float dt );
		void IntegrateVelocity( float dt );

		void Render( HDC hdc,int ox,int oy,float ex,float ey);

};


