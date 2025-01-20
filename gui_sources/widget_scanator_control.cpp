#include "widget_scanator_control.h"
#include "ui_widget_scanator_control.h"
#include <qnamespace.h>
#include <QChar>
WidgetScanatorControl::WidgetScanatorControl(QWidget* parent) : WidgetAdjustable(parent), ui(new Ui::WidgetScanatorControl)
{
    ui->setupUi(this);
	//this->setFixedWidth(470);

  numPosX = QString::number(0.0,'f',1).leftJustified(7,'0');
  numPosY = QString::number(0.0,'f',1).leftJustified(7,'0');

  ui->labelDisplayPos1->setText(QString("УГОЛ [ %1 ] [ %2 ]").arg(numPosX).arg(numPosY));
}

void WidgetScanatorControl::LinkToDevice(std::shared_ptr<ScanatorControlClass> EngineModule)
{

    ScanatorDevice = EngineModule;
	QVector<QPushButton*> ButtonsMoveLine;
	ButtonsMoveLine.append(ui->butMoveLeft);
	ButtonsMoveLine.append(ui->butMoveRight);
	ButtonsMoveLine.append(ui->butMoveDown);
	ButtonsMoveLine.append(ui->butMoveUp);
	
	QVector<QPair<double,double>> StepVectors;
    StepVectors.append(QPair<double,double>(-5,0 ));
    StepVectors.append(QPair<double,double>( 5,0 ));
    StepVectors.append(QPair<double,double>( 0,5));
    StepVectors.append(QPair<double,double>( 0,-5));

	for (int n = 1; n <= 4; n++)
	{
	QSharedPointer<QTimer> Timer = QSharedPointer<QTimer>(new QTimer);
	QPushButton* ButMoveLine = ButtonsMoveLine.at(n-1);     

				connect(ButMoveLine, &QPushButton::pressed,
				[Timer,ButMoveLine]() { Timer->start(1); });

				connect(ButMoveLine, &QPushButton::released, 
				[Timer,ButMoveLine]() { Timer->stop(); });

                auto StepVector = StepVectors[n-1];
				connect(Timer.data(), &QTimer::timeout,
				[this, EngineModule,StepVector]()
				{
                    EngineModule->SlotMoveOnStep(StepVector);
				});

	}

				connect(ui->butSetToNull, &QPushButton::clicked, 
				[this, EngineModule]()             
	            {
                    EngineModule->SetToNullWork();
				});

				connect(ui->butStartStopWork, &QPushButton::toggled, 
				[this, EngineModule](bool OnOff)             
	            {
                    EngineModule->SetModuleEnabled(OnOff);
				});

   QObject::connect(&timerUpdateState,SIGNAL(timeout()), this, SLOT(SlotDisplayScanatorState()));
   timerUpdateState.start(100);
}



void WidgetScanatorControl::SlotDisplayScanatorState()
{
  if(!ScanatorDevice) return;

      auto& Pos = ScanatorDevice->ScanatorPos;
  auto& PosReal = ScanatorDevice->ScanatorPosControl;
      auto& Vel = ScanatorDevice->ScanatorVel;

  numPosX = QString::number(Pos.first,'f',1).leftJustified(7,'0');
  numPosY = QString::number(Pos.second,'f',1).leftJustified(7,'0');
  numPosXReal = QString::number(PosReal.first,'f',1).leftJustified(7,'0');
  numPosYReal = QString::number(PosReal.second,'f',1).leftJustified(7,'0');
  numVelX = QString::number(Vel.first,'f',1).leftJustified(7,'0');
  numVelY = QString::number(Vel.second,'f',1).leftJustified(7,'0');

       ui->labelDisplayPos1->setText( QString("           УГОЛ [ %1 ] [ %2 ]").arg(numPosX).arg(numPosY));
       ui->labelDisplayPos2->setText( QString("           УГОЛ [ %1 ] [ %2 ]").arg(numPosXReal).arg(numPosYReal));
  ui->labelDisplayVelocity1->setText( QString("СКОРОСТЬ [ %1 ] [ %2 ]").arg(numVelX).arg(numVelY));
}