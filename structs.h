enum Type
{
	SPHERE = 1,
	TRIANGLE
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
	Type type;
	union {
		struct
		{
			Vec3F position;
			float radius;
		} sphere;
		struct
		{
			Vec3F v0, v1, v2;
		} triangle;
	};
	Material material;
};

struct Ray
{
	Vec3F origin;
	Vec3F direction;
	int depth;
};

struct Light
{
	Vec3F position;
	Vec3F color;
};

struct Intersection
{
	Object object;
	Vec3F position;
	float distance;
	Vec3F normale;
};

struct Mesh
{
	Vec3F position;
	float taille;
	Material material;
	std::vector<Vec3F> vertices;
	std::vector<Vec3F> facades;
};

struct Box
{
	Vec3F lb, rt;
	std::vector<Box> boxs;
	int depth;
	std::vector<Object> objects;
};

typedef std::vector<Box> Boxs;

typedef std::vector<Object> Objects;

typedef std::vector<Intersection> Intersections;