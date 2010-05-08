#ifndef CSE168_MATRIX3X3_H_INCLUDED
#define CSE168_MATRIX3X3_H_INCLUDED

#include <math.h>
#include <float.h>
#include <iostream>
#include <assert.h>

#ifdef WIN32
#pragma warning(disable:4305) // disable useless warnings
#pragma warning(disable:4244)
#endif

#include "Vector3.h"

class Matrix3x3
{

public:
    float m11, m12, m13,
          m21, m22, m23,
          m31, m32, m33;

    // Implements a 3x3 matrix: m_ij - row-i and column-j entry

public:

    Matrix3x3();
    Matrix3x3(const Vector3&, const Vector3&, const Vector3&); // sets by columns!
    Matrix3x3(float, float, float,
              float, float, float, 
              float, float, float); // sets by columns

    inline void setIdentity();     // set to the identity map
    inline void set(const Matrix3x3&); // set to the matrix.
    inline void set(const Vector3&, const Vector3&, const Vector3&);
    inline void set(float, float, float,
                    float, float, float,
                    float, float, float);
    inline void setColumn1(float, float, float);
    inline void setColumn2(float, float, float);
    inline void setColumn3(float, float, float);
    inline void setColumn1(const Vector3&);
    inline void setColumn2(const Vector3&);
    inline void setColumn3(const Vector3&);
    inline Vector3 column1() const;
    inline Vector3 column2() const;
    inline Vector3 column3() const;
	inline float det() const;
    inline Matrix3x3& invert();
	inline Matrix3x3& adj() const;

    inline void transpose();                    // Transposes it.
    inline Matrix3x3& operator+=(const Matrix3x3&);
    inline Matrix3x3& operator-=(const Matrix3x3&);
    inline Matrix3x3& operator*=(float);
    inline Matrix3x3& operator/=(float);
    inline Matrix3x3& operator*=(const Matrix3x3&);    // Matrix product
};

inline std::ostream &
operator<<(std::ostream& out, const Matrix3x3& m)
{
    return out << m.m11 << " " << m.m12 << " " << m.m13 << std::endl
               << m.m21 << " " << m.m22 << " " << m.m23 << std::endl
               << m.m31 << " " << m.m32 << " " << m.m33;
}

// binary operators
inline Matrix3x3 operator+(const Matrix3x3&, const Matrix3x3&);
inline Matrix3x3 operator-(const Matrix3x3&);
inline Matrix3x3 operator-(const Matrix3x3&, const Matrix3x3&);
inline Matrix3x3 operator*(const Matrix3x3&, float);
inline Matrix3x3 operator*(float, const Matrix3x3&);
inline Matrix3x3 operator/(const Matrix3x3&, float);
inline Matrix3x3 operator*(const Matrix3x3&, const Matrix3x3&);

inline Vector3 operator*(const Matrix4x4&, const Vector3&);



inline Matrix3x3::Matrix3x3() {setIdentity();}

inline Matrix3x3::Matrix3x3(const Vector3& a, const Vector3& b, const Vector3& c)
{
    m11 = a.x;      // Column 1
    m21 = a.y;
    m31 = a.z;
    m12 = b.x;      // Column 2
    m22 = b.y;
    m32 = b.z;
    m13 = c.x;      // Column 3
    m23 = c.y;
    m33 = c.z;
}

inline Matrix3x3::Matrix3x3(float a11, float a12, float a13,
                            float a21, float a22, float a23,
                            float a31, float a32, float a33)
// Values specified in row order!!!
{
    m11 = a11;      // Row 1
    m12 = a12;
    m13 = a13;
    m21 = a21;      // Row 2
    m22 = a22;
    m23 = a23;
    m31 = a31;      // Row 3
    m32 = a32;
    m33 = a33;
}

inline void
Matrix3x3::setIdentity()
{
    m11 = m22 = m33 = 1.0;
    m12 = m13 = m21 = m23 = m31 = m32 = 0.0;
}

inline void
Matrix3x3::set(const Vector3& a, const Vector3& b, const Vector3& c)
{
    m11 = a.x;      // Column 1
    m21 = a.y;
    m31 = a.z;
    m12 = b.x;      // Column 2
    m22 = b.y;
    m32 = b.z;
    m13 = c.x;      // Column 3
    m23 = c.y;
    m33 = c.z;
}

inline void
Matrix3x3::set(float a11, float a12, float a13,
               float a21, float a22, float a23,
               float a31, float a32, float a33)
// Values specified in row order!!!
{
    m11 = a11;      // Row 1
    m12 = a12;
    m13 = a13;
    m21 = a21;      // Row 2
    m22 = a22;
    m23 = a23;
    m31 = a31;      // Row 3
    m32 = a32;
    m33 = a33;
}

inline void
Matrix3x3::set(const Matrix3x3& M) // set to the matrix.
{
    m11 = M.m11;
    m12 = M.m12;
    m13 = M.m13;
    m21 = M.m21;
    m22 = M.m22;
    m23 = M.m23;
    m31 = M.m31;
    m32 = M.m32;
    m33 = M.m33;
}

inline void
Matrix3x3::setColumn1(float x, float y, float z)
{
    m11 = x; m21 = y; m31= z;
}

inline void
Matrix3x3::setColumn2(float x, float y, float z)
{
    m12 = x; m22 = y; m32= z;
}

inline void
Matrix3x3::setColumn3(float x, float y, float z)
{
    m13 = x; m23 = y; m33= z;
}

inline void
Matrix3x3::setColumn1(const Vector3& u)
{
    m11 = u.x; m21 = u.y; m31 = u.z;
}

inline void
Matrix3x3::setColumn2(const Vector3& u)
{
    m12 = u.x; m22 = u.y; m32 = u.z;
}

inline void
Matrix3x3::setColumn3(const Vector3& u)
{
    m13 = u.x; m23 = u.y; m33 = u.z;
}

Vector3
Matrix3x3::column1() const
{
    return Vector3(m11, m21, m31);
}

Vector3
Matrix3x3::column2() const
{
    return Vector3(m12, m22, m32);
}

Vector3
Matrix3x3::column3() const
{
    return Vector3(m13, m23, m33);
}

inline void
Matrix3x3::transpose()  // Transposes it.
{
    register float temp;
    temp = m12;
    m12 = m21;
    m21=temp;
    temp = m13;
    m13 = m31;
    m31 = temp;
    temp = m23;
    m23 = m32;
    m32 = temp;
}

float
Matrix3x3::det() const
{
	return (m11 * m22 * m33) + (m12 * m23 * m31) + (m13 * m21 * m32) - (m11 * m23 * m32) - (m12 * m21 * m33) - (m13 * m22 * m31);
}

Matrix3x3&
Matrix3x3::adj() const
{
	Matrix3x3 transpose = Matrix3x3(*this);
	transpose.transpose();
	Matrix3x3 * adjM = new Matrix3x3();

	adjM->m11 = (transpose.m22 * transpose.m33) - (transpose.m23 * transpose.m32);
	adjM->m12 = -1 * ((transpose.m21 * transpose.m33) - (transpose.m23 * transpose.m31));
	adjM->m13 = (transpose.m21 * transpose.m32) - (transpose.m22 * transpose.m31);

	adjM->m21 = -1 * ((transpose.m12 * transpose.m33) - (transpose.m13 * transpose.m32));
	adjM->m22 = (transpose.m11 * transpose.m33) - (transpose.m13 * transpose.m31);
	adjM->m23 = -1 * ((transpose.m11 * transpose.m32) - (transpose.m12 * transpose.m31));

	adjM->m31 = (transpose.m12 * transpose.m23) - (transpose.m13 * transpose.m22);
	adjM->m32 = -1 * ((transpose.m11 * transpose.m23) - (transpose.m13 * transpose.m21));
	adjM->m33 = (transpose.m11 * transpose.m22) - (transpose.m12 * transpose.m21);

	return *adjM;
}

Matrix3x3&
Matrix3x3::invert()          // Converts into inverse.
{
	float determinant = this->det();
	assert( determinant != 0 );
	Matrix3x3 adjTranspose = this->adj();
	adjTranspose.transpose();

	Matrix3x3 inverse = adjTranspose / determinant;
	this->set(inverse);

	return *this;
}

inline Matrix3x3&
Matrix3x3::operator+=(const Matrix3x3& B)
{
    m11 += B.m11;
    m12 += B.m12;
    m13 += B.m13;
    m21 += B.m21;
    m22 += B.m22;
    m23 += B.m23;
    m31 += B.m31;
    m32 += B.m32;
    m33 += B.m33;
    return *this;
}

inline Matrix3x3&
Matrix3x3::operator-=(const Matrix3x3& B)
{
    m11 -= B.m11;
    m12 -= B.m12;
    m13 -= B.m13;
    m21 -= B.m21;
    m22 -= B.m22;
    m23 -= B.m23;
    m31 -= B.m31;
    m32 -= B.m32;
    m33 -= B.m33;
    return(*this);
}

inline Matrix3x3
operator+(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3(A.m11+B.m11, A.m21+B.m21, A.m31+B.m31,
                     A.m12+B.m12, A.m22+B.m22, A.m32+B.m32,
                     A.m13+B.m13, A.m23+B.m23, A.m33+B.m33);
}

inline Matrix3x3
operator-(const Matrix3x3& A)
{
    return Matrix3x3(-A.m11, -A.m21, -A.m31,
                     -A.m12, -A.m22, -A.m32,
                     -A.m13, -A.m23, -A.m33);
}

inline Matrix3x3
operator-(const Matrix3x3& A, const Matrix3x3& B)
{
    return Matrix3x3(A.m11-B.m11, A.m21-B.m21, A.m31-B.m31,
                     A.m12-B.m12, A.m22-B.m22, A.m32-B.m32,
                     A.m13-B.m13, A.m23-B.m23, A.m33-B.m33);
}

inline Matrix3x3&
Matrix3x3::operator*=(float b)
{
    m11 *= b;
    m12 *= b;
    m13 *= b;
    m21 *= b;
    m22 *= b;
    m23 *= b;
    m31 *= b;
    m32 *= b;
    m33 *= b;
    return *this;
}

inline Matrix3x3&
Matrix3x3::operator*=(const Matrix3x3& B)    // Matrix product
{
    float t1, t2;       // temporary values
    t1 =  m11*B.m11 + m12*B.m21 + m13*B.m31;
    t2 =  m11*B.m12 + m12*B.m22 + m13*B.m32;
    m13 =  m11*B.m13 + m12*B.m23 + m13*B.m33;
    m11 = t1;
    m12 = t2;

    t1 =  m21*B.m11 + m22*B.m21 + m23*B.m31;
    t2 =  m21*B.m12 + m22*B.m22 + m23*B.m32;
    m23 =  m21*B.m13 + m22*B.m23 + m23*B.m33;
    m21 = t1;
    m22 = t2;

    t1 =  m31*B.m11 + m32*B.m21 + m33*B.m31;
    t2 =  m31*B.m12 + m32*B.m22 + m33*B.m32;
    m33 =  m31*B.m13 + m32*B.m23 + m33*B.m33;
    m31 = t1;
    m32 = t2;

    return *this;
}

inline Matrix3x3
operator*(const Matrix3x3& A, const Matrix3x3& B) // Matrix product
{
    Matrix3x3 R;
    float t1, t2;       // temporary values
    t1 =  A.m11*B.m11 + A.m12*B.m21 + A.m13*B.m31;
    t2 =  A.m11*B.m12 + A.m12*B.m22 + A.m13*B.m32;
    R.m13 =  A.m11*B.m13 + A.m12*B.m23 + A.m13*B.m33;
    R.m11 = t1;
    R.m12 = t2;

    t1 =  A.m21*B.m11 + A.m22*B.m21 + A.m23*B.m31;
    t2 =  A.m21*B.m12 + A.m22*B.m22 + A.m23*B.m32;
    R.m23 =  A.m21*B.m13 + A.m22*B.m23 + A.m23*B.m33;
    R.m21 = t1;
    R.m22 = t2;

    t1 =  A.m31*B.m11 + A.m32*B.m21 + A.m33*B.m31;
    t2 =  A.m31*B.m12 + A.m32*B.m22 + A.m33*B.m32;
    R.m33 =  A.m31*B.m13 + A.m32*B.m23 + A.m33*B.m33;
    R.m31 = t1;
    R.m32 = t2;

    return R;
}

inline Matrix3x3
operator*(const Matrix3x3& A, float b)
{
    return(Matrix3x3(A.m11*b, A.m21*b, A.m31*b,
                       A.m12*b, A.m22*b, A.m32*b,
                       A.m13*b, A.m23*b, A.m33*b));
}

inline Matrix3x3
operator*(float b, const Matrix3x3& A)
{
	return(Matrix3x3(A.m11*b, A.m21*b, A.m31*b,
                       A.m12*b, A.m22*b, A.m32*b,
                       A.m13*b, A.m23*b, A.m33*b));
}

inline Matrix3x3
operator/(const Matrix3x3& A, float b)
{
    register float bInv = 1.0/b;
    return (A*bInv);
}

inline Matrix3x3&
Matrix3x3::operator/=(float b)
{
    register float bInv = 1.0/b;
    return (*this *= bInv);
}

inline Vector3
operator*(const Matrix3x3& A, const Vector3& u)
{
    return Vector3(A.m11*u.x + A.m12*u.y + A.m13*u.z,
                   A.m21*u.x + A.m22*u.y + A.m23*u.z,
                   A.m31*u.x + A.m32*u.y + A.m33*u.z);
}

#endif // CSE168_MATRIX3X3_H_INCLUDED
