
#include "widget_aiming_control.h"
#include <QPushButton>

WidgetAimingControl::WidgetAimingControl(QWidget *parent)
	: WidgetAdjustable(parent)
{
	ui.setupUi(this);
    
    QObject::connect(&timerDisplayState,SIGNAL(timeout()), this, SLOT(SlotDisplayState()));
    ui.pushButtonFilterOnOff->hide();
    ui.checkTuningMode->hide();
    this->setFixedHeight(130);
}


WidgetAimingControl::~WidgetAimingControl()
{
}
    

void WidgetAimingControl::SlotDisplayState()
{
    auto& BlockState      = ModuleAiming->StateBlock;
    auto& BlockAimingType = ModuleAiming->AimingState;
    auto& PIDParam        = ModuleAiming->ModulePID.PIDParam;

	ui.checkWorkBlock->blockSignals(true);
	if (BlockState == StateBlockDisabled) ui.checkWorkBlock->setChecked(false);
	if (BlockState == StateBlockAtWork)  ui.checkWorkBlock->setChecked(true);

	if(BlockAimingType == AimingFast)   ui.checkMoveFastPID->setChecked(true);
    if(BlockAimingType == AimingSlow)   ui.checkMoveSlowPID->setChecked(true);
    if(BlockAimingType == AimingTuning) ui.checkTuningMode->setChecked(true);


    if(BlockState == StateBlockFault || BlockState == StateBlockBroken)
    ui.labelAimingBlockState->setText(QString("Сбой удержания"));
    else
    ui.labelAimingBlockState->setText(QString("%1 %3 %2").arg(PIDParam.RateParam,0,'f',1)
                                                         .arg(PIDParam.IntParam ,0,'f',2)
                                                         .arg(PIDParam.DiffParam,0,'f',2));
  
	ui.checkWorkBlock->blockSignals(false);
}


void WidgetAimingControl::LinkToModule(std::shared_ptr<AimingClass> Module)
{
    ModuleAiming = Module;

			connect(ui.checkWorkBlock, &QPushButton::toggled,
				[=](bool checked)
			{
				if (checked)
				{
              		ui.checkWorkBlock->setText(QString::fromLocal8Bit("Работа"));
					Module->SetModuleEnabled(true);
				}
				else
				{
              		ui.checkWorkBlock->setText(QString::fromLocal8Bit("Отключено"));
					Module->SetModuleEnabled(false);
				}
			});

			QVector<QPushButton*> Buttons;
			Buttons.append(ui.checkMoveSlowPID); Buttons.append(ui.checkMoveFastPID); Buttons.append(ui.checkTuningMode);

        int TypeEnumAiming = 0;
        for (QPushButton* But : Buttons)
        {
            TypeEnumAiming++;
            connect(But, &QPushButton::toggled,
                [=](bool checked)
            {
                if (checked)
                {
                    switch (TypeEnumAiming)
                    {
                    case 1: Module->SetAimingSpeedRegim(AimingSlow); break;
                    case 2: Module->SetAimingSpeedRegim(AimingFast); break;
                    case 3: Module->SetAimingSpeedRegim(AimingTuning); break;
                    }
                }
            });
        }

    QPushButton* But = ui.pushButtonFilterOnOff;

    connect(ui.pushButtonFilterOnOff, &QPushButton::toggled,
    [But,Module]()
    {
        qDebug() << "[ AIMING CONTROL ]" << "KALMAN FILTER NOT AVAILABLE";
        //Module->TurnOnOffKalmanFilter(But->isChecked());
    });

    timerDisplayState.start(300);
}
