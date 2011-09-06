#ifndef PS_PARTICLESYSTEM_H
#define PS_PARTICLESYSTEM_H

#include "PS_Vector.h"
#include "DSystem/include/DContainers.h"

#define DEFAULT_PARTICLE_LIFETIME 1000

using namespace PS::MATH;

namespace PS{

namespace SFX{

class CParticle{
public:
	CParticle() 
	{
		color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
		age   = 0;
		lifetime = DEFAULT_PARTICLE_LIFETIME;
		size     = 1;
		shape    = 0;
	}

public:
	vec3f position;
	vec3f velocity;
	vec4f color;

	int age;
	int lifetime;
	
	int shape;
	int size;
};


class CParticleSystem
{
private:
	DVec<CParticle> m_particles;
public:
	CParticleSystem()
	{
	}

	~CParticleSystem(){}

	CParticle& getParticle(size_t index) {return m_particles[index];}
};

}
}
#endif