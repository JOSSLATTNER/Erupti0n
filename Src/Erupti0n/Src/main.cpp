#include "CCore.h"
#include "CWindowWin32.h"

#include "Content\SCContentManager.h"
#include "Utilities\TSingleton.hpp"
#include "CMesh.h"

int main()
{



	std::vector<char> b = Erupti0n::TSingleton<Erupti0n::SCContentManager>::Instance()->LoadBinaryFile(std::string("D:\\TMP\\test.txt"));


	/*uint32_t width, height, channels;
	auto image = Erupti0n::TSingleton<Erupti0n::SCContentManager>::Instance()->LoadTexture(std::string("C:\\Users\\IO\\Pictures\\uvLayoutGrid.jpg"),width,height,channels);
*/
	Erupti0n::CCore core;

	core.RegisterWindow(1024, 1024, std::string("Erupti0n - v 0.0.1"));
	core.Run();

	return 0;
}