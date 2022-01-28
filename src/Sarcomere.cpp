#include "Sarcomere.h"
#include <src/tinyfiledialogs.h>
#include <fstream>
#include <filesystem>
#include <cmath>

Sarcomere::Sarcomere(SarcomereType type, float d10_in, float actinLength_in, int numMyosinRods, glm::vec4 sarcomereMidPoint_in)
{
	m_numMyosinRods = numMyosinRods;
	sarcomereMidPoint = sarcomereMidPoint_in;
	d10 = d10_in; //36 - 38 in a frog muscle according to Srboljub M. Mijailovich, Oliver Kayser-Herold, Boban Stojanovic, Djordje Nedic, Thomas C. Irving, Michael A. Geeves
	m_d11 = d10 / sqrt(3.0f);
	actinRadiusScalePercentage = 0.095f;
	actinLength = actinLength_in;
	sarcomereLength = 2.0f * actinLength;
	actinRadius = actinRadiusScalePercentage * d10;   // 3.5 nm according to Matsubaraand Elliott(1972) and Millman(1998),
	myosinLengthScalePercentage = 0.8f;
	myosinRadiusScalePercentage = 0.212f;  //0.43 for 15 nm radius, 0,23 for 15 nm diameter
	myosinLength = myosinLengthScalePercentage * 2.0f * actinLength;
	myosinRadius = myosinRadiusScalePercentage * d10;  // 7.8 nm according to Matsubaraand Elliott(1972) and Millman(1998),
	myosinTrunkRadius = myosinRadius / 3.0f;
	myosinHeadRadius = myosinRadius / 6.0f;
	m_LMMLength = 0.09f;
	m_HMMLength = 0.06f;
	m_HMMLength1 = 0.06f;
	m_HMMLength2 = 0.06f;
	m_HMMLength3 = 0.06f;
	//acos sinus von (myosin_trunk_diameter - myosin_head_radius_y) / (HMM_length + myosin_head_radius_x)
	m_HMMAngle = glm::asin((myosinTrunkRadius * 2.0f - (myosinHeadRadius * 2.0f)) / (m_HMMLength + (myosinHeadRadius)));
	m_scaledAngle = m_HMMAngle;
	m_HMMRotMat = glm::rotate(glm::mat4(1.0f), m_HMMAngle, glm::vec3(0.0f, 0.0f, -1.0f));
	m_lengthUnderHMM1 = glm::sqrt(glm::pow(m_HMMLength, 2) - glm::pow(((2.0f * m_d11) / sqrt(3.0f) - myosinRadius / 3.0f - actinRadius), 2));
	m_lengthUnderHMM2 = glm::sqrt(glm::pow(m_HMMLength, 2) - glm::pow((m_d11 - myosinRadius / 3.0f - actinRadius), 2));
	m_lengthUnderHMM3 = glm::sqrt(glm::pow(m_HMMLength, 2) - glm::pow(((m_d11 / glm::cos(glm::radians(15.0f))) - myosinRadius / 3.0f - actinRadius), 2));
	update_dMyosin();
	m_cycleCount = 1;
	genMyosinRods(sarcomereMidPoint, d10, m_cycleCount);
	m_type = type;

	//colors
	m_actinColor = glm::vec3(0.0f, 0.4f, 0.0f);
	m_myosinColor = glm::vec3(1.0f, 0.0f, 0.0f);
	m_myosinHeadColor = glm::vec3(0.6f, 0.2f, 0.2f);
	m_tropomyosinColor = glm::vec3(1.0f, 0.4f, 0.0f);
	m_troponinColor = glm::vec3(0.0f, 0.0f, 0.5f);
	m_LMMColor = glm::vec3(0.2f, 0.5f, 0.5f);
	m_HMMColor = glm::vec3(0.5f, 0.0f, 0.5f);

	update_dActin();
	updateRadius();
	updateVolume();
	genActinRods(type, sarcomereMidPoint, d10, m_cycleCount);

	oldSarcomereLength = sarcomereLength;
	oldD10 = d10;
	oldMyosinLength = myosinLength;
	oldMyosinRadius = myosinRadius;
	oldActinLength = actinLength;
	oldActinRadius = actinRadius;

	//generate zDisks
	m_zOffset.clear();
	m_zOffset.push_back(glm::vec4(0.0f, -0.001f, 0.0f, 0.0f));
	m_zOffset.push_back(glm::vec4(0.0f, 0.001f, 0.0f, 0.0f));

	genBuffers();
}

Sarcomere::Sarcomere(const char* filepath)
{
	deserialize(filepath);
	m_d11 = d10 / sqrt(3.0f);
	actinLengthScalePercentage = actinLength / sarcomereLength;
	actinRadiusScalePercentage = actinRadius / d10;
	myosinLengthScalePercentage = myosinLength / sarcomereLength;
	myosinRadiusScalePercentage = myosinRadius / d10;
	myosinTrunkRadius = myosinRadius / 3.0f;
	m_LMMLength = 0.09f;
	m_HMMLength = 0.06f;
	m_HMMAngle = glm::asin((myosinTrunkRadius * 2.0f - (myosinRadius / 3.0f)) / (m_HMMLength + (myosinRadius / 3.0f)));
	m_HMMRotMat = glm::rotate(glm::mat4(1.0f), m_HMMAngle, glm::vec3(0.0f, 0.0f, -1.0f));
	update_dMyosin();
	m_cycleCount = 1;
	genMyosinRods(sarcomereMidPoint, d10, m_cycleCount);

	update_dActin();
	updateRadius();
	updateVolume();
	genActinRods(m_type, sarcomereMidPoint, d10, m_cycleCount);

	m_zOffset.clear();
	m_zOffset.push_back(glm::vec4(0.0f, -0.001f, 0.0f, 0.0f));
	m_zOffset.push_back(glm::vec4(0.0f, 0.001f, 0.0f, 0.0f));

	generateDoubleHelixOffsetPositions();
	genLMM();
	genBuffers();

	oldSarcomereLength = sarcomereLength;
	oldD10 = d10;
	oldMyosinLength = myosinLength;
	oldMyosinRadius = myosinRadius;
	oldActinLength = actinLength;
	oldActinRadius = actinRadius;
}

std::vector<glm::vec4> Sarcomere::getActinRods()
{
	return m_actinRods;
}

std::vector<glm::vec4> Sarcomere::getMyosinRods()
{
	return m_myosinRods;
}

int Sarcomere::getNumActin()
{
	return static_cast<int>(m_actinRods.size());
}

int Sarcomere::getNumMyosin()
{
	return static_cast<int>(m_myosinRods.size());
}

int Sarcomere::getNumMyosinHeads()
{
	return static_cast<int>(m_myosinHeadOffsetPositions.size());
}

int Sarcomere::getNumZdiscs()
{
	return static_cast<int>(m_zOffset.size());;
}

int Sarcomere::getCycleCount()
{
	return m_cycleCount;
}

int Sarcomere::getNumPointsPerLMMHelix()
{
	return m_LMMPositions1.size();
}

int Sarcomere::getNumPointsPerHMMHelix()
{
	return m_HMMPositions1.size();
}

int Sarcomere::getNumLMMOffsetPositionsPerRod()
{
	return m_LMMOffsetPositions.size();
}

int Sarcomere::getNumHMMOffsetPositionsPerRod()
{
	return m_HMMOffsetPositions.size();
}
glm::vec3 Sarcomere::getActinColor()
{
	return m_actinColor;
}
glm::vec3 Sarcomere::getTropomyosinColor()
{
	return m_tropomyosinColor;
}
glm::vec3 Sarcomere::getTroponinColor()
{
	return m_troponinColor;
}
glm::vec3 Sarcomere::getMyosinColor()
{
	return m_myosinColor;
}
glm::vec3 Sarcomere::getLMMColor()
{
	return m_LMMColor;
}
glm::vec3 Sarcomere::getHMMColor()
{
	return m_HMMColor;
}
glm::vec3 Sarcomere::getMyosinHeadColor()
{
	return m_myosinHeadColor;
}
glm::mat4 Sarcomere::getHMMRotMat()
{
	return m_HMMRotMat;
}
void Sarcomere::setActinColor(glm::vec3 color)
{
	m_actinColor = color;
}
void Sarcomere::setTropomyosinColor(glm::vec3 color)
{
	m_tropomyosinColor = color;
}
void Sarcomere::setTroponinColor(glm::vec3 color)
{
	m_troponinColor = color;
}
void Sarcomere::setMyosinColor(glm::vec3 color)
{
	m_myosinColor = color;
}
void Sarcomere::setLMMColor(glm::vec3 color)
{
	m_LMMColor = color;
}
void Sarcomere::setHMMColor(glm::vec3 color)
{
	m_HMMColor = color;
}
void Sarcomere::setMyosinHeadColor(glm::vec3 color)
{
	m_myosinHeadColor = color;
}
float Sarcomere::getRadius()
{
	return m_cycleCount * 2.0f* m_d11;
	;
}

int Sarcomere::getNumLineSegments()
{
	return static_cast<int>(m_lineRotMatricees.size());
}

glm::vec4 Sarcomere::getMidPoint()
{
	return sarcomereMidPoint;
}

float Sarcomere::getVolume()
{
	return m_sarcomereVolume;
}

SarcomereType Sarcomere::getSarcomereType()
{
	return m_type;
}

void Sarcomere::genMyosinRods(glm::vec4 sarcomereMidPoint, float d10, int& cycleCount)
{
	std::vector<glm::vec4> mRods;
	std::vector<float> mRodAngles;
	// middleRod
	mRods.push_back(sarcomereMidPoint);
	int rodPerCycle = 6;
	//int cycleCount = 1;
	int middleCount = 0;
	glm::vec3 up = glm::vec3(1.0f, 0.0f, 0.0f);
	while (mRods.size() < m_numMyosinRods)
	{
		for (int i = 0; i < rodPerCycle; i++)
		{
			float angle = glm::radians(360.0f / rodPerCycle);
			glm::vec4 newPoint = sarcomereMidPoint;
			newPoint = newPoint + glm::vec4(m_dMyosin * cycleCount, 0.0f, 0.0f, 0.0f);
			newPoint = glm::rotate(newPoint, angle * i, glm::vec3(0.0f, 1.0f, 0.0f));
			mRods.push_back(newPoint);
			//compute angle
			glm::vec3 p = glm::normalize(glm::vec3(newPoint - sarcomereMidPoint));
			float dot = glm::dot(glm::vec2(p.x, p.z), glm::vec2(up.x, up.z));
			float det = p.x * up.z - p.z * up.x;
			float alpha = std::atan2(det, dot);
			if (alpha < 0.0f)
			{
				alpha = 2.0f * glm::pi<float>() + alpha;
			}
			mRodAngles.push_back(glm::degrees(alpha));
		}
		if (cycleCount >= 2)
		{
			int size = static_cast<int>(mRods.size() - 1);
			int k = static_cast<int>(mRods.size() - rodPerCycle);
			for (int i = 0; i < rodPerCycle; i++)
			{
				glm::vec4 a = mRods.at((size - i - k + rodPerCycle) % rodPerCycle + k);
				glm::vec4 b = mRods.at((size - i - 1 - k + rodPerCycle) % rodPerCycle + k);
				for (int j = 1; j <= middleCount; j++)
				{
					float t = (1.0f / (middleCount + 1)) * j;
					glm::vec4 newPoint;
					newPoint = mix(a, b, t);
					mRods.push_back(newPoint);
					//compute angle
					glm::vec3 p = glm::normalize(glm::vec3(newPoint - sarcomereMidPoint));
					float dot = glm::dot(glm::vec2(p.x, p.z), glm::vec2(up.x, up.z));
					float det = p.x * up.z - p.z * up.x;
					float alpha = std::atan2(det, dot);
					if (alpha < 0.0f)
					{
						alpha = 2.0f * glm::pi<float>() + alpha;
					}
					mRodAngles.push_back(glm::degrees(alpha));
				}
			}
		}
		cycleCount++;
		middleCount++;
	}
	m_myosinRods = mRods;
}


void Sarcomere::genActinRods(SarcomereType type, glm::vec4 sarcomereMidPoint, float d10, int& cycleCount)
{
	std::vector<glm::vec4> aRods;
	if (type == SarcomereType::TWO_TO_ONE)
	{
		glm::vec3 up = glm::vec3(1.0f, 0.0f, 0.0f);
		float angle = glm::radians(360.0f / 3.0f);
		int offset = 0;
		for (int j = 0; j < cycleCount * 2 - 1; j++)
		{
			if (j >= cycleCount) {
				offset += 3;
			}
			for (int i = 0; i < 3; i++) {
				glm::vec4 p1 = sarcomereMidPoint + glm::vec4(m_dMyosin * (j + 1), 0.0f, 0.0f, 0.0f);
				auto p2 = glm::rotate(p1, angle * i, glm::vec3(0.0f, 1.0f, 0.0f));
				auto p3 = glm::rotate(p1, angle * (i + 1), glm::vec3(0.0f, 1.0f, 0.0f));
				auto dir = glm::normalize(glm::vec3(p3 - p2));
				for (int k = 1 + offset; k < 4 + 3 * j - 1 - offset; k++)
				{
					if (k % 3 == 0)
					{
						continue;
					}
					aRods.push_back(p2 + glm::vec4(dir, 0.0f) * static_cast<float>(k) * m_dActin);
				}
			}
		}
		offset = 0;
		for (int j = 0; j < cycleCount * 2 - 1; j++)
		{
			if (j >= cycleCount)
			{
				offset += 3;
			}
			for (int i = 0; i < 3; i++)
			{
				glm::vec4 p1 = sarcomereMidPoint + glm::vec4(m_dMyosin * (j + 1), 0.0f, 0.0f, 0.0f);
				auto p2 = glm::rotate(p1, angle * i, glm::vec3(0.0f, 1.0f, 0.0f));
				auto p3 = glm::rotate(p1, angle * (i + 1), glm::vec3(0.0f, 1.0f, 0.0f));
				auto dir = glm::normalize(glm::vec3(p3 - p2));
				for (int k = 1 + offset; k < 4 + 3 * j - 1 - offset; k++)
				{
					if (k % 3 == 0)
					{
						continue;
					}
					aRods.push_back(p2 + glm::vec4(dir, 0.0f) * static_cast<float>(k) * m_dActin);
				}
			}
		}
	}

	if (type == SarcomereType::THREE_TO_ONE)
	{
		//generate actin rods row wise from the middle row outwards

		//first actin set
		//middle row 
		//generates offset positions for the middle row in +x and -x direction
		float offset = ((m_d11 / 2.0f) * glm::sqrt(3.0f));
		/*for (int i = 0; i < cycleCount; i++)
		{
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
		}*/
		//generate offset position row wise, generates two rows per outer loop cycle
		for (int j = 0; j < 2 * cycleCount; j++)
		{
			if (j % 2 == 0)
			{
				//offset every 2. myosin row
				if ((j + 2) % 4 == 0)
				{
					//generates offset positions for two rows in +x and -x direction
					for (int i = 0; i < cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
					}
				}
				else
				{
					//generates offset positions for two rows in +x and -x direction
					for (int i = 0; i < cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
					}
				}
			}
			else
			{
				//generates offset positions for two rows in +x and -x direction
				for (int i = 0; i < 2 * cycleCount - j / 2 - 1; i++)
				{
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
				}
			}
		}

		//second actin set
		//middle 
		//generates offset positions for the middle row in +x and -x direction
		/*for (int i = 0; i < cycleCount; i++)
		{
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
		}*/
		//generate offset position row wise, generates two rows per outer loop cycle
		for (int j = 0; j < 2 * cycleCount; j++)
		{
			if (j % 2 == 0)
			{
				//offset every 2. myosin row
				if ((j + 2) % 4 == 0)
				{
					//generates offset positions for two rows in +x and -x direction
					for (int i = 0; i < cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
					}
				}
				else
				{
					//generates offset positions for two rows in +x and -x direction
					for (int i = 0; i < cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * 2.0f * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * 2.0f * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
					}
				}
			}
			else
			{
				//generates offset positions for two rows in +x and -x direction
				for (int i = 0; i < 2 * cycleCount - j / 2 - 1; i++)
				{
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + j * offset, 1.0f));
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - j * offset, 1.0f));
				}
			}
		}
	}
	if (type == SarcomereType::FIVE_TO_ONE)
	{
		//generate actin rods row wise from the middle row outwards

		//offset between each row alternates every 2 rows
		float offset = (glm::sqrt(3.0f) / 3.0f) * m_d11;
		bool b = true;
		//offset in x /-x direction alternates for every 4. actin
		bool a = true;

		//generate middle row
		for (int i = 0; i < cycleCount; i++)
		{
			//middle row +x direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
			//middle row -x direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
		}
		// generate acin rows 
		for (int j = 1; j < 4 * cycleCount; j++)
		{
			//invert row offset
			if ((j + 1) % 4 == 0)
			{
				a = !a;
			}
			//invert column offset
			if ((j + 1) % 2 == 0)
			{
				b = !b;
			}

			//1. case: every 4. row actin is spaced with a disatnce of d11 and an offset of 0.5 * d11
			if ((j + 2) % 4 == 0)
			{
				for (int i = 0; i < 2 * cycleCount - (j / 4) - 1; i++)
				{
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
				}
			}
			//2.case for 3 consecutive colums actin is spaced with a distance of 2 * d11 and offset with d11
			else if (a)
			{
				for (int i = 0; i < cycleCount - (j / 8.0f) - 0.5f; i++)
				{
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
				}
			}
			//3.case for 3 consecutive colums actin is spaced with a distance of 2 * d11 and no offset
			else
			{
				for (int i = 0; i < cycleCount - j / 8; i++)
				{
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					if (j > 2)
					{
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					}
				}
			}
			//increment offset based on the current row + 1 beeing devisible by 2
			if (b)
			{
				offset += (glm::sqrt(3.0f) / 3.0f) * m_d11;
			}
			else
			{
				offset += ((glm::sqrt(3.0f) / 3.0f) * m_d11) / 2.0f;
			}
		}

		//generate actin rods row wise from the middle row outwards

		//offset between each row alternates every 2 rows
		offset = (glm::sqrt(3.0f) / 3.0f) * m_d11;
		b = true;
		//offset in x /-x direction alternates for every 4. actin
		a = true;

		//generate middle row
		for (int i = 0; i < cycleCount; i++)
		{
			//middle row +x direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
			//middle row -x direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z, 1.0f));
		}
		// generate acin rows 
		for (int j = 1; j < 4 * cycleCount; j++)
		{
			//invert row offset
			if ((j + 1) % 4 == 0)
			{
				a = !a;
			}
			//invert column offset
			if ((j + 1) % 2 == 0)
			{
				b = !b;
			}

			//1. case: every 4. row actin is spaced with a disatnce of d11 and an offset of 0.5 * d11
			if ((j + 2) % 4 == 0)
			{
				for (int i = 0; i < 2 * cycleCount - (j / 4) - 1; i++)
				{
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (i * m_d11) + 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (i * m_d11) - 0.5f * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
				}
			}
			//2.case for 3 consecutive colums actin is spaced with a distance of 2 * d11 and offset with d11
			else if (a)
			{
				for (int i = 0; i < cycleCount - (j / 8.0f) - 0.5f; i++)
				{
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11) + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11) - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
				}
			}
			//3.case for 3 consecutive colums actin is spaced with a distance of 2 * d11 and no offset
			else
			{
				for (int i = 0; i < cycleCount - j / 8; i++)
				{
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					if (j > 2)
					{
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - (2 * i * m_d11), sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					}
				}
			}
			//increment offset based on the current row + 1 beeing devisible by 2
			if (b)
			{
				offset += (glm::sqrt(3.0f) / 3.0f) * m_d11;
			}
			else
			{
				offset += ((glm::sqrt(3.0f) / 3.0f) * m_d11) / 2.0f;
			}
		}
	}
	if (type == SarcomereType::SIX_TO_ONE)
	{
		//offsets between rows
		float offset_case1 = (glm::sqrt(3.0f) / 3.0f) * m_d11;
		float offset_case2 = ((-6.0f + 5.0f * glm::sqrt(3.0f)) / 6.0f) * m_d11;
		float offset_case3 = (2.0f - glm::sqrt(3.0f)) * m_d11;
		//start offset
		float offset = offset_case1 / 2.0f;

		//generate actin positions row wise from the middle outwards
		//always generate one row in x and -x direction at the middle of the row, from the center towards the end.
		//also generate a mirror of this row in - z direction 
		//=> create two full rows per cycle

		//first row seperate because it only needs half the offset
		for (int i = 0; i < cycleCount; i++)
		{
			//+x and +z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
			//-x and +z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
			//+x and -z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
			//-x and -z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
		}
		//parameters to invert x_offset
		bool a = true;
		int b = 0;

		//cycle over the rows
		for (int j = 2; j < 4 * cycleCount; j++)
		{
			//increment offsets between the rows
			//case 1 offset z direction
			if ((j + 3) % 4 == 0)
			{
				offset += offset_case1;
			}
			//case 2 offset z direction
			else if (j % 2 == 0)
			{
				offset += offset_case2;
			}
			// case 3 offset z direction
			else if ((j + 1) % 4 == 0)
			{
				offset += offset_case3;
			}
			//case 1 x direction
			if (j % 8 == 0 || j % 8 == 1)
			{
				//generate the rows
				for (int i = 0; i < cycleCount - j / 8; i++)
				{
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
				}
			}
			//case 2 x direction (like case 1 but not offset by d11 in x / -x direction)
			else if ((j + 4) % 8 == 0 || (j + 4) % 8 == 1)
			{
				//generate the rows
				for (int i = 0; i < cycleCount - j / 8; i++)
				{
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
				}
			}
			//case 3 x direction 
			//has a short and a long offset in x direction which changes every column
			//the middle of the row starts with a long or a short x_offset, this always switches after 2 rows.
			else
			{
				float x_offset;
				//case where the middle of the row starts with the long x_offset
				if (a)
				{
					//begin with half the offset for the first actin after the middle
					x_offset = (2.0f - glm::sqrt(3.0f) / 3.0f) * m_d11;
					x_offset /= 2.0f;
					//generate the rows
					for (int i = 1; i < 2 * cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));

						//increment x_offset
						if (i % 2 == 0)
						{
							x_offset += (2.0f - glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
						else
						{
							x_offset += (glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
					}
				}
				//case where the middle of the row starts with the short x_offset
				else
				{
					//begin with half the offset for the first actin after the middle
					x_offset = (glm::sqrt(3.0f) / 3.0f) * m_d11;
					x_offset /= 2.0f;
					//generate the rows
					for (int i = 1; i < 2 * cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));

						//increment x_offset
						if (i % 2 == 0)
						{
							x_offset += (glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
						else
						{
							x_offset += (2.0f - glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
					}
				}
				//invert the a if the two previos rows had the same starting x_offset (check with the counting variable b
				if (b % 2 == 0)
				{
					a = !a;
				}
				b++;
			}
		}

		//second actin set

		//reset counting variables
		offset = offset_case1 / 2.0f;
		a = true;
		b = 0;

		//generate actin positions row wise from the middle outwards
		//always generate one row in x and -x direction at the middle of the row, from the center towards the end.
		//also generate a mirror of this row in - z direction 
		//=> create two full rows per cycle

		//first row seperate because it only needs half the offset
		for (int i = 0; i < cycleCount; i++)
		{
			//+x and +z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
			//-x and +z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
			//+x and -z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
			//-x and -z direction
			aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
		}
		//cycle over the rows
		for (int j = 2; j < 4 * cycleCount; j++)
		{
			//increment offsets between the rows
			//case 1 offset z direction
			if ((j + 3) % 4 == 0)
			{
				offset += offset_case1;
			}
			//case 2 offset z direction
			else if (j % 2 == 0)
			{
				offset += offset_case2;
			}
			// case 3 offset z direction
			else if ((j + 1) % 4 == 0)
			{
				offset += offset_case3;
			}
			//case 1 x direction
			if (j % 8 == 0 || j % 8 == 1)
			{
				//generate the rows
				for (int i = 0; i < cycleCount - j / 8; i++)
				{
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11 + m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11 - m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
				}
			}
			//case 2 x direction (like case 1 but not offset by d11 in x / -x direction)
			else if ((j + 4) % 8 == 0 || (j + 4) % 8 == 1)
			{
				//generate the rows
				for (int i = 0; i < cycleCount - j / 8; i++)
				{
					//+x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//-x and +z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
					//+x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x + 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
					//-x and -z direction
					aRods.push_back(glm::vec4(sarcomereMidPoint.x - 2 * i * m_d11, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
				}
			}
			//case 3 x direction 
			//has a short and a long offset in x direction which changes every column
			//the middle of the row starts with a long or a short x_offset, this always switches after 2 rows.
			else
			{
				float x_offset;
				//case where the middle of the row starts with the long x_offset
				if (a)
				{
					//begin with half the offset for the first actin after the middle
					x_offset = (2.0f - glm::sqrt(3.0f) / 3.0f) * m_d11;
					x_offset /= 2.0f;
					//generate the rows
					for (int i = 1; i < 2 * cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));

						//increment x_offset
						if (i % 2 == 0)
						{
							x_offset += (2.0f - glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
						else
						{
							x_offset += (glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
					}
				}
				//case where the middle of the row starts with the short x_offset
				else
				{
					//begin with half the offset for the first actin after the middle
					x_offset = (glm::sqrt(3.0f) / 3.0f) * m_d11;
					x_offset /= 2.0f;
					//generate the rows
					for (int i = 1; i < 2 * cycleCount - j / 4; i++)
					{
						//+x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//-x and +z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z + offset, 1.0f));
						//+x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x + x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));
						//-x and -z direction
						aRods.push_back(glm::vec4(sarcomereMidPoint.x - x_offset, sarcomereMidPoint.y, sarcomereMidPoint.z - offset, 1.0f));

						//increment x_offset
						if (i % 2 == 0)
						{
							x_offset += (glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
						else
						{
							x_offset += (2.0f - glm::sqrt(3.0f) / 3.0f) * m_d11;
						}
					}
				}
				//invert the a if the two previos rows had the same starting x_offset (check with the counting variable b
				if (b % 2 == 0)
				{
					a = !a;
				}
				b++;
			}
		}
	}

	m_actinRods = aRods;
}

void Sarcomere::genBuffers()
{
	glCreateBuffers(1, &m_zDisc_ssbo);
	glNamedBufferStorage(m_zDisc_ssbo, sizeof(glm::vec4) * m_zOffset.size(), m_zOffset.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_zDisc_ssbo);

	glCreateBuffers(1, &m_mRod_ssbo);
	glNamedBufferStorage(m_mRod_ssbo, sizeof(glm::vec4) * m_myosinRods.size(), m_myosinRods.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_mRod_ssbo);

	glCreateBuffers(1, &m_aRod_ssbo);
	glNamedBufferStorage(m_aRod_ssbo, sizeof(glm::vec4) * m_actinRods.size(), m_actinRods.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_aRod_ssbo);
}

void Sarcomere::bindBuffers()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_zDisc_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_mRod_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_aRod_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_aSphere_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_lineMatricees_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_troponin_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_LMMOffsetPositions_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_HMMOffsetPositions_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_HMMyRotationMatricees_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_HMMzRotationMatrices_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_HMMyRotationMatrices2_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_myosinHeadOffsetPositions_ssbo);
}

void Sarcomere::updateVolume()
{
	m_sarcomereVolume = glm::sqrt(3.0f) * 2.0f * glm::pow(float(m_cycleCount), 2) * glm::pow(d10, 2) * sarcomereLength;
}

void Sarcomere::updateD10()
{
	d10 = glm::sqrt((2.0f * m_sarcomereVolume) / (glm::sqrt(3.0f) * sarcomereLength)) / (2.0f * m_cycleCount);
}

void Sarcomere::updateD11()
{
	m_d11 = d10 / sqrt(3.0f);
}

void Sarcomere::updateHMMLength()
{
	float aSquared1 = glm::pow(((2.0f * m_d11) / sqrt(3.0f) - myosinRadius / 3.0f - actinRadius), 2);
	float bSquared1 = glm::pow(glm::abs(m_lengthUnderHMM1 - (2.0f - sarcomereLength) / 2), 2);
	m_HMMLength1 = glm::min(glm::sqrt(aSquared1 + bSquared1), m_HMMLength + m_HMMLength / 10.0f);
	if (m_lengthUnderHMM1 - (2.0f - sarcomereLength) < 0.0f)
	{
		m_invertAngle1 = true;
	}
	else
	{
		m_invertAngle1 = false;
	}

	float aSquared2 = glm::pow((m_d11 - myosinRadius / 3.0f - actinRadius), 2);
	float bSquared2 = glm::pow(glm::max((m_lengthUnderHMM2 - (2.0f - sarcomereLength) / 2), 0.000001f), 2);
	m_HMMLength2 = glm::min(glm::sqrt(aSquared2 + bSquared2), m_HMMLength + m_HMMLength / 10.0f);
	if (m_lengthUnderHMM2 - (2.0f - sarcomereLength) < 0.0f)
	{
		m_invertAngle2 = true;
	}
	else
	{
		m_invertAngle2 = false;
	}

	float aSquared3 = glm::pow(((m_d11 / glm::cos(glm::radians(15.0f))) - myosinRadius / 3.0f - actinRadius), 2);
	float bSquared3 = glm::pow(glm::max((m_lengthUnderHMM3 - (2.0f - sarcomereLength) / 2), 0.000001f), 2);
	m_HMMLength3 = glm::min(glm::sqrt(aSquared3 + bSquared3), m_HMMLength + m_HMMLength / 10.0f);
	if (m_lengthUnderHMM2 - (2.0f - sarcomereLength) < 0.0f)
	{
		m_invertAngle2 = true;
	}
	else
	{
		m_invertAngle2 = false;
	}
}

void Sarcomere::update_dMyosin()
{
	m_dMyosin = 2.0f * m_d11;
}

void Sarcomere::update_dActin()
{
	m_dActin = glm::sqrt(4.0f * m_dMyosin * m_dMyosin - 4.0f * m_d11 * m_d11) / 3.0f;
	if (m_type == SarcomereType::FIVE_TO_ONE)
	{
		m_dActin /= 2.0f;
	}
	if (m_type == SarcomereType::SIX_TO_ONE)
	{
		m_dActin /= 2.0f;
	}
}

void Sarcomere::updateRadius()
{
	m_sarcomereRadius = m_cycleCount * 2.0f * m_d11;;
}

void Sarcomere::updateLength()
{
	sarcomereLength = m_sarcomereVolume / (glm::sqrt(3.0f) * 2.0f * glm::pow(float(m_cycleCount), 2) * glm::pow(d10, 2));
}

void Sarcomere::updateHMMAngle()
{
	/*if (m_invertAngle1)
	{
		m_HMMAngle = -glm::asin((myosinTrunkRadius * 2.0f - (myosinRadius / 3.0f)) / (m_HMMLength + (myosinRadius / 3.0f)));
	}*/

	m_HMMAngle = glm::asin((myosinTrunkRadius * 2.0f - (myosinRadius / 3.0f)) / (m_HMMLength + (myosinRadius / 3.0f)));
}

void Sarcomere::updateOffsetBuffers()
{
	updateD11();
	update_dMyosin();
	update_dActin();
	updateRadius();
	m_cycleCount = 1;
	genMyosinRods(sarcomereMidPoint, d10, m_cycleCount);
	glNamedBufferSubData(m_mRod_ssbo, 0, sizeof(glm::vec4) * m_myosinRods.size(), m_myosinRods.data());

	genActinRods(m_type, sarcomereMidPoint, d10, m_cycleCount);
	glNamedBufferSubData(m_aRod_ssbo, 0, sizeof(glm::vec4) * m_actinRods.size(), m_actinRods.data());
}


std::vector<glm::vec4> Sarcomere::getActinParticles()
{
	return m_actinParticlePositions;
}

int Sarcomere::getNumActinParticles()
{
	return static_cast<int>(m_actinParticlePositions.size());
}

int Sarcomere::getNumTroponinParticles()
{
	return static_cast<int>(m_troponinPositions.size());
}

void Sarcomere::generateDoubleHelixOffsetPositions()
{
	m_actinParticlePositions.clear();
	m_troponinPositions.clear();
	m_tropomyosinPositions.clear();
	m_lineRotMatricees.clear();

	float helixPitch = 180.0f / (37.5f / (actinRadius * 1000.0f));
	float linePitch = 7.0f * helixPitch - 180.0f;
	float alphaR = glm::radians(helixPitch);
	float alphaL = glm::radians(helixPitch); // 180 / (37.5 / 5.1)
	float yOffset = actinRadius;//5.9f?;
	float sphereRadius = actinRadius / 2.0f;
	int numActinParticles = static_cast<int>(actinLength / yOffset);

	//first actin set, helices rotate clockwise
	//generate positions for troponin offsets aswell since they need the same rotation but a different offset
	//first helix
	for (int i = 0; i <= numActinParticles; i++)
	{
		//actin monomer Rotation
		glm::vec4 aRotVec = glm::vec4(sphereRadius, 0.0f, 0.0f, 0.0f);
		aRotVec = glm::rotate(aRotVec, i * alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		//push back positions of actin monomers , offset in y direction
		m_actinParticlePositions.push_back(glm::vec4(0.0f, i * yOffset - sarcomereLength / 2.0f, 0.0f, 1.0f) + aRotVec);

		//generate the first half of troponin offsets
		//offset them like the tropomyosin
		glm::vec4 tRotVec = glm::vec4(0.0f, 0.0f, sphereRadius, 0.0f);
		//rotate them like the helix but just repeat the first set of seven
		tRotVec = glm::rotate(tRotVec, (i % 7) * alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		//rotate the positions with the linePith of it's corresponding tropomyosin linesegment
		tRotVec = glm::rotate(tRotVec, glm::radians((i / 7) * linePitch), glm::vec3(0.0f, 1.0f, 0.0f));
		//set a troponin position on 0,6,7,13,14,20,21,27...
		if ((i % 7) == 0 || (i + 1) % 7 == 0)
		{
			m_troponinPositions.push_back(glm::vec4(0.0f, i * yOffset - sarcomereLength / 2.0f, 0.0f, 1.0f) + tRotVec);
		}
	}
	//if number of troponin particles is not even, cut the last one
	if ((m_troponinPositions.size() % 2) == 1)
	{
		m_troponinPositions.pop_back();
	}
	//second helix
	for (int i = 0; i <= numActinParticles; i++)
	{
		glm::vec4 rotVec = glm::vec4(-sphereRadius, 0.0f, 0.0f, 0.0f);
		rotVec = glm::rotate(rotVec, i * alphaL, glm::vec3(0.0f, 1.0f, 0.0f));
		m_actinParticlePositions.push_back(glm::vec4(0.0f, i * yOffset - sarcomereLength / 2.0f, 0.0f, 1.0f) + rotVec);
	}

	//second actin set, helices rotate counter clockwise
	//generate positions for troponin offsets aswell since they need the same rotation but a different offset
	//first helix
	for (int i = 0; i <= numActinParticles; i++)
	{
		//actin monomer Rotation
		glm::vec4 aRotVec = glm::vec4(sphereRadius, 0.0f, 0.0f, 0.0f);
		aRotVec = glm::rotate(aRotVec, i * -alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		//push back positions of actin monomers, offset in y direction
		m_actinParticlePositions.push_back(glm::vec4(0.0f, sarcomereLength / 2.0f - (i * yOffset), 0.0f, 1.0f) + aRotVec);

		//generate the second half of troponin offsets
		//offset them like the tropomyosin
		glm::vec4 tRotVec = glm::vec4(0.0f, 0.0f, -sphereRadius, 0.0f);
		//rotate them like the helix but just repeat the first set of seven
		tRotVec = glm::rotate(tRotVec, (i % 7) * -alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		//rotate the positions with the linePith of it's corresponding tropomyosin linesegment
		tRotVec = glm::rotate(tRotVec, -glm::radians((i / 7) * linePitch), glm::vec3(0.0f, 1.0f, 0.0f));
		//set a troponin position on 0,6,7,13,14,20,21,27...
		if ((i % 7) == 0 || (i + 1) % 7 == 0)
		{
			m_troponinPositions.push_back(glm::vec4(0.0f, sarcomereLength / 2.0f - (i * yOffset), 0.0f, 1.0f) + tRotVec);
		}
	}
	//if number of troponin particles is not even, cut the last one
	if ((m_troponinPositions.size() % 2) == 1)
	{
		m_troponinPositions.pop_back();
	}
	//second helix
	for (int i = 0; i <= numActinParticles; i++)
	{
		glm::vec4 rotVec = glm::vec4(-sphereRadius, 0.0f, 0.0f, 0.0f);
		rotVec = glm::rotate(rotVec, i * -alphaL, glm::vec3(0.0f, 1.0f, 0.0f));
		m_actinParticlePositions.push_back(glm::vec4(0.0f, sarcomereLength / 2 - (i * yOffset), 0.0f, 1.0f) + rotVec);
	}

	//create actin monomer offset position ssbo
	numParticles = static_cast<int>(m_actinParticlePositions.size());
	glCreateBuffers(1, &m_aSphere_ssbo);
	glNamedBufferStorage(m_aSphere_ssbo, sizeof(glm::vec4) * numParticles, m_actinParticlePositions.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_aSphere_ssbo);

	//generate one tropomyosin linesegment. One line segment is 7 actin monomers long, so it consits of 8 actin monomer positions
	for (int i = 0; i < 8; i++)
	{
		//tropomyosin rotation
		glm::vec4 tRotVec = glm::vec4(0.0f, 0.0f, sphereRadius, 0.0f);
		tRotVec = glm::rotate(tRotVec, i * alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		//push back positions of tropomyosin linesegment points, offset in y direction
		m_tropomyosinPositions.push_back(glm::vec4(0.0f, i * yOffset - sarcomereLength / 2.0f, 0.0f, 1.0f) + tRotVec);
		//push back first position again to properly use GL_LINE_STRIP_ADJACENCY
		if (i == 0)
		{
			m_tropomyosinPositions.push_back(glm::vec4(0.0f, i * yOffset - sarcomereLength / 2.0f, 0.0f, 1.0f) + tRotVec);
		}
	}
	//create matricees to rotate linesegments
	glm::mat4 tropomyosinRotationMatrix;
	//first half rotates clockwise
	for (int i = 0; i < int(m_actinParticlePositions.size() / 2 / 7) / 2; i++)
	{
		tropomyosinRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(i * linePitch), glm::vec3(0.0f, 1.0f, 0.0f));
		m_lineRotMatricees.push_back(tropomyosinRotationMatrix);
	}
	//second half rotates counter clockwise
	for (int i = 0; i < int(m_actinParticlePositions.size() / 2 / 7) / 2; i++)
	{
		tropomyosinRotationMatrix = glm::rotate(glm::mat4(1.0f), -glm::radians(i * linePitch), glm::vec3(0.0f, 1.0f, 0.0f));
		m_lineRotMatricees.push_back(tropomyosinRotationMatrix);
	}
	//create ssbo for tropomyosin rotation matricees
	glCreateBuffers(1, &m_lineMatricees_ssbo);
	glNamedBufferStorage(m_lineMatricees_ssbo, sizeof(glm::mat4) * m_lineRotMatricees.size(), m_lineRotMatricees.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_lineMatricees_ssbo);

	//create troponin offset position ssbo
	int numTroponinOffests = static_cast<int>(m_troponinPositions.size());
	glCreateBuffers(1, &m_troponin_ssbo);
	glNamedBufferStorage(m_troponin_ssbo, sizeof(glm::vec4) * numTroponinOffests, m_troponinPositions.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_troponin_ssbo);

	genTropomyosinBuffer();
}

void Sarcomere::genLMM()
{
	m_LMMPositions1.clear();
	m_LMMPositions2.clear();
	//set LMM and HMM helix radius to be one 10th of the myosin trunk radius
	m_LMMRadius = myosinTrunkRadius / 10.0f;
	//angle with which the point gets rotated around the middle of the helix after each itereation
	constexpr float helixPitch = 180.0f / 3.0f;
	constexpr float alphaR = glm::radians(helixPitch);
	//offset with which the point gets translated upward the y axis after each iteartion
	const float yOffset = m_LMMRadius * 3.0f;
	//number of anchor points per helix
	int numPoints = static_cast<int>(m_LMMLength / yOffset);
	//generate the anchor points for the first LMM helix
	for (int i = 0; i <= numPoints; i++)
	{
		//set the first anchor point at 0,0,0 and translate it outwards along the x axis with the radius of the helix
		glm::vec4 anchorPoint = glm::vec4(m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//translate the point upwards the y axis
		anchorPoint = glm::rotate(anchorPoint, i * alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		//rotate the point around the center of the helix
		m_LMMPositions1.push_back(glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f) + anchorPoint);
		//duplicate first point
		if (i == 0)
		{
			m_LMMPositions1.push_back(glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f) + anchorPoint);
		}
	}

	//generate the anchor points for the second LMM helix
	for (int i = 0; i <= numPoints; i++)
	{
		//set the first anchor point at 0,0,0 and translate it outwards along the x axis with the radius of the helix
		glm::vec4 anchorPoint = glm::vec4(-m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//translate the point upwards the y axis
		anchorPoint = glm::rotate(anchorPoint, i * alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		//rotate the point around the center of the helix
		m_LMMPositions2.push_back(glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f) + anchorPoint);
		//duplicate first point
		if (i == 0)
		{
			m_LMMPositions2.push_back(glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f) + anchorPoint);
		}
	}
	//gen LMM buffers
	genLMM1Buffer();
	genLMM2Buffer();
	//gen LMM offset Positions
	genLMMOffsetPositions();
	//gen HMM
	genHMM();
	//gen Myosin Heads
	genMyosinHeads();
	myosinIsGenerated = true;
}

void Sarcomere::genHMM()
{
	m_HMMPositions1.clear();
	m_HMMPositions2.clear();
	//angle with which the point gets rotated around the middle of the helix after each itereation
	constexpr float helixPitch = 180.0f / 3.0f;
	constexpr float alphaR = glm::radians(helixPitch);
	//offset with which the point gets translated upward the y axis after each iteartion
	float yOffset = m_LMMRadius * 3.0f;
	//number of anchor points per helix, needs to be atleast one to avoid reading an empty array entry later on
	int numPoints = glm::max(static_cast<int>(m_HMMLength / yOffset), 1);
	yOffset = m_HMMLength / static_cast<float>(numPoints);
	if (m_type == SarcomereType::TWO_TO_ONE)
	{
		yOffset = m_HMMLength1 / static_cast<float>(numPoints);
	}
	if (m_type == SarcomereType::THREE_TO_ONE)
	{
		yOffset = m_HMMLength2 / static_cast<float>(numPoints);
	}
	if (m_type == SarcomereType::FIVE_TO_ONE)
	{
		yOffset = m_HMMLength1 / static_cast<float>(numPoints);
	}
	if (m_type == SarcomereType::SIX_TO_ONE)
	{
		yOffset = m_HMMLength3 / static_cast<float>(numPoints);
	}
	//number of points at the end that form the straight tip of the helices
	int endOffset = 15;
	//generate the anchor points for the first HMM helix
	for (int i = 0; i <= numPoints; i++)
	{
		//set the first anchor point at 0,0,0 and translate it outwards along the x axis with the radius of the helix
		glm::vec4 anchorPoint = glm::vec4(m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//if the point belongs to the curly part of the HMM:
		if (i <= numPoints - endOffset)
		{
			//translate the point upwards the y axis
			anchorPoint = anchorPoint + glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f);
			//rotate the point around the center of the helix
			anchorPoint = glm::rotate(anchorPoint, i * alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		//for the last x points the HMM needs to be straight and diverge outwards
		else
		{
			float zOffset = (i + endOffset - numPoints) * yOffset / 4.0f;
			//translate the point away from the center of the helix in z direction
			anchorPoint = anchorPoint + glm::vec4(0.0f, i * yOffset, zOffset, 1.0f);
		}
		m_HMMPositions1.push_back(anchorPoint);
		//duplicate first point
		if (i == 0)
		{
			m_HMMPositions1.push_back(glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f) + anchorPoint);
		}
	}

	//generate the anchor points for the first HMM helix
	for (int i = 0; i <= numPoints; i++)
	{
		//set the first anchor point at 0,0,0 and translate it outwards along the -x axis with the radius of the helix
		//this offsets the second helix 180 degrees compared to the first one
		glm::vec4 anchorPoint = glm::vec4(-m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//if the point belongs to the curly part of the HMM:
		if (i <= numPoints - endOffset)
		{
			//translate the point upwards the y axis
			anchorPoint = anchorPoint + glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f);
			//rotate the point around the center of the helix
			anchorPoint = glm::rotate(anchorPoint, i * alphaR, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		//for the last x points the HMM needs to be straight and diverge outwards
		else
		{
			float zOffset = (i + endOffset - numPoints) * yOffset / 4.0f;
			//translate the point away from the center of the helix in -z direction
			anchorPoint = anchorPoint + glm::vec4(0.0f, i * yOffset, -zOffset, 1.0f);
		}
		m_HMMPositions2.push_back(anchorPoint);
		//duplicate first point
		if (i == 0)
		{
			m_HMMPositions2.push_back(glm::vec4(0.0f, i * yOffset, 0.0f, 1.0f) + anchorPoint);
		}
	}
	//gen HMM buffers
	genHMM1Buffer();
	genHMM2Buffer();
	//gen HMM offset positions
	genHMMOffsetPositions(0.0f);
}
void Sarcomere::genLMMOffsetPositions()
{
	m_LMMOffsetPositions.clear();
	//number of LMM parts per myosin half
	int numLMMPerHalf = static_cast<int>((myosinLength / 2.0f) / m_LMMyOffset);
	int overlap = static_cast<int>(m_LMMLength / m_LMMyOffset);
	//define the three starting angles around the rod
	float angle1 = 30.0f;
	float angle2 = 150.0f;
	float angle3 = 270.0f;
	//the angle each LMM part gets rotated to its next iteration
	float alpha = 40.0f;
	//generate LMM offset positions for the first Myosin half from the middle outwards
	for (int i = 0; i <= numLMMPerHalf - overlap; i++)
	{
		//set the three offset positions of the LMM for each iteration
		//translate the point out of the middle of the Myosin rod in +x direction 
		glm::vec4 pos1Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos2Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos3Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//rotate each offset position into the right place around the myosin rod
		pos1Rot = glm::rotate(pos1Rot, glm::radians(angle1), glm::vec3(0.0f, 1.0f, 0.0f));
		pos2Rot = glm::rotate(pos2Rot, glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
		pos3Rot = glm::rotate(pos3Rot, glm::radians(angle3), glm::vec3(0.0f, 1.0f, 0.0f));
		//translate each offset along the y axis of the myosin rod.
		m_LMMOffsetPositions.push_back(glm::vec4(0.0f, -i * m_LMMyOffset, 0.0f, 0.0f) + pos1Rot);
		m_LMMOffsetPositions.push_back(glm::vec4(0.0f, -i * m_LMMyOffset, 0.0f, 0.0f) + pos2Rot);
		m_LMMOffsetPositions.push_back(glm::vec4(0.0f, -i * m_LMMyOffset, 0.0f, 0.0f) + pos3Rot);
		//increment each angle
		angle1 += alpha;
		angle2 += alpha;
		angle3 += alpha;
	}
	//reset starting angles
	angle1 = 30.0f;
	angle2 = 150.0f;
	angle3 = 270.0f;
	//generate LMM offset positions for the second Myosin half from the middle outwards
	for (int i = 0; i <= numLMMPerHalf - overlap; i++)
	{
		//set the three offset positions of the LMM for each iteration
		//translate the point out of the middle of the Myosin rod in +x direction 
		glm::vec4 pos1Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos2Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos3Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//rotate each offset position into the right place around the myosin rod
		pos1Rot = glm::rotate(pos1Rot, glm::radians(angle1), glm::vec3(0.0f, 1.0f, 0.0f));
		pos2Rot = glm::rotate(pos2Rot, glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
		pos3Rot = glm::rotate(pos3Rot, glm::radians(angle3), glm::vec3(0.0f, 1.0f, 0.0f));
		//translate each offset along the y axis of the myosin rod.
		m_LMMOffsetPositions.push_back(glm::vec4(0.0f, i * m_LMMyOffset, 0.0f, 0.0f) + pos1Rot);
		m_LMMOffsetPositions.push_back(glm::vec4(0.0f, i * m_LMMyOffset, 0.0f, 0.0f) + pos2Rot);
		m_LMMOffsetPositions.push_back(glm::vec4(0.0f, i * m_LMMyOffset, 0.0f, 0.0f) + pos3Rot);
		//increment each angle
		angle1 += alpha;
		angle2 += alpha;
		angle3 += alpha;
	}
	//create ssbo for LMMoffsetPositions
	glCreateBuffers(1, &m_LMMOffsetPositions_ssbo);
	glNamedBufferStorage(m_LMMOffsetPositions_ssbo, sizeof(glm::vec4) * m_LMMOffsetPositions.size(), m_LMMOffsetPositions.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_LMMOffsetPositions_ssbo);
}


void Sarcomere::genHMMOffsetPositions(float scaleFactor)
{
	m_HMMOffsetPositions.clear();
	m_HMMyRotMats.clear();
	m_HMMyRotMatrices2.clear();
	m_HMMRotMatrices.clear();
	m_HMMAngles.clear();
	//number of HMM parts per myosin half
	int numHMMPerHalf = static_cast<int>((myosinLength / 2.0f) / m_LMMyOffset);
	int overlap = static_cast<int>(m_LMMLength / m_LMMyOffset);
	//define the three starting angles around the rod
	float angle1 = 30.0f;
	float angle2 = 150.0f;
	float angle3 = 270.0f;
	float angle = 0.0f;
	float HMMAngleFull1 = glm::asin(glm::min(((2.0f * m_d11) / sqrt(3.0f) - myosinRadius / 3.0f - actinRadius) / (m_HMMLength1 + (myosinHeadRadius)), 1.0f)); // for 2to1 myosin heads fully engaged
	float HMMAngleFull2 = glm::asin(glm::min((m_d11 - myosinRadius / 3.0f - actinRadius) / (m_HMMLength2 + (myosinHeadRadius)), 1.0f)); // for 3to1 myosin heads fully engaged
	float HMMAngleFull3 = glm::asin(glm::min(((m_d11 / glm::cos(glm::radians(15.0f))) - myosinRadius / 3.0f - actinRadius) / (m_HMMLength3 + (myosinHeadRadius)), 1.0f)); // for 6to1 myosin heads fully engaged
	//the angle each HMM part gets rotated to its next iteration
	float alpha = 40.0f;
	glm::mat4 rotMat;
	//generate HMM offset positions for the first Myosin half from the middle outwards
	for (int i = 0; i <= numHMMPerHalf - overlap; i++)
	{
		//set the three offset positions of the HMM for each iteration
		//translate the point out of the middle of the Myosin rod in +x direction 
		glm::vec4 pos1Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos2Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos3Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//rotate each offset position into the right place around the myosin rod
		pos1Rot = glm::rotate(pos1Rot, glm::radians(angle1), glm::vec3(0.0f, 1.0f, 0.0f));
		pos2Rot = glm::rotate(pos2Rot, glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
		pos3Rot = glm::rotate(pos3Rot, glm::radians(angle3), glm::vec3(0.0f, 1.0f, 0.0f));
		//translate each offset along the y axis of the myosin rod. - i * m_LMMyOffset + 6.0f * m_LMMRadius to make it fit with the LMM part
		m_HMMOffsetPositions.push_back(glm::vec4(0.0f, -m_LMMLength - i * m_LMMyOffset + 6.0f * m_LMMRadius, 0.0f, 0.0f) + pos1Rot);
		m_HMMOffsetPositions.push_back(glm::vec4(0.0f, -m_LMMLength - i * m_LMMyOffset + 6.0f * m_LMMRadius, 0.0f, 0.0f) + pos2Rot);
		m_HMMOffsetPositions.push_back(glm::vec4(0.0f, -m_LMMLength - i * m_LMMyOffset + 6.0f * m_LMMRadius, 0.0f, 0.0f) + pos3Rot);
		//fill matrix to rotate the whole HMM part to the right angle around the myosin rod in the vertex shader
		m_HMMyRotMats.push_back(glm::rotate(glm::mat4(1.0f), glm::radians(angle1), glm::vec3(0.0f, 1.0f, 0.0f)));
		m_HMMyRotMats.push_back(glm::rotate(glm::mat4(1.0f), glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f)));
		m_HMMyRotMats.push_back(glm::rotate(glm::mat4(1.0f), glm::radians(angle3), glm::vec3(0.0f, 1.0f, 0.0f)));

		if (m_type == SarcomereType::TWO_TO_ONE)
		{
			m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull1 - m_HMMAngle);
			switch ((static_cast<int>(angle1 + 90) % 360))
			{
			case 0:
				angle = scaleFactor * 0.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 40:
				angle = scaleFactor * 20.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 80:
				angle = scaleFactor * -20.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 120:
				angle = scaleFactor * 0.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 160:
				angle = scaleFactor * 20.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 200:
				angle = scaleFactor * -20.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 240:
				angle = scaleFactor * 0.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 280:
				angle = scaleFactor * 20.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 320:
				angle = scaleFactor * -20.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			default:
				break;
			}
			m_HMMRotMat = glm::rotate(glm::mat4(1.0f), m_scaledAngle, glm::vec3(0.0f, 0.0f, -1.0f));
			//fill rotation matrix for animation
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);

			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
		}
		else if (m_type == SarcomereType::THREE_TO_ONE)
		{
			m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull2 - m_HMMAngle);
			switch ((static_cast<int>(angle1 + 90) % 360))
			{
			case 0:
				angle = scaleFactor * 30.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 40:
				angle = scaleFactor * -10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 80:
				angle = scaleFactor * 10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 120:
				angle = scaleFactor * 30.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 160:
				angle = scaleFactor * -1.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 200:
				angle = scaleFactor * 10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 240:
				angle = scaleFactor * 30.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 280:
				angle = scaleFactor * -10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 320:
				angle = scaleFactor * 10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			default:
				break;
			}
			m_HMMRotMat = glm::rotate(glm::mat4(1.0f), m_scaledAngle, glm::vec3(0.0f, 0.0f, -1.0f));
			//fill rotation matrix for animation
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);

			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
		}
		if (m_type == SarcomereType::FIVE_TO_ONE)
		{
			switch ((static_cast<int>(angle1 + 90) % 360))
			{
			case 0:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull1 - m_HMMAngle);
				angle = scaleFactor * 0.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 40:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull2 - m_HMMAngle);
				angle = scaleFactor * -10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 80:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull2 - m_HMMAngle);
				angle = scaleFactor * 10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 120:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull1 - m_HMMAngle);
				angle = scaleFactor * 0.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 160:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull2 - m_HMMAngle);
				angle = scaleFactor * -10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 200:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull2 - m_HMMAngle);
				angle = scaleFactor * 10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 240:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull1 - m_HMMAngle);
				angle = scaleFactor * 0.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 280:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull2 - m_HMMAngle);
				angle = scaleFactor * -10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 320:
				m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull2 - m_HMMAngle);
				angle = scaleFactor * 10.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			default:
				break;
			}
			m_HMMRotMat = glm::rotate(glm::mat4(1.0f), m_scaledAngle, glm::vec3(0.0f, 0.0f, -1.0f));
			//fill rotation matrix for animation
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);

			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
		}
		if (m_type == SarcomereType::SIX_TO_ONE)
		{
			m_scaledAngle = m_HMMAngle + scaleFactor * (HMMAngleFull3 - m_HMMAngle);
			switch ((static_cast<int>(angle1 + 90) % 360))
			{
			case 0:
				angle = scaleFactor * 15.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 40:
				angle = scaleFactor * 5.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 80:
				angle = scaleFactor * -5.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 120:
				angle = scaleFactor * 15.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 160:
				angle = scaleFactor * 5.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 200:
				angle = scaleFactor * -5.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 240:
				angle = scaleFactor * 15.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 280:
				angle = scaleFactor * 5.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 320:
				angle = scaleFactor * -5.0f;
				rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			default:
				break;
			}
			m_HMMRotMat = glm::rotate(glm::mat4(1.0f), m_scaledAngle, glm::vec3(0.0f, 0.0f, -1.0f));
			//fill rotation matrix for animation
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);
			m_HMMRotMatrices.push_back(m_HMMRotMat);

			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
			m_HMMyRotMatrices2.push_back(rotMat);
		}
		//increment each angle
		angle1 += alpha;
		angle2 += alpha;
		angle3 += alpha;
	}
	//reset starting angles
	angle1 = 30.0f;
	angle2 = 150.0f;
	angle3 = 270.0f;
	//generate HMM offset positions for the second Myosin half from the middle outwards
	for (int i = 0; i <= numHMMPerHalf - overlap; i++)
	{
		//set the three offset positions of the HMM for each iteration
		//translate the point out of the middle of the Myosin rod in +x direction 
		glm::vec4 pos1Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos2Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		glm::vec4 pos3Rot = glm::vec4(myosinTrunkRadius + m_LMMRadius, 0.0f, 0.0f, 0.0f);
		//rotate each offset position into the right place around the myosin rod
		pos1Rot = glm::rotate(pos1Rot, glm::radians(angle1), glm::vec3(0.0f, 1.0f, 0.0f));
		pos2Rot = glm::rotate(pos2Rot, glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
		pos3Rot = glm::rotate(pos3Rot, glm::radians(angle3), glm::vec3(0.0f, 1.0f, 0.0f));
		//translate each offset along the y axis of the myosin rod. + i * m_LMMyOffset - 6.0f * m_LMMRadius to make it fit with the LMM part
		m_HMMOffsetPositions.push_back(glm::vec4(0.0f, m_LMMLength + i * m_LMMyOffset - 6.0f * m_LMMRadius, 0.0f, 0.0f) + pos1Rot);
		m_HMMOffsetPositions.push_back(glm::vec4(0.0f, m_LMMLength + i * m_LMMyOffset - 6.0f * m_LMMRadius, 0.0f, 0.0f) + pos2Rot);
		m_HMMOffsetPositions.push_back(glm::vec4(0.0f, m_LMMLength + i * m_LMMyOffset - 6.0f * m_LMMRadius, 0.0f, 0.0f) + pos3Rot);
		//increment each angle
		angle1 += alpha;
		angle2 += alpha;
		angle3 += alpha;
	}
	//create ssbo for LMMoffsetPositions
	glCreateBuffers(1, &m_HMMOffsetPositions_ssbo);
	glNamedBufferStorage(m_HMMOffsetPositions_ssbo, sizeof(glm::vec4) * m_HMMOffsetPositions.size(), m_HMMOffsetPositions.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_HMMOffsetPositions_ssbo);

	//create ssbo for HMM y-axis rotation matricees
	glCreateBuffers(1, &m_HMMyRotationMatricees_ssbo);
	glNamedBufferStorage(m_HMMyRotationMatricees_ssbo, sizeof(glm::mat4) * m_HMMyRotMats.size(), m_HMMyRotMats.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_HMMyRotationMatricees_ssbo);

	//create ssbo for HMM z-axis rotation matricees
	glCreateBuffers(1, &m_HMMzRotationMatrices_ssbo);
	glNamedBufferStorage(m_HMMzRotationMatrices_ssbo, sizeof(glm::mat4) * m_HMMRotMatrices.size(), m_HMMRotMatrices.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_HMMzRotationMatrices_ssbo);

	//create ssbo for 2. HMM y-axis rotation matricees
	glCreateBuffers(1, &m_HMMyRotationMatrices2_ssbo);
	glNamedBufferStorage(m_HMMyRotationMatrices2_ssbo, sizeof(glm::mat4) * m_HMMyRotMatrices2.size(), m_HMMyRotMatrices2.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_HMMyRotationMatrices2_ssbo);
}

void Sarcomere::genMyosinHeads()
{
	m_myosinHeadOffsetPositions.clear();
	//generate myosin head offset positions for both myosin halfs from the middle outwards
	for (int i = 0; i < m_HMMOffsetPositions.size(); i++)
	{
		//gen two heads
		glm::vec4 headPos1;
		glm::vec4 headPos2;

		//set the first head pos to the last HMM1 position
		headPos1 = m_HMMPositions1.back();
		//set the second head pos to the last HMM2 position
		headPos2 = m_HMMPositions2.back();

		//rotate both heads with an angle of "m_HMMAngle" away from the rod
		if (i < m_HMMRotMatrices.size())
		{
			headPos1 = m_HMMRotMatrices[i] * headPos1;
			headPos2 = m_HMMRotMatrices[i] * headPos2;
		}
		else
		{
			headPos1 = m_HMMRotMatrices[i - m_HMMRotMatrices.size()] * headPos1;
			headPos2 = m_HMMRotMatrices[i - m_HMMRotMatrices.size()] * headPos2;
		}
		//headPos2 = glm::rotate(headPos2, m_scaledAngle, glm::vec3(0.0f, 0.0f, -1.0f));

		//rotate the first half of head pairs 180 degrees around the x axis of the ro to let them face the right way 
		if (i < m_HMMOffsetPositions.size() / 2)
		{
			headPos1 = glm::rotate(headPos1, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			headPos2 = glm::rotate(headPos2, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		//rotate both heads towards the next actin filament
		//rotate all heads with its coresponding HMM rotation matrix around the rod
		if (i < m_HMMyRotMats.size())
		{
			headPos1 = m_HMMyRotMats[i] * m_HMMyRotMatrices2[i] * headPos1;
			headPos2 = m_HMMyRotMats[i] * m_HMMyRotMatrices2[i] * headPos2;
		}
		else
		{
			headPos1 = m_HMMyRotMats[i - m_HMMyRotMats.size()] * m_HMMyRotMatrices2[i - m_HMMyRotMatrices2.size()] * headPos1;
			headPos2 = m_HMMyRotMats[i - m_HMMyRotMats.size()] * m_HMMyRotMatrices2[i - m_HMMyRotMatrices2.size()] * headPos2;
		}
		//translate each head with the right offset along the y axis
		headPos1 += m_HMMOffsetPositions[i];
		headPos2 += m_HMMOffsetPositions[i];
		//push back the head positions
		m_myosinHeadOffsetPositions.push_back(headPos1);
		m_myosinHeadOffsetPositions.push_back(headPos2);
	}

	//create ssbo for myosinHeadoffsetPositions
	glCreateBuffers(1, &m_myosinHeadOffsetPositions_ssbo);
	glNamedBufferStorage(m_myosinHeadOffsetPositions_ssbo, sizeof(glm::vec4) * m_myosinHeadOffsetPositions.size(), m_myosinHeadOffsetPositions.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_myosinHeadOffsetPositions_ssbo);
}

void Sarcomere::genTropomyosinBuffer()
{
	glGenBuffers(1, &m_linebuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_linebuffer);
	glBufferData(GL_ARRAY_BUFFER, m_tropomyosinPositions.size() * sizeof(glm::vec4), m_tropomyosinPositions.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_linebuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Sarcomere::bindTropomyosinBuffer()
{
	glBindVertexArray(m_vao);
}

void Sarcomere::genLMM1Buffer()
{
	glGenBuffers(1, &m_LMM1buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_LMM1buffer);
	glBufferData(GL_ARRAY_BUFFER, m_LMMPositions1.size() * sizeof(glm::vec4), m_LMMPositions1.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vao2);
	glBindVertexArray(m_vao2);

	glBindBuffer(GL_ARRAY_BUFFER, m_LMM1buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Sarcomere::genLMM2Buffer()
{
	glGenBuffers(1, &m_LMM2buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_LMM2buffer);
	glBufferData(GL_ARRAY_BUFFER, m_LMMPositions2.size() * sizeof(glm::vec4), m_LMMPositions2.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vao3);
	glBindVertexArray(m_vao3);

	glBindBuffer(GL_ARRAY_BUFFER, m_LMM2buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Sarcomere::genHMM1Buffer()
{
	glGenBuffers(1, &m_HMM1buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_HMM1buffer);
	glBufferData(GL_ARRAY_BUFFER, m_HMMPositions1.size() * sizeof(glm::vec4), m_HMMPositions1.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vao4);
	glBindVertexArray(m_vao4);

	glBindBuffer(GL_ARRAY_BUFFER, m_HMM1buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Sarcomere::genHMM2Buffer()
{
	glGenBuffers(1, &m_HMM2buffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_HMM2buffer);
	glBufferData(GL_ARRAY_BUFFER, m_HMMPositions2.size() * sizeof(glm::vec4), m_HMMPositions2.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vao5);
	glBindVertexArray(m_vao5);

	glBindBuffer(GL_ARRAY_BUFFER, m_HMM2buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}
void Sarcomere::bindLMM1Buffer()
{
	glBindVertexArray(m_vao2);
}
void Sarcomere::bindLMM2Buffer()
{
	glBindVertexArray(m_vao3);
}
void Sarcomere::bindHMM1Buffer()
{
	glBindVertexArray(m_vao4);
}
void Sarcomere::bindHMM2Buffer()
{
	glBindVertexArray(m_vao5);
}

void Sarcomere::deserialize(const char* filePath)
{
	std::ifstream  ifs(std::filesystem::path(filePath).string());
	nlohmann::json json;
	try
	{
		ifs >> json;
		m_type = static_cast<SarcomereType>(json["sarcomereType"].get<int>());
		sarcomereLength = json["sarcomereLength"];
		d10 = json["d10"];
		actinLength = json["actinLength"];
		actinRadius = json["actinRadius"];
		myosinLength = json["myosinLength"];
		myosinRadius = json["myosinRadius"];
		m_numMyosinRods = json["numMyosinFilaments"];
		konserveVolume = json["checkKonserveVolume"];
		highResActin = json["checkHighResActin"];
		highResMyosin = json["checkHighResMyosin"];
		actin = json["checkActin"];
		actinMonomers = json["checkActinMonomers"];
		tropomyosin = json["checkTropomyosin"];
		troponin = json["checkTroponin"];
		myosin = json["checkMyosin"];
		myosinTrunk = json["checkMyosinTrunk"];
		LMM = json["checkLMM"];
		HMM = json["checkHMM"];
		myosinHeads = json["checkMyosinHeads"];
		halfHelix = json["checkHalfHelix"];

		std::vector<float> color = json["actinColor"].get<std::vector<float>>();
		m_actinColor = glm::vec3(color[0], color[1], color[2]);

		color = json["tropomyosinColor"].get<std::vector<float>>();
		m_tropomyosinColor = glm::vec3(color[0], color[1], color[2]);

		color = json["troponinColor"].get<std::vector<float>>();
		m_troponinColor = glm::vec3(color[0], color[1], color[2]);

		color = json["myosinColor"].get<std::vector<float>>();
		m_myosinColor = glm::vec3(color[0], color[1], color[2]);

		color = json["LMMColor"].get<std::vector<float>>();
		m_LMMColor = glm::vec3(color[0], color[1], color[2]);

		color = json["HMMColor"].get<std::vector<float>>();
		m_HMMColor = glm::vec3(color[0], color[1], color[2]);

		color = json["myosinHeadColor"].get<std::vector<float>>();
		m_myosinHeadColor = glm::vec3(color[0], color[1], color[2]);

		std::vector<float> midPoint = json["sarcomereMidPoint"].get<std::vector<float>>();
		sarcomereMidPoint = glm::vec4(midPoint[0], midPoint[1], midPoint[2], midPoint[3]);
	}
	catch (const nlohmann::json::exception&)
	{
		tinyfd_notifyPopup("Load Distance Field",
			"Selected file cannot be parsed as "
			"a valid json object.",
			"error");
	}
}

nlohmann::json Sarcomere::serialize() const
{
	nlohmann::json out;
	out["sarcomereType"] = m_type;
	out["sarcomereLength"] = sarcomereLength;
	out["d10"] = d10;
	out["actinLength"] = actinLength;
	out["actinRadius"] = actinRadius;
	out["myosinLength"] = myosinLength;
	out["myosinRadius"] = myosinRadius;
	out["numMyosinFilaments"] = m_numMyosinRods;
	out["checkKonserveVolume"] = konserveVolume;
	out["checkHighResActin"] = highResActin;
	out["checkHighResMyosin"] = highResMyosin;
	out["checkActin"] = actin;
	out["checkActinMonomers"] = actinMonomers;
	out["checkTropomyosin"] = tropomyosin;
	out["checkTroponin"] = troponin;
	out["checkMyosin"] = myosin;
	out["checkMyosinTrunk"] = myosinTrunk;
	out["checkLMM"] = LMM;
	out["checkHMM"] = HMM;
	out["checkMyosinHeads"] = myosinHeads;
	out["checkHalfHelix"] = halfHelix;
	std::vector<float> midPoint{ sarcomereMidPoint.x, sarcomereMidPoint.y, sarcomereMidPoint.z, sarcomereMidPoint.a };
	out["sarcomereMidPoint"] = midPoint;
	std::vector<float> actinColor{ m_actinColor.x, m_actinColor.y, m_actinColor.z };
	out["actinColor"] = actinColor;
	std::vector<float> tropomyosinColor{ m_tropomyosinColor.x, m_tropomyosinColor.y, m_tropomyosinColor.z };
	out["tropomyosinColor"] = tropomyosinColor;
	std::vector<float> troponinColor{ m_troponinColor.x, m_troponinColor.y, m_troponinColor.z };
	out["troponinColor"] = troponinColor;
	std::vector<float> myosinColor{ m_myosinColor.x, m_myosinColor.y, m_myosinColor.z };
	out["myosinColor"] = myosinColor;
	std::vector<float> LMMColor{ m_LMMColor.x, m_LMMColor.y, m_LMMColor.z };
	out["LMMColor"] = LMMColor;
	std::vector<float> HMMColor{ m_HMMColor.x, m_HMMColor.y, m_HMMColor.z };
	out["HMMColor"] = HMMColor;
	std::vector<float> myosinHeadColor{ m_myosinHeadColor.x, m_myosinHeadColor.y, m_myosinHeadColor.z };
	out["myosinHeadColor"] = myosinHeadColor;
	return out;
}