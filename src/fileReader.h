#pragma once

#include <string>
#include <fstream>
#include <iostream>

class FileReader
{
public:
	FileReader(std::string path);
	~FileReader();
	std::string getSource()
	{
		return m_source;
	}
	const char* getSourceChar()
	{
		return m_source.c_str();
	}
	int length()
	{
		return static_cast<int>(m_source.length());
	}
	bool isLoaded()
	{
		return m_loaded;
	}

private:

	void load(std::string path);
	bool m_loaded;
	int m_size;
	std::string m_path;
	std::string m_source;
};
