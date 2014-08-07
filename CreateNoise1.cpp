// CreateNoise1.cpp : Defines the entry point for the console application.
//

// Some includes that we'll be making use of
#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <float.h>

// Define human readable names please!
#define NOISE_UNIFORM 0
#define NOISE_GAUSSIAN 1
#define NOISE_EXPONENTIAL 2
#define NOISE_RAYLEIGH 3
#define NOISE_GAMMA 4
#define NOISE_IMPULSE 5

// Generates and returns the histogram of a GRAYSCALE image.
IplImage* DrawHistogram(IplImage* img)
{
	CvSize imgSize = cvGetSize(img);
	int area = imgSize.width*imgSize.height;

	// Holds the actual histogram image
	IplImage* ret = cvCreateImage(cvSize(257, 100), 8, 1);
	cvZero(ret);

	int freq[256] = {0};
	int max=0;

	// Loop through each pixel of the image
	for(int x=0;x<imgSize.width;x++)
	{
		for(int y=0;y<imgSize.height;y++)
		{
			// Increment the frequency
			int curr = (int)cvGetReal2D(img, y, x);
			freq[curr]++;
			if(freq[curr]>max)
				max = freq[curr];
		}
	}

	// Finally, draw the actual histogram
	for(int k=0;k<256;k++)
	{
		int value = ((float)(100*freq[k])/(float)max);
		cvLine(ret, cvPoint(k, 100), cvPoint(k, 100-value), cvScalar(255,255,255));
	}
	
	cvNot(ret, ret);
	return ret;
}

// Returns a uniformly distributed random number
double uniform()
{
	return (rand()/(float)0x7fff);
}

// Returns a gaussian random number (using uniform random numbers)
double gaussian(void)
{
   static double v, fac;
   static int phase = 0;
   double S, Z, U1, U2, u;

   if (phase)
      Z = v * fac;
   else
   {
      do
      {
         U1 = uniform();
         U2 = uniform();

         u = 2. * U1 - 1.;
         v = 2. * U2 - 1.;
         S = u * u + v * v;
      } while(S >= 1);

      fac = sqrt (-2. * log(S) / S);
      Z = u * fac;
   }

   phase = 1 - phase;

   return Z;
}

// Generates an exponential random number using a uniform random number
double exponential(float lambda=1)
{
	return -lambda*log(uniform());
}

// Generates a rayleigh random number using a uniform random number
double rayleigh(float sigma=1)
{
	return sigma*sqrt(-1*log(uniform()));
}

// Generates a gamma random number using a uniform random number
// NOTE: THIS IS NOT ACCURATE
double gamma(int k=2)
{
	double ret=0;
	while(k>0)
	{
		ret+= (-(log(uniform())));
		k--;
	}
	return ret;
}

// Generates an impulse using a uniform random number.. This is just one of the
// thousands of other possibilities.
double impulse(float amount)
{
	if(uniform()>1-amount)
		return 100;
	if(uniform()<amount)
		return -100;

	return 0;
}

// This function actually applies the specified noise to the given image
// in the given amounts
IplImage* GenerateNoise(IplImage* img, int noiseType, float amount=255)
{
	CvSize imgSize = cvGetSize(img);
	IplImage* imgTemp = cvCloneImage(img);	// This will hold the noisy image

	// Go through each pixel
	for(int y=0;y<imgSize.height;y++)
	{
		for(int x=0;x<imgSize.width;x++)
		{
			int randomValue=0;				// Our noise is additive.. this holds
			switch(noiseType)				// the amount to add/subtract
			{
			case NOISE_UNIFORM:				// I chose UNIFORM, so give me a uniform random number
				randomValue = (char)(uniform()*amount);
				break;

			case NOISE_EXPONENTIAL:			// I chose EXPONENTIAL... so exp random number please
				randomValue = (int)(exponential()*amount);
				break;

			case NOISE_GAUSSIAN:			// same here
				randomValue = (int)(gaussian()*amount);
				break;

			case NOISE_RAYLEIGH:			// ... guess!!
				randomValue = (int)(rayleigh()*amount);
				break;

			case NOISE_GAMMA:				// I chose gamma... give me a gamma random number
				randomValue = (int)(gamma()*amount);
				break;

			case NOISE_IMPULSE:				// I need salt and pepper.. pass the shakers please
				randomValue = (int)(impulse((float)amount/256)*amount);
			}
			
			// Here we "apply" the noise to the current pixel
			int pixelValue = cvGetReal2D(imgTemp, y, x)+randomValue;

			// And set this value in our noisy image
			cvSetReal2D(imgTemp, y, x, pixelValue);
		}
	}

	// return
	return imgTemp;
}

// Ah, the main function!!
int main()
{
	// Load the image with no noise
	IplImage* img = cvLoadImage("noise_tester.jpg", 0);
	IplImage* imgTemp;
	IplImage* imgHist;

	// The default choice
	int noiseType=NOISE_UNIFORM;

	// Initialize the window systems
	int trackPos = 20;
	cvNamedWindow("Image");
	cvNamedWindow("Histogram");
	cvCreateTrackbar("amount", "Image", &trackPos, 255, NULL);

	// The main loop
	bool runLoop = true;
	printf("Press space to exit\n\n");
	while(runLoop)
	{
		// Generate noise and its corresponding histogram
		imgTemp = GenerateNoise(img, noiseType, trackPos);
		imgHist = DrawHistogram(imgTemp);
		
		// Display it all
		cvShowImage("Image", imgTemp);
		cvShowImage("Histogram", imgHist);

		// And release the images. We don't want memory leaks
		cvReleaseImage(&imgHist);
		cvReleaseImage(&imgTemp);

		// Check for a key press
		char keyPress = cvWaitKey(10);

		switch(keyPress)
		{
		case '1':
			noiseType = NOISE_UNIFORM;
			printf("Uniform noise\n");
			break;

		case '2':
			noiseType = NOISE_GAUSSIAN;
			printf("Gaussian noise\n");
			break;

		case '3':
			noiseType = NOISE_EXPONENTIAL;
			printf("Exponential noise\n");
			break;

		case '4':
			noiseType = NOISE_RAYLEIGH;
			printf("Rayleigh noise\n");
			break;

		case '5':
			noiseType = NOISE_GAMMA;
			printf("Gamma noise\n");
			break;

		case '6':
			noiseType = NOISE_IMPULSE;
			printf("Impulse/Salt and pepper noise\n");
			break;

		case ' ':
			runLoop = false;
			break;
		}
	}

	// Finally, exit
	cvReleaseImage(&img);
	return 0;
}