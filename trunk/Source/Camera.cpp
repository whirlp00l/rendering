#include <stdio.h>
#include <stdlib.h>
#include "Miro.h"
#include "Camera.h"
#include "Image.h"
#include "Scene.h"
#include "Console.h" 
#include "OpenGL.h"
#include "DebugMem.h"

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
	m_aperture(1.0f)
{
    calcLookAt();
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
	Vector3 focalPlaneNormal = m_lookAt - m_eye;
	focalPlaneNormal.normalize();

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