struct Sphere
{
	Vec3F position;
	float radius;
	Vec3F color;
};

struct Ray
{
	Vec3F origin;
	Vec3F direction;
	int depth;
	float intensity;
};

struct Light
{
	Vec3F position;
	float intensity;
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