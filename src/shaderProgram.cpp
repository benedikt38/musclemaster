#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const char* vertexpath, const char* fragmentpath)
{
	GLuint vertexShader = createShader(vertexpath, GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader(fragmentpath, GL_FRAGMENT_SHADER);
	m_program = glCreateProgram();
	glAttachShader(m_program, vertexShader);
	glAttachShader(m_program, fragmentShader);
	glLinkProgram(m_program);
	checkProgramStatus(m_program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	m_vertexPath = vertexpath;
	m_fragmentPath = fragmentpath;
}

ShaderProgram::ShaderProgram(const char* vertexpath, const char* fragmentpath, const char* geometrypath)
{
	GLuint vertexShader = createShader(vertexpath, GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader(fragmentpath, GL_FRAGMENT_SHADER);
	GLuint geometryShader = createShader(geometrypath, GL_GEOMETRY_SHADER);
	m_program = glCreateProgram();
	glAttachShader(m_program, vertexShader);
	glAttachShader(m_program, fragmentShader);
	glAttachShader(m_program, geometryShader);
	glLinkProgram(m_program);
	checkProgramStatus(m_program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(geometryShader);
	m_vertexPath = vertexpath;
	m_fragmentPath = fragmentpath;
	m_geometryPath = geometrypath;
}

ShaderProgram::ShaderProgram(const char * comutepath)
{
	GLuint computeShader = createShader(comutepath, GL_COMPUTE_SHADER);
	m_program = glCreateProgram();
	glAttachShader(m_program, computeShader);
	glLinkProgram(m_program);
	checkProgramStatus(m_program);
	glDeleteShader(computeShader);
}

ShaderProgram::~ShaderProgram()
{
	glUseProgram(0);
	glDeleteProgram(m_program);
}

void ShaderProgram::use()
{
	glUseProgram(m_program);
}

GLuint ShaderProgram::getProgram()
{
	return m_program;
}

void ShaderProgram::reload(int type)
{
	if (type == 0)
	{
		GLuint vertexShader = createShader(m_vertexPath, GL_VERTEX_SHADER);
		GLuint fragmentShader = createShader(m_fragmentPath, GL_FRAGMENT_SHADER);
		glAttachShader(m_program, vertexShader);
		glAttachShader(m_program, fragmentShader);
		glLinkProgram(m_program);
		checkProgramStatus(m_program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	else if(type == 1)
	{
		GLuint vertexShader = createShader(m_vertexPath, GL_VERTEX_SHADER);
		GLuint fragmentShader = createShader(m_fragmentPath, GL_FRAGMENT_SHADER);
		GLuint geometryShader = createShader(m_geometryPath, GL_GEOMETRY_SHADER);
		glAttachShader(m_program, vertexShader);
		glAttachShader(m_program, fragmentShader);
		glAttachShader(m_program, geometryShader);
		glLinkProgram(m_program);
		checkProgramStatus(m_program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteShader(geometryShader);
	}
}

GLuint ShaderProgram::createShader(const char* path, GLenum type)
{
	GLuint shaderID = glCreateShader(type);
	loadFromSource(path, shaderID);
	glCompileShader(shaderID);
	checkShaderStatus(shaderID);
	return shaderID;

}

void ShaderProgram::loadFromSource(const char* path, GLuint shaderID) {
	FileReader fr(path);
	const char* shaderSource = fr.getSourceChar();
	GLint sourceSize = fr.length();
	glShaderSource(shaderID, 1, &shaderSource, &sourceSize);
}


void ShaderProgram::updateUniform(const GLchar* name, glm::mat4 m)
{
	GLint loc = findUniform(name);
	glUseProgram(m_program);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void ShaderProgram::updateUniform(const GLchar* name, glm::vec4 v)
{
	GLint loc = findUniform(name);
	glUseProgram(m_program);
	glUniform4fv(loc, 1, glm::value_ptr(v));
}

void ShaderProgram::updateUniform(const GLchar* name, glm::vec3 v)
{
	GLint loc = findUniform(name);
	glUseProgram(m_program);
	glUniform3fv(loc, 1, glm::value_ptr(v));
}

void ShaderProgram::updateUniform(const GLchar* name, float f)
{
	GLint loc = findUniform(name);
	glUseProgram(m_program);
	glUniform1f(loc, f);
}

void ShaderProgram::updateUniform(const GLchar* name, int i)
{
	GLint loc = findUniform(name);
	glUseProgram(m_program);
	glUniform1i(loc, i);
}

GLint ShaderProgram::findUniform(const GLchar* name)
{
	GLint loc = glGetUniformLocation(m_program, name);
	if (loc == -1) {
		std::cout << "uniformloc not found " << name<< std::endl;
	}
	return loc;
}

void ShaderProgram::checkShaderStatus(GLuint shaderID)
{
	GLint status;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* infoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shaderID, infoLogLength, NULL, infoLog);
		std::cout << "FAIL: Compiling shader program.\n" << infoLog << '\n';
		delete[] infoLog;
	}
	else 
	{
		std::cout << "SUCCESS: Compiling shader." << std::endl;
	}
}

void ShaderProgram::checkProgramStatus(GLuint programID)
{
	GLint status;
	glGetProgramiv(programID, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* infoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(programID, infoLogLength, NULL, infoLog);
		std::cout << "FAIL: Compiling shader program.\n" << infoLog << '\n';
		delete[] infoLog;
	}
	else 
	{
		std::cout << "SUCCESS: Compiling shader program." << std::endl;
	}
}