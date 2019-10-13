/* Clamp un nombre entre 0 et 255 */

unsigned char clamp_color(float v)
{
	if (v > 255.f)
		v = 255.f;
	else if (v < 0.f)
		v = 0.f;
	return (unsigned char)v;
}