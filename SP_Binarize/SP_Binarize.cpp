// SP_Binarize.cpp : Defines the entry point for the console application.
/**
* Description: Special Problem: 
* Run File: SP_Binarize <image file>
*
* Author: Raquel Abigail Bunag
* Date Created: July 24, 2014
* Date Modified: October 7, 2014
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
#include <iostream>
#include <fstream>
#include <ocl\ocl.hpp>

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

Mat CharacterExtraction(Mat inputImage){
	Mat outputImage, binaryImage, cleanImage, erodeImage;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	binaryImage = binarizeImage(inputImage);
	

	Mat element1 = getStructuringElement(MORPH_RECT, Size(6,5));

	erode(binaryImage, erodeImage, element1);
	

	findContours(erodeImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
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


		if (minRect[i].size.height <= inputImage.cols / 10 && 
			minRect[i].size.width <= inputImage.rows / 10 && 
			minRect[i].size.height > inputImage.cols / 90 &&
			minRect[i].size.width > inputImage.rows / 90){
		
			Point2f rect_points[4]; 
			minRect[i].points(rect_points);

			for (int j = 0; j < 4; j++){
				line(inputImage, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
			}
		}
	}

	return inputImage;
}

int j = 0;

int Character_Feature_Extraction(Mat inputImage, int i){
	Mat outputImage, binaryImage, imageFeature;
	int imgWidth = inputImage.rows,
		imgHeight = inputImage.cols,
		step = inputImage.step;
	vector<KeyPoint> keypoints;

	unsigned char *feature = (unsigned char*)(imageFeature.data);

	binaryImage = binarizeImage(inputImage);
	unsigned char *input = (unsigned char*)(binaryImage.data);

	imageFeature = binaryImage(Rect(0, 0, inputImage.cols, inputImage.rows));

	int x = (imageFeature.cols * imageFeature.rows) - countNonZero(imageFeature);

	//FAST(imageFeature, keypoints, CV_THRESH_OTSU);
	//drawKeypoints(imageFeature, keypoints, outputImage, Scalar(255, 0, 0));
	
	//String Filename = "C:\\Users\\Abigail_pc\\Documents\\Github\\SP_Binarize\\Debug\\Features_" + to_string(i)+" "+to_string(j)+".jpg";
	//imwrite(Filename, imageFeature);
	j++;
	return countNonZero(imageFeature);
	//return keypoints.size();
}

#define NUMBER_OF_IMAGES_PER_ALPHABET_CHARACTER 8
#define NUMBER_OF_CHARACTERS 71
String alphabet[2] = {"hiragana", "katakana"};
String characters[71] = {
							"a", "i", "u", "e", "o",
							"ka", "ki", "ku", "ke", "ko",
							"sa", "shi", "su", "se", "so",
							"ta", "ti", "tsu", "te", "to",
							"na", "ni", "nu", "ne", "no",
							"ha", "hi", "fu", "he", "ho",
							"ma", "mi", "mu", "me", "mo",
							"ya", "yu", "yo",
							"ra", "ri", "ru", "re", "ro",
							"n", "wo", "wa",
							"ga", "gi", "gu", "ge", "go",
							"za", "ji", "zu", "ze", "zo",
							"da", "di", "zu", "de", "do",
							"ba", "bi", "bu", "be", "bo",
							"pa", "pi", "pu", "pe", "po"
						};

Mat getLoadImages(){
	ofstream myfile;
	myfile.open("example.odt");

	Mat loadedImage, originalImage, grayLoadedImage, loadImage;
	String imageFileName;
	Mat clusterGraph(700,700,CV_8UC3, Scalar(0,0,0));
	

	int feature1, feature2;
	int r = 0, g = 0, b = 0;
	//int points[NUMBER_OF_CHARACTERS][3]; //[character index] [r][g][b]
	//int featurePoints[NUMBER_OF_CHARACTERS*NUMBER_OF_IMAGES_PER_ALPHABET_CHARACTER][2]; //[ training data][x coordinate][y coordinate]

	for (int i = 0; i < 15; i++){ //number of characters

		r = (rand() *i) % 255;
		g = (rand() *i) % 255;
		b = (rand() *i) % 255;
		
		for (int j = 1; j <= NUMBER_OF_IMAGES_PER_ALPHABET_CHARACTER; j++){

			//read image
			//imageFileName = "C:\\Users\\Abigail_pc\\Documents\\SP\\SP Data Set\\Hiragana\\hiragana\\"+characters[i]+" ("+ to_string(j) +").jpg";
			imageFileName = "C:\\Users\\Abigail_pc\\Desktop\\save\\characters by single person\\Hiragana\\" + characters[i] + " (" + to_string(j) + ").jpg";
			loadedImage = imread(imageFileName,1);
			loadImage = imread(imageFileName, 1);
			

			if (loadedImage.data){
				

				//resize
				resize(loadedImage, loadedImage, Size(150, 150), 0, 0, INTER_LINEAR);
				
				loadedImage.copyTo(originalImage);

				Mat element1 = getStructuringElement(MORPH_RECT, Size(45,45));
				erode(loadedImage, loadedImage, element1);

				cvtColor(loadedImage, grayLoadedImage, CV_RGB2GRAY, 0);
				threshold(grayLoadedImage, grayLoadedImage, 0, 255, CV_THRESH_OTSU);
				
				vector<vector<Point> > contours;
				vector<Vec4i> hierarchy;

				findContours(grayLoadedImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
				//vector<Rect> boundingBox(contours.size());

				//bounding box size
				Rect boundingBox;
				for (int k = 0; k < contours.size(); k++){
					if (contours.size() == 1) boundingBox = boundingRect(contours[k]);
					else if (contours.size() > 1 && k == 1) boundingBox = boundingRect(contours[k]);
				}

				
				// Get the moments
				vector<Moments> mu(contours.size());
				for (int k = 0; k < contours.size(); k++)
				{
						mu[k] = moments(contours[k], false);
				}

				vector<Point2f> mc(contours.size());
				for (int k = 0; k < contours.size(); k++)
				{
						mc[k] = Point2f(mu[k].m10 / mu[k].m00, mu[k].m01 / mu[k].m00);		
				}

				//draw center of mass
				//circle(originalImage, mc[contours.size()-1], 4, Scalar(0, 0, 255), -1, 8, 0);
				

				// Draw contours
				/*for (int k = 0; k < contours.size(); k++)
				{
					if (contours[k].size() != originalImage.cols){
						Scalar color = Scalar(0, 0, 255);
						//drawContours(originalImage, contours, k, color, 2, 8, hierarchy, 0, Point());
						circle(originalImage, mc[1], 4, color, -1, 8, 0);
					}
				}*/

				Mat gray;

				/*cvtColor(originalImage, gray, CV_RGB2GRAY, 0);
				threshold(gray, originalImage, 0, 255, CV_THRESH_OTSU);*/

				loadImage = originalImage(boundingBox);

				

				Mat element2 = getStructuringElement(MORPH_RECT, Size(8, 8));
				erode(loadImage, loadImage, element2);

				

				//imwrite("C:\\Users\\Abigail_pc\\Documents\\Github\\SP_Binarize\\Debug\\bounding_box_" + characters[i] + " (" + to_string(j) + ").jpg", loadImage);

				Mat feature1_roi, feature2_roi;
				Point centerOfMass(mc[contours.size() - 1].x, mc[contours.size() - 1].y);
				//int x = centerOfMass.x;
				//int y = centerOfMass.y;
				//int width = abs(loadImage.cols - abs(loadImage.cols - x));
				//int height = abs(loadImage.rows - abs(loadImage.rows - y));

				feature1 = feature2 = 0;

				int width = loadImage.cols;
				int height = loadImage.rows;
				int totalPixels = width*height;

				feature1_roi = loadImage(Rect(0, 0, width, height));
				int whiteFeature1 = Character_Feature_Extraction(feature1_roi, i);
				int blackFeature1 = totalPixels - whiteFeature1;

				feature1 = (blackFeature1 * 100/ whiteFeature1 * 10);
				
				/*feature2_roi = loadImage(Rect(width / 2, 0, width / 2, height / 2));
				int whiteFeature2 = Character_Feature_Extraction(feature2_roi, i);
				int blackFeature2 = totalPixels - whiteFeature2;

				feature2 = (blackFeature2 / whiteFeature2 *100) ;*/

				int horizontal, vertical, roiNumOfPixelsHorizontal, roiNumOfPixelsVertical;
				horizontal = vertical = 0;
				
				int x = (width / 2);
				roiNumOfPixelsHorizontal = 10 * height;
				feature1_roi = loadImage(Rect(x, 0, 10, height));
				horizontal = roiNumOfPixelsHorizontal - Character_Feature_Extraction(feature1_roi, i);
				

				x = (width / 4);
				feature1_roi = loadImage(Rect(x, 0, 10, height));
				horizontal += roiNumOfPixelsHorizontal - Character_Feature_Extraction(feature1_roi, i);
				

				x = ((3 * width) / 4);
				feature1_roi = loadImage(Rect(x, 0, 10, height));
				horizontal += roiNumOfPixelsHorizontal - Character_Feature_Extraction(feature1_roi, i);
				

				int y = (height / 2);
				roiNumOfPixelsVertical = width * 10;
				feature2_roi = loadImage(Rect(0, y, width, 10));
				vertical = roiNumOfPixelsVertical - Character_Feature_Extraction(feature2_roi, i);
				

				y = (height / 4);
				feature2_roi = loadImage(Rect(0, y, width, 10));
				vertical += roiNumOfPixelsVertical - Character_Feature_Extraction(feature2_roi, i);
				

				y = ((3 * height) / 4);
				feature2_roi = loadImage(Rect(0, y, width, 10));
				vertical += roiNumOfPixelsVertical - Character_Feature_Extraction(feature2_roi, i);

				feature2 = (horizontal + vertical) / 10;
				
				
				imwrite("C:\\Users\\Abigail_pc\\Documents\\Github\\SP_Binarize\\Debug\\Feature1_" + characters[i] + " (" + to_string(j) + ").jpg", feature1_roi);
				imwrite("C:\\Users\\Abigail_pc\\Documents\\Github\\SP_Binarize\\Debug\\Feature2_" + characters[i] + " (" + to_string(j) + ").jpg", feature2_roi);
			
				myfile << horizontal*10/100 << "\t" << vertical*10/100 << "\n";

				Point featuresTopPixel, featuresBottomPixel;
				featuresTopPixel.x = feature1 + 100;
				featuresTopPixel.y = feature2 + 100;
				featuresBottomPixel.x = featuresTopPixel.x;
				featuresBottomPixel.y = featuresTopPixel.y;

				rectangle(clusterGraph, featuresTopPixel, featuresBottomPixel, Scalar(b, g, r), 3);
				//myfile << featuresTopPixel.x << " " << (blackPixel / whitePixel) * 100 << "\n";
				
				//imwrite("C:\\Users\\Abigail_pc\\Documents\\"+characters[i] + " (" + to_string(j) + ").jpg", loadedImage);
			} //if
			//myfile << imageFileName << "\n";
		}
	}
	myfile.close();
	return clusterGraph;
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
	//Mat img2 = imread(argv[2], 1);
	imgWidth = img.rows;
	imgHeight = img.cols;

	//nameAndSaveImage(argv, binarizeImage(img), "Binarize_");

	//caption detection
	/*img = CaptionDetection(img);
	imwrite("C:\\Users\\Abigail_pc\\Documents\\Github\\SP_Binarize\\Debug\\Caption_Detection.jpg", img);

	//character detection
	img = CharacterExtraction(img);
	imwrite("C:\\Users\\Abigail_pc\\Documents\\Github\\SP_Binarize\\Debug\\Character_Detection.jpg", img);
	*/
	/*img = Character_Feature2_Extraction(img);
	img2 = Character_Feature_Extraction(img2);
	*/


	img = getLoadImages();

	//output image
	nameAndSaveImage(argv, img, "Graph_");
	//imwrite("img1.jpg", img);
	//imwrite("img2.jpg", img2);
	

	waitKey(0);

	return 0;
}