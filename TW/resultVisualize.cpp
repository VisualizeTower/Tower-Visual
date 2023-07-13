#include "resultVisualize.h"
#include"InterFace.h"
resultVisualize::resultVisualize(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	pCAE = dynamic_cast<InterFace*>(parent);
	assert(pCAE != nullptr);

	updateTimer = new QTimer(this);

	//�Զ������Ŵ�����
	ui.lineEdit->setText("1");
	connect(ui.radioButton, &QRadioButton::clicked, this, &resultVisualize::autoFactor);
	connect(ui.radioButton, &QRadioButton::clicked, ui.lineEdit, &QLineEdit::setDisabled);
	//���¶���
	connect(updateTimer, &QTimer::timeout, this, &resultVisualize::update);
	//����
	connect(ui.play_btn, &QPushButton::clicked, this, &resultVisualize::start);
	//��ͣ
	connect(ui.stop_btn, &QPushButton::clicked, this, &resultVisualize::stop);
	//��ֹ
	connect(ui.quit_btn, &QPushButton::clicked, this, &resultVisualize::quit);

	ui.label_frame->setText(QString::number(m_frames));//֡��ǩ
	//�����ٶȻ���
	ui.speed_Slider->setRange(1, 30);
	ui.speed_Slider->setValue(30);
	m_speed = 30;
	connect(ui.speed_Slider, &QSlider::valueChanged, this, &resultVisualize::set_fps);
	//��ʾԭʼactor
	ui.checkBox_showOriginal->setChecked(true);
	connect(ui.checkBox_showOriginal, &QCheckBox::clicked, this, &resultVisualize::showOriginalActor);
	//ѭ������
	ui.checkBox_loop->setChecked(true);
	set_loopPlay(true);
	connect(ui.checkBox_loop, &QCheckBox::clicked, this, &resultVisualize::set_loopPlay);
	//��������
	connect(this, &resultVisualize::animationFinished, [=]() {	m_frames = 0; });
}

resultVisualize::~resultVisualize()
{}

void resultVisualize::set_fps(int fps)
{
	m_speed = fps;
}

void resultVisualize::showOriginalActor(bool flag)
{
	m_tower->Node_actor->SetVisibility(flag);
	m_tower->m_BeamActor->SetVisibility(flag);
	m_tower->m_TrussActor->SetVisibility(flag);
	pCAE->m_renderWindow->Render();
}

void resultVisualize::update()
{
	if (m_frames >= p_nodes[0]->Get_DisplaymentXData().size())
	{
		if (loopPlay) emit animationFinished();
		return;
	}
	double ampFactor = ui.lineEdit->text().toDouble();//�Ŵ�����

	vtkIdType pointsNum = m_points->GetNumberOfPoints();
	//�޸ĵ������е�����
	for (vtkIdType i = 0; i < pointsNum; i++)
	{
		std::vector<double>xData = p_nodes[i]->Get_DisplaymentXData();
		std::vector<double>yData = p_nodes[i]->Get_DisplaymentYData();
		std::vector<double>zData = p_nodes[i]->Get_DisplaymentZData();
		double* p = m_originalPoints->GetPoint(i);
		m_points->SetPoint(i, p[0] + ampFactor * xData[m_frames], p[1] + ampFactor * yData[m_frames], p[2] + ampFactor * zData[m_frames]);
	}
	m_frames++;//֡++
	ui.label_frame->setText(QString::number(m_frames));//֡

	//����vtkActor��vtkPolyData������
	m_points->Modified();
	//m_vtkNodes->GetMapper()->Update();
	pCAE->m_renderWindow->Render();
}

void resultVisualize::start()
{
	if (m_vtkNodes == nullptr) return;

	int msec = m_dt / m_speed;
	qDebug() << "current time interval: " << msec << "ms";
	updateTimer->setInterval(msec);
	updateTimer->start();

	qDebug() << "play";
}

void resultVisualize::stop()
{
	updateTimer->stop();

	qDebug() << "stop";
}

void resultVisualize::quit()
{
	updateTimer->stop();
	m_frames = 0;

	removeActor();
	pCAE->m_renderWindow->Render();

	qDebug() << "quit";
}

void resultVisualize::autoFactor(bool flag)
{
	if (!flag) return;
	double max_x = 0;
	double min_x = 0;
	double max_y = 0;
	double min_y = 0;
	double max_z = 0;
	double min_z = 0;
	double max_disp = 0;
	for (auto& i : p_nodes)
	{
		if (max_x < i->m_x) max_x = i->m_x;
		if (min_x > i->m_x) min_x = i->m_x;

		if (max_y < i->m_y) max_y = i->m_y;
		if (min_y > i->m_y) min_y = i->m_y;

		if (max_z < i->m_z) max_z = i->m_z;
		if (min_z > i->m_z) min_z = i->m_z;

		vector<double> dux = i->Get_DisplaymentXData();
		vector<double> duy = i->Get_DisplaymentYData();
		vector<double> duz = i->Get_DisplaymentZData();
		double dX = abs(dux[dux.size() - 1]);
		double dY = abs(duy[duy.size() - 1]);
		double dZ = abs(duz[duz.size() - 1]);
		if (max_disp < dX) max_disp = dX;
		if (max_disp < dY) max_disp = dY;
		if (max_disp < dZ) max_disp = dZ;
	}
	double bd = 0;
	if (bd < (max_x - min_x)) bd = max_x - min_x;
	if (bd < (max_y - min_y)) bd = max_y - min_y;
	if (bd < (max_z - min_z)) bd = max_z - min_z;

	double ampFator = bd / max_disp / 10;

	ui.lineEdit->setText(QString::number(ampFator));
}

void resultVisualize::addData(vector<Node_Base*>node, Tower* tower)
{//ˢ�����ݣ����������ݱ仯��
	m_tower = tower;
	assert(m_tower);

	for (auto& i : node)
	{
		p_nodes.push_back(i);
	}
	addActorData();//��������������(copy)

	pCAE->m_Renderer_2->ResetCamera();
}

void resultVisualize::addActorData()
{
	if (m_vtkNodes != nullptr)
	{
		removeActor();
	}
	m_originalPoints = vtkSmartPointer<vtkPoints>::New();//ԭʼ����
	m_points = vtkSmartPointer<vtkPoints>::New();//������ʾ������

	vtkSmartPointer<vtkPoints> points = m_tower->m_pts;//�������е�����
	vtkSmartPointer<vtkCellArray> lines = m_tower->m_lines;//�������е�����

	m_originalPoints->ShallowCopy(points);//����ͬһ������
	m_points->DeepCopy(points);//����һ���µ�����

	vtkSmartPointer<vtkPolyData> nodesPolyData = vtkSmartPointer<vtkPolyData>::New();
	nodesPolyData->SetPoints(m_points);

	vtkNew<vtkVertexGlyphFilter> VertexFilter;
	VertexFilter->SetInputData(nodesPolyData);
	VertexFilter->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(VertexFilter->GetOutputPort());
	m_vtkNodes = vtkSmartPointer<vtkActor>::New();
	m_vtkNodes->SetMapper(mapper);
	m_vtkNodes->GetProperty()->SetColor(1, 0, 0);
	m_vtkNodes->GetProperty()->SetPointSize(3);

	//create the visualized line
	vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();
	//������(pt)�������һϵ�е㹹�����ݼ�(vtkDataSet)�ļ��νṹ
	linesPolyData->SetPoints(m_points);
	//�����ݵ�������ʽ(lines)�γɵĵ�Ԫ����(cell Data)�������ݼ����˽ṹ
	linesPolyData->SetLines(lines);

	vtkSmartPointer<vtkPolyDataMapper> linesmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	linesmapper->SetInputData(linesPolyData);

	m_vtklines = vtkSmartPointer<vtkActor>::New();
	m_vtklines->SetMapper(linesmapper);
	m_vtklines->GetProperty()->SetColor(1, 0, 0);

	pCAE->m_Renderer_2->AddActor(m_vtkNodes);
	pCAE->m_Renderer_2->AddActor(m_vtklines);
	pCAE->m_renderWindow->Render();
}

void resultVisualize::removeActor()
{
	pCAE->m_Renderer_2->RemoveActor(m_vtkNodes);
	pCAE->m_Renderer_2->RemoveActor(m_vtklines);

	m_vtkNodes = nullptr;
	m_vtklines = nullptr;
}