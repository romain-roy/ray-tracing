bool intersect_sphere(Ray &ray, Object &object, Intersection &intersection)
{
	Vec3F pos = ray.origin - object.geom.sphere.position;
	float a = 1.f; // dot2(ray.direction);
	float b = 2.f * dot(ray.direction, pos);
	float c = dot2(pos) - object.geom.sphere.radius * object.geom.sphere.radius;
	float delta = b * b - 4.f * a * c;
	if (delta <= 0.f)
	{
		return false;
	}
	else
	{
		float t1 = (-b - sqrtf(delta)) / (2.f * a);
		float t2 = (-b + sqrtf(delta)) / (2.f * a);
		intersection.distance = t1 > 0.f ? t1 : t2 > 0.f ? t2 : -1.f;
	}
	if (intersection.distance > 0.f)
	{
		intersection.position = ray.origin + ray.direction * intersection.distance;
		intersection.normale = normalize(intersection.position - object.geom.sphere.position);
		intersection.object = object;
		return true;
	}
	return false;
}

bool intersect_triangle(Ray &ray, Object &object, Intersection &intersection)
{
	Vec3F edge1, edge2, h, s, q;
	float a, f, u, v;
	const float epsilon = 0.0000001f;
	edge1 = object.geom.triangle.v1 - object.geom.triangle.v0;
	edge2 = object.geom.triangle.v2 - object.geom.triangle.v0;
	h = cross(ray.direction, edge2);
	a = dot(edge1, h);
	if (a > -epsilon && a < epsilon)
	{
		return false;
	}
	f = 1.f / a;
	s = ray.origin - object.geom.triangle.v0;
	u = f * (dot(s, h));
	if (u < 0.f || u > 1.f)
	{
		return false;
	}
	q = cross(s, edge1);
	v = f * (dot(ray.direction, q));
	if (v < 0.f || u + v > 1.f)
	{
		return false;
	}
	float t = f * dot(edge2, q);
	if (t > epsilon)
	{
		intersection.distance = t;
		intersection.position = ray.origin + ray.direction * t;
		intersection.normale = normalize(cross(edge1, edge2));
		intersection.object = object;
		return true;
	}
	else
	{
		return false;
	}
}

bool intersect_box(Ray &ray, Box &box, Intersection &intersection)
{
	Vec3F dir_frac;
	dir_frac.x = 1.f / ray.direction.x;
	dir_frac.y = 1.f / ray.direction.y;
	dir_frac.z = 1.f / ray.direction.z;
	float t1 = (box.lb.x - ray.origin.x) * dir_frac.x;
	float t2 = (box.rt.x - ray.origin.x) * dir_frac.x;
	float t3 = (box.lb.y - ray.origin.y) * dir_frac.y;
	float t4 = (box.rt.y - ray.origin.y) * dir_frac.y;
	float t5 = (box.lb.z - ray.origin.z) * dir_frac.z;
	float t6 = (box.rt.z - ray.origin.z) * dir_frac.z;
	float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
	if (tmax < 0.f)
	{
		return false;
	}
	if (tmin > tmax)
	{
		return false;
	}
    intersection.distance = tmin;
	return true;
}

bool intersect_scene(Ray &ray, Box &box, Intersection &intersection)
{
	Intersections intersections;
	size_t object_count = box.objects.size();
	for (unsigned int k = 0; k < object_count; k++)
	{
		Object obj = box.objects.at(k);
		switch (obj.geom.type)
		{
		case TRIANGLE:
			if (intersect_triangle(ray, obj, intersection))
			{
				intersections.push_back(intersection);
			}
			break;
		case SPHERE:
			if (intersect_sphere(ray, obj, intersection))
			{
				intersections.push_back(intersection);
			}
			break;
		}
	}
	if (!intersections.empty())
	{
		intersection = intersections.at(0);
		size_t intersections_count = intersections.size();
		for (unsigned int k = 1; k < intersections_count; k++)
		{
			if (intersections.at(k).distance < intersection.distance)
			{
				intersection = intersections.at(k);
			}
		}
		return true;
	}
	return false;
}