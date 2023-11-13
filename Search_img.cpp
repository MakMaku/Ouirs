// Search_img.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <time.h>

using namespace std;
using namespace cv;

//Поиск координат эталона
vector<int> Search_Kmad(Mat C_y, Mat R_y) {
	Mat Kmad(C_y.rows - R_y.rows, C_y.cols - R_y.cols, CV_64F);
	double sum = 0, m_R = mean(R_y)(0), m_C = mean(C_y)(0);

	//Расчет корреляционной матрицы
	for (int di = 0; di < (C_y.rows - R_y.rows); di++)
		for (int dj = 0; dj < (C_y.cols - R_y.cols); dj++) {
			for (int i = 0; i < R_y.rows; i += 2)
				for (int j = 0; j < R_y.cols; j += 2) {
					//sum += abs(R_y.at<uchar>(i, j) - C_y.at<uchar>(i + di, j + dj));
					sum += (R_y.at<uchar>(i, j) - m_R) * (C_y.at<uchar>(i + di, j + dj) - m_C);
				}
			Kmad.at<double>(di, dj) = sum / (C_y.cols * C_y.rows);
			sum = 0;
		}

	sum = Kmad.at<double>(0, 0);
	int i_out = 0, j_out = 0;
	//Нахождение максимума корреляционной матрицы
	for (int i = 0; i < Kmad.rows; i++)
		for (int j = 0; j < Kmad.cols; j++) {
			if (sum < Kmad.at<double>(i, j)) {
				sum = Kmad.at<double>(i, j);
				i_out = i;
				j_out = j;
			}
		}

	return { i_out, j_out };
}

vector<int> Looking(Mat C_y, Mat R_y, vector<int> coord) {
	Mat Kmad(3 * R_y.rows, 3 * R_y.cols, CV_64F);

	double sum = 0, m_R = mean(R_y)(0), m_C = mean(C_y)(0);

	int di0 = coord[0] - ceil(1.5 * R_y.rows); di0 < 0 ? 0 : di0;
	int dik = coord[0] + ceil(1.5 * R_y.rows); dik >= C_y.rows ? C_y.rows : dik;
	int dj0 = coord[1] - ceil(1.5 * R_y.cols); dj0 < 0 ? 0 : dj0;
	int djk = coord[1] + ceil(1.5 * R_y.cols); djk >= C_y.cols ? C_y.cols : djk;

	//Расчет корреляционной матрицы
	for (int di = di0; di < dik; di++)
		for (int dj = dj0; dj < djk; dj++) {
			for (int i = 0; i < R_y.rows; i += 2)
				for (int j = 0; j < R_y.cols; j += 2) {
					//sum += abs(R_y.at<uchar>(i, j) - C_y.at<uchar>(i + di, j + dj));
					sum += (R_y.at<uchar>(i, j) - m_R) * (C_y.at<uchar>(i + di, j + dj) - m_C);
				}
			Kmad.at<double>(di - di0, dj - dj0) = sum / (C_y.cols * C_y.rows);
			sum = 0;
		}
	sum = Kmad.at<double>(0, 0);
	int i_out = di0, j_out = dj0;

	//Нахождение максимума корреляционной матрицы
	for (int i = 0; i < Kmad.rows; i++)
		for (int j = 0; j < Kmad.cols; j++) {
			if (sum < Kmad.at<double>(i, j)) {
				sum = Kmad.at<double>(i, j);
				i_out = i + di0;
				j_out = j + dj0;
			}
		}

	return { i_out, j_out };
}

int main() {
	//cv::namedWindow("Example 2-3", cv::WINDOW_AUTOSIZE);
	//Открываем видео
	cv::VideoCapture cap;
	cap.open("try1 - Trim_cut.mp4");
	if (cap.isOpened() == 0){
		cout << "The video file cannot be opened." << endl;
		return -1;
	}
	cv::Mat frame;

	//Получаем первый кадр и показываем его
	cap >> frame;
	//resize(frame, frame, Size(700, 500));

	string source = "Boloto.png";
	Mat R = imread(source, IMREAD_COLOR), R_yrb, R_y, C_yrb, C_y, trying;
	//Изменяем цветовую схему и выводим канал яркости
	cvtColor(R, R_yrb, COLOR_BGR2YCrCb);
	extractChannel(R_yrb, R_y, 0);

	cvtColor(frame, C_yrb, COLOR_BGR2YCrCb);
	extractChannel(C_yrb, C_y, 0);

	clock_t tStart = clock(), tEnd = clock(), tFrame, tBegin = clock();
	vector<int> coord = Search_Kmad(C_y, R_y);
	cout << "\nTime taken:\t" << (double)(clock() - tStart) / CLOCKS_PER_SEC << endl;

	Rect rect(coord[1], coord[0], R_y.cols, R_y.rows);

	rectangle(frame, rect, Scalar(0, 255, 0));

	imshow("Example", frame);
	int count = 0;
	while (true) {
		tFrame = clock();
		cap >> frame;
		if (frame.empty()) break; // Ran out of film
		//resize(frame, frame, Size(700, 500));

		if ((double)(tEnd - tStart) >= 100) {

			cvtColor(frame, C_yrb, COLOR_BGR2YCrCb);
			extractChannel(C_yrb, C_y, 0);

			coord = Looking(C_y, R_y, coord);
			rect = Rect(coord[1], coord[0], R_y.cols, R_y.rows);
			tStart = clock();
		}

		rectangle(frame, rect, Scalar(0, 255, 0));
		cv::imshow("Example", frame);
		tEnd = clock();
		if ((char)cv::waitKey(1) >= 0) break;
		cout << "\nFrame duration:\t" << (double)(clock() - tFrame) / CLOCKS_PER_SEC << endl;
	}

	cout << "\nAlgoritm duration:\t" << (double)(clock() - tBegin) / CLOCKS_PER_SEC << endl;
	waitKey(0);
	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
