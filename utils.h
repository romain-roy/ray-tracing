template <typename T>
void permute(T &a, T &b)
{
	T c = a;
	a = b;
	b = c;
}

template <typename T>
T abs(T a)
{
	if (a < 0)
		return -a;
	return a;
}
