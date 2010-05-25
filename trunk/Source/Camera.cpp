#include <stdio.h>
#include <stdlib.h>
#include "Miro.h"
#include "Camera.h"
#include "Image.h"
#include "Scene.h"
#include "Console.h" 
#include "OpenGL.h"
#include "DebugMem.h"
#include <time.h>

Camera * g_camera = 0;

static bool firstRayTrace = true; 

const float HalfDegToRad = DegToRad/2.0f;

Camera::Camera() :
    m_bgColor(0,0,0),
    m_renderer(RENDER_OPENGL),
    m_eye(0,0,0),
    m_viewDir(0,0,-1),
    m_up(0,1,0),
    m_lookAt(FLT_MAX, FLT_MAX, FLT_MAX),
    m_fov((45.)*(PI/180.)),
	m_focal_plane_distance(10.0f),
	m_aperture(0.1f)
{
    calcLookAt();
	calcAperturePlaneAxes();
}


Camera::~Camera()
{
}


void
Camera::click(Scene* pScene, Image* pImage)
{
    calcLookAt();
    static bool firstRayTrace = false;

    if (m_renderer == RENDER_OPENGL)
    {
        glDrawBuffer(GL_BACK);
        pScene->openGL(this);
        firstRayTrace = true;
    }
    else if (m_renderer == RENDER_RAYTRACE)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDrawBuffer(GL_FRONT);
        if (firstRayTrace)
        {
            pImage->clear(bgColor());
            pScene->raytraceImage(this, g_image);
            firstRayTrace = false;
        }
        
        g_image->draw();
    }
}


void
Camera::calcLookAt()
{
    // this is true when a "lookat" is not used in the config file
    if (m_lookAt.x != FLT_MAX)
    {
        setLookAt(m_lookAt);
        m_lookAt.set(FLT_MAX, FLT_MAX, FLT_MAX);
    }
}


void
Camera::drawGL()
{
    // set up the screen with our camera parameters
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov(), g_image->width()/(float)g_image->height(),
                   0.01, 10000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Vector3 vCenter = eye() + viewDir();
    gluLookAt(eye().x, eye().y, eye().z,
              vCenter.x, vCenter.y, vCenter.z,
              up().x, up().y, up().z);
}


Ray
Camera::eyeRay(int x, int y, int imageWidth, int imageHeight)
{
    // first compute the camera coordinate system 
    // ------------------------------------------

    // wDir = e - (e+m_viewDir) = -m_vView
    const Vector3 wDir = Vector3(-m_viewDir).normalize(); 
    const Vector3 uDir = cross(m_up, wDir).normalize(); 
    const Vector3 vDir = cross(wDir, uDir);    



    // next find the corners of the image plane in camera space
    // --------------------------------------------------------

    const float aspectRatio = (float)imageWidth/(float)imageHeight; 


    const float top     = tan(m_fov*HalfDegToRad); 
    const float right   = aspectRatio*top; 

    const float bottom  = -top; 
    const float left    = -right; 



    // transform x and y into camera space 
    // -----------------------------------

    const float imPlaneUPos = left   + (right - left)*(((float)x+0.5f)/(float)imageWidth); 
    const float imPlaneVPos = bottom + (top - bottom)*(((float)y+0.5f)/(float)imageHeight); 

    return Ray(m_eye, (imPlaneUPos*uDir + imPlaneVPos*vDir - wDir).normalize(), 1.0f);
}

bool
Camera::getFocalPlaneIntersection(Vector3 &desiredFocalPlanePt, const Vector3 hitPt) const
{
	/*
	 * The IDEA:
	 *
	 * Equation for a plane: x is in a plane with normal n that contains point p if:
	 *		dot( n, x - p ) = 0;
	 * The point on the focal plane that we are interested in can be represented as:
	 *		cameraEyePt + t * dirToHitPtFromCamera
	 * for some value of t.
	 *
	 * We can combine these two equations to solve for t.
	 */
	Vector3 focalPlaneNormal = m_viewDir;

	Vector3 focalPlanePt = m_eye + m_focal_plane_distance * focalPlaneNormal;

	Vector3 dirFromCamera = hitPt - m_eye;
	dirFromCamera.normalize();

	float t = ( dot( focalPlaneNormal, focalPlanePt ) - dot( focalPlaneNormal, m_eye ) ) / dot( focalPlaneNormal, dirFromCamera );

	// the intersection with the focal plane is AT the camera or behind it
	if( t <= 0 )
		return false;

	// calculate the point on the focal plane we are interested in
	desiredFocalPlanePt = m_eye + t * dirFromCamera;
	return true;
}

Vector3 
Camera::getRandomApertureSample() const
{
	// randomizer is seeded by Scene.cpp

	float tAxis1 = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
	float tAxis2 = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]

	// since the random sample can be anywhere within the disc, the t values can range from [-1,1]
	float posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
	if( posOrNeg <= 0.5 )
		tAxis1 *= -1;
	posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
	if( posOrNeg <= 0.5 )
		tAxis2 *= -1;

	Vector3 randomSample = m_eye + tAxis1 * m_aperture_plane_axis1 + tAxis2 * m_aperture_plane_axis2;
	return randomSample;
}

void
Camera::calcAperturePlaneAxes()
{
	// we need a point in the plane OTHER than the eye point.
	// we can just use arbitrary values of x and y, and then solve for z such that the point is in the plane
	Vector3 point1( m_eye.x + 1, m_eye.y + 1, 0 ); // we'll solve for z in a moment...

	// note that m_viewDir is normal to the aperture plane
	point1.z = m_eye.z - ( m_viewDir.x * ( point1.x - m_eye.x ) - m_viewDir.y * ( point1.y - m_eye.y ) ) / m_viewDir.z;

	// now we can get our first axis
	m_aperture_plane_axis1 = point1 - m_eye;
	m_aperture_plane_axis1.normalize();

	// make the second axis orthonormal to the first
	m_aperture_plane_axis2 = cross( m_viewDir, m_aperture_plane_axis1 );
	m_aperture_plane_axis2.normalize();

	// now make the length of each axis the radius of the aperture
	m_aperture_plane_axis1 *= m_aperture;
	m_aperture_plane_axis2 *= m_aperture;
}