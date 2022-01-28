#include "FileReader.h"

FileReader::FileReader(std::string path)
{
	m_path = path;
	load(path);
}

FileReader::~FileReader()
{

}

void FileReader::load(std::string path)
{
	m_loaded = false;
	std::ifstream file(path);
	if (file.is_open()) {
		m_source = std::string(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
		m_loaded = true;
		m_size = m_source.size();
	}
	else {
		std::cout << "fileNotFoundException" << std::endl;
	}
}
