#include "structs.h"

const float PI = (float)atan(1.f) * 4.f;

/* Distribution sur matériaux rugueux */

float RDM_Beckmann(float NdotH, float alpha)
{
	if (NdotH > 0.f)
	{
		float cos2 = NdotH * NdotH;
		float tan2 = (1.f - cos2) / cos2;
		float cos4 = cos2 * cos2;
		float alpha2 = alpha * alpha;
		return expf(-tan2 / alpha2) / (PI * alpha2 * cos4);
	}
	return 0.f;
}

/* Coefficient de réflexion de Fresnel */

float RDM_Fresnel(float LdotH, float extIOR, float intIOR)
{
	float cosI = LdotH;
	float ratioIOR = extIOR / intIOR;
	float sinT2 = ratioIOR * ratioIOR * (1.f - (cosI * cosI));
	if (sinT2 > 1.f)
		return 1.f;
	float cosT = sqrtf(1.f - sinT2);
	float Rs = powf(cosI * extIOR - cosT * intIOR, 2) / powf(cosI * extIOR + cosT * intIOR, 2);
	float Rp = powf(cosT * extIOR - cosI * intIOR, 2) / powf(cosT * extIOR + cosI * intIOR, 2);
	return 0.5f * (Rs + Rp);
}

float RDM_G1(float DdotH, float DdotN, float alpha)
{
	float tan = sqrtf(1.f - DdotN * DdotN) / DdotN;
	float b = 1.f / (alpha * tan);
	float k = DdotH / DdotN;
	if (k > 0.f)
		if (b < 1.6f)
			return (3.535f * b + 2.181f * b * b) / (1.f + 2.276f * b + 2.577f * b * b);
		else
			return 1.f;
	else
		return 0.f;
}

/* Fonction d'ombrage et de masquage de Smith */

float RDM_Smith(float LdotH, float LdotN, float VdotH, float VdotN, float alpha)
{
	return RDM_G1(LdotH, LdotN, alpha) * RDM_G1(VdotH, VdotN, alpha);
}

/* Specular */

Vec3F RDM_bsdf_s(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material &m)
{
	float d = RDM_Beckmann(NdotH, m.roughness);
	float f = RDM_Fresnel(LdotH, 1.f, m.IOR);
	float g = RDM_Smith(LdotH, LdotN, VdotH, VdotN, m.roughness);
	return m.specularColor * d * f * g / (4.f * LdotN * VdotN);
}

/* Diffuse */

Vec3F RDM_bsdf_d(Material &m)
{
	return m.diffuseColor / PI;
}

/* Full BSDF */

Vec3F RDM_bsdf(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material &m)
{
	return RDM_bsdf_d(m) + RDM_bsdf_s(LdotH, NdotH, VdotH, LdotN, VdotN, m);
}

Vec3F shade(Vec3F n, Vec3F v, Vec3F l, Vec3F lc, Material &mat)
{
	Vec3F h = normalize(v + l);
	float LdotH = dot(l, h);
	float VdotH = dot(v, h);
	float NdotH = dot(n, h);
	float LdotN = dot(l, n);
	float VdotN = dot(v, n);
	Vec3F bsdf = RDM_bsdf(LdotH, NdotH, VdotH, LdotH, VdotN, mat);
	return lc * bsdf * LdotN;
}