#define DEPTH_BOX 10 // Nombre de sous-boîtes englobantes

struct Box
{
	Vec3F lb, rt;
	std::vector<Box> boxs;
	int depth;
	std::vector<Object> objects;
};

typedef std::vector<Box> Boxs;

void create_boxs(Boxs &boxs)
{
	size_t boxs_count = boxs.size();
	for (unsigned int b = 0; b < boxs_count; b++)
	{
		if (boxs.at(b).depth < DEPTH_BOX)
		{
			Box new_box1, new_box2;
			size_t objects_count = boxs.at(b).objects.size();
			float longueur = boxs.at(b).rt.x - boxs.at(b).lb.x;
			float hauteur = boxs.at(b).rt.y - boxs.at(b).lb.y;
			if (longueur > hauteur)
			{
				new_box1.lb = boxs.at(b).lb;
				new_box1.rt = {boxs.at(b).lb.x + longueur / 2, boxs.at(b).rt.y, boxs.at(b).rt.z};
				new_box2.lb = {new_box1.rt.x, boxs.at(b).lb.y, boxs.at(b).lb.z};
				new_box2.rt = boxs.at(b).rt;
				for (unsigned int o = 0; o < objects_count; o++)
				{
					if (boxs.at(b).objects.at(o).geom.triangle.v0.x < new_box1.rt.x && boxs.at(b).objects.at(o).geom.triangle.v1.x < new_box1.rt.x && boxs.at(b).objects.at(o).geom.triangle.v2.x < new_box1.rt.x)
						new_box1.objects.push_back(boxs.at(b).objects.at(o));
					else if (boxs.at(b).objects.at(o).geom.triangle.v0.x > new_box1.rt.x && boxs.at(b).objects.at(o).geom.triangle.v1.x > new_box1.rt.x && boxs.at(b).objects.at(o).geom.triangle.v2.x > new_box1.rt.x)
						new_box2.objects.push_back(boxs.at(b).objects.at(o));
					else
					{
						new_box1.objects.push_back(boxs.at(b).objects.at(o));
						new_box2.objects.push_back(boxs.at(b).objects.at(o));
					}
				}
			}
			else
			{
				new_box1.lb = boxs.at(b).lb;
				new_box1.rt = {boxs.at(b).rt.x, boxs.at(b).lb.y + hauteur / 2, boxs.at(b).rt.z};
				new_box2.lb = {boxs.at(b).lb.x, new_box1.rt.y, boxs.at(b).lb.z};
				new_box2.rt = boxs.at(b).rt;
				for (unsigned int o = 0; o < objects_count; o++)
				{
					if (boxs.at(b).objects.at(o).geom.triangle.v0.y < new_box1.rt.y && boxs.at(b).objects.at(o).geom.triangle.v1.y < new_box1.rt.y && boxs.at(b).objects.at(o).geom.triangle.v2.y < new_box1.rt.y)
						new_box1.objects.push_back(boxs.at(b).objects.at(o));
					else if (boxs.at(b).objects.at(o).geom.triangle.v0.y > new_box1.rt.y && boxs.at(b).objects.at(o).geom.triangle.v1.y > new_box1.rt.y && boxs.at(b).objects.at(o).geom.triangle.v2.y > new_box1.rt.y)
						new_box2.objects.push_back(boxs.at(b).objects.at(o));
					else
					{
						new_box1.objects.push_back(boxs.at(b).objects.at(o));
						new_box2.objects.push_back(boxs.at(b).objects.at(o));
					}
				}
			}
			new_box1.depth = boxs.at(b).depth + 1;
			new_box2.depth = new_box1.depth;
			boxs.at(b).boxs.push_back(new_box1);
			boxs.at(b).boxs.push_back(new_box2);
			create_boxs(boxs.at(b).boxs);
		}
	}
}