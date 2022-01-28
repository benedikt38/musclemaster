
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <src/nlohmann/json.hpp>

enum class SarcomereType
{
	TWO_TO_ONE = 0,
	THREE_TO_ONE = 1,
	FIVE_TO_ONE = 2,
	SIX_TO_ONE = 3
};


class Sarcomere
{
public:
	Sarcomere(SarcomereType type, float d10, float actinLength, int numMyosinRods = 500, glm::vec4 sarcomereMidPoint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	Sarcomere(const char* filepath);
	std::vector<glm::vec4> getActinRods();
	std::vector<glm::vec4> getActinParticles();
	std::vector<glm::vec4> getMyosinRods();
	int getNumActin();
	int getNumActinParticles();
	int getNumTroponinParticles();
	int getNumMyosin();
	int getNumMyosinHeads();
	int getNumZdiscs();
	int getCycleCount();
	int getNumPointsPerLMMHelix();
	int getNumPointsPerHMMHelix();
	int getNumLMMOffsetPositionsPerRod();
	int getNumHMMOffsetPositionsPerRod();
	glm::vec3 getActinColor();
	glm::vec3 getTropomyosinColor();
	glm::vec3 getTroponinColor();
	glm::vec3 getMyosinColor();
	glm::vec3 getLMMColor();
	glm::vec3 getHMMColor();
	glm::vec3 getMyosinHeadColor();
	glm::mat4 getHMMRotMat();
	void setActinColor(glm::vec3 color);
	void setTropomyosinColor(glm::vec3 color);
	void setTroponinColor(glm::vec3 color);
	void setMyosinColor(glm::vec3 color);
	void setLMMColor(glm::vec3 color);
	void setHMMColor(glm::vec3 color);
	void setMyosinHeadColor(glm::vec3 color);
	float getRadius();
	int getNumLineSegments();
	glm::vec4 getMidPoint();
	float getVolume();
	SarcomereType getSarcomereType();
	void updateVolume();
	void updateD10();
	void updateD11();
	void updateHMMLength();
	void update_dMyosin();
	void update_dActin();
	void updateRadius();
	void updateLength();
	void updateHMMAngle();
	void updateOffsetBuffers();

	void generateDoubleHelixOffsetPositions();

	void genLMM();

	void bindTropomyosinBuffer();

	void genLMM1Buffer();

	void genLMM2Buffer();

	void genHMM1Buffer();

	void genHMM2Buffer();

	void bindLMM1Buffer();

	void bindLMM2Buffer();

	void bindHMM1Buffer();

	void bindHMM2Buffer();

	void deserialize(const char* filePath);

	nlohmann::json serialize() const;

	void genHMM();

	void genLMMOffsetPositions();

	void genHMMOffsetPositions(float scaleFactor);

	void genMyosinHeads();

	void genTropomyosinBuffer();

	void bindBuffers();

	void genMyosinRods(glm::vec4 sarcomereMidPoint, float d10, int& cycleCount);
	void genActinRods(SarcomereType Type, glm::vec4 sarcomereMidPoint, float d10, int& cycleCount);

	float sarcomereLength;
	float oldSarcomereLength;
	float oldSarcomereRadius;
	float d10;
	float oldD10;
	float myosinLength;
	float oldMyosinLength;
	float myosinRadius;
	float oldMyosinRadius;
	float myosinTrunkRadius;
	float myosinHeadRadius;
	float actinLength;
	float oldActinLength;
	float actinRadius;
	float oldActinRadius;
	float myosinLengthScalePercentage;
	float myosinRadiusScalePercentage;
	float actinLengthScalePercentage;
	float actinRadiusScalePercentage;
	int numParticles;
	bool myosinIsGenerated = false;
	//imgui checkbox parameter
	bool konserveVolume;
	bool highResActin;
	bool highResMyosin;
	bool actin;
	bool actinMonomers;
	bool tropomyosin;
	bool troponin;
	bool myosin;
	bool myosinTrunk;
	bool LMM;
	bool HMM;
	bool myosinHeads;
	bool halfHelix;

private:
	std::vector<glm::vec4> m_actinRods;
	std::vector<glm::vec4> m_myosinRods;
	std::vector<glm::vec4> m_zOffset;
	std::vector<glm::vec4> m_actinParticlePositions;
	std::vector<glm::vec4> m_LMMPositions1;
	std::vector<glm::vec4> m_LMMPositions2;
	std::vector<glm::vec4> m_HMMPositions1;
	std::vector<glm::vec4> m_HMMPositions2;
	std::vector<glm::vec4> m_myosinHeadOffsetPositions;
	std::vector<glm::mat4> m_HMMyRotMats;
	std::vector<glm::vec4> m_LMMOffsetPositions;
	std::vector<glm::vec4> m_HMMOffsetPositions;
	std::vector<glm::vec4> m_tropomyosinPositions;
	std::vector<glm::vec4> m_troponinPositions;
	std::vector<glm::mat4> m_lineRotMatricees;
	std::vector<glm::mat4> m_HMMRotMatrices;
	std::vector<glm::mat4> m_HMMyRotMatrices2;
	std::vector<float> m_HMMAngles;
	//colors
	glm::vec3 m_actinColor;
	glm::vec3 m_myosinColor;
	glm::vec3 m_myosinHeadColor;
	glm::vec3 m_tropomyosinColor;
	glm::vec3 m_troponinColor;
	glm::vec3 m_LMMColor;
	glm::vec3 m_HMMColor;
	void genBuffers();
	int m_numMyosinRods;
	int m_cycleCount = 1;
	glm::vec4 sarcomereMidPoint;
	float m_dActin;//distance between two actin filaments
	float m_dMyosin; //distance between two myosin filaments
	float m_d11;
	float m_sarcomereVolume;
	float m_sarcomereRadius;
	float m_LMMRadius;
	float m_LMMyOffset = 0.0145f;
	float m_LMMLength;
	float m_HMMLength;
	float m_HMMLength1;
	float m_HMMLength2;
	float m_HMMLength3;
	float m_lengthUnderHMM1;
	float m_lengthUnderHMM2;
	float m_lengthUnderHMM3;
	float m_HMMAngle;
	float m_scaledAngle;
	bool m_invertAngle1 = false;
	bool m_invertAngle2 = false;
	bool m_invertAngle3 = false;
	glm::mat4 m_HMMRotMat;
	SarcomereType m_type;
	GLuint m_zDisc_ssbo;
	GLuint m_mRod_ssbo;
	GLuint m_aRod_ssbo;
	GLuint m_aSphere_ssbo;
	GLuint m_troponin_ssbo;
	GLuint m_lineMatricees_ssbo;
	GLuint m_LMMOffsetPositions_ssbo;
	GLuint m_HMMOffsetPositions_ssbo;
	GLuint m_HMMyRotationMatricees_ssbo;
	GLuint m_HMMyRotationMatrices2_ssbo;
	GLuint m_HMMzRotationMatrices_ssbo;
	GLuint m_myosinHeadOffsetPositions_ssbo;
	GLuint m_linebuffer;
	GLuint m_LMM1buffer;
	GLuint m_LMM2buffer;
	GLuint m_HMM1buffer;
	GLuint m_HMM2buffer;
	GLuint m_vao;
	GLuint m_vao2;
	GLuint m_vao3;
	GLuint m_vao4;
	GLuint m_vao5;
};