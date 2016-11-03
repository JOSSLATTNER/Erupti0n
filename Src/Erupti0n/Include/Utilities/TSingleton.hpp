#pragma once


namespace Erupti0n
{
	template<typename T>
	class TSingleton
	{
	private:
		static TSingleton<T>* pInstance;
		T* m_pTInstance;

	private:
		TSingleton() { m_pTInstance = new T; };
		TSingleton(const TSingleton& a_rTSingleton);
		TSingleton(const TSingleton&& a_rTSingleton);
		TSingleton& operator= (const TSingleton& a_rTSingleton);
		TSingleton& operator= (const TSingleton&& a_rrTSingleton);
		~TSingleton() { if (pInstance)delete m_pTInstance; };


	public:
		static T* Instance();
	};

	template<typename T>
	T* TSingleton<T>::Instance()
	{
		if (!pInstance)
			pInstance = new TSingleton();
		return pInstance->m_pTInstance;
	}

	template<typename T>
	TSingleton<T>* TSingleton<T>::pInstance = nullptr;
}