#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream> 
#include <fstream>
#include <sstream>
#include "windows.h"

const std::string VIDEO_NAME = "BAD APPLE!! but it's played in the code that generated this video";

//size of the screenshotted notepad window
const int NOTEPAD_WIDTH = 1506;
const int NOTEPAD_HEIGHT = 1284;

//coords of notepad window
const int NOTEPAD_X = 447;
const int NOTEPAD_Y = 386;

//num of pixels to display as 1 char
const int PIXELLISATION_RATE = 8;

//wait time (ms) between running notepad and screenshot
const int RUN_SPEED = 150;

//rgb limit between priting space or char
const int BLACK_WHITE_LIMIT = 250;

//print debug log or not
const bool DEBUG = false;

const std::string SOURCE_CODE_PATH = "ConsoleApplication1.cpp";
const std::string SOURCE_VIDEO_PATH = "badapple.mp4";
const std::string SOURCE_AUDIO_PATH = "badapple.mp3";

//screenshot generating and saving functions
BITMAPINFOHEADER createBitmapHeader(int width, int height);
cv::Mat captureScreenMat(HWND hwnd);
void printDebug(std::string debugString);

int main()
{
	//open the code file
	std::fstream fs;
	fs.open(SOURCE_CODE_PATH);

	//declare the other streams that will be used later
	std::stringstream formattedCodeStream;
	std::ofstream frameToTextFile;

	//open the video file
	std::string videofilepath = SOURCE_VIDEO_PATH;
	cv::VideoCapture cap(videofilepath);
	if (!cap.isOpened()) {
		std::cout << "Cannot open the video file.\n";
		return -1;
	}

	int frameCount = cap.get(cv::CAP_PROP_FRAME_COUNT);
	cv::Size* outputSize = new cv::Size(NOTEPAD_HEIGHT, NOTEPAD_HEIGHT);
	int currentWidth = 0;
	cv::Mat frame;

	//iterate over the frames in the vid
	for (int f = 0; f < frameCount; f++) {
		cap.set(cv::CAP_PROP_POS_FRAMES, f);
		cap.read(frame);
		cvtColor(frame, frame, cv::COLOR_BGR2GRAY);

		//skip the UTF byte order mark (three junk characters)
		fs.get();
		fs.get();
		fs.get();

		//iterate over the pixels of the frame
		for (int i = 0; i < frame.rows; i += 8) {
			uchar* rowPointer = frame.ptr<uchar>(i);
			for (int j = 0; j < frame.cols; j += 8) {

				//store the pixel value
				uchar pixelValue = rowPointer[j];

				//if white, insert two spaces (for the format)
				if (pixelValue > BLACK_WHITE_LIMIT) {
					formattedCodeStream << "  ";
					currentWidth++;
				}

				//otherwise, insert the next non-blank char of the code file
				else {
					while (fs.peek() == ' ' || fs.peek() == '\n' || fs.peek() == '\t') {
						fs.get();
					}
					formattedCodeStream << (char)fs.get();

					//do it again so the format is closer to 4:3
					while (fs.peek() == ' ' || fs.peek() == '\n' || fs.peek() == '\t') {
						fs.ignore();
					}
					formattedCodeStream << (char)fs.get();
					currentWidth++;
				}

				//if we inserted enough chars to fill a line, insert a line break
				if (currentWidth == (int)(frame.cols/8)) {
					currentWidth = 0;
					formattedCodeStream << std::endl;
				}
			}
		}

		//write the formatted code to a txt file
		frameToTextFile.open("BAD APPLE!!.txt", std::ios::out | std::ios::trunc);
		std::stringstream formattedCodeLineStream;
		frameToTextFile << formattedCodeStream.str();
		frameToTextFile.close();

		printDebug("file written");

		//open that text file on notepad
		std::system("start notepad BAD APPLE!!.txt");

		printDebug("notepad opened");

		//sleep a bit so notepad has the time to display the .txt's content before it gets screenshot
		Sleep(RUN_SPEED);

		//take a screenshot of the notepad window
		HWND hwnd = GetDesktopWindow();
		cv::Mat source = captureScreenMat(hwnd);

		printDebug("frame " + std::to_string(f) + " screenshotted");

		//close notepad
		std::system("taskkill /IM notepad.exe /F");

		//save the screenshot
		std::vector<uchar> buffer;
		cv::imencode(".png", source, buffer);
		cv::imwrite("frames\\frame_"+std::to_string(f)+".png", source);

		printDebug("screenshot " + std::to_string(f) + " saved");

		//clear the memory
		buffer.clear();

		//reset the stream
		formattedCodeStream.str(std::string());

		//close and reopen the code file so the cursor is reset at position 0
		fs.close(); 
		fs.open("ConsoleApplication1.cpp");
	}

	//create a video with all the frames
	system("ffmpeg -start_number 0 -r 30 -i frames\\frame_%d.png -vcodec mpeg4 video_output.avi");
	printDebug("created video_output.avi");

	//add the original bad apple in the corner
	system("ffmpeg -i video_output.avi -vf \"movie = badapple.mp4, scale = 366:-1[inner]; [in][inner]overlay = 1139:1012[out]\" video_output_with_overlay.avi");
	printDebug("created video_output_with_overlay.avi");

	//add audio to the video
	system("ffmpeg -i video_output_with_overlay.avi -i badapple.mp3 -map 0:v -map 1:a \"BAD APPLE!! but it's played in the code that generated this video.avi\"");
	printDebug("created final video");

	return 0;
}

void printDebug(std::string debugString) {
	if (!DEBUG) return;
	std::cout << std::endl << "DEBUG : " << debugString << std::endl;
}

BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
	BITMAPINFOHEADER  bi;
	// create a bitmap
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	return bi;
}

cv::Mat captureScreenMat(HWND hwnd)
{
	cv::Mat src;
	// get handles to a device context (DC)
	HDC hwindowDC = GetDC(hwnd);
	HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	// create mat object
	src.create(NOTEPAD_HEIGHT, NOTEPAD_WIDTH, CV_8UC4);

	// create a bitmap
	HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, NOTEPAD_WIDTH, NOTEPAD_HEIGHT);
	BITMAPINFOHEADER bi = createBitmapHeader(NOTEPAD_WIDTH, NOTEPAD_HEIGHT);

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);

	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, NOTEPAD_WIDTH, NOTEPAD_HEIGHT, hwindowDC, NOTEPAD_X, NOTEPAD_Y, NOTEPAD_WIDTH, NOTEPAD_HEIGHT, SRCCOPY);
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, NOTEPAD_HEIGHT, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);
	return src;
}

//thanks to https://superkogito.github.io/blog/2020/07/26/capture_screen_using_gdiplus.html for these last 2 functions