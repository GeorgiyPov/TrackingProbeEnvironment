#include "debug_output_filter.h"
#include "rotation_find_executor.h"

using namespace std;
#define TAG "[ ROT_FIND ]" 

RotationFindProcessClass::RotationFindProcessClass(QObject* parrent)
{
  qDebug() << "[ CREATE ROTATION EXECUTOR EMPTY]";
}

RotationFindProcessClass::RotationFindProcessClass(std::shared_ptr<PassTwoCoordClass> Base, std::shared_ptr<PassTwoCoordClass> Rot,QObject* parrent) : QObject(parrent)
{
  BaseObject = Base;
	RotationObject = Rot;
}

void RotationFindProcessClass::SetRotatedModules(std::shared_ptr<PassTwoCoordClass> Base, std::shared_ptr<PassTwoCoordClass> Rot)
{
  BaseObject = Base;
	RotationObject = Rot;
}
void RotationFindProcessClass::StopProcess()
{
    timerStepper.stop();
    QObject::disconnect(&timerStepper, SIGNAL(timeout()), this, SLOT(SlotMakeStep()));
}
void RotationFindProcessClass::StartProcess()
{
    QObject::connect(&timerStepper, SIGNAL(timeout()), this, SLOT(SlotMakeStep()));
    timerStepper.start(200);
}

void RotationFindProcessClass::SlotStartRotationFind(bool StartStop)
{
    if(!BaseObject || !RotationObject) { qDebug() << TAG << "ROTATION OBJECTS IS NOT SET CANT START CALIBRATION"; return;};
    if(StartStop) StartProcess(); else StopProcess();
}

void RotationFindProcessClass::SlotMakeStep()
{
//	auto CoordBase = BaseObject->GetOutput();     
//	auto CoordRot  = RotationObject->GetOutput();
//
//	//CoordBase >> InputTransform >> CoordBase;  // DEFAULT NO TRANSFORM SCALE = 1;
//	//CoordRot >> OutputTransform >> CoordRot;  // DEFAULT NO TRANSFORM SCALE = 1;
//
//	auto& AimCoord = RotationContainer.MeasureFilter.GetWaitInputCoord();
//	BaseObject->SetInput(AimCoord);
//
//  CoordBase >> RotationContainer.MeasureFilter >> RotationContainer;
//  CoordRot >>  RotationContainer.MeasureFilter >> RotationContainer;
//  
//  if(RotationContainer.IsDataFull()) StopProcess();
//
//    qDebug() << TAG << "WAIT: " << AimCoord.first 
//                                << AimCoord.second 
//                                << RotationContainer.MeasureFilter.DataFilter.accumulate_counter;
//
//    qDebug() << TAG << " INPUT : " << CoordBase.first << CoordBase.second 
//	                  << " OUTPUT: " << CoordRot.first  << CoordRot.second;
//
}

