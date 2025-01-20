#ifndef ScanatorControlClass_H
#define ScanatorControlClass_H

#include "interface_pass_coord.h"
#include "aiming_class.h"
//#include "RotateOperationContainer.h"
#include <QSettings>
#include "thread_operation_nodes.h"
#include "rotate_vector_class.h"
#include <QTimer>
#include "state_block_enum.h"
#include "engine_udp_interface.h"
#include "transform_coord_class.h"
#include "message_command_structures.h"
#include "message_struct_generic.h"

typedef ControlMessage1 ScanatorControlStruct;


class ScanatorControlClass :public QObject,public PassTwoCoordClass
{
    Q_OBJECT
public:
    ScanatorControlClass(QObject* parrent = 0);
	QString TAG_NAME{"[ SCANATOR ]"};

    StateEnumBlock StateBlock{StateBlockDisabled};

	~ScanatorControlClass();

	void SetInput(const QPair<double,double>& Coord);
	const QPair<double,double>& GetOutput();
	int GetID(); 

    void LoadSettings();
    bool isAtLimit();
	void SetToNullReal();
	void SetToNullWork();
	void SetModuleEnabled(bool OnOff);

	TimeIntegratorClass PortMoveVelocity; // EMULATE MOVE WITH VELOCITY, INTEGRATOR LINKED TO SlotMoveToPos

	QPair<double, double> ScanatorPos;
	QPair<double, double> ScanatorVel;
	QPair<double, double> ScanatorAccel;
	QPair<double, double> ScanatorError;

	QPair<double, double> ScanatorPosControl;

	QPair<double, double> NullRelativePos{0.0,0.0};
	QPair<double, double> NullAbsolutePos{0.0,0.0};

	Statistic CheckPointsStat;

private:
	UDPEngineInterface* EnginePort = 0;

    MessageStructGeneric<ScanatorControlStruct,MESSAGE_HEADER> CommandScanator;
					   AimStateStruct* CommandChannel1{&CommandScanator.DATA.StateChannel1};
					   AimStateStruct* CommandChannel2{&CommandScanator.DATA.StateChannel2};

	RotateVectorClass   RotEngineToCamera;
    TransformCoordClass AngleToDAC;
public slots:

	void SlotMoveToPos(const QPair<double, double>& Pos);
	void SlotMoveOnStep(const QPair<double, double>& StepVector);
	void SlotMoveWithVelocity(const QPair<double, double>& VelVector);
	void SlotMoveWithAcceleration(const QPair<double, double>& AccelVector);
	void SlotMoveVector(const QPair<double,double>& Pos, 
	                    const QPair<double,double>& Vel, 
						const QPair<double,double>& Accel, 
						const QPair<double,double>& RelPos);
};

#endif //ScanatorControlClass_H
