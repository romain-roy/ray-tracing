template <typename T>
struct Vec3
{
	T x, y, z;
};

using Vec3F = Vec3<float>;

template <typename T>
float dot(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
float dot2(const Vec3<T> &v)
{
	return dot(v, v);
}

template <typename T>
float norm(const Vec3<T> &v)
{
	return std::sqrt(dot2(v));
}

template <typename T>
Vec3<T> normalize(const Vec3<T> &v)
{
	return v / norm(v);
}

template <typename T>
Vec3<T> cross(const Vec3<T> &u, const Vec3<T> &v)
{
	Vec3<T> w;
	w.x = u.y * v.z - u.z * v.y;
	w.y = u.z * v.x - u.x * v.z;
	w.z = u.x * v.y - u.y * v.x;
	return w;
}

template <typename T>
bool operator==(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

template <typename T>
bool operator!=(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

template <typename T>
Vec3<T> operator+(const Vec3<T> &a, const Vec3<T> &b)
{
	return Vec3<T>{a.x + b.x, a.y + b.y, a.z + b.z};
}

template <typename T>
Vec3<T> operator-(const Vec3<T> &a, const Vec3<T> &b)
{
	return Vec3<T>{a.x - b.x, a.y - b.y, a.z - b.z};
}

template <typename T>
Vec3<T> operator*(const Vec3<T> &a, const Vec3<T> &b)
{
	return Vec3<T>{a.x * b.x, a.y * b.y, a.z * b.z};
}

template <typename T>
Vec3<T> operator/(const Vec3<T> &a, const Vec3<T> &b)
{
	return Vec3<T>{a.x / b.x, a.y / b.y, a.z / b.z};
}

template <typename T>
Vec3<T> operator*(const Vec3<T> &a, const float &b)
{
	return Vec3<T>{a.x * b, a.y * b, a.z * b};
}

template <typename T>
Vec3<T> operator/(const Vec3<T> &a, const float &b)
{
	return Vec3<T>{a.x / b, a.y / b, a.z / b};
}

template <typename T>
bool operator>(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.x > b.x && a.y > b.y && a.z > b.z;
}

template <typename T>
bool operator<(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.x < b.x && a.y < b.y && a.z < b.z;
}

Vec3F reflect(const Vec3F &I, const Vec3F &N)
{
	return I - N * 2.f * dot(N, I);
}

template <typename T>
void print(const Vec3<T> &v)
{
	printf("%.1f %.1f %.1f\n", v.x, v.y, v.z);
}