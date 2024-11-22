#include "halcon_matching.hpp"
//#include "line_matching.hpp"
//#include "agileX_matching.hpp"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>

void templateMatching(const cv::Mat& templateImg, const cv::Mat& mask, const cv::Mat& sceneImg, cv::Point2f& translation, double& rotation, double& score)
{
    // 使用ORB特征检测器和描述符提取器
    cv::Ptr<cv::ORB> orb = cv::ORB::create();
    std::vector<cv::KeyPoint> templateKeypoints, sceneKeypoints;
    cv::Mat templateDescriptors, sceneDescriptors;
    orb->detectAndCompute(templateImg, cv::noArray(), templateKeypoints, templateDescriptors);
    orb->detectAndCompute(sceneImg, cv::noArray(), sceneKeypoints, sceneDescriptors);

    // 使用FLANN匹配器进行特征匹配
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    std::vector<std::vector<cv::DMatch>> knnMatches;
    matcher->knnMatch(templateDescriptors, sceneDescriptors, knnMatches, 2);

    // 应用比率测试过滤匹配
    const float ratioThresh = 0.7f;
    std::vector<cv::DMatch> goodMatches;
    for (size_t i = 0; i < knnMatches.size(); i++)
    {
        if (knnMatches[i][0].distance < ratioThresh * knnMatches[i][1].distance)
        {
            goodMatches.push_back(knnMatches[i][0]);
        }
    }

    // 使用RANSAC估计变换矩阵
    std::vector<cv::Point2f> templatePoints, scenePoints;
    for (size_t i = 0; i < goodMatches.size(); i++)
    {
        templatePoints.push_back(templateKeypoints[goodMatches[i].queryIdx].pt);
        scenePoints.push_back(sceneKeypoints[goodMatches[i].trainIdx].pt);
    }

    cv::Mat H = cv::findHomography(templatePoints, scenePoints, cv::RANSAC);

    // 从变换矩阵中提取平移和旋转参数
    translation = cv::Point2f(H.at<double>(0, 2), H.at<double>(1, 2));
    rotation = atan2(H.at<double>(1, 0), H.at<double>(0, 0)) * 180 / CV_PI;
    score = 0;
}

int main()
{
    std::string templateFolderPath = R"(D:\001Projects\002_agile_x\001projects\001TemplateMatching\03Docs\Dr.Suyijing\TemplateMatching)";
    std::string outputFilePath = "./matching_result.txt";

    std::vector<std::string> templateFiles;
    cv::glob(templateFolderPath + "/*.bmp", templateFiles);

    bool debugMode = 0;
    int start_template = 0;
    int end_template = 0;
    int start_index = 0;
    std::ofstream outputFile;
    if (!debugMode) {
        outputFile.open(outputFilePath);
    }

    int kk = 0;
    for (const std::string& templateFile : templateFiles)
    {
        if (kk < start_template) {
            kk++;
            continue;
        }
        if (kk > end_template) {
            break;
        }
        kk++;
        std::cout << "template" << templateFile << "\n";
        cv::Mat templateImg = cv::imread(templateFile, cv::IMREAD_GRAYSCALE);
        cv::equalizeHist(templateImg, templateImg);

        std::string templateName = templateFile.substr(templateFolderPath.length() + 1);
        std::string sceneFolderPath = templateFolderPath + "/" + templateName.substr(0, templateName.find_last_of('.'));
        std::cout << "sceneFolderPath" << sceneFolderPath << "\n";
        cv::Mat mask = cv::imread(sceneFolderPath + "/mask.bmp", cv::IMREAD_GRAYSCALE);
        cv::Rect roi = cv::boundingRect(mask);
        mask = mask(roi);
        cv::Mat crop_template = templateImg(roi);

        std::vector<std::string> sceneFiles;
        cv::glob(sceneFolderPath + "/*.bmp", sceneFiles);

        int matchCount = 0;
        double totalTime = 0;
        std::string outputFileName = templateName.substr(0, templateName.find_last_of('.')) + "_results.txt";
        std::ofstream outputFile2;
        if (!debugMode) {
            outputFile2.open(outputFileName);
        }

        std::ifstream centerFile(sceneFolderPath + "/center.txt");
        if (!centerFile) {
            centerFile.close();
            return -1;
        }
        nlohmann::json paraFileJson;
        centerFile >> paraFileJson;
        double modelCenterX = paraFileJson["x"];
        double modelCenterY = paraFileJson["y"];
        centerFile.close();

        for (const std::string& sceneFile : sceneFiles)
        {
            if (matchCount < start_index) {
                matchCount++;
                continue;
            }
            std::cout << matchCount << "--sceneFile" << sceneFile << "\n";
            cv::Mat sceneImg = cv::imread(sceneFile, cv::IMREAD_GRAYSCALE);
            cv::equalizeHist(sceneImg, sceneImg);
            cv::Point2f translation;
            double rotation, score = -1;

            auto startTime = std::chrono::high_resolution_clock::now();
            //templateMatching(templateImg, sceneImg, translation, rotation, score);
            //templateMatchingAgileX(crop_template, mask, sceneImg, translation, rotation, score, roi);
            templateMatchingHalcon(sceneFolderPath, sceneImg, translation, rotation, score);
            //std::cout << "halcon" << translation << " " << rotation << " " << score << "\n";
            //templateMatchingEdge(sceneFolderPath, sceneImg, translation, rotation, score);
            std::cout << "edge" << translation << " " << rotation << " " << score << "\n";
            auto endTime = std::chrono::high_resolution_clock::now();

            double matchTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            totalTime += matchTime;

            // 将场景图像按估计得到的参数进行平移旋转
            cv::Mat transformedSceneImg;
            cv::Mat translationMatrix = (cv::Mat_<double>(2, 3) << 1, 0, -translation.x, 0, 1, -translation.y);
            warpAffine(sceneImg, transformedSceneImg, translationMatrix, sceneImg.size());
            cv::Mat rotationMatrix = cv::getRotationMatrix2D(cv::Point2f(modelCenterX, modelCenterY), rotation, 1);
            warpAffine(transformedSceneImg, transformedSceneImg, rotationMatrix, sceneImg.size());
            //cv::imwrite("asd.bmp", transformedSceneImg);
            //rotation = abs(rotation);
            //rotation = rotation < 360-rotation ? rotation : 360-rotation;

            // 再次计算残余偏差
            cv::Point2f residualTranslation;
            double residualRotation, residualScore = -1;
            //templateMatching(templateImg, transformedSceneImg, residualTranslation, residualRotation, residualScore);
            templateMatchingHalcon(sceneFolderPath, transformedSceneImg, residualTranslation, residualRotation, residualScore);
            //residualRotation = abs(residualRotation);
            //residualRotation = residualRotation < 360-residualRotation ? residualRotation : 360-residualRotation;

            // 输出匹配结果
            if (!debugMode) {
                outputFile << "Template: " << templateName << std::endl;
                outputFile << "Scene: " << sceneFile << std::endl;
                outputFile << "Translation: (" << translation.x << ", " << translation.y << ")" << std::endl;
                outputFile << "Rotation: " << rotation << " degrees" << std::endl;
                outputFile << "Score: " << score << std::endl;
                outputFile << "Residual Translation: (" << residualTranslation.x << ", " << residualTranslation.y << ")" << std::endl;
                outputFile << "Residual Rotation: " << residualRotation << " degrees" << std::endl;
                outputFile << "Residual Score: " << residualScore << std::endl;
                outputFile << "Matching Time: " << matchTime << " ms" << std::endl;
                outputFile << std::endl;

                if (score > 0) {
                    outputFile2 << translation.x << " " << translation.y << " " << rotation << " " << score << " "
                        << residualTranslation.x << " " << residualTranslation.y << " " << residualRotation << " " << residualScore << std::endl;
                }
            }

            if (score > 0) {
                matchCount++;
            }
            if (debugMode)
            {
                // 将sceneImg和templateImg叠加在一起显示
                cv::Mat debugImg(sceneImg.rows, sceneImg.cols, CV_8UC3);
                std::vector<cv::Mat> channels(3);
                channels[0] = sceneImg;
                channels[1] = sceneImg;
                channels[2] = templateImg;
                cv::merge(channels, debugImg);

                cv::Mat resizedDebugImg;
                double scale = 640.0 / debugImg.cols;
                cv::resize(debugImg, resizedDebugImg, cv::Size(), scale, scale);
                std::string text = "d: (" + std::to_string(translation.x) + ", " + std::to_string(translation.y) + ")";
                cv::putText(resizedDebugImg, text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
                text = "r: " + std::to_string(rotation) + " degrees s:" + std::to_string(score) + "  i:" + std::to_string(matchCount);
                cv::putText(resizedDebugImg, text, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
                cv::imshow("Before Correction", resizedDebugImg);

                channels[0] = transformedSceneImg;
                channels[1] = transformedSceneImg;
                channels[2] = templateImg;
                cv::merge(channels, debugImg);

                cv::Mat resizedTransformedDebugImg;
                cv::resize(debugImg, resizedTransformedDebugImg, cv::Size(), scale, scale);
                text = "d: (" + std::to_string(residualTranslation.x) + ", " + std::to_string(residualTranslation.y) + ")";
                cv::putText(resizedTransformedDebugImg, text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
                text = "r: " + std::to_string(residualRotation) + " degrees";
                cv::putText(resizedTransformedDebugImg, text, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
                cv::imshow("After Correction", resizedTransformedDebugImg);

                if (cv::waitKey() == 'q') {
                    break;
                }
            }
        }

        double matchingRate = (double)matchCount / sceneFiles.size() * 100;
        double averageTime = totalTime / matchCount;

        if (!debugMode)
        {
            outputFile2.close();
            outputFile << "Template: " << templateName << std::endl;
            outputFile << "Matching Rate: " << matchingRate << "%" << std::endl;
            outputFile << "Average Matching Time: " << averageTime << " ms" << std::endl;
            outputFile << "----------------------------------------------" << std::endl;
        }
    }

    if (!debugMode)
    {
        outputFile.close();
    }

    return 0;
}