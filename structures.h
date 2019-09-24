enum Type
{
	SPHERE = 1,
	PLANE
};

struct Geometry
{
	Type type;
	union {
		struct
		{
			Vec3F position;
			float radius;
		} sphere;
		struct
		{
			Vec3F normale;
			float distance;
		} plane;
	};
};

struct Material
{
	float IOR;
	float roughness;
	Vec3F specularColor;
	Vec3F diffuseColor;
};

struct Object
{
	Geometry geom;
	Vec3F color;
	Material mat;
};

struct Ray
{
	Vec3F origin, direction;
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
	Object object;
	Vec3F position;
	float distance;
	Vec3F normale;
	Material mat;
};

typedef std::vector<Object> Objects;

typedef std::vector<Intersection> Intersections;

typedef std::vector<Light> Lights;
