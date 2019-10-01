struct Material
{
	float IOR;
	float roughness;
	Vec3F specularColor;
	Vec3F diffuseColor;
};

struct Sphere
{
	Vec3F position;
	float radius;
	Material material;
};

struct Ray
{
	Vec3F origin;
	Vec3F direction;
};

struct Light
{
	Vec3F position;
	Vec3F color;
};

struct Intersection
{
	Sphere sphere;
	Vec3F position;
	float distance;
	Vec3F normale;
};

typedef std::vector<Sphere> Spheres;

typedef std::vector<Intersection> Intersections;

typedef std::vector<Light> Lights;
