#include "definitions.h"
#include "shaderProgram.h"
#include "trackballCamera.h"
#include "Camera.h"
#include "object.h"
#include "renderer.h"
#include "RenderCone.h"
#include "RenderSphere.h"
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <iostream>
#include <stbImage\stb_image.h>
#include <array>
#include "imGUI/imgui.h"
#include "imGUI/imgui_glfw.h"
#include <src/iconfont/IconsMaterialDesignIcons.h>
#include <src/tinyfiledialogs.h>
#include "Sarcomere.h"
#include<filesystem>

#define WIDTH 1920
#define HEIGHT 1080

std::unique_ptr<ImGui::ImGui> gui;
void updateFramerate(double currentTime, double lastTime, double frames, GLFWwindow* window) {
	std::ostringstream strs;
	strs << (currentTime - lastTime) * 1 / frames;
	std::string title = "MS per frame: " + strs.str();
	glfwSetWindowTitle(window, title.c_str());
}

/*****************************************Key Callbacks*****************************************/
static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		gui->keyCallback(window, key, scancode, action, mods);
		return;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, 1);
	}
}



int main() {
	/*****************************************Init GLFW Stuff*****************************************/
	if (!glfwInit())
		exit(EXIT_FAILURE);
	GLFWwindow* window;
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);*/
	/*glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Framework", 0, 0);
	glfwSetWindowPos(window, 0, 0);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glewInit();
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);

	double dt = 0.00001;
	glfwSetTime(0.0);
	double currentTime = glfwGetTime();
	double lastTime = glfwGetTime();
	double frameTime = lastTime;
	double accumulator = 0.0;
	double renderAccumulator = 0.0;
	double iterationCount = 0.0;

	Camera camera(WIDTH, HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 0.0001f, 10.0f);

	/*****************************************Imgui Init***************************************************/
	gui = std::make_unique<ImGui::ImGui>(window, false);
	glfwSetKeyCallback(window, key_callback);
	ImGui::StyleColorsClassic();

	glfwSetMouseButtonCallback(
		window, [](GLFWwindow * w, const int button, const int action, const int m) {
		gui->mouseButtonCallback(w, button, action, m);
	});
	glfwSetScrollCallback(window,
		[](GLFWwindow * w, const double xoffset, const double yoffset) {
		gui->scrollCallback(w, xoffset, yoffset);
	});
	glfwSetCharCallback(window, [](GLFWwindow * w, const unsigned int c) {
		gui->charCallback(w, c);
	});

	//generate Gometry
	glm::vec4 sarcomereMidPoint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	SarcomereType sarcomereType = SarcomereType::TWO_TO_ONE;
	//36 - 38 in a frog muscle according to Srboljub M. Mijailovich, Oliver Kayser-Herold, Boban Stojanovic, Djordje Nedic, Thomas C. Irving, Michael A. Geeves
	float d10 = 0.037f;
	//d11 is half of the distance between 2 Myosin Filamets (according to Claassen et al ~39/2 nm ) so d10 is sqrt(3) * d11
	float actinLength = 1.0f;
	int numMyosin = 500;
	std::unique_ptr<Sarcomere> sarcomere;

	/*****************************************Generate Matrices*****************************************/

	glm::mat4 rodRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 secondHalfRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 discRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	discRotationMatrix = glm::rotate(discRotationMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	glm::mat4 rodTranslationMatrix;
	glm::mat4 scaleActinLengthMatrix;
	glm::mat4 scaleActinWidthMatrix;
	glm::mat4 scaleMyosinLengthMatrix;
	glm::mat4 scaleMyosinWidthMatrix;
	glm::mat4 scaleMyosinTrunkWidthMatrix;
	glm::mat4 scaleSarcomereRadiusMatrix;

	float HMMAngleScale = 0.0f;

	//colors
	glm::vec3 actinColor;
	glm::vec3 myosinColor;
	glm::vec3 myosinHeadColor;
	glm::vec3 tropomyosinColor;
	glm::vec3 troponinColor;
	glm::vec3 LMMColor;
	glm::vec3 HMMColor;

	//objects
	std::unique_ptr<RenderCone> zDiscs;
	std::unique_ptr<RenderCone> myosinRods;
	std::unique_ptr<RenderCone> actinRods;
	std::unique_ptr<RenderSphere> actinParticles;

	/*****************************************Load Shaders*****************************************/
	ShaderProgram zBandShader = ShaderProgram(SHADERS_PATH "/zDisc.vert", SHADERS_PATH "/zDisc.frag");

	ShaderProgram aRodShader = ShaderProgram(SHADERS_PATH "/aRods.vert", SHADERS_PATH "/aRods.frag");

	ShaderProgram aSphereShader = ShaderProgram(SHADERS_PATH "/aSpheres.vert", SHADERS_PATH "/aSpheres.frag");

	ShaderProgram mRodShader = ShaderProgram(SHADERS_PATH "/mRods.vert", SHADERS_PATH "/mRods.frag");

	ShaderProgram tropomyosinShader = ShaderProgram(SHADERS_PATH "/tropomyosinLines.vert", SHADERS_PATH "/tropomyosinLines.frag", SHADERS_PATH "/tropomyosinLines.geom");

	ShaderProgram LMMShader = ShaderProgram(SHADERS_PATH "/LMMHelix.vert", SHADERS_PATH "/LMMHelix.frag", SHADERS_PATH "/LMMHelix.geom");

	ShaderProgram HMMShader = ShaderProgram(SHADERS_PATH "/HMMHelix.vert", SHADERS_PATH "/HMMHelix.frag", SHADERS_PATH "/HMMHelix.geom");

	ShaderProgram myosinHeadShader = ShaderProgram(SHADERS_PATH "/myosinHeads.vert", SHADERS_PATH "/myosinHeads.frag");

	ShaderProgram troponinShader = ShaderProgram(SHADERS_PATH "/troponinSpheres.vert", SHADERS_PATH "/troponinSpheres.frag");

	//imgui checkbox parameter
	bool b_konserveVolume = false;
	bool b_highResActin = false;
	bool b_highResMyosin = false;
	bool b_actin = false;
	bool b_actinMonomers = false;
	bool b_tropomyosin = false;
	bool b_troponin = false;
	bool b_myosin = false;
	bool b_myosinTrunk = false;
	bool b_LMM = false;
	bool b_HMM = false;
	bool b_myosinHeads = false;
	bool b_halfHelix = false;
	bool b_structureIsGenerated = false;
	bool b_fieldLoaded = false;
	GLuint vao;
	glGenVertexArrays(1, &vao);
	/*****************************************Render Loop***************************************************/
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		currentTime = glfwGetTime();
		dt = currentTime - lastTime;
		accumulator += dt;
		if (currentTime - frameTime > 1.0) {
			updateFramerate(currentTime, frameTime, iterationCount, window);
			frameTime = currentTime;
			std::cout << iterationCount << std::endl;
		}

		/*****************************************Update Imgui Parameters*****************************************/
		gui->newFrame();

		{
			float guiClearColor[3] = { 1.0,0.0,0.0 };
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("");
			if (ImGui::Button(ICON_MDI_CONTENT_SAVE " Save"))
			{
				const char* fileEnding = "*.json";
				const char* filePath = tinyfd_saveFileDialog("Save", nullptr, 1, &fileEnding, "JSON-Files");
				if (filePath)
				{
					//initialize imgui parameters
					sarcomere->konserveVolume = b_konserveVolume;
					sarcomere->highResActin = b_highResActin;
					sarcomere->highResMyosin = b_highResMyosin;
					sarcomere->actin = b_actin;
					sarcomere->actinMonomers = b_actinMonomers;
					sarcomere->tropomyosin = b_tropomyosin;
					sarcomere->troponin = b_troponin;
					sarcomere->myosin = b_myosin;
					sarcomere->myosinTrunk = b_myosinTrunk;
					sarcomere->LMM = b_LMM;
					sarcomere->HMM = b_HMM;
					sarcomere->myosinHeads = b_myosinHeads;
					//initialize colors
					sarcomere->setActinColor(actinColor);
					sarcomere->setMyosinColor(myosinColor);
					sarcomere->setMyosinHeadColor(myosinHeadColor);
					sarcomere->setTropomyosinColor(tropomyosinColor);
					sarcomere->setTroponinColor(troponinColor);
					sarcomere->setLMMColor(LMMColor);
					sarcomere->setHMMColor(HMMColor);
					nlohmann::json json = sarcomere->serialize();
					{
						std::filesystem::path path = filePath;
						while (path.extension() != ".json")
						{
							path = path.string() + ".json";
						}
						std::ofstream output(path);
						output << std::setw(4) << json;
						std::cout << "Sarcomere saved successfully.\n";
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_UPLOAD " Load"))
			{
				const char* fileEnding = "*.json";
				const char* filePath =
					tinyfd_openFileDialog("Load", nullptr, 1, &fileEnding, "JSON-Files", false);
				if (filePath)
				{
					if (sarcomere)
					{
						sarcomere->deserialize(filePath);
					}
					else
					{
						sarcomere = std::make_unique<Sarcomere>(filePath);
						b_fieldLoaded = true;
					}
					b_konserveVolume = sarcomere->konserveVolume;
					b_highResActin = sarcomere->highResActin;
					b_highResMyosin = sarcomere->highResMyosin;
					b_actin = sarcomere->actin;
					b_actinMonomers = sarcomere->actinMonomers;
					b_tropomyosin = sarcomere->tropomyosin;
					b_troponin = sarcomere->troponin;
					b_myosin = sarcomere->myosin;
					b_myosinTrunk = sarcomere->myosinTrunk;
					b_LMM = sarcomere->LMM;
					b_HMM = sarcomere->HMM;
					b_myosinHeads = sarcomere->myosinHeads;
					b_halfHelix = sarcomere->halfHelix;
					b_structureIsGenerated = false;
				}
			}
			ImGui::BeginVertical(1, ImVec2(0, 85));
			ImGui::Text("Actin");
			ImGui::Checkbox("genActin", &b_actin);
			ImGui::SameLine();
			if (b_actin)
			{
				ImGui::Checkbox("High Res?", &b_highResActin);
			}
			else
			{
				b_highResActin = false;
			}
			if (b_highResActin)
			{
				b_actinMonomers = true;
				ImGui::Text("Additional Actin Structur");
				ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
				ImGui::Checkbox("ActinMonomers", &b_actinMonomers);
				ImGui::PopStyleColor(1);
				ImGui::SameLine();
				ImGui::Checkbox("Tropomyosin", &b_tropomyosin);
				ImGui::SameLine();
				ImGui::Checkbox("Troponin", &b_troponin);
			}
			ImGui::EndVertical();
			/*****************************************************************************/
			ImGui::BeginVertical(2, ImVec2(0, 110));
			ImGui::Text("Myosin");
			ImGui::Checkbox("genMyosin", &b_myosin);
			ImGui::SameLine();
			if (b_myosin)
			{
				ImGui::Checkbox("High Res?", &b_highResMyosin);
			}
			else
			{
				b_highResMyosin = false;
			}
			if (b_highResMyosin)
			{
				b_myosinTrunk = true;
				ImGui::Text("Additional Myosin Structur");
				ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
				ImGui::Checkbox("MyosinTrunk", &b_myosinTrunk);
				ImGui::PopStyleColor(1);
				ImGui::SameLine();
				ImGui::Checkbox("Myosin Heads", &b_myosinHeads);

				ImGui::Checkbox("LMM Structure", &b_LMM);
				ImGui::SameLine();
				ImGui::Checkbox("HMM Structure", &b_HMM);
				ImGui::SameLine();
				ImGui::Checkbox("only one Helix", &b_halfHelix);
			}
			ImGui::EndVertical();
			const char* sarcomereTypes[] = { "TWO_TO_ONE", "THREE_TO_ONE", "FIVE_TO_ONE", "SIX_TO_ONE" };
			static int sType = static_cast<int>(sarcomereType);
			ImGui::Combo("Sarcomere Type", &sType, sarcomereTypes, IM_ARRAYSIZE(sarcomereTypes));

			ImGui::InputFloat("d10", &d10, 0.001f, 1.0f);
			ImGui::InputFloat(" actinlength", &actinLength, 0.01f, 1.0f);
			ImGui::InputFloat3("Sarcomere Origin", &sarcomereMidPoint.x);
			ImGui::InputInt("number of myosin rods", &numMyosin);
			if (ImGui::Button("Generate Sarcomere") || b_fieldLoaded == true)
			{
				b_structureIsGenerated = false;
				//initialize sarcomere data
				if (!b_fieldLoaded)
				{
					if (sType == 0)
					{
						sarcomere = std::make_unique<Sarcomere>(SarcomereType::TWO_TO_ONE, d10, actinLength, numMyosin, sarcomereMidPoint);
					}
					else if (sType == 1)
					{
						sarcomere = std::make_unique<Sarcomere>(SarcomereType::THREE_TO_ONE, d10, actinLength, numMyosin, sarcomereMidPoint);
					}
					else if (sType == 2)
					{
						sarcomere = std::make_unique<Sarcomere>(SarcomereType::FIVE_TO_ONE, d10, actinLength, numMyosin, sarcomereMidPoint);
					}
					else if (sType == 3)
					{
						sarcomere = std::make_unique<Sarcomere>(SarcomereType::SIX_TO_ONE, d10, actinLength, numMyosin, sarcomereMidPoint);
					}
					else
					{
						break;
					}
				}
				b_fieldLoaded = false;
				//initialize imgui parameters
				sarcomere->konserveVolume = b_konserveVolume;
				sarcomere->highResActin = b_highResActin;
				sarcomere->highResMyosin = b_highResMyosin;
				sarcomere->actin = b_actin;
				sarcomere->actinMonomers = b_actinMonomers;
				sarcomere->tropomyosin = b_tropomyosin;
				sarcomere->troponin = b_troponin;
				sarcomere->myosin = b_myosin;
				sarcomere->myosinTrunk = b_myosinTrunk;
				sarcomere->LMM = b_LMM;
				sarcomere->HMM = b_HMM;
				sarcomere->myosinHeads = b_myosinHeads;
				sarcomere->halfHelix = b_halfHelix;
				//initialize colors
				actinColor = sarcomere->getActinColor();
				myosinColor = sarcomere->getMyosinColor();
				myosinHeadColor = sarcomere->getMyosinHeadColor();
				tropomyosinColor = sarcomere->getTropomyosinColor();
				troponinColor = sarcomere->getTroponinColor();
				LMMColor = sarcomere->getLMMColor();
				HMMColor = sarcomere->getHMMColor();

				//initialize Matrizees
				rodTranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -sarcomere->sarcomereLength / 2.0f, 0.0f));
				scaleActinLengthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, sarcomere->actinLength, 1.0f));
				scaleMyosinLengthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, sarcomere->myosinLength, 1.0f));
				scaleMyosinTrunkWidthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->myosinTrunkRadius, 1.0f, sarcomere->myosinTrunkRadius));
				scaleMyosinWidthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->myosinRadius, 1.0f, sarcomere->myosinRadius));
				scaleSarcomereRadiusMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->getRadius(), 1.0f, sarcomere->getRadius()));


				//initialize objects
				zDiscs = std::make_unique<RenderCone>(glm::vec3(sarcomere->getMidPoint()), 1.0f, 1.0f, 0.01f, 6, 1);
				myosinRods = std::make_unique<RenderCone>(glm::vec3(sarcomere->getMidPoint()), 1.0f, 1.0f, 1.0f, 10, 1);
				actinRods = std::make_unique<RenderCone>(glm::vec3(sarcomere->getMidPoint()), 1.0f, 1.0f, 1.0f, 10, 1);

				GLint m_viewport[4];
				glGetIntegerv(GL_VIEWPORT, m_viewport);
				int yViewport = m_viewport[2];
				if (!b_structureIsGenerated)
				{
					if (sarcomere->numParticles < 1)
					{
						sarcomere->generateDoubleHelixOffsetPositions();
					}
					if (sarcomere->myosinIsGenerated == false)
					{
						sarcomere->genLMM();
					}

					scaleActinWidthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->actinRadius / 2.0f));
					aSphereShader.updateUniform("projectionMatrix", camera.projection());
					aSphereShader.updateUniform("rotationMatrix", rodRotationMatrix);
					aSphereShader.updateUniform("scaleWidthMatrix", scaleActinWidthMatrix);
					aSphereShader.updateUniform("numParticles", sarcomere->numParticles);
					aSphereShader.updateUniform("basePointSize", sarcomere->actinRadius / 2.0f);
					aSphereShader.updateUniform("viewportY", yViewport);

					tropomyosinShader.updateUniform("projectionMatrix", camera.projection());
					tropomyosinShader.updateUniform("radius", sarcomere->actinRadius / 8.0f);
					tropomyosinShader.updateUniform("numLineSegments", sarcomere->getNumLineSegments());
					auto test = sarcomere->getNumLineSegments();
					tropomyosinShader.updateUniform("secondHalfRotationMatrix", secondHalfRotationMatrix);
					tropomyosinShader.updateUniform("rotationMatrix", rodRotationMatrix);
					tropomyosinShader.updateUniform("translationMatrix", rodTranslationMatrix);
					tropomyosinShader.updateUniform("pointDist", sarcomere->actinRadius);
					tropomyosinShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					tropomyosinShader.updateUniform("diffColor", tropomyosinColor);

					troponinShader.updateUniform("projectionMatrix", camera.projection());
					troponinShader.updateUniform("rotationMatrix", rodRotationMatrix);
					troponinShader.updateUniform("scaleWidthMatrix", scaleActinWidthMatrix);
					troponinShader.updateUniform("numParticles", sarcomere->getNumTroponinParticles());
					troponinShader.updateUniform("basePointSize", sarcomere->actinRadius / 4.0f);
					troponinShader.updateUniform("viewportY", yViewport);
					troponinShader.updateUniform("diffColor", troponinColor);

					scaleActinWidthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->actinRadius, 1.0f, sarcomere->actinRadius));
					aRodShader.updateUniform("projectionMatrix", camera.projection());
					aRodShader.updateUniform("rotationMatrix", rodRotationMatrix);
					aRodShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					aRodShader.updateUniform("translationMatrix", rodTranslationMatrix);
					aRodShader.updateUniform("secondHalfRotationMatrix", secondHalfRotationMatrix);
					aRodShader.updateUniform("scaleHeightMatrix", scaleActinLengthMatrix);
					aRodShader.updateUniform("scaleWidthMatrix", scaleActinWidthMatrix);
					aRodShader.updateUniform("diffColor", actinColor);

					mRodShader.updateUniform("projectionMatrix", camera.projection());
					mRodShader.updateUniform("rotationMatrix", rodRotationMatrix);
					mRodShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					mRodShader.updateUniform("translationMatrix", rodTranslationMatrix);
					mRodShader.updateUniform("myosinLength", sarcomere->myosinLength);
					mRodShader.updateUniform("scaleHeightMatrix", scaleMyosinLengthMatrix);
					mRodShader.updateUniform("diffColor", myosinColor);

					LMMShader.updateUniform("projectionMatrix", camera.projection());
					LMMShader.updateUniform("rotationMatrix", rodRotationMatrix);
					LMMShader.updateUniform("radius", sarcomere->myosinTrunkRadius / 20.0f);
					LMMShader.updateUniform("numLineSegments", sarcomere->getNumLMMOffsetPositionsPerRod());
					LMMShader.updateUniform("secondHalfRotationMatrix", secondHalfRotationMatrix);
					LMMShader.updateUniform("diffColor", LMMColor);

					HMMShader.updateUniform("projectionMatrix", camera.projection());
					HMMShader.updateUniform("rotationMatrix", rodRotationMatrix);
					HMMShader.updateUniform("radius", sarcomere->myosinTrunkRadius / 20.0f);
					HMMShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
					HMMShader.updateUniform("secondHalfRotationMatrix", secondHalfRotationMatrix);
					HMMShader.updateUniform("diffColor", HMMColor);

					myosinHeadShader.updateUniform("projectionMatrix", camera.projection());
					myosinHeadShader.updateUniform("rotationMatrix", rodRotationMatrix);
					myosinHeadShader.updateUniform("scaleWidthMatrix", scaleMyosinWidthMatrix);
					myosinHeadShader.updateUniform("numParticles", sarcomere->getNumMyosinHeads());
					myosinHeadShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
					myosinHeadShader.updateUniform("basePointSize", sarcomere->myosinHeadRadius);
					myosinHeadShader.updateUniform("viewportY", yViewport);
					myosinHeadShader.updateUniform("diffColor", myosinHeadColor);
				}

				//update Uniforms
				zBandShader.updateUniform("projectionMatrix", camera.projection());
				zBandShader.updateUniform("rotationMatrix", discRotationMatrix);
				zBandShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
				zBandShader.updateUniform("sarcomereRadius", scaleSarcomereRadiusMatrix);

				b_structureIsGenerated = true;
			}

			if (sarcomere)
			{
				if (b_actin && b_highResActin)
				{
					ImGui::ColorEdit3("Actin Color", glm::value_ptr(actinColor));
					aSphereShader.updateUniform("diffColor", sarcomere->getActinColor());
					aRodShader.updateUniform("diffColor", actinColor);
					sarcomere->setActinColor(actinColor);
				}
				if (b_actin && !b_highResActin)
				{
					if (ImGui::ColorEdit3("Actin Color", glm::value_ptr(actinColor)))
					{
						aRodShader.updateUniform("diffColor", actinColor);
						aSphereShader.updateUniform("diffColor", actinColor);
						sarcomere->setActinColor(actinColor);
					}
				}
				if (b_highResActin)
				{
					if (b_tropomyosin)
					{
						if (ImGui::ColorEdit3("tropomyosin Color", glm::value_ptr(tropomyosinColor)))
						{
							tropomyosinShader.updateUniform("diffColor", tropomyosinColor);
							sarcomere->setTropomyosinColor(tropomyosinColor);
						}
					}
					if (b_troponin)
					{
						if (ImGui::ColorEdit3("troponin Color", glm::value_ptr(troponinColor)))
						{
							troponinShader.updateUniform("diffColor", troponinColor);
							sarcomere->setTropomyosinColor(troponinColor);
						}
					}
				}
				if (b_myosin)
				{
					if (ImGui::ColorEdit3("Myosin Color", glm::value_ptr(myosinColor)))
					{
						mRodShader.updateUniform("diffColor", myosinColor);
						sarcomere->setMyosinColor(myosinColor);
					}
				}
				if (b_highResMyosin)
				{
					if (b_LMM)
					{
						if (ImGui::ColorEdit3("LMM Color", glm::value_ptr(LMMColor)))
						{
							LMMShader.updateUniform("diffColor", LMMColor);
							sarcomere->setLMMColor(LMMColor);
						}
					}
					if (b_HMM)
					{
						if (ImGui::ColorEdit3("HMM Color", glm::value_ptr(HMMColor)))
						{
							HMMShader.updateUniform("diffColor", HMMColor);
							sarcomere->setHMMColor(HMMColor);
						}
					}
					if (b_myosinHeads)
					{
						if (ImGui::ColorEdit3("Myosin Head Color", glm::value_ptr(myosinHeadColor)))
						{
							myosinHeadShader.updateUniform("diffColor", myosinHeadColor);
							sarcomere->setMyosinHeadColor(myosinHeadColor);
						}
					}
				}
				ImGui::Checkbox("Konserve Volume", &b_konserveVolume);

				if (b_actin)
				{
					ImGui::DragFloat("scaleActinLength", &sarcomere->actinLength, 0.01f, 0.01f, 4.0f);
					if (sarcomere->actinLength != sarcomere->oldActinLength)
					{
						scaleActinLengthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, sarcomere->actinLength, 1.0f));
						if (b_highResActin)
						{
							sarcomere->generateDoubleHelixOffsetPositions();

							aSphereShader.updateUniform("numParticles", sarcomere->numParticles);

							if (b_tropomyosin)
							{
								tropomyosinShader.updateUniform("numLineSegments", sarcomere->getNumLineSegments());
							}
							if (b_troponin)
							{
								troponinShader.updateUniform("numParticles", sarcomere->getNumTroponinParticles());
							}
						}
						else
						{
							aRodShader.updateUniform("scaleHeightMatrix", scaleActinLengthMatrix);
						}
						sarcomere->actinLengthScalePercentage = sarcomere->actinLength / sarcomere->sarcomereLength;
						sarcomere->oldActinLength = sarcomere->actinLength;
					}

					ImGui::DragFloat("scaleActinWidth", &sarcomere->actinRadius, 0.0001f, 0.0001f, 0.0001f);
					if (sarcomere->actinRadius != sarcomere->oldActinRadius)
					{
						scaleActinWidthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->actinRadius, 1.0f, sarcomere->actinRadius));

						if (b_highResActin)
						{
							sarcomere->generateDoubleHelixOffsetPositions();

							aSphereShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
							aSphereShader.updateUniform("numParticles", sarcomere->numParticles);
							aSphereShader.updateUniform("scaleWidthMatrix", scaleActinWidthMatrix);
							aSphereShader.updateUniform("basePointSize", sarcomere->actinRadius / 2.0f);
							if (b_tropomyosin)
							{
								tropomyosinShader.updateUniform("radius", sarcomere->actinRadius / 8.0f);
								tropomyosinShader.updateUniform("pointDist", sarcomere->actinRadius);
								tropomyosinShader.updateUniform("scaleWidthMatrix", scaleActinWidthMatrix);
								tropomyosinShader.updateUniform("numActinFilaments", sarcomere->getNumActin() / 2);
								tropomyosinShader.updateUniform("numLineSegments", sarcomere->getNumLineSegments());
							}
							if (b_troponin)
							{
								troponinShader.updateUniform("scaleWidthMatrix", scaleActinWidthMatrix);
								troponinShader.updateUniform("numParticles", sarcomere->getNumTroponinParticles());
								troponinShader.updateUniform("numParticles", sarcomere->getNumTroponinParticles());
								troponinShader.updateUniform("basePointSize", sarcomere->actinRadius / 4.0f);
							}
						}
						else
						{
							aRodShader.updateUniform("scaleWidthMatrix", scaleActinWidthMatrix);
						}
						sarcomere->actinRadiusScalePercentage = sarcomere->actinRadius / sarcomere->d10;
						sarcomere->oldActinRadius = sarcomere->actinRadius;
					}
				}
				if (b_myosin)
				{
					ImGui::DragFloat("scaleMyosinLength", &sarcomere->myosinLength, 0.01f, 0.01f, 6.0f);
					if (sarcomere->myosinLength != sarcomere->oldMyosinLength)
					{
						scaleMyosinLengthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, sarcomere->myosinLength, 1.0f));
						mRodShader.updateUniform("scaleHeightMatrix", scaleMyosinLengthMatrix);
						mRodShader.updateUniform("myosinLength", sarcomere->myosinLength);

						if (b_highResMyosin)
						{
							sarcomere->genLMM();
							if (b_LMM)
							{
								LMMShader.updateUniform("numLineSegments", sarcomere->getNumLMMOffsetPositionsPerRod());
							}
							if (b_HMM)
							{
								HMMShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
							}
							if (b_myosinHeads)
							{
								myosinHeadShader.updateUniform("numParticles", sarcomere->getNumMyosinHeads());
							}
						}
						sarcomere->myosinLengthScalePercentage = sarcomere->myosinLength / sarcomere->sarcomereLength;
						sarcomere->oldMyosinLength = sarcomere->myosinLength;
					}
					if (ImGui::DragFloat("Scale HMM Angle", &HMMAngleScale, 0.01f, 0.00f, 1.00f))
					{
						sarcomere->genHMMOffsetPositions(HMMAngleScale);
						sarcomere->genMyosinHeads();
					}
					ImGui::DragFloat("scaleMyosinWidth", &sarcomere->myosinRadius, 0.0001f, 0.0001f, 0.0001f);
					if (sarcomere->myosinRadius != sarcomere->oldMyosinRadius)
					{
						sarcomere->myosinTrunkRadius = sarcomere->myosinRadius / 3.0f;
						scaleMyosinTrunkWidthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->myosinTrunkRadius, 1.0f, sarcomere->myosinTrunkRadius));
						scaleMyosinWidthMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->myosinRadius, 1.0f, sarcomere->myosinRadius));
						if (b_highResMyosin)
						{
							mRodShader.updateUniform("scaleWidthMatrix", scaleMyosinTrunkWidthMatrix);
							sarcomere->updateHMMAngle();
							sarcomere->genLMM();
							sarcomere->genHMMOffsetPositions(HMMAngleScale);
							sarcomere->genMyosinHeads();
							myosinHeadShader.updateUniform("basePointSize", sarcomere->myosinRadius / 6.0f);
							if (b_LMM)
							{
								LMMShader.updateUniform("radius", sarcomere->myosinTrunkRadius / 20.0f);
								LMMShader.updateUniform("numLineSegments", sarcomere->getNumLMMOffsetPositionsPerRod());
							}
							if (b_HMM)
							{
								HMMShader.updateUniform("radius", sarcomere->myosinTrunkRadius / 20.0f);
								HMMShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
							}
						}
						else
						{
							mRodShader.updateUniform("scaleWidthMatrix", scaleMyosinWidthMatrix);
						}
						sarcomere->myosinRadiusScalePercentage = sarcomere->myosinRadius / sarcomere->d10;
						sarcomere->oldMyosinRadius = sarcomere->myosinRadius;
					}

				}
				ImGui::DragFloat("scaleSarcomereLength", &sarcomere->sarcomereLength, 0.001f, 0.01f, 8.0f);
				if (sarcomere->sarcomereLength != sarcomere->oldSarcomereLength)
				{
					if (b_konserveVolume == true)
					{
						//derive new d10 value based on a fixed sarcomere volume and the new sarcomere length
						sarcomere->updateD10();
						sarcomere->updateOffsetBuffers();
						sarcomere->oldD10 = sarcomere->d10;
						scaleSarcomereRadiusMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->getRadius(), 1.0f, sarcomere->getRadius()));
						zBandShader.updateUniform("sarcomereRadius", scaleSarcomereRadiusMatrix);
						if (HMMAngleScale == 1)
						{
							sarcomere->updateHMMLength();
						}
						sarcomere->genHMMOffsetPositions(HMMAngleScale);
						sarcomere->genMyosinHeads();
					}
					else
					{
						sarcomere->updateVolume();
					}
					mRodShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					zBandShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					aSphereShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					tropomyosinShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					aRodShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					sarcomere->generateDoubleHelixOffsetPositions();
					if (b_actin)
					{
						if (b_highResActin)
						{

							aSphereShader.updateUniform("numParticles", sarcomere->numParticles);
							if (b_tropomyosin)
							{
								tropomyosinShader.updateUniform("numLineSegments", sarcomere->getNumLineSegments());
							}
							if (b_troponin)
							{
								troponinShader.updateUniform("numParticles", sarcomere->getNumTroponinParticles());
							}
						}
					}
					if (b_myosin)
					{
						mRodShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
						if (b_highResMyosin)
						{
							sarcomere->genLMM();
							sarcomere->genHMMOffsetPositions(HMMAngleScale);
							sarcomere->genMyosinHeads();

							if (b_LMM)
							{
								LMMShader.updateUniform("numLineSegments", sarcomere->getNumLMMOffsetPositionsPerRod());
							}
							if (b_HMM)
							{
								HMMShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
							}
							if (b_myosinHeads)
							{
								myosinHeadShader.updateUniform("numParticles", sarcomere->getNumMyosinHeads());
								myosinHeadShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
							}
						}
					}
					sarcomere->oldSarcomereLength = sarcomere->sarcomereLength;
				}
				ImGui::DragFloat("scaled10", &sarcomere->d10, 0.0001f, 0.0001f, 0.4f);
				if (sarcomere->d10 != sarcomere->oldD10)
				{
					if (b_konserveVolume == true)
					{
						//derive new sarcomere length based on a fixed sarcomere volume and the new d10 value
						sarcomere->updateLength();
						sarcomere->oldSarcomereLength = sarcomere->sarcomereLength;
						sarcomere->generateDoubleHelixOffsetPositions();
						aSphereShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
						tropomyosinShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
						aRodShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
						if (b_actin)
						{
							if (b_highResActin)
							{

								aSphereShader.updateUniform("numParticles", sarcomere->numParticles);
								if (b_tropomyosin)
								{
									tropomyosinShader.updateUniform("numLineSegments", sarcomere->getNumLineSegments());
								}
								if (b_troponin)
								{
									troponinShader.updateUniform("numParticles", sarcomere->getNumTroponinParticles());
								}
							}
						}
						if (b_myosin)
						{
							mRodShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
							if (b_highResMyosin)
							{
								sarcomere->genLMM();

								if (b_LMM)
								{
									LMMShader.updateUniform("numLineSegments", sarcomere->getNumLMMOffsetPositionsPerRod());
								}
								if (b_HMM)
								{
									HMMShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
								}
								if (b_myosinHeads)
								{
									myosinHeadShader.updateUniform("numParticles", sarcomere->getNumMyosinHeads());
									myosinHeadShader.updateUniform("numLineSegments", sarcomere->getNumHMMOffsetPositionsPerRod());
								}
							}
						}
						if (HMMAngleScale == 1)
						{
							sarcomere->updateHMMLength();
						}
						zBandShader.updateUniform("sarcomereLength", sarcomere->sarcomereLength);
					}
					else
					{
						sarcomere->updateVolume();
					}
					sarcomere->updateOffsetBuffers();
					scaleSarcomereRadiusMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(sarcomere->getRadius(), 1.0f, sarcomere->getRadius()));
					sarcomere->genHMMOffsetPositions(HMMAngleScale);
					sarcomere->genMyosinHeads();
					zBandShader.updateUniform("sarcomereRadius", scaleSarcomereRadiusMatrix);
					sarcomere->oldD10 = sarcomere->d10;
				}
				ImGui::Text("sarcomereVolume = %f", sarcomere->getVolume());
				if (b_myosin)
				{
					ImGui::Text("number of myosin rods = %d", sarcomere->getNumMyosin());
				}
				if (b_actin)
				{
					ImGui::Text("number of actin rods = %d", sarcomere->getNumActin() / 2);
					if (b_actinMonomers)
					{
						ImGui::Text("number of actin monomers = %d", (sarcomere->getNumActin() / 2) * sarcomere->getNumActinParticles());
					}
					if (b_tropomyosin)
					{
						ImGui::Text("number of trpomyosin lines = %d", (sarcomere->getNumActin() / 2) * sarcomere->getNumLineSegments());
					}
					if (b_troponin)
					{
						ImGui::Text("number troponin particles = %d", (sarcomere->getNumActin() / 2) * sarcomere->getNumTroponinParticles());
					}
				}
			}
		}

		camera.update(window);

		if (sarcomere)
		{
			//rebind ssbos
			sarcomere->bindBuffers();
			//render data
			//render zDiscs
			zBandShader.use();
			zBandShader.updateUniform("viewMatrix", camera.view());
			zDiscs->render(sarcomere->getNumZdiscs());
			//bind empty vao because otherwise it binds a wrong one
			glBindVertexArray(vao);

			//render Myosin
			if (b_myosin)
			{
				mRodShader.use();
				if (b_highResMyosin)
				{
					mRodShader.updateUniform("scaleWidthMatrix", scaleMyosinTrunkWidthMatrix);
				}
				else
				{
					mRodShader.updateUniform("scaleWidthMatrix", scaleMyosinWidthMatrix);
				}
				mRodShader.updateUniform("viewMatrix", camera.view());
				myosinRods->render(sarcomere->getNumMyosin());
				if (b_highResMyosin)
				{
					if (b_LMM)
					{
						//render first LMM Helix
						LMMShader.use();
						LMMShader.updateUniform("viewMatrix", camera.view());
						sarcomere->bindLMM1Buffer();
						glDrawArraysInstanced(GL_LINE_STRIP_ADJACENCY, 0, sarcomere->getNumPointsPerLMMHelix(), sarcomere->getNumLMMOffsetPositionsPerRod() * sarcomere->getNumMyosin());

						//render second LMM Helix
						if (!b_halfHelix)
						{
							LMMShader.use();
							LMMShader.updateUniform("viewMatrix", camera.view());
							sarcomere->bindLMM2Buffer();
							glDrawArraysInstanced(GL_LINE_STRIP_ADJACENCY, 0, sarcomere->getNumPointsPerLMMHelix(), sarcomere->getNumLMMOffsetPositionsPerRod() * sarcomere->getNumMyosin());
						}
					}
					if (b_HMM)
					{
						//render first HMM Helix
						HMMShader.use();
						HMMShader.updateUniform("viewMatrix", camera.view());
						sarcomere->bindHMM1Buffer();
						glDrawArraysInstanced(GL_LINE_STRIP_ADJACENCY, 0, sarcomere->getNumPointsPerHMMHelix(), sarcomere->getNumHMMOffsetPositionsPerRod() * sarcomere->getNumMyosin());

						//render second HMM Helix
						if (!b_halfHelix)
						{
							HMMShader.use();
							HMMShader.updateUniform("viewMatrix", camera.view());
							sarcomere->bindHMM2Buffer();
							glDrawArraysInstanced(GL_LINE_STRIP_ADJACENCY, 0, sarcomere->getNumPointsPerHMMHelix(), sarcomere->getNumHMMOffsetPositionsPerRod() * sarcomere->getNumMyosin());
						}
					}
					if (b_myosinHeads)
					{
						//render myosin heads
						myosinHeadShader.use();
						myosinHeadShader.updateUniform("viewMatrix", camera.view());
						glDrawArraysInstanced(GL_POINTS, 0, 1, sarcomere->getNumMyosin() * sarcomere->getNumMyosinHeads());
					}
				}

			}
			//render actin
			if (b_actin)
			{
				//if high res render double helix actin structure
				if (b_highResActin)
				{
					//render actin monomers
					aSphereShader.use();
					aSphereShader.updateUniform("viewMatrix", camera.view());
					glDrawArraysInstanced(GL_POINTS, 0, 1, (sarcomere->getNumActin() * sarcomere->getNumActinParticles()) / 2);
					if (b_troponin)
					{
						//render troponin
						troponinShader.use();
						troponinShader.updateUniform("viewMatrix", camera.view());
						glDrawArraysInstanced(GL_POINTS, 0, 1, (sarcomere->getNumActin() / 2) * sarcomere->getNumTroponinParticles());
					}
					if (b_tropomyosin)
					{
						//render tropomyosin
						tropomyosinShader.use();
						tropomyosinShader.updateUniform("viewMatrix", camera.view());
						sarcomere->bindTropomyosinBuffer();
						int test = (static_cast<int>(sarcomere->getNumActinParticles() / 2 / 7) / 2) * 2;
						glDrawArraysInstanced(GL_LINE_STRIP_ADJACENCY, 0, 9, (sarcomere->getNumActin() / 2) * sarcomere->getNumLineSegments());
					}
				}
				//if no high res render simple actin rod structure
				else
				{
					//render actin rods
					aRodShader.use();
					aRodShader.updateUniform("viewMatrix", camera.view());
					actinRods->render(sarcomere->getNumActin());
				}

			}
		}
		/*if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
		{
			zBandShader.reload(0);
			aRodShader.reload(0);
			aSphereShader.reload(1);
			mRodShader.reload(0);
			tropomyosinShader.reload(0);
			LMMShader.reload(0);
			HMMShader.reload(0);
			myosinHeadShader.reload(1);
			troponinShader.reload(1);	
		}*/
		gui->render();
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
}