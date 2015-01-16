#include "Mesh.h"

Mesh::Mesh(void)
{
	Vec3ZeroOut(&this->pos);
	Vec3ZeroOut(&this->color);
}

Mesh::~Mesh(void)
{
}
