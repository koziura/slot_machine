#ifndef TGAIMAGE_H
#define TGAIMAGE_H

#include <fstream>
#include <baseimage.h>

struct TgaImage: BaseImage {
	unsigned char type_code;
	unsigned char bitCount;
};


#endif // TGAIMAGE_H
