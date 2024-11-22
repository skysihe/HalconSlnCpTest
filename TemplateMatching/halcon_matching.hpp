#pragma once
#include <opencv2/opencv.hpp>
#include <halconcpp/HalconCpp.h>

#include "nlohmann/json.hpp"
#include <fstream>
//#include "HShapeModel.h"
HalconCpp::HObject Mat2HObject(const cv::Mat &image)
{
    HalconCpp::HObject Hobj = HalconCpp::HObject();
    int hgt = image.rows;
    int wid = image.cols;
    int i;
    //  CV_8UC3
    if (image.type() == CV_8UC3)
    {
        std::vector<cv::Mat> imgchannel;
        split(image, imgchannel);
        cv::Mat imgB = imgchannel[0];
        cv::Mat imgG = imgchannel[1];
        cv::Mat imgR = imgchannel[2];
        uchar *dataR = new uchar[hgt * wid];
        uchar *dataG = new uchar[hgt * wid];
        uchar *dataB = new uchar[hgt * wid];
        for (i = 0; i < hgt; i++)
        {
            memcpy(dataR + wid * i, imgR.data + imgR.step * i, wid);
            memcpy(dataG + wid * i, imgG.data + imgG.step * i, wid);
            memcpy(dataB + wid * i, imgB.data + imgB.step * i, wid);
        }
        GenImage3(&Hobj, "byte", wid, hgt, (Hlong)dataR, (Hlong)dataG,
                  (Hlong)dataB);
        delete[] dataR;
        delete[] dataG;
        delete[] dataB;
        dataR = NULL;
        dataG = NULL;
        dataB = NULL;
    }
    //  CV_8UCU1
    else if (image.type() == CV_8UC1)
    {
        uchar *data = new uchar[hgt * wid];
        for (i = 0; i < hgt; i++)
            memcpy(data + wid * i, image.data + image.step * i, wid);
        GenImage1(&Hobj, "byte", wid, hgt, (Hlong)data);
        delete[] data;
        data = NULL;
    }
    return Hobj;
}

void templateMatchingHalcon(const std::string& sceneFilePath, const cv::Mat& sceneImg, cv::Point2f& translation, double& rotation, double &score)
{
    std::string log;
    std::ifstream paraFile(sceneFilePath + "/model.shm");
    if (!paraFile) {
        log = "Failed to load template \"model.shm\" in ";
        // context.LogError(log + templateFilePath);
        return;
    }
    HalconCpp::HTuple ModelID;
    HalconCpp::ReadShapeModel((sceneFilePath + "/model.shm").c_str(), &ModelID);

    std::ifstream centerFile(sceneFilePath + "/center.txt");
    if (!centerFile) {
        log = "Failed to load template \"center.txt\" in";
        // context.LogError(log + templateFilePath);
        centerFile.close();
        return;
    }

    nlohmann::json paraFileJson;
    centerFile >> paraFileJson;
    double modelCenterX = paraFileJson["x"];
    double modelCenterY = paraFileJson["y"];
    centerFile.close();

    // Perform template matching
    HalconCpp::SetGenericShapeModelParam(ModelID, "angle_start", -20.0f/180*3.14159f);
    HalconCpp::SetGenericShapeModelParam(ModelID, "angle_end", 20.0f/180*3.14159f);
    HalconCpp::SetGenericShapeModelParam(ModelID, "num_matches", 1);
    HalconCpp::SetGenericShapeModelParam(ModelID, "min_score", 0.3);

    HalconCpp::HTuple hv_MatchResultID, hv_NumMatchResult;
    HalconCpp::FindGenericShapeModel(Mat2HObject(sceneImg), ModelID, &hv_MatchResultID, &hv_NumMatchResult);
    HalconCpp::HTuple hv_Row, hv_Column, hv_ScaleRow, hv_ScaleColumn;
    HalconCpp::HTuple hv_HomMat2D, hv_Angle, hv_Score;

    if (hv_NumMatchResult.I() > 0) {
        // Matching 01: Retrieve parameters of the detected match
        GetGenericShapeModelResult(hv_MatchResultID, 0, "angle", &hv_Angle);
        GetGenericShapeModelResult(hv_MatchResultID, 0, "hom_mat_2d", &hv_HomMat2D);
        GetGenericShapeModelResult(hv_MatchResultID, 0, "score", &hv_Score);

        GetGenericShapeModelResult(hv_MatchResultID, 0, "row", &hv_Row);
        GetGenericShapeModelResult(hv_MatchResultID, 0, "column", &hv_Column);
        GetGenericShapeModelResult(hv_MatchResultID, 0, "scale_row", &hv_ScaleRow);
        GetGenericShapeModelResult(hv_MatchResultID, 0, "scale_column", &hv_ScaleColumn);

        double row = hv_Row.D();
        double column = hv_Column.D();
        double angle = -hv_Angle.D();
        //angle = angle < 0.0 ? (angle + 2.0 * CV_PI) : angle;

        translation = cv::Point2f(column - modelCenterX, row - modelCenterY);
        rotation = angle * 180 / CV_PI;
        score = hv_ScaleColumn.D();
    }
}
