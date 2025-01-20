#include "debug_output_filter.h"
#include "scanator_control_class.h"
using namespace std;
#define TAG "[ ENGINE CTRL ]" 

static OutputFilter OutputFilter2;

ScanatorControlClass::~ScanatorControlClass() { qDebug() << TAG << "DELETE"; }

ScanatorControlClass::ScanatorControlClass(QObject* parrent) : QObject(parrent)
{
  EnginePort = new UDPEngineInterface("192.168.0.178",2323); 
  EnginePort->BindTo(QHostAddress::Any,2323);

  AngleToDAC = TransformCoordClass(SettingsRegister::GetValue("GAIN_ANGLE_DAC"),
                                   SettingsRegister::GetValue("DAC_OFFSET"));
  qDebug() << "GAIN ANGLE DAC: " << SettingsRegister::GetValue("GAIN_ANGLE_DAC");
  qDebug() << "DAC OFFSET    : " << SettingsRegister::GetValue("DAC_OFFSET");

  SlotMoveToPos(this->NullRelativePos);

  PortMoveVelocity | *this; // LINK INTEGRATOR TO SlotMoveToPos, EMULATE MOVE WITH VELOCTIY
}

bool ScanatorControlClass::isAtLimit() { return false; }

const QPair<double, double>& ScanatorControlClass::GetOutput() { return ScanatorPos; }

void ScanatorControlClass::SetInput(const QPair<double,double>& Coord) { this->SlotMoveToPos(Coord); }

void ScanatorControlClass::SetToNullReal() { SlotMoveToPos(NullAbsolutePos); }
void ScanatorControlClass::SetToNullWork() { SlotMoveToPos(NullRelativePos); }

void ScanatorControlClass::SetModuleEnabled(bool OnOff)
{
  if( OnOff) StateBlock = StateBlockAtWork;
  if(!OnOff) StateBlock = StateBlockDisabled;
  qDebug() << TAG_NAME << "ENABLED: " << OnOff;
}

void ScanatorControlClass::SlotMoveVector(const QPair<double,double>& Pos, 
                                          const QPair<double,double>& Vel, 
                                          const QPair<double,double>& Accel, 
                                          const QPair<double,double>& RelPos)
  {

  ScanatorPos   = Pos; 
  ScanatorPos   = Vel; 
  ScanatorVel   = Accel; 
  ScanatorError = RelPos;

  CommandChannel1->Position     = ScanatorPos.first    * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel2->Position     = ScanatorPos.second   * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel1->Velocity     = ScanatorVel.first    * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel2->Velocity     = ScanatorVel.second   * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel1->Acceleration = ScanatorAccel.first  * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel2->Acceleration = ScanatorAccel.second * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel1->PositionRel  = ScanatorError.first  * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel2->PositionRel  = ScanatorError.second * AngleToDAC.Scale + AngleToDAC.Offset;

  EnginePort->SlotSendCommand(CommandScanator.toByteArray()); 
}


void ScanatorControlClass::SlotMoveToPos(const QPair<double, double>& Pos)
{
    if(StateBlock != StateBlockAtWork) return;

    ScanatorPos = Pos; 
    CommandChannel1->ClearStruct(); CommandChannel2->ClearStruct();
    CommandChannel1->Position = ScanatorPos.first  * AngleToDAC.Scale + AngleToDAC.Offset;
    CommandChannel2->Position = ScanatorPos.second * AngleToDAC.Scale + AngleToDAC.Offset;

    if(EnginePort != 0)
       EnginePort->SlotSendCommand(CommandScanator.toByteArray()); 

}

void ScanatorControlClass::SlotMoveOnStep(const QPair<double, double>& StepVector)
{
  if(StateBlock != StateBlockAtWork) return;

  CommandChannel1->ClearStruct(); CommandChannel2->ClearStruct();
  ScanatorPos = ScanatorPos + StepVector; 
  if(ScanatorPos.first  < 0 ) ScanatorPos.first = 0;
  if(ScanatorPos.second < 0 ) ScanatorPos.second = 0;

  CommandChannel1->Position = ScanatorPos.first  * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel2->Position = ScanatorPos.second * AngleToDAC.Scale + AngleToDAC.Offset;

  if(EnginePort != 0)
     EnginePort->SlotSendCommand(CommandScanator.toByteArray()); 
}

void ScanatorControlClass::SlotMoveWithVelocity(const QPair<double, double>& VelVector)
{
  if(StateBlock != StateBlockAtWork) return;

  ScanatorVel = VelVector; 
  CommandChannel1->ClearStruct(); CommandChannel2->ClearStruct();
  CommandChannel1->Velocity = ScanatorVel.first  * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel2->Velocity = ScanatorVel.second * AngleToDAC.Scale + AngleToDAC.Offset;
  EnginePort->SlotSendCommand(CommandScanator.toByteArray()); 
}

void ScanatorControlClass::SlotMoveWithAcceleration(const QPair<double, double>& AccelVector)
{
  if(StateBlock != StateBlockAtWork) return;

  ScanatorAccel = AccelVector; 
  CommandChannel1->ClearStruct(); CommandChannel2->ClearStruct();
  CommandChannel1->Acceleration = ScanatorAccel.first  * AngleToDAC.Scale + AngleToDAC.Offset;
  CommandChannel2->Acceleration = ScanatorAccel.second * AngleToDAC.Scale + AngleToDAC.Offset;
  EnginePort->SlotSendCommand(CommandScanator.toByteArray()); 
}

void ScanatorControlClass::LoadSettings()
{
  auto RotateParamCamEng = SettingsRegister::GetString(QString("ROTATE_CAM_ENG"));
  auto RotateParamEngCam = SettingsRegister::GetString(QString("ROTATE_ENG_CAM"));

  this->RotEngineToCamera.LoadRotationFromFile(RotateParamEngCam);
}

int ScanatorControlClass::GetID() {return SettingsRegister::GetValue("BLOCK_ID_SCANATOR"); }; 