#include "stdafx.h"
#include "body.h"
#include "world.h"
#include "gpe_math.h"
#include <math.h>
#include <stdio.h>

#define atx(X) ox + (X) * ex
#define aty(Y) oy + (Y) * ey

void Geometry::SetOrient( float angle)
{
	matWorld = Matrix2::Rotation(angle);
}

GeoCircle::GeoCircle( float r )
{
	radius = r;
	matWorld = Matrix2::Identity();
}

void GeoCircle::ComputeMass( float density , float *M , float *I)
{
	float r2 = radius * radius;
	// masa
	*M = PI * r2 * density;
	// Momento de inercia
	*I = *M * r2;
}

void GeoCircle::Render(HDC hdc, Body *body,int ox,int oy,float ex,float ey) 
{
	// dibujo un circulo
	Vector2 pos = body->position;
	float r = radius;
	Ellipse(hdc, atx(pos.x - r) ,aty(pos.y -r), atx(pos.x + r), aty(pos.y + r));

	// dibujo una linea para captar la orientacion del circulo
	Vector2 dir = Vector2(cos(body->orient),sin(body->orient));
	MoveToEx(hdc , atx(pos.x) , aty(pos.y) ,NULL);
	pos = pos + dir*r;
	LineTo(hdc , atx(pos.x) , aty(pos.y));

}

// --------------------------------------------------
GeoPolygon::GeoPolygon(float w,float h)
{
	m_vertexCount = 4;
	float hw = w/2;			// half width
	float hh = h/2;			// half height
	vertices[0] = Vector2( -hw, -hh );
	vertices[1] = Vector2( hw, -hh );
	vertices[2] = Vector2(  hw,  hh );
	vertices[3] = Vector2( -hw,  hh );
	normals[0] = Vector2(  0.0f,  -1.0f );
	normals[1] = Vector2(  1.0f,   0.0f );
	normals[2] = Vector2(  0.0f,   1.0f );
	normals[3] = Vector2( -1.0f,   0.0f );
	matWorld = Matrix2::Identity();

	Close();

}


GeoPolygon::GeoPolygon(Vector2 p1,Vector2 p2,Vector2 p3)
{
	m_vertexCount = 3;
	vertices[0] = p1;
	vertices[1] = p2;
	vertices[2] = p3;
	normals[0] = (p1-p2).normal();
	normals[1] = (p2-p3).normal();
	normals[2] = (p3-p1).normal();
	normals[0].Normalize();
	normals[1].Normalize();
	normals[2].Normalize();
	matWorld = Matrix2::Identity();

	Close();

}

GeoPolygon::GeoPolygon( Vector2 *p, int count )
{
	// copio los vertices
	m_vertexCount = count;
	for(int i = 0; i < m_vertexCount; ++i)
		vertices[i] = p[i];
	// cierro la figura
	vertices[m_vertexCount] = p[0];

	// calculo las normales
	for(int i = 0; i < m_vertexCount; ++i)
	{
		Vector2 face = vertices[i+1] - vertices[i];
		normals[i] = -(vertices[i+1] - vertices[i]).normal();
		normals[i].Normalize( );
	}
	normals[m_vertexCount] = normals[0];
	matWorld = Matrix2::Identity();
}


void GeoPolygon::Close()
{
	vertices[m_vertexCount] = vertices[0];
	normals[m_vertexCount] = normals[0];
}

void GeoPolygon::ComputeMass(float density , float *M , float *I)
{
	// Calcula el area  
	// http://en.wikipedia.org/wiki/Centroid
	// y el momento de inercia
	// http://en.wikipedia.org/wiki/Second_moment_of_area
	// formulas y ecuaciones 
	// http://richardson.eng.ua.edu/Former_Courses//CE_331_fa09/Projects/A_and_I_of_Polygon.pdf
	// surveyor-s formula para el calculo de area de un poligono
	// http://steiner.math.nthu.edu.tw/disk5/js/cardioid/12.pdf


	// Se supone que el pto N = pto 0 , ya que el poligono es cerrado
	vertices[m_vertexCount] = vertices[0];

	Vector2 c( 0.0f, 0.0f ); // centro de masa
	float area = 0.0f;
	float Ixy = 0.0f;

	for(int i = 0; i < m_vertexCount; ++i)
	{
		Vector2 p1 = vertices[i];
		Vector2 p2 = vertices[i+1];
		float D = cross( p1, p2 );
		float triangleArea = 0.5f * D;
		area += triangleArea;
		c += (p1 + p2) * (triangleArea / 3.0);
		float xx = p1.x * p1.x + p2.x * p1.x + p2.x * p2.x;
		float yy = p1.y * p1.y + p2.y * p1.y + p2.y * p2.y;
		Ixy += (0.25f * D / 3.0) * (xx + yy);
	}
	c *= 1.0f / area;
	*M = density * area;
	*I = density * Ixy;

	// ajusto para que la posicion sea el centro de gravedad
	for(int i = 0; i <= m_vertexCount; ++i)
	{
		vertices[i].x -= c.x;
		vertices[i].y -= c.y;
	}
}


// helper para algortimo de colision convexo - convexo 
Vector2 GeoPolygon::GetSupport( const Vector2& dir )
{
	float bestProjection = -10000;
	Vector2 bestVertex;

	for(int i = 0; i < m_vertexCount; ++i)
	{
		Vector2 v = vertices[i];
		float projection = dot( v, dir );

		if(projection > bestProjection)
		{
			bestVertex = v;
			bestProjection = projection;
		}
	}

	return bestVertex;
}

void GeoPolygon::Render(HDC hdc, Body *body,int ox,int oy,float ex,float ey) 
{
	// dibujo un poligono
	POINT pt[MAX_VERTEX_COUNT];
	for(int i = 0; i < m_vertexCount; ++i)
	{
		Vector2 pos = body->position + matWorld * vertices[i];
		pt[i].x = atx(pos.x);
		pt[i].y = aty(pos.y);
	}
	pt[m_vertexCount] = pt[0];
	Polygon(hdc, pt , m_vertexCount+1);

	// dibujo las normales
	if(false)
	{
		HPEN hpen = CreatePen(PS_SOLID,1,RGB(64,0,0));
		HPEN hpenOld = (HPEN)SelectObject(hdc,hpen);
		for(int i = 0; i < m_vertexCount; ++i)
		{
			Vector2 pos = body->position + matWorld * (vertices[i] + vertices[i+1])*0.5;
			MoveToEx(hdc,atx(pos.x),aty(pos.y),NULL);
			pos += matWorld*normals[i]*2;
			LineTo(hdc,atx(pos.x),aty(pos.y));
		}

		SelectObject(hdc,hpenOld);
		DeleteObject(hpen);
	}
}

// --------------------------------------------------


Body::Body()
{
	position = Vector2(0,0);
	velocity = Vector2(0,0);
	angularVelocity = 0;
	torque = 0;
	orient = 0;
	force = Vector2(0,0);
	staticFriction = 0.5f;
	dynamicFriction = 0.3f;
	restitution = 0.5f;
	color = RGB(0,0,0);
	id = 0;
	shape = NULL;
	gravity_factor = 1;
	strcpy(name,"");
}

Body::~Body()
{
	if(shape!=NULL)
	{
		delete shape;
		shape = NULL;
	}
}


void Body::SetMass( float m )
{
	mass = m;
	inverseMass = mass!=0 ? 1/mass : 0;
}
void Body::SetInertia( float I )
{
	inertia = I;
	inverseInertia = inertia!=0 ? inverseInertia = 1/inertia: 0;
}

void Body::Create(Geometry *p_shape, float p_density)
{
	// interface con la geometria
	density = p_density;
	shape = p_shape;
	shape->ComputeMass(density , &mass , &inertia);
	// computo los inversos de la masa y la inercia. En la mayor parte de las ecuaciones de la dinamica de cuerpos rigidos
	// la masa y la inercia aparecen dividiendo, con lo cual es mas practico almacenar 1/mass y 1/inertia 
	inverseMass = mass!=0 ? inverseMass = 1/mass : 0;
	inverseInertia = inertia!=0 ? inverseInertia = 1/inertia: 0;
}

void Body::CreateCircle(float r,Vector2 pos, float p_density)
{
	position = pos;
	Create((Geometry *)new GeoCircle(r) , p_density);
}

void Body::CreateBox(float w,float h,Vector2 pos, float p_density)
{
	position = pos;
	Create((Geometry *)new GeoPolygon(w,h) , p_density);
}

void Body::CreateTri(float ax, float ay , float bx, float by ,float cx, float cy, float p_density)
{
	position = Vector2(0,0);
	Create((Geometry *)new GeoPolygon(Vector2(ax,ay),Vector2(bx,by),Vector2(cx,cy)), p_density);
}

void Body::CreatePoly(Vector2 pt[],int cant_pt, float p_density)
{
	position = Vector2(0,0);
	Create((Geometry *)new GeoPolygon(pt,cant_pt), p_density);
}


void Body::Render( HDC hdc,int ox,int oy,float ex,float ey)
{
	// dibujo a traves de la interface de geometria asociada al cuerpo
	shape->Render(hdc,this,ox,oy,ex,ey);
}

void Body::ApplyForce( const Vector2& f )
{
	force += f;
}

void Body::ApplyImpulse( const Vector2& impulse, const Vector2& contactPoint )
{
	velocity +=  impulse * inverseMass;
	angularVelocity += inverseInertia * cross( contactPoint, impulse );
}

// al poner en cero el inverso de la masa, se comporta como si el objeto tuviese masa infinita
// o sea que es estatico para los efectos de la simulacion
void Body::SetStatic()
{
	inertia = 0.0f;
	inverseInertia  = 0.0f;
	mass = 0.0f;
	inverseMass = 0.0f;
}

void Body::SetOrient( float angle)
{
	orient = angle;
	shape->SetOrient( angle );
}


// Acceleration
//    F = mA
// => A = F * 1/m

// Explicit Euler
// x += v * dt
// v += (1/m * F) * dt

// Semi-Implicit (Symplectic) Euler
// v += (1/m * F) * dt
// x += v * dt


// Para integrar conviene hacerlo en 2 etapas, avanzando el tiempo por la mitad. La explicacion se puede encontrar aca
// http://www.niksula.hut.fi/~hkankaan/Homepages/gravity.html

// se supone que dt = elapsed_time / 2
void Body::IntegrateForces( float dt )
{
	if(inverseMass == 0.0f)
		return;

	velocity += (force * inverseMass + gravity*gravity_factor) * dt;
	angularVelocity += torque * inverseInertia * dt;
}

void Body::IntegrateVelocity( float dt )
{
	if(inverseMass == 0.0f)
		return;
	position += velocity * dt;
	orient += angularVelocity * dt;
	orient = fmod(orient,2.0f*3.1415f);
	SetOrient( orient );
}
