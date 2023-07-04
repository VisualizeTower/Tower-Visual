#include "Wind.h"
#include"InterFace.h"
#include"WindList.h"
#include <cmath>
#include <QMessageBox>
#include"RandomWind.h"

Wind::Wind(InterFace* pInterFace, QWidget *parent): QDialog(parent)
{
	ui.setupUi(this);
	//m_pInterFace = dynamic_cast<InterFace*>(parent);
	//Q_ASSERT(m_pInterFace != nullptr);
	m_pInterFace = pInterFace;
	Initialize();
	//CreateCombobox();
	connect(ui.rad_t, &QRadioButton::clicked, this, [=]() {ui.stackedWidget->setCurrentIndex(0); });
	connect(ui.rad_f, &QRadioButton::clicked, this, [=]() {ui.stackedWidget->setCurrentIndex(1); });
	connect(ui.rad_stab, &QRadioButton::clicked, this, [=]() {ui.stackedWidget_2->setCurrentIndex(0); });
	connect(ui.rad_ran, &QRadioButton::clicked, this, [=]() {ui.stackedWidget_2->setCurrentIndex(1); });

	connect(ui.rad_A, &QRadioButton::clicked, [=]() {alf = 0.12; });
	connect(ui.rad_B, &QRadioButton::clicked, [=]() {alf = 0.16; });
	connect(ui.rad_C, &QRadioButton::clicked, [=]() {alf = 0.20; });
	connect(ui.rad_D, &QRadioButton::clicked, [=]() {alf = 0.30; });

	ShowObject();
	void (QComboBox:: * intChanged)(int) = &QComboBox::currentIndexChanged;
	connect(ui.object_com, intChanged, this, &Wind::ShowObject);
	connect(ui.sure_btn_2, &QPushButton::clicked, this, &Wind::ui_Speed);
	connect(ui.count_btn, &QPushButton::clicked, this, &Wind::BtnOk);

	
}

Wind::~Wind()
{}

void Wind::Initialize()
{
	ui.ice_edi->setText("5");
	ui.wind_edi->setText("10");
	ui.direction_edi->setText("30");
	layout = new QVBoxLayout(ui.widget_5);
	// 创建 QVTKOpenGLNativeWidget
	m_VtkWidget = new QVTKOpenGLNativeWidget(this);
	layout->addWidget(m_VtkWidget);
	// 创建 VTK 渲染窗口
	renderWindow = m_VtkWidget->renderWindow();
	m_Renderer = vtkSmartPointer<vtkRenderer>::New();
	renderWindow->AddRenderer(m_Renderer);

	//添加坐标系
	vtkAxesActor* axesActor = vtkAxesActor::New();
	//以Widget方式,在左下角的视口中显示坐标系，可进行鼠标交互
	vtkOrientationMarkerWidget* widget = vtkOrientationMarkerWidget::New();
	widget->SetOrientationMarker(axesActor);
	widget->SetInteractor(m_VtkWidget->interactor());
	widget->SetViewport(0, 0, 0.2, 0.2);
	widget->SetEnabled(true);
	widget->InteractiveOn();

	// 更改背景颜色
	m_Renderer->SetBackground(1.0, 1.0, 1.0);              // 底部颜色值
}

void Wind::CreateCombobox()
{
	ExampleNum =  m_pInterFace->ui.treeWidget->topLevelItem(1)->childCount();//塔腿数量
	
	for (int i = 0; i < ExampleNum; i++)
	{
		ui.object_com->addItem("杆塔实例" + QString::number(i + 1));
		
	}
}

void Wind::ShowObject()
{
	int Index = ui.object_com->currentIndex();
	if (Index == -1)return;
	if (Index < ExampleNum)
	{
		id_Part = Index + 1;
		m_Renderer->RemoveAllViewProps();  // 清空当前显示的所有actor
		m_pTower = m_pInterFace->TP.Find_Entity(id_Part);
		m_pInterFace->Show_Tower(m_pTower);
		m_pTower->Show_VTKbeam(m_Renderer);
		m_pTower->Show_VTKtruss(m_Renderer);
	}
	m_Renderer->ResetCamera();
	ui.widget_5->update();
}

void Wind::ui_Speed()
{
	if (ui.wind_com->currentIndex() == 0)
	{
		if (w_l == nullptr)
		{
			w_l = new WindList(this);
			w_l->show();
		}
		else
		{
			w_l->show();
		}
	}
	else if (ui.wind_com->currentIndex() == 1)
	{
		if (ran == nullptr)
		{
			ran = new RandomWind(m_pInterFace);
			ran->show();
		}
		else
		{
			ran->show();
		}
	}
	
}

void Wind::ReadData()
{
	//QStringList headerLabels;
	//headerLabels << "节点编号" << "Z(m)" << "风压系数";
	//ui.tableWidget->setColumnCount(headerLabels.count());
	//ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	//ui.tableWidget->verticalHeader()->setVisible(false);
	//ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	//ui.tableWidget->setRowCount(num_node);  // 默认N行

	//for (int k = 0; k < num; k++)
	//{
	//	const auto& node = m_pTower->m_Nodes[k];
	//	double uz = CountUz((node.z + 14000) / 1000);

	//	ui.tableWidget->setItem(k, 0, new QTableWidgetItem(QString::number(node.m_idNode)));
	//	ui.tableWidget->setItem(k, 1, new QTableWidgetItem(QString::number((node.z / 1000) + 14)));
	//	ui.tableWidget->setItem(k, 2, new QTableWidgetItem(QString::number(uz)));
	//}
	QStringList headerLabels;
	headerLabels << "单元编号" << "单元长度" << "风压系数"<<"风载荷";
	ui.tableWidget->setColumnCount(headerLabels.count());
	ui.tableWidget->setHorizontalHeaderLabels(headerLabels);
	ui.tableWidget->verticalHeader()->setVisible(false);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidget->setRowCount(num_node);  // 默认N行
	for (int i = 0; i < num_node; i++)
	{
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(m_pTower->m_Elements[i].m_idElement)));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number((L_ele[i]))));
		ui.tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(Uz_ele[i])));
		ui.tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(0)));
	}
}

void Wind::BtnOk()
{
	if (m_pTower == nullptr)
	{
		QMessageBox::warning(this, "提示", "请创建分析对象！");
		return;
	}

	num_node = m_pTower->m_Nodes.size();
	num_ele = m_pTower->m_Elements.size();
	bool groundSelected = false;

	QList<QRadioButton*> radioButtons = { ui.rad_A, ui.rad_B, ui.rad_C, ui.rad_D };

	for (QRadioButton* radioButton : radioButtons)
	{
		if (radioButton->isChecked())
		{
			groundSelected = true;
			break;
		}
	}

	if (!groundSelected)
	{
		QMessageBox::warning(this, "提示", "请先选择地面粗糙程度！");
		return;
	}
	CountElePara();
	ReadData();
}


double Wind::CountUz(double h)
{
	double uz = 0.0;
	if (ui.rad_A->isChecked())
	{
		uz = 1.284 * pow(h / 10, 0.24);
		if (uz < 1.09)
		{
			uz = 1.09;
		}
	}
	else if (ui.rad_B->isChecked())
	{
		uz = 1.000 * pow(h / 10, 0.30);
		if (uz < 1.00)
		{
			uz = 1.00;
		}
	}
	else if (ui.rad_C->isChecked())
	{
		uz = 0.544 * pow(h / 10, 0.44);
		if (uz < 0.65)
		{
			uz = 0.65;
		}
	}
	else if (ui.rad_D->isChecked())
	{
		uz = 0.262 * pow(h / 10, 0.60);
		if (uz < 0.51)
		{
			uz = 0.51;
		}
	}

	return uz;
}

void Wind::CountElePara()
{
	// 算每个节点的风压系数
	for (int k = 0; k < num_node; k++)
	{
		const auto& node = m_pTower->m_Nodes[k];
		double uz = CountUz((node.z + 14000) / 1000);
		Uz_node.push_back(uz);
	}

	// 计算每个单元的长度和风压系数
	for (int i = 0; i < num_ele; i++)
	{
		const auto& ele = m_pTower->m_Elements[i];
		int ipt1 = ele.m_idNode[0] - 1; // id
		int ipt2 = ele.m_idNode[1] - 1;
		const auto& node1 = m_pTower->m_Nodes[ipt1];
		const auto& node2 = m_pTower->m_Nodes[ipt2];

		double Vx[3];
		Vx[0] = (node2.x - node1.x) / 1000;
		Vx[1] = (node2.y - node1.y) / 1000;
		Vx[2] = (node2.z - node1.z) / 1000;

		double m_L = sqrt(Vx[0] * Vx[0] + Vx[1] * Vx[1] + Vx[2] * Vx[2]);
		L_ele.push_back(m_L);

		double uz_ele = (Uz_node[ipt1] + Uz_node[ipt2]) / 2;
		Uz_ele.push_back(uz_ele);
	}
}

void Wind::CountWindForce()
{
	double v = ui.wind_edi->text().toDouble();
	double W = v * v / 1600;
	double Usc = 1.1;
	double beitaC = 1.0;
	double B = 1.0;
}
