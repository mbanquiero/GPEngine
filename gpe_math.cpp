#include "stdafx.h"
#include "gpe_math.h"
#include "math.h"

Vector2::Vector2()
{

}

Vector2::Vector2( const float * vector)
{
	x = vector[0];
	y = vector[1];
}

Vector2::Vector2( float px, float py )
{
	x = px;
	y = py;
}

Vector2::~Vector2()
{

}

Vector2& Vector2::Set(float px, float py)
{
	this->x = px; this->y = py;
	return (*this);
}



Vector2& Vector2::operator+=( const Vector2& v2)
{
	x += v2.x;
	y += v2.y;
	return (*this);
}

Vector2& Vector2::operator-=( const Vector2& v2)
{
	x -= v2.x;
	y -= v2.y;
	return (*this);
}

Vector2& Vector2::operator*=( float val )
{
	x *= val;
	y *= val;
	return (*this);
}

Vector2& Vector2::operator/=( float val )
{
	x /= val;
	y /= val;
	return (*this);
}

Vector2 Vector2::operator+() const
{
	return Vector2(x,y);
}

Vector2 Vector2::operator+( const Vector2& v2) const
{
	return Vector2(x+v2.x,y+v2.y);
}

Vector2 Vector2::operator-() const
{
	return Vector2(-x,-y);
}

Vector2 Vector2::operator-( const Vector2& v2) const
{
	return Vector2(x-v2.x,y-v2.y);
}

Vector2 Vector2::operator*( float val) const
{
	return Vector2(x*val,y*val);
}

Vector2 Vector2::operator/( float val) const
{
	return Vector2(x/val,y/val);
}

bool Vector2::operator==( const Vector2& v2) const
{
	return x==v2.x && y==v2.y;
}

bool Vector2::operator!=( const Vector2& v2) const
{
	return !((*this)== v2);
}


float Vector2::LengthSq() const
{
	return x*x + y*y;
}

float Vector2::Length() const
{
	return (float)sqrt(LengthSq());
}

float Vector2::Normalize()
{
	float len = Length();
	if(len>EPSILON)
		(*this) /= len;
	return len;
}

// retorna un vector normal a si mismo 
Vector2 Vector2::normal()
{
	return(Vector2(-y,x));
}


void Vector2::Rotate( float an)
{
	float cosa = cos( an );
	float sina = sin( an );

	float xp = x * cosa - y * sina;
	float yp = x * sina + y * cosa;

	x = xp;
	y = yp;
}


// producto vectorial 2d 

// vector x escalar = vector normal a v, y escalado por k
Vector2 cross( const Vector2& v, float k )
{
	return Vector2( k * v.y, -k * v.x );
}

// escalar x vector = vector
Vector2 cross( float k, const Vector2& v )
{
	return -cross(v,k);
}

// vector x vector = escalar 
// devuelve la magnitud del vector equivalente al cross 3d standard
float cross( const Vector2& a, const Vector2& b )
{
	return a.x * b.y - a.y * b.x;
}


// producto escalar
float dot( const Vector2& a, const Vector2& b )
{
	return a.x*b.x + a.y*b.y;
}

float Random(float x0 , float x1)
{
	return (float)rand()/(float)RAND_MAX * (x1-x0) + x0;
}


bool BiasGreaterThan( float a, float b )
{
	float k_biasRelative = 0.95f;
	float k_biasAbsolute = 0.01f;
	return a >= b * k_biasRelative + a * k_biasAbsolute;
}

Matrix2::Matrix2( float a , float b , float c , float d )
{
	m00 = a;
	m01 = b;
	m10 = c;
	m11 = d;
}

Matrix2 Matrix2::Identity()
{
	return Matrix2(
						1,	0,
						0,	1
					);
}


Matrix2 Matrix2::Rotation(float angle)
{
	float c = cos( angle);
	float s = sin( angle);

	return Matrix2(		c,	-s,
						s,	 c
				);
}



Matrix2 Matrix2::Transpose()
{
	return Matrix2( m00, m10, m01, m11 );
}

Vector2 Matrix2::operator*( Vector2 & rhs ) 
{
	return Vector2( m00 * rhs.x + m01 * rhs.y, m10 * rhs.x + m11 * rhs.y );
}

Matrix2 Matrix2::operator*( Matrix2& rhs ) 
{
	// [00 01]  [00 01]
	// [10 11]  [10 11]

	return Matrix2(
		m00 * rhs.m00 + m01 * rhs.m10,
		m00 * rhs.m01 + m01 * rhs.m11,
		m10 * rhs.m00 + m11 * rhs.m10,
		m10 * rhs.m01 + m11 * rhs.m11
		);
}

