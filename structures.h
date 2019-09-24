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

struct Object
{
	Geometry geom;
	Vec3F color;
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
};

struct Intersection
{
	Object object;
	Vec3F position;
	float distance;
	Vec3F normale;
};

typedef std::vector<Object> Objects;

typedef std::vector<Intersection> Intersections;
