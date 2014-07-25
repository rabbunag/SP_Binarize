// SP_Binarize.cpp : Defines the entry point for the console application.
/**
* Description: Binarization of an input image file
* Run File: SP_Binarize <image file>
*
* Author: Raquel Abigail Bunag
* Date Created: July 24, 2014
* Date Modified: July 25, 2014
*/

#include "stdafx.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace cv;
using namespace std;

//global variables
string windowName = "Manga";
Mat img;

//functions
Mat binarizeImage(Mat img, string imageFile){
	//image
	img = imread(imageFile, 1);
	Mat img_gray;

	//grayscale
	cvtColor(img, img_gray, CV_BGR2GRAY);

	//otsu's method
	threshold(img_gray, img, 0, 255, CV_THRESH_OTSU);

	return img;
}

void nameAndSaveImage(char ** filenNameAndDestination){
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

		string binarizeString = "Binarize_";
		string nameAndDestinationString = binarizeString + nameAndDestination;

		imwrite(nameAndDestinationString, img);

	}
	else {
		nameAndDestination = filenNameAndDestination[2];
		imwrite(nameAndDestination, img);
	}
}

//main function
int main(int argc, char** argv)
{
	if (argv[1] == NULL){
		printf("Include an image file");
		return 0;
	}

	img = binarizeImage(img, argv[1]);

	//window
	namedWindow(windowName, WINDOW_AUTOSIZE);
	imshow(windowName, img);

	nameAndSaveImage(argv);

	waitKey(0);

	return 0;
}