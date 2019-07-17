#ifndef SINGLETON_H
#define SINGLETON_H

template<class T>
class Singleton
{
public:
	/*!
	 * \brief Singleton
	 */
	Singleton() {
		assert(singleton == nullptr);
		singleton = static_cast<T*>(this);
	}
	/*!
	 * \brief ~Singleton
	 */
	virtual ~Singleton() {
		singleton = nullptr;
	}
	/*!
	 * \brief instance
	 * \return
	 */
	static T& instance() {
		return *singleton;
	}
	/*!
	 * \brief instancePtr
	 * \return
	 */
	static T* instancePtr() {
		return singleton;
	}

protected:
	static T* singleton;
};
template<class T>
/*!
 * \brief Singleton<T>::singleton
 */
T* Singleton<T>::singleton = nullptr;

#endif // SINGLETON_H
