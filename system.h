#ifndef SYSTEM_H
#define SYSTEM_H

#include "singleton.h"

class ModelView;
struct BaseImage;

class System: public Singleton<System>
{
public:
	System(int* pargc, char** argv);
	~System();

	void setup();
	void run();

	/*!
	 * \brief loadTgaImage
	 * \param filename
	 * \param base_image
	 * \return
	 */
	bool loadTgaImage(const char* filename, BaseImage* base_image);
	/*!
	 * \brief loadBmpImage
	 * \param fileName
	 * \param base_image
	 * \return
	 */
	bool loadBmpImage(const char* fileName, BaseImage* base_image);

private:
	void shutdown();

private:
	std::shared_ptr<ModelView> m_view_ptr;
};

#endif // SYSTEM_H
