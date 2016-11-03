#pragma once
#include "Shared.h"
#include <iostream>    
#include <fstream>
#include "Utilities\LibjpegHelper.h"

namespace Erupti0n
{

	class CFileloader
	{
	private:
		unsigned char* m_pTextureBuffer = nullptr;

	public:
		CFileloader();
		~CFileloader();

	public:
		std::vector<char> GetBinaryFile(std::string & const a_rBinaryFileName);
		std::vector<float> GetTexture(std::string& const a_rFilePath, uint32_t& a_rWidth, uint32_t& a_rHeight, uint32_t& a_rChannels);
	};
}