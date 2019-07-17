#include "system.h"
#include "modelview.h"
#include "bmpimage.h"
#include "tgaimage.h"

System::System(int* pargc, char** argv):
	m_view_ptr(new ModelView(pargc, argv))
{
}

System::~System()
{
	shutdown();
}

void System::setup()
{
}

void System::shutdown()
{
}

void System::run()
{
	m_view_ptr->loop();
}


bool System::loadBmpImage(const char *fileName, BaseImage* base_image)
{
	FILE* file;

	BmpImage* bmp = (BmpImage*)base_image;

	file = fopen(fileName, "rb");

	if (file == NULL)
	{
		//MessageBox(NULL, L"Error: Invaild file path!", L"Error", MB_OK);
		return false;
	}

	if (fread(bmp->header, 1, 54, file) != 54)
	{
		//MessageBox(NULL, L"Error: Invaild file!", L"Error", MB_OK);
		return false;
	}

	if (!bmp->isValidHeader())
	{
		//MessageBox(NULL, L"Error: Invaild file!", L"Error", MB_OK);
		return false;
	}

	bmp->flushDataFromHeader();

	fread(bmp->data.get(), 1, bmp->size, file);

	fclose(file);

	return true;
}

bool System::loadTgaImage(const char* filename, BaseImage* base_image)
{
	TgaImage* tga_image = (TgaImage*)base_image;

	FILE* filePtr;
	unsigned char ucharBad;
	short int sintBad;
	long imageSize;
	int colorMode;
	unsigned char colorSwap;

	// Open the TGA file.
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
	{
		return false;
	}

	// Read the two first bytes we don't need.
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

	// Which type of image gets stored in imageTypeCode.
	fread(&tga_image->type_code, sizeof(unsigned char), 1, filePtr);

	// For our purposes, the type code should be 2 (uncompressed RGB image)
	// or 3 (uncompressed black-and-white images).
	if (tga_image->type_code != 2 && tga_image->type_code != 3)
	{
		fclose(filePtr);
		return false;
	}

	// Read 13 bytes of data we don't need.
	fread(&sintBad, sizeof(short int), 1, filePtr);
	fread(&sintBad, sizeof(short int), 1, filePtr);
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
	fread(&sintBad, sizeof(short int), 1, filePtr);
	fread(&sintBad, sizeof(short int), 1, filePtr);

	// Read the image's width and height.
	fread(&base_image->width, sizeof(short int), 1, filePtr);
	fread(&base_image->height, sizeof(short int), 1, filePtr);

	// Read the bit depth.
	fread(&tga_image->bitCount, sizeof(unsigned char), 1, filePtr);

	// Read one byte of data we don't need.
	fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

	// Color mode -> 3 = BGR, 4 = BGRA.
	colorMode = tga_image->bitCount / 8;
	imageSize = tga_image->width * tga_image->height * colorMode;

	// Allocate memory for the image data.
	tga_image->data.reset(new unsigned char[sizeof(unsigned char) * imageSize]);

	// Read the image data.
	fread(tga_image->data.get(), sizeof(unsigned char), imageSize, filePtr);

	// Change from BGR to RGB so OpenGL can read the image data.
	for (int imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode)
	{
		colorSwap = tga_image->data.get()[imageIdx];
		tga_image->data.get()[imageIdx] = base_image->data.get()[imageIdx + 2];
		tga_image->data.get()[imageIdx + 2] = colorSwap;
	}

	fclose(filePtr);
	return true;
}

