#include "Creat_WireData.h"
#include <vtkMath.h>

Node* Creat_WireData::Find_Node(int id, vector<Node>& node)
{
	for (auto& i : node)
	{
		if (i.m_idNode == id)
		{
			return &i;
		}
	}
	cout << "没找到！\n";
	return nullptr;
}

void Creat_WireData::CreateWire(vector<Node>& node)
{
	for (int i = 0; i < pt.size() - 1; i++)
	{
		double lxi = sqrt((pt[i].x - pt[i + 1].x) * (pt[i].x - pt[i + 1].x) + (pt[i].y - pt[i + 1].y) * (pt[i].y - pt[i + 1].y));//档距
		double  hi = pt[i + 1].z - pt[i].z;//高差
		double k = gama / stress;
		double Li = (2 / k) * sinh(k * lxi / 2); //线长
		double li = pt[i + 1].x - pt[i].x; //两点x之差
		double mi = pt[i + 1].y - pt[i].y; //两点y之差
		double nni = sqrt(li * li + mi * mi); //二维中两点之差
		double dxi = li / N;
		double dyi = mi / N;
		double dzi = nni / N;
		double y0i = k * ((1 - cosh((1 / k) * (li / 2)) * sqrt(1 + (hi / Li) * (hi / Li)))) + hi / 2;

		if (i > 0 && i < pt.size())node.pop_back(); //删除重节点
		for (int j = 0; j < N + 1; j++)
		{
			double Zi = j * dzi;
			double x = j * dxi + pt[i].x;
			double y = j * dyi + pt[i].y;
			double z = ((1. / k) * (hi / Li)) * (sinh(k * lxi / 2) + sinh(k * (2 * Zi - lxi) / 2)) - ((2 / k) * sinh(k * Zi / 2) *
				sinh(k * (lxi - Zi) / 2)) * sqrt(1 + (hi / Li) * (hi / Li)) + pt[i].z;
			node.push_back(Node(x, y, z));
		}

	}
	for (int i = 0; i < node.size(); i++)
	{
		node[i].m_idNode = i + 1;//给节点赋编号
	}
}

void Creat_WireData::CreatWire_ele(int a, int b, vector<Element_Wire>& m_Elements)
{
	//a--起点，b--终点
	for (int i = a; i < b; i++)
	{
		m_Elements.push_back(Element_Wire(0, i, i + 1));
	}
	for (int i = 0; i < m_Elements.size(); i++)
	{
		m_Elements[i].m_idElement = i + 1;//给单元赋编号
	}
}

vector<int> Creat_WireData::Find_Spacer(int fenli, double S, int N, vector<Node> xgd, vector<Node> wire0, int dangwei) const//有bug 
{
	vector<int> idnodes;
	int S_l = S / 1000;
	for (auto& i : wire0)
	{
		double error = 0.0005 * (sqrt(pow((xgd[dangwei].x - xgd[dangwei - 1].x), 2) + pow((xgd[dangwei].y - xgd[dangwei - 1].y), 2)) / N);
		double Lm = 0.001 * sqrt(pow(i.x - xgd[dangwei - 1].x, 2) + pow(i.y - xgd[dangwei - 1].y, 2));//所有点距第一个悬挂点的距离
		if (i.y >= xgd[dangwei - 1].y)
		{
			if ((abs(Lm - S_l))  < error)
			{
				for (int k = 0; k < fenli; ++k)
				{
					idnodes.push_back(i.m_idNode + ((xgd.size() - 1) * N + 1) * k);

				}
			}
			else if ((abs(Lm - S_l)) - error < 0.1)
			{

			}
		}

	}
	return idnodes;
}

void Creat_WireData::Creat_Spacer(vector<Element_Wire>& spacer, vector<int> ids)
{
	ids.push_back(ids[0]);
	int id1 = ids[0];
	int idelement = spacer.size();
	size_t size = ids.size();
	for (int i = 1; i < size; i++)
	{
		int id2 = ids[i];
		spacer.push_back(Element_Wire(idelement, id1, id2));//总的单元
		//spacer.push_back(Element(idelement, id1, id2));  可加入到梁单元
		id1 = id2;
		idelement++;
	}
}

void Creat_WireData::Creat_Strain_Strand(vector<Node> nz_pts, int c, vector<int> ids, vector<Node>& node, vector<Element_Wire>& m_Elements)
{
	int maxid = node.size();
	Node n1 = Node(maxid + 1, nz_pts[c].x, nz_pts[c].y, nz_pts[c].z); //耐张点
	node.push_back(n1);
	int idelement = m_Elements.size();
	size_t size = ids.size();
	for (int i = 0; i < size; i++)
	{
		int id2 = ids[i];
		m_Elements.push_back(Element_Wire(idelement, maxid + 1, id2));//总的单元
		idelement++;
	}
}

void Creat_WireData::Find_Real_XGD(vector<Node> wire_temp, double L, int N, int choose, vector<Node> list_xgd, vector<Node>& real_xgd)
{
	for (auto& i : wire_temp)
	{
		if (choose == 0)//左耐张
		{
			double error = 0.500 * sqrt(pow((list_xgd[1].x - list_xgd[0].x), 2) + pow((list_xgd[1].y - list_xgd[0].y), 2) + pow((list_xgd[1].z - list_xgd[0].z), 2)) / N;
			double nz = sqrt(pow((i.x - list_xgd[0].x), 2) + pow((i.y - list_xgd[0].y), 2) + pow((i.z - list_xgd[0].z), 2));
			if (abs(nz - L) < error)
			{
				real_xgd.push_back(Node(i.x, i.y, i.z));
				for (int i = 1; i < list_xgd.size(); i++)
				{
					real_xgd.push_back(Node(list_xgd[i].x, list_xgd[i].y, list_xgd[i].z));
				}
			}

		}
		else if (choose == 1)//右耐张
		{
			int size = list_xgd.size() - 1;
			double error = 0.500 * sqrt(pow((list_xgd[size].x - list_xgd[size - 1].x), 2) + pow((list_xgd[size].y - list_xgd[size - 1].y), 2) + pow((list_xgd[size].z - list_xgd[size - 1].z), 2)) / N;
			double nz = sqrt(pow((i.x - list_xgd[size].x), 2) + pow((i.y - list_xgd[size].y), 2) + pow((i.z - list_xgd[size].z), 2));
			if (abs(nz - L) < error)
			{
				for (int i = 0; i < list_xgd.size() - 1; i++)
				{
					real_xgd.push_back(Node(list_xgd[i].x, list_xgd[i].y, list_xgd[i].z));
				}
				real_xgd.push_back(Node(i.x, i.y, i.z));
			}
		}
		else if (choose == 2)//两边耐张
		{
			double error_1 = 0.500 * sqrt(pow((list_xgd[1].x - list_xgd[0].x), 2) + pow((list_xgd[1].y - list_xgd[0].y), 2) + pow((list_xgd[1].z - list_xgd[0].z), 2)) / N;
			int size = list_xgd.size() - 1;
			double error_2 = 0.500 * sqrt(pow((list_xgd[size].x - list_xgd[size - 1].x), 2) + pow((list_xgd[size].y - list_xgd[size - 1].y), 2) + pow((list_xgd[size].z - list_xgd[size - 1].z), 2)) / N;
			double nz_1 = sqrt(pow((i.x - list_xgd[0].x), 2) + pow((i.y - list_xgd[0].y), 2) + pow((i.z - list_xgd[0].z), 2));
			double nz_2 = sqrt(pow((i.x - list_xgd[size].x), 2) + pow((i.y - list_xgd[size].y), 2) + pow((i.z - list_xgd[size].z), 2));
			if (abs(nz_1 - L) < error_1)
			{
				real_xgd.push_back(Node(i.x, i.y, i.z));
				for (int i = 1; i < list_xgd.size() - 1; i++)
				{
					real_xgd.push_back(Node(list_xgd[i].x, list_xgd[i].y, list_xgd[i].z));
				}
			}
			if (abs(nz_2 - L) < error_2)
			{
				real_xgd.push_back(Node(i.x, i.y, i.z));
			}
		}

	}
}

void Creat_WireData::Creat_Sag_Wire(double sag,int N, vector<Node> node, vector<Node>& m_node)
{
	for (int i = 0; i < node.size() - 1; i++)
	{
		double lxi = sqrt((node[i].x - node[i + 1].x) * (node[i].x - node[i + 1].x) + (node[i].y - node[i + 1].y) * (node[i].y - node[i + 1].y));//档距
		double  hi = node[i + 1].z - node[i].z;//高差
		double l_maxi = sqrt(hi * hi + lxi * lxi);
		double beita = lxi / l_maxi;//cos(B);
		double k = sag * ((8 * beita) / (lxi * lxi));
		double Li = (2 / k) * sinh(k * lxi / 2); //线长
		double li = node[i + 1].x - node[i].x; //两点x之差
		double mi = node[i + 1].y - node[i].y; //两点y之差
		double nni = sqrt(li * li + mi * mi); //二维中两点之差
		double dxi = li / N;
		double dyi = mi / N;
		double dzi = nni / N;
		double y0i = k * ((1 - cosh((1 / k) * (li / 2)) * sqrt(1 + (hi / Li) * (hi / Li)))) + hi / 2;

		if (i > 0 && i < node.size())node.pop_back(); //删除重节点
		for (int j = 0; j < N + 1; j++)
		{
			double Zi = j * dzi;
			double x = j * dxi + node[i].x;
			double y = j * dyi + node[i].y;
			double z = ((1. / k) * (hi / Li)) * (sinh(k * lxi / 2) + sinh(k * (2 * Zi - lxi) / 2)) - ((2 / k) * sinh(k * Zi / 2) *
				sinh(k * (lxi - Zi) / 2)) * sqrt(1 + (hi / Li) * (hi / Li)) + node[i].z;
			m_node.push_back(Node(x, y, z));
		}
	}
	for (int i = 0; i < m_node.size(); i++)
	{
		m_node[i].m_idNode = i + 1;//给节点赋编号
	}
}
