void create_mesh(Mesh &mesh, Boxs &boxs)
{
	Vec3F min = {mesh.vertices.at(0).x, mesh.vertices.at(0).y, mesh.vertices.at(0).z};
	Vec3F max = min;

	/* Centrer l'objet */

	Vec3F somme_vertices = mesh.vertices.at(0);
	size_t vertices_count = mesh.vertices.size();
	float norm_max = norm(mesh.vertices.at(0));
	for (unsigned int i = 1; i < vertices_count; i++)
	{
		somme_vertices = somme_vertices + mesh.vertices.at(i);
		if (norm(mesh.vertices.at(i)) > norm_max)
			norm_max = norm(mesh.vertices.at(i));
		min.x = std::min(min.x, mesh.vertices.at(i).x);
		min.y = std::min(min.y, mesh.vertices.at(i).y);
		min.z = std::min(min.z, mesh.vertices.at(i).z);
		max.x = std::max(max.x, mesh.vertices.at(i).x);
		max.y = std::max(max.y, mesh.vertices.at(i).y);
		max.z = std::max(max.z, mesh.vertices.at(i).z);
	}
	norm_max /= mesh.taille;

	Vec3F centre_gravite = somme_vertices / (float)vertices_count;
	Vec3F offset = mesh.position;
	offset = offset - (centre_gravite / norm_max);

	/* Normaliser sa taille */

	for (unsigned int i = 0; i < vertices_count; i++)
		mesh.vertices.at(i) = mesh.vertices.at(i) / norm_max;

	/* Création de la boîte englobante */

	Box box;
	box.lb = min / norm_max + offset;
	box.rt =  max / norm_max + offset;
	box.depth = 0;

	/* Création des triangles */

	size_t facades_count = mesh.facades.size();
	for (unsigned int i = 0; i < facades_count; i++)
	{
		Object triangle;
		triangle.material = mesh.material;
		triangle.type = TRIANGLE;
		triangle.triangle.v0 = mesh.vertices.at((int)mesh.facades.at(i).x) + offset;
		triangle.triangle.v1 = mesh.vertices.at((int)mesh.facades.at(i).y) + offset;
		triangle.triangle.v2 = mesh.vertices.at((int)mesh.facades.at(i).z) + offset;
		box.objects.push_back(triangle);
	}

	boxs.push_back(box);
}

/* Parser */

bool parse(std::string filename, Mesh &mesh)
{
	int nv, nf;

	/* Container holding last line read */

	std::string readLine;

	/* Containers for delimiter positions */

	int delimiterPos_1, delimiterPos_2, delimiterPos_3, delimiterPos_4;

	/* Open file for reading */

	std::ifstream in(filename.c_str());

	/* Check if file is in OFF format */

	getline(in, readLine);
	if (readLine != "OFF")
	{
		std::cout << "The file to read is not in OFF format." << std::endl;
		return false;
	}

	/* Read values for Nv and Nf */

	getline(in, readLine);
	delimiterPos_1 = (int)readLine.find(" ", 0);
	nv = atoi(readLine.substr(0, delimiterPos_1 + 1).c_str());
	delimiterPos_2 = (int)readLine.find(" ", delimiterPos_1);
	nf = atoi(readLine.substr(delimiterPos_1, delimiterPos_2 + 1).c_str());

	/* Vertices */

	for (int n = 0; n < nv; n++)
	{
		Vec3F v;
		getline(in, readLine);
		delimiterPos_1 = (int)readLine.find(" ", 0);
		v.x = (float)atof(readLine.substr(0, delimiterPos_1).c_str());
		delimiterPos_2 = (int)readLine.find(" ", delimiterPos_1 + 1);
		v.y = (float)atof(readLine.substr(delimiterPos_1, delimiterPos_2).c_str());
		delimiterPos_3 = (int)readLine.find(" ", delimiterPos_2 + 1);
		v.z = (float)atof(readLine.substr(delimiterPos_2, delimiterPos_3).c_str());
		mesh.vertices.push_back(v);
	}

	/* Façades */

	for (int n = 0; n < nf; n++)
	{
		Vec3F f;
		getline(in, readLine);
		delimiterPos_1 = (int)readLine.find(" ", 0);
		delimiterPos_2 = (int)readLine.find(" ", delimiterPos_1 + 1);
		f.x = (float)atof(readLine.substr(delimiterPos_1, delimiterPos_2).c_str());
		delimiterPos_3 = (int)readLine.find(" ", delimiterPos_2 + 1);
		f.y = (float)atof(readLine.substr(delimiterPos_2, delimiterPos_3).c_str());
		delimiterPos_4 = (int)readLine.find(" ", delimiterPos_3 + 1);
		f.z = (float)atof(readLine.substr(delimiterPos_3, delimiterPos_4).c_str());
		mesh.facades.push_back(f);
	}

	return true;
}