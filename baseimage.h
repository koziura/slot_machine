#ifndef BASEIMAGE_H
#define BASEIMAGE_H

struct BaseImage {
	BaseImage(): width(0), height(0), data(nullptr)	{
	}

	virtual ~BaseImage() {
	}

	const unsigned char* bits() {
		return data.get();
	}

	unsigned int width, height;
	std::shared_ptr<unsigned char> data;
};

#endif // BASEIMAGE_H
