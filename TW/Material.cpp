#include "Material.h"

Material::Material(int id, QString iName, double iE, double iPoisson, double iDensity, double iStress, double iThermal)
{
	m_id = id;
	m_Name= iName;
	E= iE;//弹性模量
	Poisson= iPoisson;//泊松
	Density= iDensity;//密度
	UltimateStress = iStress;//极限应力
	Thermal= iThermal;//热膨胀
}

Material::~Material()
{
}
