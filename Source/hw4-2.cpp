#include <iostream>
#include <opencv.hpp>
using namespace std;
using namespace cv;

const string input_path = "../TestImg/";
const string output_path = "../Result/";
const int KSIZE = 3;
const int DEV = 0;
const int LEVEL = 9;


vector<Mat> lapA, lapB, mask_vec;
Mat gauA, gauB;
Mat srcA, srcB, mask;
uchar *pic_ptr;

int main(int argc, char* argv[]) {
	
	srcA = imread(input_path + argv[1]);
	if(srcA.empty()) {
		cout << "Open image failed." << endl;
		return 0;
	}
	srcB = imread(input_path + argv[2]);
	if(srcB.empty()) {
		cout << "Open image failed." << endl;
		return 0;
	}
	
	srcA.copyTo(mask);
	for(int i = 0; i < mask.rows; i++) {
		uchar *ptr = mask.ptr(i);
		for(int j = 0; j < mask.cols; j++) {
			for(int k = 0; k < 3; k++) {
				if(j <= mask.cols/2)
					*ptr++ = 255;
				else
					*ptr++ = 0;
			}
		}
	}
	for(int i = 0; i < LEVEL; i++) {
		Mat dst, tmp;
		GaussianBlur(mask, dst, Size(KSIZE, KSIZE), DEV, DEV);
		resize(dst, tmp, Size(dst.cols/2, dst.rows/2));
		mask_vec.push_back(dst);
		mask = tmp;
	}
	
	gauA = srcA.clone();
	for(int i = 0; i < LEVEL; i++) {
		Mat dst, tmp;
		GaussianBlur(gauA, dst, Size(KSIZE,KSIZE), DEV, DEV);
		resize(dst, tmp, Size(dst.cols/2, dst.rows/2));
		lapA.push_back(gauA-dst);
		gauA = tmp;
	}
	gauB = srcB.clone();
	for(int i = 0; i < LEVEL; i++) {
		Mat dst, tmp;
		GaussianBlur(gauB, dst, Size(KSIZE,KSIZE), DEV, DEV);
		resize(dst, tmp, Size(dst.cols/2, dst.rows/2));
		lapB.push_back(gauB-dst);
		gauB = tmp;
	}
	
	
	Mat result, append, scale, last;
	for(int k = 0; k < LEVEL+1; k++) {
		if(k <= 0) {
			result = gauA;
			append = gauB;
			scale = mask;
		} else {
			result = lapA.back();
			lapA.pop_back();
			append = lapB.back();
			lapB.pop_back();
			scale = mask_vec.back();
			mask_vec.pop_back();
		}
		for(int i = 0; i < result.rows; i++) {
			uchar *Aptr = result.ptr(i);
			uchar *Bptr = append.ptr(i);
			uchar *Mptr = scale.ptr(i);
			for(int j = 0; j < result.cols; j++) {
				/*
				Aptr[0] = Aptr[0] * Mptr[0]/255 + Bptr[0] * (1- Mptr[0]/255);
				Aptr[1] = Aptr[1] * Mptr[1]/255 + Bptr[1] * (1- Mptr[1]/255);
				Aptr[2] = Aptr[2] * Mptr[2]/255 + Bptr[2] * (1- Mptr[2]/255);
				Aptr+=3;
				Bptr+=3;
				*/
				 
				 if(j >= result.cols/2) {
					Aptr[0] = Bptr[0];
					Aptr[1] = Bptr[1];
					Aptr[2] = Bptr[2];
					Aptr+=3;
					Bptr+=3;
				 } else {
					Aptr+=3;
					Bptr+=3;
				 }
			}
		}
		if(k > 0)
			result += last;
		last.release();
		if(k >= LEVEL) {
			resize(result, last, Size(result.cols*2, result.rows*2));
		} else
			resize(result, last, Size(lapA.back().cols, lapB.back().rows));
	}
	
	namedWindow("result", WINDOW_NORMAL);
	imshow("result", result);
	imwrite(output_path + argv[3], last);
	waitKey();
	
	return 0;
}
