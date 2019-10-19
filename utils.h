/* Clamp un nombre entre 0 et 255 */

unsigned char clamp_color(float v)
{
	if (v > 255.f)
		v = 255.f;
	else if (v < 0.f)
		v = 0.f;
	return (unsigned char)v;
}

bool coord_out_of_scene(Vec3F coord)
{
	if (coord.x < 0.f || coord.x > 1000.f || coord.y < 0.f || coord.y > 1000.f || coord.z < 0.f || coord.z > 1000.f)
		return true;
	return false;
}

bool triangle_inf_x(Object &object, float coord)
{
	if (object.triangle.v0.x < coord && object.triangle.v1.x < coord && object.triangle.v2.x < coord)
		return true;
	return false;
}

bool triangle_sup_x(Object &object, float coord)
{
	if (object.triangle.v0.x > coord && object.triangle.v1.x > coord && object.triangle.v2.x > coord)
		return true;
	return false;
}

bool triangle_inf_y(Object &object, float coord)
{
	if (object.triangle.v0.y < coord && object.triangle.v1.y < coord && object.triangle.v2.y < coord)
		return true;
	return false;
}

bool triangle_sup_y(Object &object, float coord)
{
	if (object.triangle.v0.y > coord && object.triangle.v1.y > coord && object.triangle.v2.y > coord)
		return true;
	return false;
}