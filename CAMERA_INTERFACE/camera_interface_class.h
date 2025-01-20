#pragma once
#include <qnamespace.h>
#include <iostream>
#include <typeinfo>
#include <QObject>
#include <queue>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QImage>
#include "interface_image_source.h"
#include <QTimer>


#undef signals
extern "C" 
{ 
    //#include <arv.h> 
	#include <stdlib.h>
}
#define signals Q_SIGNALS

class CameraInterfaceClassAravis;
struct ArvStreamCallbackData
{
	//ArvStream *stream;
	int counter;
	//gboolean done;
    CameraInterfaceClassAravis* Receiver;

};

struct CameraRectRegion
{
    int x;
    int y;
    int width;
    int heigh;

};
class CameraControlInterface
{
    public:
    virtual int StartCameraStream() = 0;
    virtual int StopCameraStream() = 0;
    virtual void SlotSetCameraRegion(int x, int y, int width, int height ) = 0;
};

class CameraInterfaceClassAravis: public ImageSourceInterface, public CameraControlInterface
{
    Q_OBJECT
    public:
    CameraInterfaceClassAravis(QObject* parent = 0);
    ~CameraInterfaceClassAravis();
	//GError *error = NULL;
    QString TAG_NAME{"[ CAMERA ]"};
    QString CAMERA_INFO{"[ CAMERA NO DATA ]"};

    CameraRectRegion CameraRegion;
    std::pair<int,int> SizeImage{400,400}; // MAY BE CHANGED IN INIT
    std::pair<int,int> ImagePos{20,20}; // MAY BE CHANGED IN INIT

    void GetCurrentCameraRegion();

    int  StartCameraStream();
    int  StopCameraStream();
    void SlotSetCameraRegion(int x, int y, int width, int height );
	int GetID(); 

    int  InitCamera();
    void DeinitCamera();
    //void PutNewFrameToStorage(ArvBuffer* buffer);
    QTimer timerDisplayTestImage;

    cv::Mat ImageToProcess;
     QImage ImageToDisplay;

    QImage& GetImageToDisplay();
    cv::Mat& GetImageToProcess();

    void GetImageToDisplay(QImage& ImageDst) { ImageDst = ImageToDisplay.copy();};
    void GetImageToProcess(cv::Mat& ImageDst) { ImageDst = ImageToProcess.clone();};

    const std::vector<QPair<int,int>>& GetPoints();  
    const std::vector<QRect>& GetRects();  
    const QString& GetInfo();  

    std::vector<QPair<int,int>> CameraPoints{2};
    std::vector<QRect>          CameraRects{2};


	//ArvCamera *camera;
    std::vector<uint8_t*> Buffers;
    std::vector<uint8_t*>::iterator CurrentBuffer;

    size_t payload;

	ArvStreamCallbackData callback_data;

    public slots:
    void SlotDisplayProcessImage();
};
