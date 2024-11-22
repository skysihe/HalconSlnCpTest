// HalconTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

//int main()
//{
//    std::cout << "Hello World!\n";
//}

#include "HalconCpp.h"
using namespace HalconCpp;

//int main()
//{
//    try {
//        HObject ho_Image;
//        ReadImage(&ho_Image, "example_image.png");
//        HTuple hv_WindowHandle;
//        OpenWindow(0, 0, 512, 512, 0, "", "", &hv_WindowHandle);
//        SetPart(hv_WindowHandle, 0, 0, 512, 512);
//        DispObj(ho_Image, hv_WindowHandle);
//        std::cout << "Press Enter to exit...";
//        std::cin.get();
//    }
//    catch (HException& ex) {
//        std::cerr << "HALCON Exception: " << ex.ErrorMessage().Text() << std::endl;
//    }
//    return 0;
//}

#include "HalconCpp.h"
#include <iostream>

using namespace HalconCpp;

//int main()
//{
//    try {
//        // 1. 读取图像
//        HObject image;
//        ReadImage(&image, "example_image.jpg"); // 请确保路径正确，图像在工作目录中
//
//        // 2. 创建形状模型
//        HTuple modelID;
//        CreateShapeModel(image,                   // 输入图像
//            0,                      // 起始角度 (弧度)
//            HTuple(360).Rad(),      // 结束角度 (弧度)
//            HTuple(1).Rad(),        // 角度步长 (弧度)
//            "auto",                 // 金字塔分辨率
//            "use_polarity",         // 极性使用
//            "auto",                 // 最小对比度
//            5,                      // 金字塔层数
//            modelID);               // 输出模型 ID
//
//        std::cout << "Shape model created with ID: " << modelID[0].I() << std::endl;
//
//        // 3. 使用 SetGenericShapeModelParam 设置模型参数
//        // 设置最小得分为 0.7，表示匹配得分低于 70% 的结果将被过滤掉
//        SetGenericShapeModelParam(modelID, "min_score", 0.7);
//
//        // 设置金字塔层数
//        SetGenericShapeModelParam(modelID, "num_levels", 4);
//
//        // 4. 执行形状匹配
//        HTuple row, column, angle, score;
//        FindShapeModel(image,                // 输入图像
//            modelID,              // 形状模型 ID
//            0,                    // 起始角度
//            HTuple(360).Rad(),    // 结束角度
//            0.7,                  // 最小得分
//            0,                    // 最小个数
//            0,                    // 最大个数
//            "least_squares",      // 子像素化方法
//            0,                    // 限制搜索范围
//            0.5,                  // 超时时间（秒）
//            row,                  // 输出：行坐标
//            column,               // 输出：列坐标
//            angle,                // 输出：角度（弧度）
//            score);               // 输出：得分
//
//        // 5. 检查并输出结果
//        if (score.Length() > 0) {
//            std::cout << "Match found!" << std::endl;
//            std::cout << "Location: Row = " << row[0].D()
//                << ", Column = " << column[0].D()
//                << ", Angle = " << angle[0].D()
//                << ", Score = " << score[0].D() << std::endl;
//        }
//        else {
//            std::cout << "No match found." << std::endl;
//        }
//
//        // 6. 释放模型资源
//        ClearShapeModel(modelID);
//    }
//    catch (HException& e) {
//        // 捕获并输出 HALCON 异常信息
//        std::cerr << "Error: " << e.ErrorMessage() << std::endl;
//    }
//
//    return 0;
//}
//



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
