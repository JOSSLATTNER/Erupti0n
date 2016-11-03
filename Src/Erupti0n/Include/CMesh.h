#pragma once
#include "Shared.h"
#include "CGraphics.h"

namespace Erupti0n
{
	struct CMesh
	{
		std::vector<float> m_VertexPosition;
		std::vector<float> m_VertexColor;
		std::vector<float> m_VertexUv;
		std::vector<float> m_VertexNormal;
		std::vector<uint16_t> m_VertexIndices;
	};
}