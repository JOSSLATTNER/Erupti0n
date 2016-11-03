#include "Content\CFileloader.h"


void jpeg_error_exit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	jpeg_error_mgr* myerr = cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	//longjmp(myerr->setjmp_buffer, 1);
}

Erupti0n::CFileloader::CFileloader()
{
}

Erupti0n::CFileloader::~CFileloader()
{
}

std::vector<char> Erupti0n::CFileloader::GetBinaryFile(std::string & const a_rBinaryFileName)
{
	std::ifstream file(a_rBinaryFileName, std::ios::binary);
	if (file.fail())
	{
		LOG("[ERROR] Can't load file!");
		return std::vector<char>();
	}

	std::streampos start, end;

	start = file.tellg();
	file.seekg(0, std::ios::end);
	end = file.tellg();

	std::vector<char> buffer(end - start);
	file.seekg(0,std::ios::beg);
	file.read(&buffer[0], end - start);
	file.close();

	return buffer;
}

std::vector<float> Erupti0n::CFileloader::GetTexture(std::string& const a_rFilePath, uint32_t& a_rWidth, uint32_t& a_rHeight, uint32_t& a_rChannels)
{
	if (this->m_pTextureBuffer != nullptr)
		free(this->m_pTextureBuffer);

	FILE* inputFile;

	int rc;

	struct stat fileInfo {};
	unsigned long jpgSize;
	unsigned char* jpgBuffer;

	jpeg_decompress_struct jdI{};
	jpeg_error_mgr jerror{};

	if (stat(a_rFilePath.c_str(), &fileInfo))
	{
		LOG("[ERROR] Failed to load File stats!");
	}

	jpgSize = fileInfo.st_size;
	jpgBuffer = (unsigned char*)malloc(jpgSize + 100);
	fopen_s(&inputFile, a_rFilePath.c_str(), "rb");

	std::vector<char> binFile = this->GetBinaryFile(a_rFilePath);

	//memcpy(&jpgBuffer, &binFile[0], binFile.size());

	jdI.err = jpeg_std_error(&jerror);
	jdI.err->error_exit = jpeg_error_exit;


	jpeg_create_decompress(&jdI);
	//jdI.out_color_space = JCS_RGB;
	//
	//
	jpeg_stdio_src(&jdI, inputFile);
	jpeg_read_header(&jdI, TRUE);

	jdI.out_color_space = J_COLOR_SPACE::JCS_RGB;

	jpeg_start_decompress(&jdI);

	int x, y, channels, fileSize;
	unsigned int type;
	J_COLOR_SPACE space;

	x = jdI.output_width;
	y = jdI.output_height;
	channels = jdI.num_components;


	a_rWidth = x;
	a_rHeight = y;
	a_rChannels = channels;

	fileSize = x * y * channels *sizeof(float);

	unsigned char* buffer;
	unsigned char* rowPointer[1];

	this->m_pTextureBuffer = (unsigned char*)malloc(fileSize);

	while (jdI.output_scanline < jdI.output_height)
	{
		rowPointer[0] = (unsigned char*)this->m_pTextureBuffer + 3 * jdI.output_width * jdI.output_scanline;
		jpeg_read_scanlines(&jdI, (JSAMPARRAY)rowPointer, 1);
	}
	jpeg_finish_decompress(&jdI);

	std::vector<float> tmp(this->m_pTextureBuffer, this->m_pTextureBuffer + x*y*channels);

	for (int i = 0; i < tmp.size(); ++i)
	{
		tmp[i] = (float)tmp[i] / 255.0f;
	}
	return tmp;
}
