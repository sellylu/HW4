#include <iostream>
#include <fstream>
#include <opencv.hpp>
using namespace std;
using namespace cv;

template<class T> void insertion( std::vector<T> &cont, T value ) {
	typename std::vector<T>::iterator it = std::upper_bound( cont.begin(), cont.end(), value, std::less<T>() );
	cont.insert( it, value );
}

string config_path_in = "../config.csv";
string input_file;
string output_file;

Mat src, dst;
uchar *pic_ptr;
int MAX_SCALE = 0;

int main(int argc, char* argv[]) {

	if(argc > 1)
		config_path_in = argv[1];
	fstream config_file;
	cout << config_path_in << endl;
	config_file.open(config_path_in, ios::in);
	if (config_file.is_open()) {
		config_file >> input_file;
		config_file >> output_file;
	} else {
		cout << "Open file failed." << endl;
		return 0;
	}

	src = imread(input_file, IMREAD_GRAYSCALE);
	if(src.empty()) {
		cout << "Open image failed." << endl;
		return 0;
	}
	imshow("original", src);
	dst = src.clone();
	
	// Get binary image
	for(int i = 0; i < dst.rows; i++) {
		pic_ptr = dst.ptr(i);
		for(int j = 0; j < dst.cols; j++) {
			*pic_ptr++ = pic_ptr[0] > 127? 0:255;
		}
	}
	
	vector<uchar> elem;
	
	// Forward Pass
	for(int i = 0; i < dst.rows; i++) {
		pic_ptr = dst.ptr(i);
		for(int j = 0; j < dst.cols; j++) {
			elem.clear();
			// Check if background
			if(pic_ptr[0] <= 0) {
				pic_ptr++;
				continue;
			}
			
			// Get dst-upper corner
			uchar *pad_ptr = dst.ptr(i-1, j-1);
			for(int jx = 0; jx < 3; jx++) {
				insertion(elem, *pad_ptr++);
			}
			pad_ptr = dst.ptr(i, j-1);
			insertion(elem, *pad_ptr);
			
			*pic_ptr++ = elem[0]+1;
		}
	}
	
	// Backward Pass
	for(int i = dst.rows-1; i >= 0; i--) {
		for(int j = dst.cols-1; j >= 0; j--) {
			pic_ptr = dst.ptr(i, j);
			elem.clear();
			// Check if background
			if(pic_ptr[0] <= 0) {
				pic_ptr--;
				continue;
			}
			
			// Get dst-upper corner
			uchar *pad_ptr = dst.ptr(i+1, j-1);
			for(int jx = 0; jx < 3; jx++) {
				uchar tmp = pad_ptr[0]+1;
				insertion(elem, tmp);
				pad_ptr++;
			}
			pad_ptr = dst.ptr(i, j);
			insertion(elem, *pad_ptr++);
			uchar tmp = pad_ptr[0]+1;
			insertion(elem, tmp);
			
			pic_ptr[0] = elem[0];
			MAX_SCALE = MAX_SCALE > pic_ptr[0]? MAX_SCALE:pic_ptr[0];
			pic_ptr--;
		}
		
	}
	
	// Scale
	for(int i = 0; i < dst.rows; i++) {
		pic_ptr = dst.ptr(i);
		for(int j = 0; j < dst.cols; j++) {
			*pic_ptr++ = pic_ptr[0]*255.0/MAX_SCALE;
		}
	}
	
	imshow("processed", dst);
	imwrite(output_file, dst);
	
	waitKey();
	
	return 0;
}
