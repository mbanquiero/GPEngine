
#include "stdafx.h"
#include "body.h"
#include "world.h"
#include "contact.h"
#include "collision.h"

BOOL CircleCircle( Body *a, Body *b ,ContactSolver *contact_info)
{
  GeoCircle *A = (GeoCircle *)a->shape;
  GeoCircle *B = (GeoCircle *)b->shape;

  Vector2 normal = b->position - a->position;

  float dist_sqr = normal.LengthSq( );
  float radius = A->radius + B->radius;

  // Not in contact
  if(dist_sqr >= radius * radius)
  {
	contact_info->contact_count = 0;
	return FALSE;
  }

  float distance = sqrt( dist_sqr);

  contact_info->contact_count = 1;

  if(distance == 0.0f)
  {
    contact_info->penetration = A->radius;
    contact_info->normal = Vector2( 1, 0 );
    contact_info->contacts [0] = a->position;
  }
  else
  {
    contact_info->penetration = radius - distance;
    contact_info->normal = normal / distance; 
    contact_info->contacts[0] = contact_info->normal * A->radius + a->position;
  }

  return TRUE;
}



BOOL CirclePolygon( Body *a, Body *b ,ContactSolver *contact_info)
{
	GeoCircle *A = (GeoCircle *)a->shape;
	GeoPolygon *B = (GeoPolygon*)b->shape;

	contact_info->contact_count = 0;

	// Transofrmo a al espacio del poligono, que es mas facil de tratar que en el global 
	// o en el espacio del circulo
	Vector2 center = a->position;
	center = B->matWorld.Transpose( ) * (center - b->position);
	
	// Calcular el lado donde se produce la minima penetracion
	float separation = -1000000;
	int faceNormal = 0;
	for(int i = 0; i < B->m_vertexCount; ++i)
	{
		float s = dot( B->normals[i], center - B->vertices[i] );

		if(s > A->radius)
			return FALSE;

		if(s > separation)
		{
			separation = s;
			faceNormal = i;
		}
	}

	// algortimo sacado de 
	// TODO: la pagina no la encuentro mas...

	// Grab face's vertices
	Vector2 v1 = B->vertices[faceNormal];
	Vector2 v2 = B->vertices[faceNormal+1];

	// Check to see if center is within polygon
	if(separation < EPSILON)
	{
		contact_info->contact_count = 1;
		contact_info->normal = -(B->matWorld * B->normals[faceNormal]);
		contact_info->contacts[0] = contact_info->normal * A->radius + a->position;
		contact_info->penetration = A->radius;
		return TRUE;
	}

	// Determine which voronoi region of the edge center of circle lies within
	float dot1 = dot( center - v1, v2 - v1 );
	float dot2 = dot( center - v2, v1 - v2 );
	contact_info->penetration = A->radius - separation;

	// Closest to v1
	if(dot1 <= 0.0f)
	{
		if((center-v1).LengthSq() > A->radius * A->radius)
			return FALSE;

		contact_info->contact_count = 1;
		Vector2 n = v1 - center;
		n = B->matWorld * n;
		n.Normalize( );
		contact_info->normal = n;
		v1 = B->matWorld * v1 + b->position;
		contact_info->contacts[0] = v1;
	}

	// Closest to v2
	else if(dot2 <= 0.0f)
	{
		if((center- v2).LengthSq() > A->radius * A->radius)
			return FALSE;

		contact_info->contact_count = 1;
		Vector2 n = v2 - center;
		v2 = B->matWorld * v2 + b->position;
		contact_info->contacts[0] = v2;
		n = B->matWorld * n;
		n.Normalize( );
		contact_info->normal = n;
	}
	// Closest to face
	else
	{
		Vector2 n = B->normals[faceNormal];
		if(dot( center - v1, n ) > A->radius)
			return FALSE;

		n = B->matWorld * n;
		contact_info->normal = -n;
		contact_info->contacts[0] = contact_info->normal * A->radius + a->position;
		contact_info->contact_count = 1;
	}

	return TRUE;
}



float FindAxisLeastPenetration( int *faceIndex, Body *a, Body *b )
{
	GeoPolygon *A = (GeoPolygon*)a->shape;
	GeoPolygon *B = (GeoPolygon*)b->shape;

	float bestDistance = -100000;
	int bestIndex = -1;

	for(int i = 0; i < A->m_vertexCount; ++i)
	{
		// Retrieve a face normal from A
		Vector2 n = A->normals[i];
		Vector2 nw = A->matWorld * n;

		// Transform face normal into B's model space
		Matrix2 buT = B->matWorld.Transpose( );
		n = buT * nw;

		// Retrieve support point from B along -n
		Vector2 s = B->GetSupport( -n );

		// Retrieve vertex on face from A, transform into
		// B's model space
		Vector2 v = A->vertices[i];
		v = A->matWorld * v + a->position;

		v -= b->position;
		v = buT * v;

		// Compute penetration distance (in B's model space)
		float d = dot( n, s - v );

		// Store greatest distance
		if(d > bestDistance)
		{
			bestDistance = d;
			bestIndex = i;
		}
	}

	*faceIndex = bestIndex;
	return bestDistance;
}

void FindIncidentFace( Vector2 *v, Body *RefBody, Body *IncBody, int referenceIndex )
{

	GeoPolygon *RefPoly = (GeoPolygon *)RefBody->shape;
	GeoPolygon *IncPoly = (GeoPolygon *)IncBody->shape;
	
	Vector2 referenceNormal = RefPoly->normals[referenceIndex];

	// Calculate normal in incident's frame of reference
	referenceNormal = RefPoly->matWorld * referenceNormal; // To world space
	referenceNormal = IncPoly->matWorld.Transpose( ) * referenceNormal; // To incident's model space

	// Find most anti-normal face on incident polygon
	int incidentFace = 0;
	float minDot = 100000;
	for(int i = 0; i < IncPoly->m_vertexCount; ++i)
	{
		float d = dot( referenceNormal, IncPoly->normals[i] );
		if(d < minDot)
		{
			minDot = d;
			incidentFace = i;
		}
	}

	// Assign face vertices for incidentFace
	v[0] = IncPoly->matWorld * IncPoly->vertices[incidentFace] + IncBody->position;
	incidentFace = incidentFace + 1 >= IncPoly->m_vertexCount ? 0 : incidentFace + 1;
	v[1] = IncPoly->matWorld * IncPoly->vertices[incidentFace] + IncBody->position;
}

int Clip( Vector2 n, float c, Vector2 *face )
{
	int sp = 0;
	Vector2 out[2] = {
		face[0],
		face[1]
	};

	// Retrieve distances from each endpoint to the line
	// d = ax + by - c
	float d1 = dot( n, face[0] ) - c;
	float d2 = dot( n, face[1] ) - c;

	// If negative (behind plane) clip
	if(d1 <= 0.0f) out[sp++] = face[0];
	if(d2 <= 0.0f) out[sp++] = face[1];

	// If the points are on different sides of the plane
	if(d1 * d2 < 0.0f) // less than to ignore -0.0f
	{
		// Push interesection point
		float alpha = d1 / (d1 - d2);
		out[sp] = face[0] + (face[1] - face[0])*alpha;
		++sp;
	}

	// Assign our new converted values
	face[0] = out[0];
	face[1] = out[1];

	return sp;
}

BOOL PolygonPolygon( Body *a, Body *b ,ContactSolver *contact_info)
{
	GeoPolygon *A = (GeoPolygon*)a->shape;
	GeoPolygon *B = (GeoPolygon*)b->shape;

	contact_info->contact_count = 0;

	// Check for a separating axis with A's face planes
	int faceA;
	float penetrationA = FindAxisLeastPenetration( &faceA, a, b );
	if(penetrationA >= 0.0f)
		return FALSE;

	// Check for a separating axis with B's face planes
	int faceB;
	float penetrationB = FindAxisLeastPenetration( &faceB, b, a );
	if(penetrationB >= 0.0f)
		return FALSE;

	int referenceIndex;
	bool flip; // Always point from a to b

	Body *RefBody; // Reference
	Body *IncBody; // Incident

	// Determine which shape contains reference face
	if(BiasGreaterThan( penetrationA, penetrationB ))
	{
		RefBody = a;
		IncBody = b;
		referenceIndex = faceA;
		flip = false;
	}

	else
	{
		RefBody = b;
		IncBody = a;
		referenceIndex = faceB;
		flip = true;
	}

	GeoPolygon *RefPoly = (GeoPolygon *)RefBody->shape; // Reference
	GeoPolygon *IncPoly  = (GeoPolygon *)IncBody->shape; // Incident

	// World space incident face
	Vector2 incidentFace[2];
	FindIncidentFace( incidentFace, RefBody, IncBody, referenceIndex );

	//        y
	//        ^  ->n       ^
	//      +---c ------posPlane--
	//  x < | i |\
	//      +---+ c-----negPlane--
	//             \       v
	//              r
	//
	//  r : reference face
	//  i : incident poly
	//  c : clipped point
	//  n : incident normal

	// Setup reference face vertices
	Vector2 v1 = RefPoly->vertices[referenceIndex];
	Vector2 v2 = RefPoly->vertices[referenceIndex+1];

	// Transform vertices to world space
	v1 = RefPoly->matWorld * v1 + RefBody->position;
	v2 = RefPoly->matWorld * v2 + RefBody->position;


	// Calculate reference face side normal in world space
	Vector2 sidePlaneNormal = (v2 - v1);
	sidePlaneNormal.Normalize( );

	// Orthogonalize
	Vector2 refFaceNormal( sidePlaneNormal.y, -sidePlaneNormal.x );

	// ax + by = c
	// c is distance from origin
	float refC = dot( refFaceNormal, v1 );
	float negSide = -dot( sidePlaneNormal, v1 );
	float posSide =  dot( sidePlaneNormal, v2 );

	// Clip incident face to reference face side planes
	if(Clip( -sidePlaneNormal, negSide, incidentFace ) < 2)
		return FALSE; // Due to floating point error, possible to not have required points

	if(Clip(  sidePlaneNormal, posSide, incidentFace ) < 2)
		return FALSE; // Due to floating point error, possible to not have required points

	// Flip
	contact_info->normal = flip ? -refFaceNormal : refFaceNormal;

	// Keep points behind reference face
	int cp = 0; // clipped points behind reference face
	float separation = dot( refFaceNormal, incidentFace[0] ) - refC;
	if(separation <= 0.0f)
	{
		contact_info->contacts[cp] = incidentFace[0];
		contact_info->penetration = -separation;
		++cp;
	}
	else
		contact_info->penetration = 0;

	separation = dot( refFaceNormal, incidentFace[1] ) - refC;
	if(separation <= 0.0f)
	{
		contact_info->contacts[cp] = incidentFace[1];

		contact_info->penetration += -separation;
		++cp;

		// Average penetration
		contact_info->penetration /= (float)cp;
	}

	contact_info->contact_count = cp;
	return TRUE;
}
