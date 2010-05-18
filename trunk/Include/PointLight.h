#ifndef CSE168_POINTLIGHT_H_INCLUDED
#define CSE168_POINTLIGHT_H_INCLUDED

#include <vector>
#include "Vector3.h"

class PointLight
{
public:
	PointLight() { m_type = PointLight::POINT_LIGHT; }
	~PointLight() {}
    virtual void setPosition(const Vector3& v)  {m_position = v;}
    void setColor(const Vector3& v)     {m_color = v;}
    void setWattage(float f)            {m_wattage = f;}
    
    float wattage() const               {return m_wattage;}
    const Vector3 & color() const       {return m_color;}
    const Vector3& position() const     {return m_position;}

	bool isAreaLight() const { return m_type == PointLight::AREA_LIGHT; }

    virtual void preCalc() {} // use this if you need to

protected:
    Vector3 m_position;
    Vector3 m_color;
    float m_wattage;

	enum Type
	{
		POINT_LIGHT,
		AREA_LIGHT
	};

	Type m_type;
};

typedef std::vector<PointLight*> Lights;

#endif // CSE168_POINTLIGHT_H_INCLUDED
