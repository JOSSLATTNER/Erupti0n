#pragma once

#include "Platform.h"
#include "Utilities.h"
#include "Utilities\TSingleton.hpp"

#include "vulkan\vulkan.h"
#include "glm\glm.hpp"
#include <assert.h>
#include <string>
#include <vector>
#include <iostream>

#define LOG(m) fprintf(stderr,"[Log] %s\n",m)

inline void VkAssert(VkResult aResult, const std::string& a_Message)
{
	if (VK_SUCCESS != aResult)
		std::cout << "[Error]" << a_Message << std::endl; 
}

//http://stackoverflow.com/questions/13389631/whats-a-good-hash-function-for-struct-with-3-unsigned-chars-and-an-int-for-uno
template <typename T>
std::size_t  HashFNV(const T& obj)
{
	const unsigned char* p = reinterpret_cast<const unsigned char*>(&obj);
	std::size_t h = 2166136261;

	for (unsigned int i = 0; i < sizeof(obj); ++i)
		h = (h * 16777619) ^ p[i];

	return h;
}