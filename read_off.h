#include <iostream>
#include <fstream>

int nv, nf;

typedef std::vector<Vec3F> Vertices;
typedef std::vector<Vec3F> Facades;

/* Parser */

bool parse(std::string filename, Vertices &vertices, Facades &facades)
{
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
		vertices.push_back(v);
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
		facades.push_back(f);
	}

	return true;
}