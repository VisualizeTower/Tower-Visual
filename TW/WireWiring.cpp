#include "WireWiring.h"
#include"InterFace.h"
#include"Material.h"
#include"Section_L.h"
#include"Section.h"
#include"NominalHeight.h"
#include"HangPoint.h"
#include"TowerList.h"
void WireWiring::GetAllPartTXT(string path, vector<string>& files)
{
	intptr_t hFile = 0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼,�ݹ����
			//�������,���ļ�����·������vector��
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (string(fileinfo.name).substr(string(fileinfo.name).find_last_of(".") + 1) == "txt")
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void WireWiring::ReadWireWiring(InterFace* InterFace)
{
	m_pInterFace = InterFace;
	vector<string> fileNames;
	string path("../PartTXT"); 	//�Լ�ѡ��Ŀ¼����
	GetAllPartTXT(path, fileNames);

	//qDebug() << "fileNames.size()��"<< fileNames.size()<<"\t";
	for (int i = 0; i < fileNames.size(); i++)
	{
		str = QString::fromStdString(fileNames[i]);
		if (!str.isEmpty())
		{
			//qDebug() << str;
			Qf1.setFileName(str);
			if (!Qf1.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				qDebug() << "�ļ���ʧ�ܣ�";
				return;
			}
			Stream1.setDevice(&Qf1);

			QString keyword;
			while (!Stream1.atEnd())
			{
				QString qLine = Stream1.readLine();

				if (qLine.startsWith("**") || qLine.isEmpty())
				{
					// ����˵���кͿ���
					continue;
				}
				else if (qLine.startsWith("*"))
				{
					QStringList parts = qLine.mid(1).split(",", Qt::SkipEmptyParts);
					keyword = parts[0].trimmed();
					if (keyword.compare("Line", Qt::CaseInsensitive) == 0)
					{
						QString lineName = parts[1].trimmed();//��·����
						QString towerNumber = parts[2].trimmed();//������
						bool isExist = 0;
						for (const auto& j : m_pInterFace->TWG)
						{
							if (j.second->m_Name == lineName)
							{
								towerWire = j.second;
								isExist = 1;
							}
						}
						if (isExist == false)
						{
							towerWire = new TowerWireGroup();
							towerWire->m_Name = lineName;
							int id = m_pInterFace->TWG.size() + 1;
							towerWire->m_id = id;
							m_pInterFace->TWG.Add_Entity(towerWire);
							
						}
						tower = new TowerList();
						tower->m_Name = towerNumber;
						int id = towerWire->TWG_TP.size() + 1;
						tower->m_id = id;
						towerWire->TWG_TP.Add_Entity(tower);
						
					}
					else if (keyword.compare("Parts", Qt::CaseInsensitive) == 0)
					{
						PartList(parts);
					}
					else if (keyword.compare("High", Qt::CaseInsensitive) == 0)
					{
						HighList(parts);
					}
					else if (keyword.compare("HangPoint", Qt::CaseInsensitive) == 0)
					{
						HangPointList(parts);
					}
				}
			}

		}
	}
}

void WireWiring::PartList(QStringList parts)
{
	int PartsNumber = parts[1].toInt();//��������
	int processedCount = 0;// ��¼�Ѵ����Ĳ�������
	QString keyword;//��¼�����Ĺؼ���
	Part_Base* Part = nullptr;
	while (processedCount < PartsNumber && !Stream1.atEnd())
	{
		QString qLine = Stream1.readLine();

		if (qLine.startsWith("**") || qLine.isEmpty())
		{
			// ����˵���кͿ���
			continue;
		}
		else if (qLine.startsWith("*"))
		{

			QStringList Partparts = qLine.mid(1).split(",", Qt::SkipEmptyParts);
			keyword = Partparts[0].trimmed();
			if (keyword.compare("Part", Qt::CaseInsensitive) == 0)
			{
				Part = new Part_Base();
				QString lineName = Partparts[1].trimmed();//��������
				int towerNumber = Partparts[2].toInt();//�������
				QString drawingNumber = Partparts[3].trimmed();//ͼֽ����
				Part->m_id = towerNumber;
				tower->TP_Part.Add_Entity(Part);
			}
			else if (Part != nullptr)
			{
				if (keyword.compare("Node", Qt::CaseInsensitive) == 0)
				{
					NodeList(Part, Partparts);
				}
				else if (keyword.compare("Element_Beam3D", Qt::CaseInsensitive) == 0)
				{
					Element_Beam3DList(Part, Partparts);
					//std::cout << "m_pInterFace->Ms.size() =  "<< m_pInterFace->Ms.size() << "\n";//==83��
					processedCount++;
				}
			}

		}
	}
}

void WireWiring::NodeList(Part_Base* part, QStringList parts)
{
	int NodeCount = parts[1].toInt();
	int processedCount = 0;// ��¼�Ѵ����Ĳ�������
	while (processedCount < NodeCount)
	{
		QString NodeLine = Stream1.readLine();
		if (NodeLine.startsWith("**") || NodeLine.isEmpty())
		{
			// ����˵���кͿ���
			continue;
		}
		QStringList NodeParts = NodeLine.split(QRegExp("[\\s,]+"), Qt::SkipEmptyParts);
		int NodeNumber = NodeParts[0].toInt();
		double NodeX = NodeParts[1].toDouble()/1000 ;
		double NodeY = NodeParts[2].toDouble() /1000;
		double NodeZ = NodeParts[3].toDouble()/1000 ;
		part->m_Nodes.push_back(Node(NodeNumber, NodeX, NodeY, NodeZ, 0, 0));
		processedCount++;
	}
}

void WireWiring::Element_Beam3DList(Part_Base* part, QStringList parts)
{
	int Element_Beam3DCount = parts[1].toInt();
	int processedCount = 0;// ��¼�Ѵ����Ĳ�������
	while (processedCount < Element_Beam3DCount)
	{
		QString Element_Beam3DLine = Stream1.readLine();
		if (Element_Beam3DLine.startsWith("**") || Element_Beam3DLine.isEmpty())
		{
			// ����˵���кͿ���
			continue;
		}
		QStringList Element_Beam3DParts = Element_Beam3DLine.split(QRegExp("[\\s,]+"), Qt::SkipEmptyParts);
		int Element_Beam3DNumber = Element_Beam3DParts[0].toInt();
		int Element_Beam3DidNode1 = Element_Beam3DParts[1].toDouble();
		int Element_Beam3DidNode2 = Element_Beam3DParts[2].toDouble();
		double Element_Beam3DDirectionX= Element_Beam3DParts[3].toDouble();
		double Element_Beam3DDirectionY = Element_Beam3DParts[4].toDouble();
		double Element_Beam3DDirectionZ = Element_Beam3DParts[5].toDouble();
		QString Element_Beam3DMaterial = Element_Beam3DParts[6];//����
		QString Element_Beam3DSection = Element_Beam3DParts[7];//����
		double Direction[3] = { Element_Beam3DDirectionX,Element_Beam3DDirectionY,Element_Beam3DDirectionZ };
		int id = GetSectionId1(Element_Beam3DMaterial, Element_Beam3DSection);//������
		part->m_Elements_beams.push_back(Element_Beam(Element_Beam3DNumber, Element_Beam3DidNode1, Element_Beam3DidNode2, id, Direction,1));
		processedCount++;
	}
}

int WireWiring::GetSectionId1(QString MaterialName, QString SectionName)
{
	//1����--��Ӧ�Ĳ��ϱ�źͽ�����
	int MaterialId = 0;
	for (const auto& j : m_pInterFace->ME_Material)
	{
		if (MaterialName == j.second->m_Name)//�Ҳ��ϱ��
		{
			MaterialId = j.second->m_id;
			break;
		}
	}
	int SectionId = 0;
	double Iu = 0;
	double Iv = 0;
	double J = 0;
	double A = 0;
	for (const auto& j : m_pInterFace->ME_Section_L)
	{
		if (SectionName == j.second->m_Name)//�ҽ�����
		{
			SectionId = j.second->m_id;
		}
	}
	//�Ҳ��������ŵ����-���Լ���Iu, Iv, J, A��Ȼ�����ӵ�ME_Section_L����
	if (SectionId == 0)
	{
		QString currentSegment = "";
		QStringList segments; // ���ڱ����ֺ�Ĳ���
		for (int i = 0; i < SectionName.length(); i++)
		{
			QChar currentChar = SectionName.at(i);
			if (currentChar.isLetter())
			{
				if (!currentSegment.isEmpty())
				{
					segments.append(currentSegment); // ����ֺ�Ĳ������ӵ� QStringList
					currentSegment = ""; // ���� currentSegment
				}
				segments.append(currentChar); // ����ĸ�ַ����ӵ� QStringList
			}
			else if (currentChar.isDigit()) { currentSegment += currentChar; }
		}

		// ����Ƿ���δ���ӵ����ֲ���
		if (!currentSegment.isEmpty()) {
			segments.append(currentSegment);
		}
		SectionId = m_pInterFace->ME_Section_L.size() + 1;
		GetLIxyz(segments[1].toDouble()/1000.0, segments[3].toDouble()/1000.0, &Iu, &Iv, &J, &A);
		Section_L* section = new Section_L(SectionId, SectionName, Iu, Iv, J, A);
		m_pInterFace->ME_Section_L.Add_Entity(section);
	}
	// 2�� �ж�--Ms.size()=0�������
	int MsSize = m_pInterFace->Ms.size();

	if (MsSize == 0)
	{
		Iu = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->Iu;
		Iv = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->Iv;
		J = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->J;
		A = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->A;
		Section* section = new Section(MsSize + 1, SectionName, Iu, Iv, J, MaterialId, A);
		m_pInterFace->Ms.Add_Entity(section);//���ӽ�ȥ
		return 1;
	}
	// 3������size��Ϊ0������£���ԭ����Ms����û���ҵ���ͬ�Ĳ��ϺŻ��߽�������
	else
	{
		int index = 0;//�൱��һ����ʶ��
		for (const auto& i : m_pInterFace->Ms)
		{
			//1�����Ѿ����ӵĽ��������ҵõ��Ѿ��еý������ƻ��߲��ϱ��
			if (i.second->m_Name == SectionName && i.second->ClassM == MaterialId)	
				index= i.second->m_id;
			if(index != 0)	return index;
		}
		if (index == 0)
		{
			Iu = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->Iu;
			Iv = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->Iv;
			J = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->J;
			A = m_pInterFace->ME_Section_L.Find_Entity(SectionId)->A;
			Section* section = new Section(MsSize + 1, SectionName, Iu, Iv, J, MaterialId, A);
			m_pInterFace->Ms.Add_Entity(section);//���ӽ�ȥ
			return MsSize + 1;
		}
	}
	return -1;
}

void WireWiring::GetLIxyz(double a, double b, double* Iu, double* Iv, double* J, double* S)
{
	double a2 = a * a;
	double a3 = a2 * a;
	double b2 = b * b;
	double b3 = b2 * b;
	*Iu = 1. / 12 * b * (2 * a2 - 2 * a * b + b2) * (2 * a - b);
	*Iv = b * (2 * a3 * a - 4 * a3 * b + 8 * a2 * b2 - 6 * a * b3 + b3 * b) / (24 * a - 12 * b);


	double L2 = a, t2 = b;
	*J = pow(t2, 0.3e1) * (0.1e1 / 0.3e1 - 0.105e0 * t2 * (0.1e1 - pow(t2, 0.4e1) * pow(L2 - t2, -0.4e1) / 0.192e3) / (L2 - t2)) * (L2 - t2) +
		L2 * pow(t2, 0.3e1) * (0.1e1 / 0.3e1 - 0.21e0 * t2 * (0.1e1 - pow(t2, 0.4e1) * pow(L2, -0.4e1) / 0.12e2) / L2) + 0.7e-1 * pow(0.4e1 * t2 - 0.2e1 * sqrt(0.2e1) * sqrt(t2 * t2), 0.4e1);
	*S = 2 * a * b - b * b;
}

void WireWiring::HighList(QStringList parts)
{
	int HighCount = parts[1].toInt();
	for (int i = 0; i < HighCount; ++i)
	{
		int processedCount = 0;// ��¼�Ѵ����ĺ�������
		NominalHeight* nominalHeight = new NominalHeight();
		// ��ȡ�ʹ���ÿ��������ϵ�����
		while (processedCount < 3)
		{
			QString line = Stream1.readLine();
			if (line.startsWith("**") || line.isEmpty())
			{
				// ������ "**" ��ͷ���кͿ���
				continue;
			}
			
			// ��������
			if (processedCount == 0)
			{
				QStringList HighLine = line.split(QRegExp("[\\s,]+"), Qt::SkipEmptyParts);
				int highNumber = HighLine[0].toInt();
				double height = HighLine[1].toDouble();
				nominalHeight->m_id = highNumber;
				nominalHeight->hight = height;
				processedCount++;
			}
			else if (processedCount == 1)
			{
				QStringList PartLine = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
				QStringList partBody = PartLine.mid(0);
				for (int j = 0; j < partBody.size(); j++)
				{
					nominalHeight->BodyList.push_back(partBody[j].toInt());
				}
				processedCount++;
			}
			else if (processedCount == 2)
			{
				QStringList LegLine = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
				QStringList partLeg = LegLine.mid(0);
				for (int j = 0; j < partLeg.size(); j++)
				{
					nominalHeight->LegList.push_back(partLeg[j].toInt());
				}
				processedCount++;
				tower->TP_Height.Add_Entity(nominalHeight);
			}
		}
	}
}

void WireWiring::HangPointList(QStringList parts)
{
	int HangPointCount = parts[1].toInt();
	int processedCount = 0;
	while (processedCount < HangPointCount)
	{
		QString HangPointLine = Stream1.readLine();
		if (HangPointLine.startsWith("**") || HangPointLine.isEmpty())
		{
			// ����˵���кͿ���
			continue;
		}
		QStringList HangPointParts = HangPointLine.split(QRegExp("[\\s,]+"), Qt::SkipEmptyParts);
		int id = HangPointParts[0].toInt();
		QString StringClass = HangPointParts[1];
		QString WireLoge = HangPointParts[2];
		if (StringClass == "V")
		{
			QString Partnumber = HangPointParts[3];
			int nodeid = HangPointParts[4].toInt();
			QStringList Partid = Partnumber.split(QRegExp("[\\s-]+"), Qt::SkipEmptyParts);
			int FindPartid1 = Partid[1].toInt();
			int id1 = tower->TP_Part.Find_Entity(FindPartid1)->TP_HangPoint.size() + 1;
			QString StringClass1 = StringClass;//����V1
			HangPoint* hangPoint1 = new HangPoint(id1, StringClass1, WireLoge, nodeid);
			tower->TP_Part.Find_Entity(FindPartid1)->TP_HangPoint.Add_Entity(hangPoint1);

			QString Partnumber2 = HangPointParts[5];
			int nodeid2 = HangPointParts[6].toInt();
			QStringList Partid2 = Partnumber2.split(QRegExp("[\\s-]+"), Qt::SkipEmptyParts);
			int FindPartid2 = Partid2[1].toInt();
			int id2 = tower->TP_Part.Find_Entity(FindPartid2)->TP_HangPoint.size() + 1;
			QString StringClass2 = StringClass ;//����V2
			HangPoint* hangPoint2 = new HangPoint(id2, StringClass2, WireLoge, nodeid2);
			tower->TP_Part.Find_Entity(FindPartid2)->TP_HangPoint.Add_Entity(hangPoint2);
		}
		else
		{
			QString Partnumber = HangPointParts[3];
			int nodeid = HangPointParts[4].toInt();
			QStringList Partid = Partnumber.split(QRegExp("[\\s-]+"), Qt::SkipEmptyParts);
			int FindPartid = Partid[1].toInt();
			int id1 = tower->TP_Part.Find_Entity(FindPartid)->TP_HangPoint.size() + 1;
			HangPoint* hangPoint = new HangPoint(id1, StringClass, WireLoge, nodeid);
			tower->TP_Part.Find_Entity(FindPartid)->TP_HangPoint.Add_Entity(hangPoint);
		} 
		processedCount++;
	}
}

