#include "camera_interface_class.h"
#include "register_settings.h"
#include <QDebug>

#include <chrono>
#include <semaphore>
#include <thread>
#include "register_settings.h"

std::counting_semaphore<2> bufferSemaphore{2};

CameraInterfaceClassAravis::CameraInterfaceClassAravis(QObject* parent): ImageSourceInterface(parent) 
{
   qDebug() << "CAMERA INTERFACE CREATE";

   InitCamera();
   //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   bufferSemaphore.acquire();
   bufferSemaphore.acquire();
   //CameraPoints.resize(2);
   //CameraRects.resize(2);

   QObject::connect(&timerDisplayTestImage,SIGNAL(timeout()),this, SLOT(SlotDisplayProcessImage()));
}

CameraInterfaceClassAravis::~CameraInterfaceClassAravis()
{
	this->StopCameraStream();
	if(Buffers.size())
	for(auto Buffer: Buffers) delete Buffer;
}

//extern "C" void stream_callback (void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer)
//{
//	ArvStreamCallbackData *callback_data = (ArvStreamCallbackData *) user_data;
//
//	// This code is called from the stream receiving thread
//
//	switch (type) 
//	{
//		case ARV_STREAM_CALLBACK_TYPE_INIT:
//			//Stream thread started.
//			//Here you may want to change the thread priority arv_make_thread_realtime() or
//			//arv_make_thread_high_priority() 
//			break;
//		case ARV_STREAM_CALLBACK_TYPE_START_BUFFER:
//			//The first packet of a new frame was received 
//			break;
//		case ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE:
//			 //We need to pull buffer, process, then push it back for reuse by the stream receiving thread 
//			g_assert (buffer != NULL);
//			callback_data->Receiver->PutNewFrameToStorage(buffer);
//
//			break;
//		case ARV_STREAM_CALLBACK_TYPE_EXIT:
//			break;
//	}
//}

//void CameraInterfaceClassAravis::PutNewFrameToStorage(ArvBuffer* buffer)
//{
//			if (arv_buffer_get_status(buffer) != ARV_BUFFER_STATUS_SUCCESS) return;
//
//	        buffer = arv_stream_pop_buffer(callback_data.stream);
//
//			//qDebug() << "ACQUIRED BUFFER : " << arv_buffer_get_image_width(buffer) << arv_buffer_get_image_height(buffer);
//
//			std::copy_n((uint8_t*)arv_buffer_get_data(buffer,&payload),payload,*CurrentBuffer);
//			CurrentBuffer++; if(CurrentBuffer == Buffers.end()) CurrentBuffer = Buffers.begin();
//			arv_stream_push_buffer(callback_data.stream, buffer);
//
//            ImageToProcess = cv::Mat(SizeImage.first,SizeImage.second,CV_8UC1,*CurrentBuffer); 
//			emit SignalNewImage(ImageToProcess);
//
//			ImageToDisplay = QImage(ImageToProcess.data,
//										ImageToProcess.cols,
//										ImageToProcess.rows,
//						static_cast<int>(ImageToProcess.step),
//								QImage::Format_Grayscale8 );
//			//emit SignalNewImage(ImageToDisplay);
//		      emit SignalNewImage();
//
//}

QImage& CameraInterfaceClassAravis::GetImageToDisplay() { return ImageToDisplay; }
cv::Mat& CameraInterfaceClassAravis::GetImageToProcess() { return ImageToProcess; }

int CameraInterfaceClassAravis::InitCamera()
{
    //Connect to the first available camera, then acquire 10 buffers.
    ImagePos = SettingsRegister::GetPair("CAMERA_IMAGE_POS"); SizeImage = SettingsRegister::GetPair("CAMERA_IMAGE_SIZE");
	qDebug() << TAG_NAME << "SET ROI: " << ImagePos.first << ImagePos.second << "SIZE: " << SizeImage.first << SizeImage.second;

	callback_data.Receiver = this;

	//camera = arv_camera_new (NULL, &error); //Connect to the first available camera
	//							if(!ARV_IS_CAMERA(camera))  return -1; 

	//qDebug() << "FOUND CAMERA " << arv_camera_get_model_name (camera, NULL);
	//qDebug() << "          ID " << arv_camera_get_device_id (camera, NULL);
	//qDebug() << "      VenDOR " << arv_camera_get_vendor_name (camera, NULL);


	//arv_camera_set_acquisition_mode (camera, ARV_ACQUISITION_MODE_CONTINUOUS, &error); 
	//							if(error) return error->code;  

	//callback_data.counter = 0;
	//callback_data.done = FALSE;
	//callback_data.stream = NULL;


	//callback_data.stream = arv_camera_create_stream (camera, stream_callback, &callback_data, &error); 
	//							if(!ARV_IS_STREAM (callback_data.stream)) { return -1; }

    
    //payload = arv_camera_get_payload (camera, &error); //Retrieve the payload size for buffer creation
				//				            if(error) return error->code;  
    

    //Insert some buffers in the stream buffer pool 
	//palyload = width*height*pixel_size;
	//for (int i = 0; i < 4; i++) 
	//{
	//	arv_stream_push_buffer (callback_data.stream, arv_buffer_new (payload, NULL));  //SYSTEM BUFFERS TO RECEIVE IMAGE
	//	Buffers.push_back(new uint8_t[payload]); //USER STORAGE BUFFERS TO PROCESSS
	//}
	//
	//CurrentBuffer = Buffers.begin();
	//
	//
	//arv_camera_set_region (camera, ImagePos.first,ImagePos.first, SizeImage.first, SizeImage.second, NULL);

	return EXIT_SUCCESS;
}

int CameraInterfaceClassAravis::StartCameraStream ()
{
    

    //qDebug() << "START CAMERA STREAM";
    //arv_camera_start_acquisition (camera, &error);
				//				if(error) return error->code;  

    //timerDisplayTestImage.start(40);

	return EXIT_SUCCESS;
}

void CameraInterfaceClassAravis::DeinitCamera()
{
    //StopCameraStream();
    //g_clear_object(&callback_data.stream); //Destroy the stream object 
    //g_clear_object(&camera);               //Destroy the camera instance 

	//for (auto Buffer: Buffers) delete Buffer; 
	//Buffers.clear();
}

int CameraInterfaceClassAravis::StopCameraStream()
{

	//arv_camera_stop_acquisition (camera, &error);
	//if(error) { qDebug() << "Error: ", error->message; return EXIT_FAILURE; }
	//
	//return EXIT_SUCCESS;
    return 0;
}

void CameraInterfaceClassAravis::SlotDisplayProcessImage() { imshow("TEST IMAGE: ", ImageToProcess); }

void CameraInterfaceClassAravis::SlotSetCameraRegion(int x, int y, int width, int height )
{
	qDebug() << TAG_NAME << "SET ROI: " << x << y << width << height;
	//arv_camera_set_region (camera, x, y, width, height, NULL);
	//GetCurrentCameraRegion();
}

void CameraInterfaceClassAravis::GetCurrentCameraRegion() 
{ 
	//arv_camera_get_region (camera, &CameraRegion.x, &CameraRegion.y, &CameraRegion.width, &CameraRegion.width, NULL); 
}

const std::vector<QPair<int,int>>& CameraInterfaceClassAravis::GetPoints()  
{
		if(CameraPoints.empty()) CameraPoints.resize(2);
		   CameraPoints[0].first  = CameraRegion.x; 
		   CameraPoints[0].second = CameraRegion.y; CameraPoints[1] = CameraPoints[0]; 
													CameraPoints[2] = CameraPoints[0];
	return CameraPoints;
}

const std::vector<QRect>& CameraInterfaceClassAravis::GetRects()  
{
		if(CameraRects.empty()) CameraRects.resize(2);
		   CameraRects[0].setRect(CameraRegion.x    ,CameraRegion.y,
								  CameraRegion.width,CameraRegion.heigh); CameraRects[1] = CameraRects[0]; 
																		  CameraRects[2] = CameraRects[0];
	return CameraRects;
}
const QString& CameraInterfaceClassAravis::GetInfo()  { return CAMERA_INFO; }

int CameraInterfaceClassAravis::GetID() {return SettingsRegister::GetValue("BLOCK_ID_CAMERA"); }; 
