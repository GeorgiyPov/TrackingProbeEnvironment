#include "imitator_image_aim.h"
#include "register_settings.h"

std::pair<int,int> AimImageImitatorClass::SizeImage{420,420}; // MAY BE CHANGED IN INIT
std::pair<int,int> AimImageImitatorClass::SizeAimBoundry{420-40,420-40}; // MAY BE CHANGED IN INIT

AimImageImitatorClass::AimImageImitatorClass(QObject* parent) : ImageSourceInterface(parent)
{

//QString AimImagePath = "D:/DATA/UFO.png"; qDebug() << "LOAD IMAGE: " <<  AimImagePath;
//QString BlotchImagePath = "D:/DATA/blotch.png"; qDebug() << "LOAD IMAGE: " <<  BlotchImagePath;

QString DATA_DIR = SettingsRegister::GetString("DATA_DIR");
QString AimImagePath = QString("%1/UFO.png").arg(DATA_DIR);       qDebug() << "LOAD IMAGE: " <<  AimImagePath;
QString BlotchImagePath = QString("%1/blotch.png").arg(DATA_DIR); qDebug() << "LOAD IMAGE: " <<  BlotchImagePath;

ImageTestObject = cv::imread(AimImagePath.toStdString(),cv::IMREAD_GRAYSCALE );
BlotchObject = cv::imread(BlotchImagePath.toStdString(),cv::IMREAD_GRAYSCALE );

SizeImage = SettingsRegister::GetPair("CAMERA_IMAGE_SIZE");
SizeAimBoundry = std::pair<int,int>(SizeImage.first - 40, SizeImage.second - 40);

OutputImage = cv::Mat(SizeImage.first,SizeImage.second,CV_8UC1); OutputImage = cv::Scalar(0,0,0);

AIM_RECT = cv::Rect(300,300,ImageTestObject.cols,ImageTestObject.rows);

auto BLOTCH_RECT = AIM_RECT; BLOTCH_RECT.x += 30; BlotchObject.copyTo(OutputImage(BLOTCH_RECT)); 

ImageTestObject.copyTo(OutputImage(AIM_RECT));

DynamicControl.LinkToImageImitator(this);
 ManualControl.LinkToImageImitator(this);

ThinningTimePeriod = QTime::currentTime();

ImagePoints.resize(1);
ImageRects.resize(1);
ImagePoints[0] = QPair<double,double>(0,0);
ImageRects[0] = QRect(1,1,4,4);
}

void AimImageImitatorClass::GenerateAimImage()
{
    if(!IsROIValid(AIM_RECT)) qDebug() << "INVALID ROI: " << AIM_RECT.x << AIM_RECT.y << AIM_RECT.width << AIM_RECT.height;
    if(!IsROIValid(AIM_RECT)) CheckCorrectROI(AIM_RECT);

    auto BLOTCH_RECT = AIM_RECT; BLOTCH_RECT.x += 30;
    OutputImage = cv::Scalar(0);
    BlotchObject.copyTo(OutputImage(BLOTCH_RECT)); 
    ImageTestObject.copyTo(OutputImage(AIM_RECT)); // DRAW NEW AIM IMAGE

    ImageToDisplay = QImage(OutputImage.data,
                                OutputImage.cols,
                                OutputImage.rows,
               static_cast<int>(OutputImage.step),
                         QImage::Format_Grayscale8 );

    emit ImageSourceInterface::SignalNewImage(ImageToDisplay);

    if(ThinningTimePeriod.msecsTo(QTime::currentTime()) >= 20)
    {
    ImageToProcess = OutputImage.clone();
    ThinningTimePeriod = QTime::currentTime();
    emit ImageSourceInterface::SignalNewImage();
    }

}

void AimImageImitatorClass::GetImageToDisplayColor(QImage& ImageDst)
{
  if(ImageDst.width() != OutputImage.cols || ImageDst.height() != OutputImage.rows) 
     ImageDst = QImage(OutputImage.cols,OutputImage.rows,QImage::Format_ARGB32);

    //MutexImageAccess.lock();
    uint8_t* RowData = 0;
    QRgb*    ImageDest = 0;
		for (int row = 0; row < this->OutputImage.rows; row++)
		{
			RowData = this->OutputImage.ptr<uint8_t>(row);
      ImageDest = (QRgb *)ImageDst.scanLine(row);

			for (int col = 0; col < this->OutputImage.cols; col++)
			{
          ImageDest++; *ImageDest = qRgb(RowData[col], RowData[col], RowData[col]);
			}
		}

    //MutexImageAccess.unlock();
}

void AimImageImitatorClass::SlotMoveAimPos(int XStep, int YStep)
{
    OutputImage(AIM_RECT) = cv::Scalar(0,0,0); // CLEAR OLD AIM IMAGE

    AIM_RECT.x += XStep; AIM_RECT.y += YStep;
    if(AIM_RECT.x < 0)   AIM_RECT.x = 0;   if(AIM_RECT.y < 0)   AIM_RECT.y = 0;
    if(AIM_RECT.x > SizeAimBoundry.first-AIM_RECT.width) AIM_RECT.x = SizeAimBoundry.first-AIM_RECT.width; 
    if(AIM_RECT.y > SizeAimBoundry.second-AIM_RECT.height) AIM_RECT.y =SizeAimBoundry.second-AIM_RECT.height;

    GenerateAimImage();
}

void AimImageImitatorClass::SlotSetAimPos(int PosX, int PosY)
{
    OutputImage(AIM_RECT) = cv::Scalar(0,0,0); // CLEAR OLD AIM IMAGE
    AIM_RECT.x = PosX - AIM_RECT.width/2; AIM_RECT.y = PosY - AIM_RECT.height/2;
    if(AIM_RECT.x < 0)   AIM_RECT.x = 0;   if(AIM_RECT.y < 0)   AIM_RECT.y = 0;
    if(AIM_RECT.x > SizeAimBoundry.first-AIM_RECT.width) AIM_RECT.x = SizeAimBoundry.first-AIM_RECT.width; 
    if(AIM_RECT.y > SizeAimBoundry.second-AIM_RECT.height) AIM_RECT.y =SizeAimBoundry.second-AIM_RECT.height;

    GenerateAimImage();
}

QImage& AimImageImitatorClass::GetImageToDisplay() { return ImageToDisplay; }
cv::Mat& AimImageImitatorClass::GetImageToProcess() { return ImageToProcess; }
const QString& AimImageImitatorClass::GetInfo()  { return INFO_STRING; }

AimImageImitatorClass::~AimImageImitatorClass()
{
 qDebug() << "STOP AND DEINIT CAMERA";
}

const std::vector<QPair<int,int>>& AimImageImitatorClass::GetPoints()  
{
  ImagePoints[0].first = AIM_RECT.x + AIM_RECT.width/2; 
  ImagePoints[0].second = AIM_RECT.y + AIM_RECT.height/2; 
  return ImagePoints;
}
const std::vector<QRect>& AimImageImitatorClass::GetRects()  
{
   ImageRects[0].setRect(AIM_RECT.x, AIM_RECT.y,  AIM_RECT.width,  AIM_RECT.height );
   return ImageRects;
}


//==================================================================================
void AimControlInterface::StartMove(){ timerMoveAim.start(20); }
void AimControlInterface::StopMove() { timerMoveAim.stop(); }

void AimControlInterface::LinkToImageImitator(AimImageImitatorClass* ImitatorObjectPtr)
{
connect(&timerMoveAim, SIGNAL(timeout()), this ,SLOT(SlotMoveAim()));
ImageImitator = ImitatorObjectPtr;
}
//==================================================================================

void ManualAimControl::SlotMoveAim()
{
   if(Axis == 0) ImageImitator->SlotMoveAimPos(Step*Direction, 0); else ImageImitator->SlotMoveAimPos(0, Step*Direction);
}

void DynamicAimControl::SlotMoveAim()
{
    XNumberPos+=0.5; YNumberPos+=1; 
    if(XNumberPos == 360*AimSpeed) XNumberPos = 0; 
    if(YNumberPos == 360*AimSpeed) YNumberPos = 0; 
    auto& ImageSize = AimImageImitatorClass::SizeImage;
    auto& AimBoundry = AimImageImitatorClass::SizeAimBoundry;

    XCoordPos = (float)ImageSize.first/2  + (float)AimBoundry.first/4*sin(2*M_PI/360*XNumberPos/AimSpeed); 
    YCoordPos = (float)ImageSize.second/2 + (float)AimBoundry.second/4*sin(2*M_PI/360*YNumberPos/AimSpeed);

    ImageImitator->SlotSetAimPos(XCoordPos, YCoordPos);

}

void ManualAimControl::SlotStartMoveAim(int Dir, int MoveAxis, int MoveStep)
{
   Direction = Dir;
   Axis = MoveAxis;
   Step = MoveStep;
   this->StartMove();
}

//==================================================================================
bool AimImageImitatorClass::IsROIValid(cv::Rect& ROI)
{
const int& ImageWidth = OutputImage.cols;
const int& ImageHeight = OutputImage.rows;
//qDebug() << "CHECK ROI: " << ROI.x << ROI.y << ROI.width << ROI.height;

if((ROI.x + ROI.width + 1) > ImageWidth) return false;
if((ROI.y + ROI.height + 1) > ImageHeight) return false;
if (ROI.x < 0) return false;
if (ROI.y < 0) return false;

return true;
}

void AimImageImitatorClass::CheckCorrectROI(cv::Rect& ROI)
{
const int& ImageWidth = OutputImage.cols;
const int& ImageHeight = OutputImage.rows;

const int OFFSET_X = ROI.x + ROI.width - ImageWidth + 1;
const int OFFSET_Y = ROI.y + ROI.height - ImageHeight + 1;

if (OFFSET_X > 0) ROI.x = ImageWidth - ROI.width - 1;
if (OFFSET_Y > 0) ROI.y = ImageHeight - ROI.height - 1;

if (ROI.x < 0) ROI.x = 0;
if (ROI.y < 0) ROI.y = 0;

}

int AimImageImitatorClass::GetID() {return SettingsRegister::GetValue("BLOCK_ID_SCANATOR"); }; 