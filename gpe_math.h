
#pragma once


const float PI = 3.141592741f;
const float EPSILON = 0.0001f;


struct Vector2  
{
public:
	float x,y;
public:
	Vector2();
	Vector2( const float * vector);
	Vector2( float px, float py);
	~Vector2();

	Vector2& Set(float px, float py);


	// assignment operators
	Vector2& operator += ( const Vector2& v2);
	Vector2& operator -= ( const Vector2& v2);
	Vector2& operator *= ( float val);
	Vector2& operator /= ( float val);

	// unary operators
	Vector2 operator + () const;
	Vector2 operator - () const;

	// binary operators
	Vector2 operator + ( const Vector2& v2) const;
	Vector2 operator - ( const Vector2& v2) const;
	Vector2 operator * ( float val) const;
	Vector2 operator / ( float val) const;


	bool operator == ( const Vector2& v2) const;
	bool operator != ( const Vector2& v2) const;


	float LengthSq() const;
	float Length() const;
	float mod() {return Length();};
	float Normalize();
	Vector2 normal();

	void Rotate( float an);

};


// producto vectorial 2d 
Vector2 cross( const Vector2& v, float k );
Vector2 cross( float k, const Vector2& v );
float cross( const Vector2& a, const Vector2& b );

// producto escalar
float dot( const Vector2& a, const Vector2& b );

// numero aleatorio
float Random(float x0 , float x1);

bool BiasGreaterThan( float a, float b );

// matriz de 2 x 2
class Matrix2
{
public:
	float m00, m01;
	float m10, m11;

	Matrix2( float a = 0, float b = 0, float c = 0, float d = 0 );
	static Matrix2 Identity();
	static Matrix2 Rotation(float angle);
	
	Matrix2 Transpose();
	Vector2 operator*( Vector2 & rhs );
	Matrix2 operator*( Matrix2& rhs );
};


