#ifndef AIMINGCLASS_H
#define AIMINGCLASS_H
#include "interface_pass_coord.h"

#include "pid_param_struct.h"
#include "type_block_enum.h"
#include "state_block_enum.h"
#include "pid_class.h"
#include <QSettings>
#include "register_settings.h"
#include "engine_statistics.h"
#include "thread_operation_nodes.h"
#include "transform_coord_class.h"

class TimeIntegratorClass : public PassTwoCoordClass
{
	public:
	QPair<double,double> CoordIntegral{0.0,0.0};

    std::chrono::time_point<std::chrono::high_resolution_clock> LastTimePoint;
	double StepPeriod = 0;

	void SetInput(const QPair<double,double>& Coord)
	{
	auto TimePoint = std::chrono::high_resolution_clock::now();
	    StepPeriod = std::chrono::duration<double>((TimePoint - LastTimePoint)).count(); 

	LastTimePoint = TimePoint; if(StepPeriod*1000 > 20) return;

    CoordIntegral.first  += Coord.first *StepPeriod;
    CoordIntegral.second += Coord.second*StepPeriod;
	}
	const QPair<double,double>& GetOutput() { return CoordIntegral;};

};


template <class T>
class PortAdapter : public PassTwoCoordClass
{
public:
    PortAdapter(){};
    void LinkAdapter(T* LinkObject,std::function<void(T&,QPair<double,double>)> SetFunction, std::function<QPair<double,double>(T&)> GetFunction)
    { InputFunction = SetFunction; OutputFunction = GetFunction; Receiver = LinkObject; };

    T* Receiver =  nullptr;

    std::function<void(T&,QPair<double,double>)> InputFunction = nullptr;
    std::function<QPair<double,double>(T&)> OutputFunction = nullptr;

    void SetInput(const QPair<double,double>& coord) { if(InputFunction != nullptr) InputFunction(*Receiver,coord);};
    const QPair<double,double>& GetOutput() { if(OutputFunction != nullptr) return OutputFunction(*Receiver);};

};

class SignalPortAdapter : public QObject
{
Q_OBJECT
public:
    SignalPortAdapter(){};
    SignalPortAdapter(PassTwoCoordClass* Port){ LinkToPort(Port);};
    PassTwoCoordClass* SetCoordPort = 0;
    void LinkToPort(PassTwoCoordClass* Port) { SetCoordPort = Port;};

public slots: 
void SlotSetCoord(QPair<double,double> Coord) { if(SetCoordPort != 0) Coord >> *SetCoordPort; }

};


class AimingParamOptimizator: public PassTwoCoordClass
{
  public:
  AimingParamOptimizator();
  AimingParamOptimizator(std::vector<PIDParamStruct> ParamTable);
  void operator=(const AimingParamOptimizator& Optimizator);

  int BestPIDParamNumber = 0;
  std::vector<PIDParamStruct> PIDParamTable;
  StatisticGroup PIDParamGroupStat = StatisticGroup(10,200);
  Statistic CurrentStatistic = Statistic(200);
  PIDParamStruct BestPIDParam;

  int LimitDispersionCounter = 0;

	bool isAimingFaultStatistic();
  bool FLAG_CONTROL_PARAM_REGIM = false;
  bool FLAG_REGISTRATION_REGIM = true;

  void SetInput(const QPair<double,double>& Coord);
  const QPair<double,double>& GetOutput() { return CurrentStatistic.GetDispersionCoord(); }
  void AppendPIDParam(PIDParamStruct PIDParam);
  void Reset();
};


class AimingClass : public PassTwoCoordClass
{
public:
    
	AimingClass();
	~AimingClass();

	QString TAG_NAME{"[ AIMING ]"};

	TypeEnumBlock  TypeBlock   = TypeEnumBlock::AimingBlock;
	StateEnumBlock StateBlock  = StateEnumBlock::StateBlockDisabled;
	TypeEnumAiming AimingState = AimingSlow;

  void SetPIDParamFromTable(int Number);
	void SetPIDParam(PIDParamStruct param);
	void SetAimingSpeedRegim(TypeEnumAiming Aiming);
	double GetAbsError();
	void   Reset();


	std::vector<QPair<double,double>> ErrorRecord;
  std::vector<QPair<double,double>>::iterator CurrentRecord;
  SubstractNode<double> Substract;

	const QPair<double, double>& GetOutput();
  const QPair<double, double>& GetAimPosition();  //AIM POSITION IS SET MANUAL OR FROM CAMERA 
  const QPair<double, double>& GetBeamPosition(); //BEAM POSITION IS SET FROM CAMERA 
                                                  //CAMERA CAN GET SIMULTENOUSLY AIM AND BEAM POSITION
  const QPair<double, double>& GetAimingError(); //BEAM POSITION IS SET FROM CAMERA 

  void SetModuleEnabled(bool OnOff); 
	int GetID() { return SettingsRegister::GetValue("BLOCK_ID_AIMING");}

	void SetInput           (const QPair<double,double>& Coord);
	void SetAimingPosition  (QPair<double, double> Coord);
  void SetAimingCorrection(QPair<double, double> Coord);
  void MoveAimingPosition (QPair<double, double> Coord);

  void LoadPIDParam(QString SettingsFile);
  void LoadSettings();

  void SlotFilterenable(bool OnOff);
  bool isAimingFault();

  BlockCounterClass PassCounter = BlockCounterClass(200,true);

	PortAdapter<AimingClass> PortSetAiming;
	PortAdapter<AimingClass> PortMoveAiming;
	PortAdapter<AimingClass> PortCalibration;
  SignalPortAdapter PortSignalSetAiming{(PassTwoCoordClass*)&PortSetAiming};

	PIDParamStruct AimingSlowParam;
	PIDParamStruct AimingWorkSlowParam;
	PIDParamStruct AimingFastParam;
  TimeIntegratorClass Integrator;

	PIDClass ModulePID;
	//KalmanFilterClass Filter;

  AimingParamOptimizator AimingOptimizator;
  Statistic AimingStatistic;
  Statistic FaultStatistic;

  std::vector<PIDParamStruct> PIDParamTable;
  int PIDParamNumber = 0;

  //DESIERED COORDS THAT GET FROM CAMERA OR SET MANUAL
	QPair<double, double> CoordNullPosition {0.0,0.0}; 
	QPair<double, double> CoordAim          {0.0,0.0}; 
  QPair<double, double> CoordAimCorrection{0.0,0.0}; 
	QPair<double, double> CoordBeamPos      {0.0,0.0}; 

  //PROCESSING COORDS DURING AIMING
	QPair<double, double> CoordAimingError    {0.0,0.0};
	QPair<double, double> VectorVelocityOutput{0.0,0.0};
	QPair<double, double> VectorAccelOutput   {0.0,0.0};
	QPair<double, double> VectorOutput        {0.0,0.0};
	//==========================================

	TransformCoordClass PixToRadian;
	TransformCoordClass RadianToPix;
	TransformCoordClass Saturation{1,0,400,400};
};

#endif //AIMINGCLASS_H
