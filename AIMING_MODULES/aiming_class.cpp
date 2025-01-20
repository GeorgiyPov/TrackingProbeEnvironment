#include "aiming_class.h"
#include "debug_output_filter.h"
#include "state_block_enum.h"
#include <QTimer>
#include <QFile>

#define TAG "[   AIMING   ]" 

OutputFilter OutputFilter1;

AimingClass::~AimingClass() 
{ 
  qDebug() << TAG << "DELETE";
}


const QPair<double, double>& AimingClass::GetOutput() { return this->VectorOutput; }

AimingClass::AimingClass()
{
	//======================================================================

  PortSetAiming.LinkAdapter(this,&AimingClass::SetAimingPosition,&AimingClass::GetAimPosition);
  PortMoveAiming.LinkAdapter(this,&AimingClass::MoveAimingPosition,&AimingClass::GetAimPosition);
  PortCalibration.LinkAdapter(this,&AimingClass::MoveAimingPosition,&AimingClass::GetBeamPosition);

	this->AimingSlowParam.Common = 1;
	this->AimingSlowParam.RateParam = 10.0;
	this->AimingSlowParam.IntParam  = 0.0002;
	this->AimingSlowParam.DiffParam = 0.0002;

	this->AimingWorkSlowParam.Common = 1;
	this->AimingWorkSlowParam.RateParam = 10.0;
	this->AimingWorkSlowParam.IntParam  = 0.0002;
	this->AimingWorkSlowParam.DiffParam = 0.0002;

	this->AimingFastParam.Common = 1;           // for move by velocity one integrator plant
	this->AimingFastParam.RateParam = 0.45;
	this->AimingFastParam.IntParam  = 0.60;
	this->AimingFastParam.DiffParam = 0.0164;
	ModulePID.SetPIDParam(AimingSlowParam);
	//==================================================================

	ErrorRecord.assign(100,QPair<double,double>(0,0));
	CurrentRecord = ErrorRecord.begin();

	this->PixToRadian.TransformCoord = [this](QPair<double,double> CoordError)
							{
								PixToRadian.TransformedCoord.first = (CoordError.first * PixToRadian.Scale) * M_PI / (60.0 * 60.0 * 180.0);
								PixToRadian.TransformedCoord.second = (CoordError.second * PixToRadian.Scale) * M_PI / (60.0 * 60.0 * 180.0);
							};

	this->RadianToPix.TransformCoord = [this](QPair<double,double> CoordError)
							{
								RadianToPix.TransformedCoord.first = (CoordError.first*(60.0*60.0*180.0)/M_PI)/PixToRadian.Scale;
								RadianToPix.TransformedCoord.second = (CoordError.second*(60.0*60.0*180.0)/M_PI)/PixToRadian.Scale;
							};

	//Filter.enableFiltering(false);
}


bool AimingClass::isAimingFault()
{
  return StateBlock == StateBlockDisabled || StateBlock == StateBlockFault || StateBlock == StateBlockBroken;
}

double AimingClass::GetAbsError() { return std::hypot(CoordAimingError.first, CoordAimingError.second); }


void AimingClass::SetPIDParamFromTable(int Number)
{
  if(Number > PIDParamTable.size() - 1) return;
  if(PIDParamNumber == Number) return;
  SetPIDParam(PIDParamTable[Number]); PIDParamNumber = Number;
}
void AimingClass::SetPIDParam(PIDParamStruct param)
{
  auto CurrentParam = param;
if (AimingState == AimingSlow) AimingSlowParam = param;
if (AimingState == AimingFast) AimingFastParam = param;

ModulePID.SetPIDParam(CurrentParam);

}

void AimingClass::SetAimingSpeedRegim(TypeEnumAiming Aiming)
{
	AimingState = Aiming;
	if (AimingState == AimingSlow) ModulePID.SetPIDParam(AimingSlowParam);
  if (AimingState == AimingFast) ModulePID.SetPIDParam(AimingFastParam);
  if (AimingState == AimingTuning) ModulePID.SetPIDParam(PIDParamTable[0]);
}

const QPair<double, double>& AimingClass::GetAimPosition()  { return CoordAim; }
const QPair<double, double>& AimingClass::GetBeamPosition() { return CoordBeamPos; }
const QPair<double, double>& AimingClass::GetAimingError()  { return CoordAimingError; }

void AimingClass::SetAimingPosition(QPair<double, double> Coord)          { CoordAim = Coord; qDebug() << TAG_NAME << "SET AIM: " << Coord;}
void AimingClass::SetAimingCorrection(QPair<double, double> Coord)        { CoordAimCorrection = Coord; }
void AimingClass::MoveAimingPosition(QPair<double, double> RelativeCoord) { CoordAim = CoordAim + RelativeCoord; }

void AimingClass::Reset()
{
  qDebug() << TAG << "RESET AIMING";
  this->SetModuleEnabled(false);
  this->ModulePID.ResetPID();
  this->VectorOutput = QPair<double,double>(0,0);
  this->SetAimingSpeedRegim(AimingSlow);
  AimingStatistic.Reset();
  FaultStatistic.Reset();
  QTimer::singleShot(8000,[this](){this->SetModuleEnabled(true);}); 
}

void AimingClass::SetModuleEnabled(bool OnOff) 
{ 
  if( OnOff) StateBlock = StateBlockAtWork; 
  if(!OnOff) StateBlock = StateBlockDisabled; 
  qDebug() << TAG_NAME << "BLOCK ENABLED: " << OnOff;
};

void AimingClass::SetInput(const QPair<double,double>& Coord)
{

  if (this->StateBlock == StateBlockDisabled) { VectorOutput = Coord; return; }

     Coord >> Substract;
  CoordAim >> Substract >> CoordAimingError >> ModulePID >> Integrator >> Saturation >> VectorOutput;
  VectorOutput.second = -VectorOutput.second;

        
  auto Error = Coord - CoordAim;
  qDebug() << OutputFilter1(50) << TAG_NAME << "ERROR: " << Error.first << Error.second << "CONTORL: " << VectorOutput.first << VectorOutput.second;

  //if(AimingState == AimingTuning) CoordAimingError >> AimingOptimizator; SetPIDParamFromTable(AimingOptimizator.BestPIDParamNumber); 
   
}

void AimingClass::SlotFilterenable(bool OnOff) 
{ 
  //Filter.enableFiltering(OnOff); 
}


void AimingClass::LoadPIDParam(QString SettingsFile)
{
    QFile file(SettingsFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;


    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();

        if(line.contains("==")) continue;

        QStringList list_preference = line.split(" ");

        if(list_preference.size() < 3) continue;

        double Common = 1;
        double rate_param = list_preference.at(0).toDouble();
        double int_param = list_preference.at(1).toDouble();
        double diff_param = list_preference.at(2).toDouble();

        this->PIDParamTable.push_back(PIDParamStruct(rate_param,diff_param,int_param));

    }

    AimingOptimizator = AimingParamOptimizator(PIDParamTable);

    file.close();
}

void AimingParamOptimizator::operator=(const AimingParamOptimizator& Optimizator)
{
  BestPIDParamNumber = Optimizator.BestPIDParamNumber;
  PIDParamTable = Optimizator.PIDParamTable;
  PIDParamGroupStat = Optimizator.PIDParamGroupStat;
  CurrentStatistic = Optimizator.CurrentStatistic;
  BestPIDParam = Optimizator.BestPIDParam;

  int LimitDispersionCounter = Optimizator.LimitDispersionCounter;
  bool FLAG_CONTROL_PARAM_REGIM = Optimizator.FLAG_CONTROL_PARAM_REGIM;
  bool FLAG_REGISTRATION_REGIM = Optimizator.FLAG_REGISTRATION_REGIM;
}

void AimingClass::LoadSettings()
{
  auto PIDParamFile = SettingsRegister::GetString("PID_PARAM");
  auto BeamPosFile = SettingsRegister::GetString("AIMING_BEAM_POS");

  QSettings BeamPosSettings(BeamPosFile, QSettings::IniFormat);
  BeamPosSettings.beginGroup("BEAMS");
  int XPointerPos = BeamPosSettings.value("XPointer").toInt();
  int YPointerPos = BeamPosSettings.value("XPointer").toInt();

  BeamPosSettings.endGroup();

  auto PointerCenteredCoord = QPair<double,double>(XPointerPos,YPointerPos);
  
  this->CoordAim = this->CoordNullPosition;

  this->LoadPIDParam(PIDParamFile);
}

bool AimingParamOptimizator::isAimingFaultStatistic()
{
   auto& Error =  CurrentStatistic.DispersionCoordDistance; 
   if(Error > 3) { LimitDispersionCounter++; CurrentStatistic.Reset();} 

   if(LimitDispersionCounter >= 10) { LimitDispersionCounter = 0; return true; }

   return false;
}

void AimingParamOptimizator::SetInput(const QPair<double,double>& Coord)
{

  Coord >> CurrentStatistic;
  if(FLAG_CONTROL_PARAM_REGIM && isAimingFaultStatistic()) 
  {
      BestPIDParamNumber--; if(BestPIDParamNumber < 0) BestPIDParamNumber = 0;
      BestPIDParam = PIDParamTable[BestPIDParamNumber];
  };

  if(!FLAG_REGISTRATION_REGIM) return;

                  Coord >> PIDParamGroupStat; 
   BestPIDParamNumber = PIDParamGroupStat.BestStatNumber;
            BestPIDParam = PIDParamTable[BestPIDParamNumber];

  if(PIDParamGroupStat.IsStatisticsLoaded()) 
  {
                           PIDParamGroupStat.PerformAvailableData();
   FLAG_REGISTRATION_REGIM = false;
   FLAG_CONTROL_PARAM_REGIM = true;
   qDebug() << "FOUND BEST PID PARAM NUMBER: " << BestPIDParamNumber << BestPIDParam.RateParam;
  }
}

void AimingParamOptimizator::AppendPIDParam(PIDParamStruct PIDParam)
{
   PIDParamTable.push_back(PIDParam);
}


AimingParamOptimizator::AimingParamOptimizator(std::vector<PIDParamStruct> ParamTable)
{
   this->PIDParamTable = ParamTable;
   if(PIDParamTable.size() != 0) BestPIDParam = PIDParamTable[BestPIDParamNumber];
   PIDParamGroupStat = StatisticGroup(PIDParamTable.size(),1500);

    double dispersion_threshold = 1.6;
    PIDParamGroupStat.FindBestStatisticCoord = [dispersion_threshold](std::map<int,Statistic>::iterator BeginStatistic,
                                                                      std::map<int,Statistic>::iterator endStatistic) -> int
    {
        int min_pos = 1;
        auto current_stat = BeginStatistic;
        std::vector<float> dispersions;

        while(current_stat != endStatistic) { dispersions.push_back(current_stat->second.AmplitudeCoordDeviation); current_stat++;};
        std::reverse(dispersions.begin(), dispersions.end());

        auto compare_to_threshold = [dispersion_threshold](double i){ return i < dispersion_threshold; };
        auto min_element = std::find_if(dispersions.begin(), dispersions.end(),compare_to_threshold);
             min_pos = std::distance(dispersions.begin(),min_element);
             min_pos = dispersions.size() - min_pos;

             //qDebug() << "FOUND BEST PID WITH ERROR AMPLITUDE  - " << *min_element  
             //                                                      << " NUMBER - " << min_pos  
             //                                                      << " SIZE - " << dispersions.size();

        return min_pos-1;
    };

}


AimingParamOptimizator::AimingParamOptimizator()
{
}
void AimingParamOptimizator::Reset()
{
  BestPIDParam = PIDParamTable[0];
  PIDParamGroupStat.Reset();
  BestPIDParamNumber = 0;

  FLAG_CONTROL_PARAM_REGIM = false;
  FLAG_REGISTRATION_REGIM = true;
}

//std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint = std::chrono::high_resolution_clock::now();
//std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint2 = std::chrono::high_resolution_clock::now();
//auto StepPeriod = std::chrono::duration<double>((TimePoint2 - TimePoint)).count();
//*CurrentRecord = CoordAimingError; CurrentRecord++; if(CurrentRecord == ErrorRecord.end()) CurrentRecord = ErrorRecord.begin();
