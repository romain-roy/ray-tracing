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
bool operator==(const Vec3<T> &a, const Vec3<T> &b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
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

Vec3F reflect(const Vec3F &I, const Vec3F &N)
{
	return I - N * 2.f * dot(N, I);
}

template <typename T>
void print(const Vec3<T> &v)
{
	printf("%.1f %.1f %.1f\n", v.x, v.y, v.z);
}