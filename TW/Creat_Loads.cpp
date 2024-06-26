#include "Creat_Loads.h"
#include <iostream> // cout
#include"ConcentrateForce.h"
#include"Gravity.h"
#include"Wind.h"
#include"Polynomial.h"
#include"InterFace.h"
#include"IceLoad.h"
#include"Wind.h"
#include<QMessageBox.h>
#pragma execution_character_set("utf-8")
Creat_Loads::Creat_Loads(Instance* instance,QWidget *parent): QDialog(parent)
{
	ui.setupUi(this);

	m_pInterFace = dynamic_cast<InterFace*>(parent);
	Q_ASSERT(m_pInterFace != nullptr);
	m_instance = instance;

	ui.line_name->setText("Load - ");
	//按钮响应事件
	connect(ui.btn_continue, &QPushButton::clicked, this, &Creat_Loads::btn_continue_clicked);
	connect(ui.btn_cancle, &QPushButton::clicked, this, &Creat_Loads::reject);	
}

Creat_Loads::~Creat_Loads()
{}

void Creat_Loads::btn_continue_clicked()
{
	int size = m_pInterFace->ME_AnalysisSteps.size();//这一步必须要，不然下面条件没有判断
	if (size == 0)
	{
		QMessageBox::information(this, "Tips", "请先创建分析步！"); return;
	}
	else
	{
		int index = ui.listWidget->currentRow();//取选的载荷
		m_str = ui.line_name->text();

		if (index == 0)
		{//集中力
			ConcentrateForce* concentrateForce = new ConcentrateForce(this);
			concentrateForce->show();
		}
		else if (index == 1)
		{//重力
			Gravity* gravity = new Gravity(this);
			gravity->show();
		}
		else if (index == 2)
		{//覆冰
			IceLoad* iceLoad = new IceLoad(this);
			iceLoad->show();
		}
		else if (index == 3)
		{//多项式函数
			Polynomial* polynomial = new Polynomial;
			polynomial->show();
		}
		else if (index == 4)
		{
			//风载荷
			Wind* wd = new Wind(this);
			wd->show();
		}
		this->accept();
	}
}
