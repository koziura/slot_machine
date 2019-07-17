#ifndef BMPIMAGE_H
#define BMPIMAGE_H

#include <baseimage.h>

struct BmpImage: BaseImage {
	BmpImage() {}

	~BmpImage() {
	}

	bool isValidHeader() {
		return header[0] == 'B' && header[1] == 'M';
	}

	bool isNull() {
		return data.get() == nullptr;
	}

	void flushDataFromHeader() {
		dataPos     = *((unsigned int*)&header[0x0A]);
		size        = *((unsigned int*)&header[0x22]);
		width       = *((unsigned int*)&header[0x12]);
		height      = *((unsigned int*)&header[0x16]);

		if (!size) {
			size = width * height * 3;
		}

		if (!dataPos) {
			dataPos = 54;
		}

		data.reset(new unsigned char[size]);
	}

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int size;
};

#endif // BMPIMAGE_H
