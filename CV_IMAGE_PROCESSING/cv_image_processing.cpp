#include "cv_image_processing.h"
#include "AIM_IMAGE_IMITATION/imitator_image_aim.h"
#include "interface_pass_coord.h"
#include <memory>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <qnamespace.h>
#include <chrono>
#include <debug_output_filter.h>
#include <QThread>
#include "register_settings.h"
OutputFilter Filter40{40};

bool operator==(std::pair<int,int> pair, std::pair<int,int> pair2) { return pair.first == pair2.first && pair.second == pair2.second; };
bool operator==(std::pair<int,int> pair, int value) { return pair.first == value && pair.second == value; };
bool operator!=(std::pair<int,int> pair, std::pair<int,int> pair2) { return pair.first != pair2.first && pair.second != pair2.second; };
bool operator!=(std::pair<int,int> pair, int value) { return pair.first != value && pair.second != value; };

CVImageProcessing::CVImageProcessing(QObject* parent) : ImageSourceInterface(parent)
{
  qRegisterMetaType<const cv::Mat&>();
  //if(QFile::exists("D:/DATA/FindingTemplate.png"))
  //ImageTemplate = imread("D:/DATA/FindingTemplate.png",cv::IMREAD_GRAYSCALE);

  auto ImageFilePath = QString("%1/DATA/FindingTemplate.png").arg(SettingsRegister::GetString("DATA_DIR"));
  if(QFile::exists(ImageFilePath))
  ImageTemplate = imread(ImageFilePath.toStdString(),cv::IMREAD_GRAYSCALE);

  TemplateRect = cv::Size(ImageTemplate.cols,ImageTemplate.rows);
  
  qDebug() << "LOAD TEMPLATE: SIZE: " << TemplateRect.width << TemplateRect.height;

  Trajectory.resize(30,std::make_pair(0,0));
  TrajectoryIncrement.resize(30,std::make_pair(0,0));
  TrajectoryIncrementAvarage.resize(30,std::make_pair(0,0));
  //std::cout << cv::getBuildInformation() << std::endl;

  QObject::connect(&timerDisplay,SIGNAL(timeout()),this, SLOT(SlotDisplayProcessingImage()));
}

CVImageProcessing::~CVImageProcessing()
{
 qDebug() << "STOP AND DEINIT CAMEA";
}


void CVImageProcessing::FindImageCentroid(cv::Mat& Image)
{

    //cv::medianBlur(Image,ImageProcessing,7);
    //cv::threshold(Image,ImageProcessing,Threshold,256,cv::THRESH_BINARY);
		double sum_x = 0.0, sum_y = 0.0, sum = 0.0;

		for (int row = 0; row < Image.rows; row++)
		{
			uint8_t* RowData = Image.ptr<uint8_t>(row);
			for (int col = 0; col < Image.cols; col++)
			{
				  if (RowData[col] < Threshold) { RowData[col] = 0; continue; } RowData[col] = 255;

					sum += RowData[col];
					sum_x += RowData[col] * col;
					sum_y += RowData[col] * row;
			}
		}

		if (sum == 0.0) return;

    this->PosAim.first = sum_x / sum; // get spot center coord
    this->PosAim.second = sum_y / sum;
}

const std::vector<QPair<int,int>>& CVImageProcessing::GetPoints()  
{
   std::copy(Trajectory.begin(), Trajectory.end(),ImagePoints.begin());
   ImagePoints[0] = PosAim;
   return ImagePoints;
}
const std::vector<QRect>& CVImageProcessing::GetRects()  
{
   ImageRects[0].setRect(ROIAim.x, ROIAim.y, ROIAim.width, ROIAim.height);
   ImageRects[1].setRect(PosAim.first-20, PosAim.second-20, 40, 40);
   return ImageRects;
}

const QString& CVImageProcessing::GetInfo()  { return ProcessInfo; }

cv::Mat& CVImageProcessing::GetImageToProcess() { return ImageProcessing; }

QImage& CVImageProcessing::GetImageToDisplay()
{
    MutexImageAccess.lock();
    ImageToDisplay = QImage(ImageProcessing.data,   //GRAY IMAGE
                            ImageProcessing.cols,
                            ImageProcessing.rows,
           static_cast<int>(ImageProcessing.step),
              QImage::Format_Grayscale8 );

    MutexImageAccess.unlock();
    return ImageToDisplay;
}

void CVImageProcessing::GetImageToProcess(cv::Mat& ImageDst)
{
    MutexImageAccess.lock();
    ImageDst = ImageProcessing.clone();
    MutexImageAccess.unlock();
}

void CVImageProcessing::GetImageToDisplay(QImage& ImageDst)
{
  if(ImageDst.width() != ImageProcessing.cols || ImageDst.height() != ImageProcessing.rows) 
     ImageDst = QImage(ImageProcessing.cols,ImageProcessing.rows,QImage::Format_ARGB32);

    MutexImageAccess.lock();
    uint8_t* RowData = 0;
    QRgb*    ImageDest = 0;
		for (int row = 0; row < this->ImageProcessing.rows; row++)
		{
			RowData = this->ImageProcessing.ptr<uint8_t>(row);
      ImageDest = (QRgb *)ImageDst.scanLine(row);

			for (int col = 0; col < this->ImageProcessing.cols; col++)
			{
          ImageDest++; *ImageDest = qRgb(RowData[col], RowData[col], RowData[col]);
			}
		}

    MutexImageAccess.unlock();
}

void CVImageProcessing::FindImageTemplate(cv::Mat& Image)
{
  if(ImageTemplate.empty()) qDebug() << "[IMG PROC]: TEMPLATE EMPTY";

  ProcessTimePoint = std::chrono::high_resolution_clock::now();

  int result_cols = Image.cols - ImageTemplate.cols + 1;
  int result_rows = Image.rows - ImageTemplate.rows + 1;

  ProcessResult.create( result_rows, result_cols, CV_32FC1 );

  //==========================================================================================================
  MutexImageAccess.lock();
                                  ImageInput = Image; 
                                  ImageProcessing = Image.clone();
    if(Trajectory.front() != 0) { ImageProcessing = ImageProcessing(ROIAim); FLAG_FAST_MATCHING = true; }

    FilterBlotch.FilterImage(ImageProcessing, ImageProcessing); FLAG_DISPLAY_IMAGE = true;
  MutexImageAccess.unlock();
  //==========================================================================================================

  cv::matchTemplate( ImageProcessing, ImageTemplate, ProcessResult, cv::TM_SQDIFF); 

  cv::normalize( ProcessResult, ProcessResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );
  double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;

  cv::minMaxLoc( ProcessResult, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

  if(FLAG_FAST_MATCHING) { PosAim.first = minLoc.x + ROIAim.x; PosAim.second = minLoc.y + ROIAim.y; }
  else { PosAim.first = minLoc.x; PosAim.second = minLoc.y; }  

  auto Dur = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - ProcessTimePoint);

  TrajectoryIncrement.push_back(std::make_pair<float,float>(float(PosAim.first - Trajectory.back().first),
                                               float(PosAim.second - Trajectory.back().second)));
  auto Vel = TrajectoryIncrement.back();
 
  VelAimAvarage = std::accumulate(TrajectoryIncrement.end() - 10, 
                                 TrajectoryIncrement.end(),
                                 std::make_pair<float,float>(0,0),
                                 []( std::pair<float,float> acc, const std::pair<float,float>& val) mutable -> std::pair<float,float>
                                                                                      { acc.first += val.first/10.0;
                                                                                        acc.second += val.second/10.0;
                                                                                      return acc;}
                                 );
  qDebug() << "PROCESS DURATION - " << Dur.count();

  ROIAim = cv::Rect(PosAim.first + VelAimAvarage.first - 10, PosAim.second + VelAimAvarage.second - 20, 80, 80);

  if(!IsROIValid(ROIAim)) CheckCorrectROI(ROIAim);

  Trajectory.erase(Trajectory.begin());
  Trajectory.push_back(std::make_pair(PosAim.first,PosAim.second));

  TrajectoryIncrement.erase(TrajectoryIncrement.begin());
  //TrajectoryIncrementAvarage.push_back(VelAimAvarage);
  //TrajectoryIncrementAvarage.erase(TrajectoryIncrementAvarage.begin());
}

void CVImageProcessing::SlotProcessImage(const cv::Mat& Image)
{
    ImageProcessing = Image.clone(); ImageInput = ImageProcessing;
    //FindImageTemplate(ImageInput); PutImageToDisplay(ImageInput); ROIAim = ROIAim*1.2;

    FindImageCentroid(ImageInput); ContoursProcessor.SetImage(ImageInput); ROIAim = ContoursProcessor.GetMaxContourRect(); 
    ROIAim = ROIAim*1.2;
    emit ImageSourceInterface::SignalNewImage();

    PassTwoCoordClass::PassCoord(); // PASS COORD TO LINKED RECEIVER
}

void CVImageProcessing::SlotProcessImage()
{
  MutexImageAccess.lock();
  ImageInput = SourceImage->GetImageToProcess().clone(); ImageProcessing = ImageInput;
  MutexImageAccess.unlock();

  //FindImageTemplate(ImageInput); PutImageToDisplay(ImageInput); ROIAim = ROIAim*1.2;
  FindImageCentroid(ImageInput); 
  ContoursProcessor.SetImage(ImageInput); ROIAim = ContoursProcessor.GetMaxContourRect(); 
  ROIAim = ROIAim*2.0;
  emit ImageSourceInterface::SignalNewImage();

  PassTwoCoordClass::PassCoord(); // PASS COORD TO LINKED RECEIVER
}


void CVImageProcessing::LinkToModule(std::shared_ptr<ImageSourceInterface> Source)
{
                    SourceImage = Source;
   QObject::connect(SourceImage.get(),SIGNAL(SignalNewImage()),this,SLOT(SlotProcessImage()), Qt::QueuedConnection);
}

std::shared_ptr<CVImageProcessing> operator|(std::shared_ptr<ImageSourceInterface> Source, std::shared_ptr<CVImageProcessing> Rec)
{
    Rec->LinkToModule(Source); return Rec;
}


void CVImageProcessing::CalcVelocity() { }

bool CVImageProcessing::IsROIValid(cv::Rect& ROI)
{
const int& ImageWidth = ImageInput.cols;
const int& ImageHeight = ImageInput.rows;

if((ROI.x + ROI.width + 1) > ImageWidth) return false;
if((ROI.y + ROI.height + 1) > ImageHeight) return false;
if (ROI.x < 0) return false;
if (ROI.y < 0) return false;

return true;
}

void CVImageProcessing::CheckCorrectROI(cv::Rect& ROI)
{
const int& ImageWidth = ImageInput.cols;
const int& ImageHeight = ImageInput.rows;

const int OFFSET_X = ROI.x + ROI.width - ImageWidth + 1;
const int OFFSET_Y = ROI.y + ROI.height - ImageHeight + 1;
//qDebug() << "CHECK   ROI: " << ROI.x << ROI.y << ROI.width << ROI.height;

if (OFFSET_X > 0) ROI.x = ImageWidth - ROI.width - 1;
if (OFFSET_Y > 0) ROI.y = ImageHeight - ROI.height - 1;

if (ROI.x < 0) ROI.x = 0;
if (ROI.y < 0) ROI.y = 0;
//qDebug() << "CORRECT ROI: " << ROI.x << ROI.y << ROI.width << ROI.height << " OFFSET_X: " << OFFSET_X << " OFFSET_Y: " << OFFSET_Y ;

}




void CVImageProcessing::SlotDisplayProcessingImage()
{
   //if(FLAG_DISPLAY_IMAGE)
   //cv::imshow("PROCESSING IMAGE",ImageProcessing);
   //cv::imshow("PROCESSING IMAGE",ImageToDisplay);

   //FLAG_DISPLAY_IMAGE = false;
}

cv::Rect& operator*(cv::Rect& rect, float Scale)
{
  rect.x -= (rect.width*Scale - rect.width)/2;
  rect.y -= (rect.height*Scale - rect.height)/2;
  rect.width *= Scale;
  rect.height *= Scale;
  return rect;
}

cv::Rect& operator*(float Scale, cv::Rect& rect)
{
   return rect*Scale;
}


     //auto max_element_it = std::max_element(ContourAreas.begin(), ContourAreas.end());
     //NumberMaxContour = std::distance(ContourAreas.begin(), max_element_it);
     //qDebug() << "HIERARCHY";
     //qDebug() << "=======================================";
     //qDebug() << "NUMBER: " << n++ << "NEXT: " << ContInfo[0] 
     //                              << "PREV: " << ContInfo[1]  
     //                              << "FIRST:" << ContInfo[2]
     //                              << "PARenT:" << ContInfo[3];
     //qDebug() << "=======================================";
//=====================================

void FindContoursProcessor::SetImage(const cv::Mat& Image)
{
	Contours.clear(); Hierarchy.clear();

	cv::findContours(Image,Contours,Hierarchy,cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);

     ContourRects.clear();
     ContourAreas.clear();
	 if(Contours.size() <= 1) return;

     int n = 0;
     int LastContMaxArea = 0;
     for(auto& ContInfo: Hierarchy)
     {
      if(ContInfo[3] != -1) { n++; continue; }
      ContourRects.push_back(cv::boundingRect(Contours[n]));
      ContourAreas.push_back(cv::contourArea(Contours[n]));

      if(ContourAreas.back() > LastContMaxArea) { LastContMaxArea = ContourAreas.back(); 
                                                  NumberMaxContour = ContourRects.size()-1; }
      n++;
     }
     //qDebug() << Filter40 << "MAX NUMBER CONT: " << NumberMaxContour << "SIZE: " << ContourRects.size();
}

cv::Rect FindContoursProcessor::GetMaxContourRect()
{
  if(ContourRects.size() <= NumberMaxContour) return cv::Rect(0,0,30,30);
  return ContourRects[NumberMaxContour];
}

void FindContoursProcessor::ClearBlotchRects(cv::Mat& OutputImage)
{
	if(Contours.size() <= 1) return;
    for(int n = 0; n < ContourRects.size(); n++) 
    {
      if(n == NumberMaxContour) continue;

      auto& rect = ContourRects[n]; CheckROI(rect, OutputImage.cols-1);

      OutputImage(rect) = cv::Scalar(0,0,0);
    }
}
void FindContoursProcessor::PrintContoursRect(cv::Mat& OutputImage)
{
	if(Contours.size() <= 1) return;
    for(int n = 0; n < ContourRects.size(); n++) 
    {
      if(n == NumberMaxContour) continue;

      auto& rect = ContourRects[n]*1.2; CheckROI(rect, OutputImage.cols-1);

      //cv::ellipse(OutputImage,cv::RotatedRect(cv::Point2f(rect.x + rect.width/2, rect.y + rect.height/2),
      //										cv::Size2f(rect.width, rect.height), 0.0),
      //										cv::Scalar(600,600,600),1);

      cv::rectangle(OutputImage,rect,cv::Scalar(600,600,600),1);
    }

}

bool FindContoursProcessor::isROIValid(const cv::Rect& ROI, int SizeImage)
{
	int OFFSET_X = ROI.x + ROI.width - 255;
	int OFFSET_Y = ROI.y + ROI.height - 255;

	if (OFFSET_X > 0 || OFFSET_Y > 0)  return false;
	if (ROI.x    < 1 || ROI.y    < 1)  return false;

    return true;
}

void FindContoursProcessor::CheckROI(cv::Rect& ROI, int SizeImage)
{
	if(isROIValid(ROI, SizeImage)) return;

	int OFFSET_X = ROI.x + ROI.width - SizeImage;
	int OFFSET_Y = ROI.y + ROI.height - SizeImage;

	if (OFFSET_X > 0) ROI.x -= OFFSET_X;
	if (OFFSET_Y > 0) ROI.y -= OFFSET_Y;

	if (ROI.x < 1) ROI.x = 1;
	if (ROI.y < 1) ROI.y = 1;

}

void FilterBlotchClass::FilterImage(cv::Mat& FilteringImage, cv::Mat& RawImage)
{
	try 
	{ 
		FindContours(RawImage); 
		FindContours.ClearBlotchRects(FilteringImage); 
        //FindContours.PrintContoursRect(FilteringImage);
	}
	catch (const cv::Exception& cv_ec) 
	{ 
		if(cv_ec.code == cv::Error::StsAssert) { qDebug() << Qt::endl << TAG_NAME << " ASSREST FAILED OPENCV"; return;}
		if(cv_ec.code == cv::Error::BadROISize) { qDebug() << Qt::endl << TAG_NAME << " BAD ROI OPENCV"; return;}
          
		qDebug() << Qt::endl << TAG_NAME << cv_ec.what() << cv_ec.code;	
	}

}


void CVImageProcessing::SetProcessingRegim(int Regim) { this->WorkRegim = Regim; }
void CVImageProcessing::StartStopProcessing(bool StartStop) { }
void CVImageProcessing::SetThreshold(int Thresh) {qDebug() << "THRES: " << Thresh; this->Threshold = Thresh; }

int CVImageProcessing::GetID() { return SettingsRegister::GetValue("BLOCK_ID_IMG_PROC");} 
//cv::Rect& operator*(float Scale, cv::Rect& rect)
//{
//return rect*Scale;
//}
//cv::Rect& operator*(cv::Rect& rect, float Scale)
//{
//	rect.x -= (rect.width*Scale - rect.width)/2;
//	rect.y -= (rect.height*Scale - rect.height)/2;
//	rect.width *= Scale;
//	rect.height *= Scale;
//	return rect;
//}

//sum >> StatProcessor;
//if(StatProcessor.IsValueLoaded() && sum < StatProcessor.AvarageValue/10) return;
//this->spot_coord_abs >> StatProcessor;

//this->x_pos_spot_in_strob = x_pos_spot_in_roi + roi_rect_in_strob.x;
//this->y_pos_spot_in_strob = y_pos_spot_in_roi + roi_rect_in_strob.y;
//this->PosAim.first = x_pos_spot_in_strob + strob_coord.first;
//this->PosAim.second = y_pos_spot_in_strob + strob_coord.second;

//this->roi_rect_in_strob.x = x_pos_spot_in_strob - roi_rect_in_strob.width / 2;
//this->roi_rect_in_strob.y = y_pos_spot_in_strob - roi_rect_in_strob.height / 2;
//CheckROIPos(roi_rect_in_strob);
//====================================================================
		//cv::Mat ROIImage = StrobImage(this->roi_rect_in_strob);
		//if (this->FlagThresholdAutoControl) Threshold = CalcThresholdlByLuminosity(ROIImage);
		//ROIImageFiltered = ROIImage;
		//cv::medianBlur(ROIImage, this->ROIImageFiltered, 5);