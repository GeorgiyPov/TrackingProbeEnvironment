#include "Tracker.h"

//функция обнаружения объекта для передачи данных на слежение
void TrackerFirst::find(cv::Mat image)
{
    cv::Mat copy_image;
    image.copyTo(copy_image);

    //обработка по поиску объекта для получения координаты
    cv::Mat thresh;
    cv::threshold(copy_image, thresh, 180, 255, CV_8UC1);
    cv::morphologyEx(thresh, thresh, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, { 3,3 }));

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(thresh, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    if (contours.size() > 0)
    {
        int max_num;
        int max_length=0;
        for (size_t i = 0; i < contours.size() && i < 20; i++)
        {
            if (arcLength(contours[i], true) > max_length)
            {
                max_num = i;
                max_length = arcLength(contours[i], true);
            }
        }

        int x_min = contours[max_num][0].x;
        int y_min = contours[max_num][0].y;
        int x_max = contours[max_num][0].x;
        int y_max = contours[max_num][0].y;
        for (size_t j = 0; j < contours[max_num].size() && j < 200; j++)
        {
            if (contours[max_num][j].x < x_min)
                x_min = contours[max_num][j].x;

            if (contours[max_num][j].x > x_max)
                x_max = contours[max_num][j].x;

            if (contours[max_num][j].y < y_min)
                y_min = contours[max_num][j].y;

            if (contours[max_num][j].y > y_max)
                y_max = contours[max_num][j].y;
        }
        bbox = cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min);
        cv::rectangle(copy_image, { x_min, y_min }, { x_max, y_max }, 255, 1);
    }
    
    //cv::imshow("sdsd", copy_image);
    cv::waitKey(1);
    iterator++;
    if (iterator > 30)
    {
        follow_flag = true;
        init_follow = false;
        iterator = 0;
    }

}


//функция слежения за объектом
void TrackerFirst::follow(cv::Mat image)
{
    if (init_follow == false)
    {
        auto params = cv::tracking::TrackerKCF::Params();
        params.desc_npca = cv::TrackerKCF::GRAY;
        params.desc_pca = cv::TrackerKCF::GRAY;
        params.compressed_size = 1;
        tracker = cv::tracking::TrackerKCF::create(params);
        tracker->init(image, bbox);
        cv::Mat final_image;
        image.copyTo(final_image);
        cv::rectangle(final_image, bbox, 255, 5);
        //cv::imshow("sdsd", final_image);
        cv::waitKey(1);
        init_follow = true;


        KF.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, 1, 0,
            0, 1, 0, 1,
            0, 0, 1, 0,
            0, 0, 0, 1);

        KF.measurementMatrix = (cv::Mat_<float>(2, 4) << 1, 0, 0, 0,
            0, 1, 0, 0);

        setIdentity(KF.processNoiseCov, cv::Scalar(1e-4));

        setIdentity(KF.measurementNoiseCov, cv::Scalar(1e-1));

        setIdentity(KF.errorCovPost, cv::Scalar(1));

        KF.statePost = (cv::Mat_<float>(4, 1) << 0, 0, 0, 0);

    }
    else 
    {
        bbox_prev = bbox;
        tracker->update(image, bbox);
        predict();
        if (bbox.width > 0 && bbox.height > 0)
        {
            cv::Mat final_image;
            image.copyTo(final_image);
            cv::rectangle(final_image, bbox, 120, 5);
            cv::circle(final_image, predicted_coords, 10, 0, 2);
            //cv::imshow("sdsd", final_image);
            cv::waitKey(1);
            if (bbox_prev == bbox)
            {
                iterator2++;
                if (iterator2 > 2)
                {
                    follow_flag = false;
                    iterator2 = 0;
                }
            }
            else
            {
                iterator2 = 0;
            }
        }
        else
        {
            follow_flag = false;
        }
        
    }
}

void TrackerFirst::predict()
{
    cv::Point current_coords = cv::Point(bbox.x + bbox.width/2, bbox.y + bbox.width/2);
    cv::Mat prediction = KF.predict();
    predicted_coords = cv::Point(prediction.at<float>(0), prediction.at<float>(1));

    cv::Mat measurement = (cv::Mat_<float>(2, 1) << current_coords.x, current_coords.y);
    KF.correct(measurement);
}

int TrackerFirst::thread(cv::Mat image)
{
    if (follow_flag == true)
    {
        follow(image);
        return 0;
    }
    else
    {
        find(image);
        return 0;
    }

} 

//void PassCoords::avg(cv::Rect bbox1, cv::Rect bbox2)
//{
//    int x_center = (bbox1.x + bbox2.x) / 2 + (bbox1.width + bbox2.width) / 4;
//    int y_center = (bbox1.y + bbox2.y) / 2 + (bbox1.height + bbox2.height) / 4;
//    coords = {x_center, y_center}; 
//
//}
