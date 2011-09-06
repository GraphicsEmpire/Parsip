#ifndef __singleton_h
#define __singleton_h

#include <cassert>

namespace PS
{
	namespace BASE
	{
		/**
		 *	@class	Singleton
		 *	@brief	basic Singleton class
		 */
		template <typename T>
		class Singleton
		{
		protected:
			
			static T* mInstance; /**< Instance of Singleton type. */
			
		public:
			
			/**
			 *	static method for returning the instance
			 *	@return T& instance of Singleton
			 */
			static T& instance ()
			{
				if (!mInstance)
				{
					mInstance = new T ();
#ifdef _WIN32
					std::atexit(destroy);
#else
					atexit(destroy);
#endif
				}
				return *(mInstance);
			}
			
			/**
			 *	destroys the singleton
			 */
			static void destroy()
			{
				if (mInstance != NULL)
				{
					delete mInstance;
					mInstance = NULL;
				}
			}
			
			/**
			 *	destructor
			 */
			virtual	~Singleton ()
			{
			}
			
		protected:
			
			/**
			 *	standard constructor
			 */
			Singleton () { }
		};
		
		template <typename T> T* Singleton <T>::mInstance = 0;
	}
}

#endif // __singleton_h
