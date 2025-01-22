#pragma once
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/tracking.hpp"
#include <opencv2/core/ocl.hpp>



class TrackerFirst
{
public:
	// используемый треккер
	cv::Ptr<cv::TrackerKCF> tracker;
	bool follow_flag = false;
	// флаг инициальзации слежения
	bool init_follow = false;
	// ROI - прямоугольник слежения 
	cv::Rect bbox;
	cv::Rect bbox_prev;

	// счетчик для перехода
	int iterator = 0;
	int iterator2 = 0;
	// функция поиски объекта
	void find(cv::Mat image);
	// функция слежения за объектом
	void follow(cv::Mat image);

	//необходимо добавить фильтр Калмана для предиктора
	cv::KalmanFilter KF = cv::KalmanFilter(4, 2, 0);


	void predict();
	cv::Point predicted_coords;
	// основная функция, куда будет передано изображения
	// переключает между find() и follow()
	int thread(cv::Mat image);
};


//class PassCoords
//{
//public:
//	std::vector<int> coords = { 0, 0 };
//	void avg(cv::Rect bbox1, cv::Rect bbox2);
//};
