
#include "BarcodeFormat.h"
#include "MultiFormatWriter.h"
#include "BitMatrix.h"
#include "TextUtfEncoding.h"
// #include "ZXStrConvWorkaround.h"
#include "lodepng.h"
#include "generate_qrcode.h"
#include "generate_qrbmp.h"

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <stdexcept>

using namespace ZXing;

// int create_qrcode(char *text, char *filePath, int w, int ecc)
// {
// 	int width = w, height = w;
// 	int margin = 10;
// 	int eccLevel = ecc;
// 	std::string qr_text = text;
// 	std::string qr_filePath = filePath;

// 	try
// 	{
// 		MultiFormatWriter writer(BarcodeFormat::QR_CODE);
// 		if (margin >= 0)
// 			writer.setMargin(margin);
// 		if (eccLevel >= 0)
// 			writer.setEccLevel(eccLevel);

// 		auto matrix = writer.encode(TextUtfEncoding::FromUtf8(qr_text), width, height);

// 		std::vector<unsigned char> buffer(matrix.width() * matrix.height(), '\0');
// 		unsigned char black = 0;
// 		unsigned char white = 255;
// 		for (int y = 0; y < matrix.height(); ++y)
// 		{
// 			for (int x = 0; x < matrix.width(); ++x)
// 			{
// 				buffer[y * matrix.width() + x] = matrix.get(x, y) ? black : white;
// 			}
// 		}

// 		unsigned error = BmpUtils::saveQrcode(qr_filePath.c_str(), &buffer[0], matrix.width(), matrix.height(), 16);
// 		if (error)
// 		{
// 			std::cout << "saveQrcode fail." << std::endl;
// 			return -1;
// 		}
// 		// unsigned error = lodepng::encode(qr_filePath, buffer, matrix.width(), matrix.height(), LCT_GREY);
// 		// if (error) {
// 		// 	std::cerr << "Error: " << lodepng_error_text(error) << std::endl
// 		// 		<< "Failed to write image: " << qr_filePath << std::endl;
// 		// 	return -1;
// 		// }
// 	}
// 	catch (const std::exception &e)
// 	{
// 		std::cerr << e.what() << std::endl;
// 		return -1;
// 	}

// 	return 0;
// }

int create_qrcode(char *text, char *filePath, int w, int ecc)
{
	unsigned char *data = new unsigned char[w * w];
	qrcode_create(text, w, ecc, (char *)data);

	unsigned char error = QrBmpUtils::saveQrcodeBmp(filePath, data, w, w, 16);
	if (error)
	{
		std::cout << "saveQrcode fail." << std::endl;
		return -1;
	}

	return 0;
}

int qrcode_create(char *text, int w, int ecc, char *data)
{
	int width = w, height = w;
	int margin = 10;
	int eccLevel = ecc;
	std::string qr_text = text;

	if (data == NULL)
	{
		std::cout << "data null" << std::endl;
		return -1;
	}

	try
	{
		MultiFormatWriter writer(BarcodeFormat::QR_CODE);
		if (margin >= 0)
			writer.setMargin(margin);
		if (eccLevel >= 0)
			writer.setEccLevel(eccLevel);

		auto matrix = writer.encode(TextUtfEncoding::FromUtf8(qr_text), width, height);

		std::vector<unsigned char> buffer(matrix.width() * matrix.height(), '\0');
		unsigned char black = 0;
		unsigned char white = 255;
		for (int y = 0; y < matrix.height(); ++y)
		{
			for (int x = 0; x < matrix.width(); ++x)
			{
				buffer[y * matrix.width() + x] = matrix.get(x, y) ? black : white;
			}
		}
		std::copy(buffer.begin(), buffer.end(), data);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
