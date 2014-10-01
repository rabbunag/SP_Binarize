// SP_Binarize.cpp : Defines the entry point for the console application.
/**
* Description: Binarization of an input image file
* Run File: SP_Binarize <image file>
*
* Author: Raquel Abigail Bunag
* Date Created: July 24, 2014
* Date Modified: October 1, 2014
*/

/**
* Headers
*/
#include "stdafx.h"
#include "cv.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace cv;
using namespace std;

/**
* Global Variables
*/
Mat img;
int imgWidth, imgHeight;
int dilation_elem = 0;
int dilation_size = 0;
int erosion_elem = 0;
int erosion_size = 0;

/**
* Functions
*/
Mat binarizeImage(Mat inputOutputImage){
	Mat img_gray;

	//grayscale
	cvtColor(inputOutputImage, img_gray, CV_BGR2GRAY);

	//otsu's method
	threshold(img_gray, inputOutputImage, 0, 255, CV_THRESH_OTSU);

	return inputOutputImage;
}

void nameAndSaveImage(char ** filenNameAndDestination, Mat img, char * prefix){
	//naming and saving result
	char * nameAndDestination = "";
	char *next = NULL;
	if (filenNameAndDestination[2] == NULL) {
		char * characterHandler;
		characterHandler = strtok_s(filenNameAndDestination[1], "\\", &next);

		while (characterHandler != NULL){
			nameAndDestination = characterHandler;
			characterHandler = strtok_s(NULL, "\\", &next);
		}

		string binarizeString = prefix;
		string nameAndDestinationString = binarizeString + nameAndDestination;

		imwrite(nameAndDestinationString, img);

	}
	else {
		nameAndDestination = filenNameAndDestination[2];
		imwrite(nameAndDestination, img);
	}
}

//Modified version of thresold_callback function 
//from http://docs.opencv.org/doc/tutorials/imgproc/shapedescriptors/bounding_rotated_ellipses/bounding_rotated_ellipses.html
Mat fittingEllipse(int, void*, Mat inputImage)
{
	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	// Detect edges using Threshold
	threshold(inputImage, inputImage, 224, 250, THRESH_BINARY);
	
	findContours(inputImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<RotatedRect> minEllipse(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 5)
			minEllipse[i] = fitEllipse(Mat(contours[i]));
	}

	//Draw ellipse/caption
	Mat drawing = Mat::zeros(inputImage.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(255, 255, 255);

		if (minEllipse[i].size.height >= inputImage.cols / 20 && //IJIP-290-libre.pdf
			minEllipse[i].size.width >= inputImage.rows / 20 && //IJIP-290-libre.pdf
			minEllipse[i].size.height < inputImage.cols / 2 &&
			minEllipse[i].size.width < inputImage.rows &&
			(
			(minEllipse[i].angle >= 0 && minEllipse[i].angle <= 30) ||
			(minEllipse[i].angle >= 60 && minEllipse[i].angle <= 120) ||
			(minEllipse[i].angle >= 150 && minEllipse[i].angle <= 210) ||
			(minEllipse[i].angle >= 240 && minEllipse[i].angle <= 300) ||
			(minEllipse[i].angle >= 330 && minEllipse[i].angle <= 360)
			)) {
			//color = Scalar(0, 0, 255);
			ellipse(drawing, minEllipse[i], color, -1, 8);
		}
	}
	drawing = binarizeImage(drawing);
	return drawing;
}

Mat invertImage(Mat img){
	// get the image data
	int height = img.cols,
		width = img.rows,
		step = img.step;
	unsigned char *input = (unsigned char*)(img.data);

	for (int i = 0; i < imgWidth; i++)
	{
		for (int j = 0; j < imgHeight; j++)
		{
			input[step * i + j] = 255 - input[step * i + j]; // b 
			input[step * i + j + 1] = 255 - input[step * i + j + 1]; // g 
			input[step * i + j + 2] = 255 - input[step * i + j + 2]; // r
		}
	}
	return img;
}

Mat CaptionDetection(Mat inputImage){
	Mat outputImage, binaryImage, captionDetectImage;

	binaryImage = captionDetectImage = binarizeImage(inputImage);
	threshold(captionDetectImage, captionDetectImage, 224, 250, 0); //IJIP-290-libre.pdf
	
	GaussianBlur(captionDetectImage, captionDetectImage, Size(9, 9), 0, 0);
	captionDetectImage = fittingEllipse(0, 0, captionDetectImage);
	
	binaryImage = invertImage(binaryImage);

	outputImage = inputImage;

	for (int i = 0; i < inputImage.rows; i++) {
		for (int j = 0; j < inputImage.cols; j++) {
			if (captionDetectImage.at<uchar>(i, j) == 0) {
				outputImage.at<Vec3b>(i, j)[0] = outputImage.at<Vec3b>(i, j)[1] = outputImage.at<Vec3b>(i, j)[2] = 0;
			}
		}
	}

	return outputImage;
}

Mat fillExpand(Mat inputImage){
	cv::Mat image = inputImage;

	cv::Mat image_thresh;
	cv::threshold(image, image_thresh, 125, 255, cv::THRESH_BINARY);

	// Loop through the border pixels and if they're black, floodFill from there
	cv::Mat mask;
	image_thresh.copyTo(mask);
	for (int i = 0; i < mask.cols; i++) {
		if (mask.at<char>(0, i) == 255) {
			cv::floodFill(mask, cv::Point(i, 0), 255, 0, 10, 10);
		}
		if (mask.at<char>(mask.rows - 1, i) == 255) {
			cv::floodFill(mask, cv::Point(i, mask.rows - 1), 255, 0, 10, 10);
		}
	}
	for (int i = 0; i < mask.rows; i++) {
		if (mask.at<char>(i, 0) == 255) {
			cv::floodFill(mask, cv::Point(0, i), 255, 0, 10, 10);
		}
		if (mask.at<char>(i, mask.cols - 1) == 255) {
			cv::floodFill(mask, cv::Point(mask.cols - 1, i), 255, 0, 10, 10);
		}
	}


	// Compare mask with original.
	cv::Mat newImage;
	image.copyTo(newImage);
	for (int row = 0; row < mask.rows; ++row) {
		for (int col = 0; col < mask.cols; ++col) {
			if (mask.at<char>(row, col) == 0) {
				newImage.at<char>(row, col) = 255;
			}
		}
	}

	return newImage;
}

Mat CharacterExtraction(Mat inputImage){
	Mat outputImage, binaryImage, cleanImage;
	vector<KeyPoint> keypoints;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	binaryImage = binarizeImage(inputImage);
	

	Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 3));

	erode(binaryImage, binaryImage, element1);
	

	findContours(binaryImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<RotatedRect> minRect(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(Mat(contours[i]));
	}
	
	Mat drawing = Mat::zeros(inputImage.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(0, 0, 255);
		drawContours(drawing, contours, i, Scalar(255, 0, 0), 1, 8, vector<Vec4i>(), 0, Point());


		if (minRect[i].size.height <= inputImage.cols / 30 && 
			minRect[i].size.width <= inputImage.rows / 30  ){

			Point2f rect_points[4]; 
			minRect[i].points(rect_points);

			for (int j = 0; j < 4; j++){
				line(inputImage, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
			}
		}
	}

	/**TODO: same output as the caption detection*/

	//FAST(binaryImage, keypoints, CV_THRESH_OTSU);
	//drawKeypoints(binaryImage, keypoints, outputImage, Scalar(255, 0, 0));

	//opening
	/*cleanImage = Erosion(0, 0, binaryImage);
	cleanImage = Dilation(0, 0, cleanImage);*/

	return inputImage;
}

//main function
int main(int argc, char** argv)
{
	if (argv[1] == NULL){
		printf("Include an image file");
		return 0;
	}

	//image
	img = imread(argv[1], 1);
	imgWidth = img.rows;
	imgHeight = img.cols;

	//caption detection
	img = CaptionDetection(img);

	//character detection
	img = CharacterExtraction(img);
	
	//output image
	nameAndSaveImage(argv, img, "Character_Detection_");

	waitKey(0);

	return 0;
}