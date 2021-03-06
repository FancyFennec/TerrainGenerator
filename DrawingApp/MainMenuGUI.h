#pragma once

#include <iostream>
#include <fstream>

#include "Imgui/imgui.h"
#include "Imgui/imgui-SFML.h"
#include "GlobalVariables.h"
#include "Scene.h"
#include "CommandManager.h"

void mainMenuGUI(Scene& scene);
void DrawEditMenu();
void OpenFilePopup(Scene & scene);
void SaveFilePopup(const Scene& scene);
void DrawFileMenu();
void loadLayerFromFile(const std::string& folderPath, const std::string& fileName, Scene & scene);
void loadScene(const std::string& fileName, Scene & scene);
sf::Texture extractTextureFromBuffer(int width, int height, const std::vector<char>& buffer, int layerCount);
void loadDataIntoScene(int layerCount, const sf::Texture &tex, Scene & scene, const std::vector<std::string> &layerNames);
void extractMetaData(std::ifstream &newistrm, int &layerCount, int &width, int &height);
std::vector<char> extractImageData(std::ifstream &newistrm);
std::vector<std::string> extractLayerNames(int layerCount, std::ifstream &newistrm);
void saveScene(const Scene& scene, const std::string& tmpPath, const std::string& folderPath);
void saveTemporaryTextureDataToFile(std::ifstream &istrm, std::ofstream &ostrm);
void saveLayerNames(const Scene& scene, std::ofstream &ostrm);
void saveMetaData(const Scene& scene, std::ofstream &ostrm);
void saveTemporaryTextureData(const Scene& scene, const std::string& tmpPath);
void saveCurrentLayerAsPNG(const Scene& scene, const std::string& folderPath);
void saveCurrentLayerAsJPEG(const Scene& scene, const std::string& folderPath);
void saveSceneAsPNG(const Scene& scene, const std::string& folderPath);
void saveSceneAsJPEG(const Scene& scene);

bool useAlphaDrawingState = true;
bool saveFilePopupIsOpen = false;
bool openFilePopupIsOpen = false;
std::string input;

enum class FILE_TYPE {
	PNG,
	JPEG,
	SINGLE_PNG,
	SINGLE_JPEG,
	SCN
};

FILE_TYPE file_type;

void mainMenuGUI(Scene& scene)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			DrawFileMenu();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			DrawEditMenu();
			ImGui::EndMenu();
		}

		if (ImGui::ArrowButton("##Back", ImGuiDir_Left)) CommandManager::moveBackward();
		if (ImGui::ArrowButton("##Forward", ImGuiDir_Right)) CommandManager::moveForward();
		if (ImGui::Checkbox("GUI##Checkbox", &SHOW_GUI));
		if (ImGui::Checkbox("Alpha##Checkbox", &useAlphaDrawingState)) DRAWING_STATE = useAlphaDrawingState ? DrawingState::ALPHA : DrawingState::NORMAL;

		ImGui::SameLine(WINDOW_WIDTH - 60);
		if (ImGui::Button("Close")) mainWindow.close();

		ImGui::EndMainMenuBar();

		if (saveFilePopupIsOpen) SaveFilePopup(scene);
		if (openFilePopupIsOpen) OpenFilePopup(scene);
	}
}

void DrawEditMenu()
{
	if (ImGui::MenuItem("menu item"))
	{
		std::cout << "Menu item pressed" << std::endl;
	}
}

void OpenFilePopup(Scene & scene)
{
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	ImGui::OpenPopup("Open File");
	if (ImGui::BeginPopupModal("Open File", &openFilePopupIsOpen)) {
		std::string folderPath = SAVED_FILES_DIRECTORY;
		std::string tmpPath = TMP_DIRECTORY;

		for (auto p : fs::directory_iterator(folderPath)) {
			std::string fileName = p.path().string().substr(folderPath.size() + 1); //Remove folder name

			switch (file_type) {
            case(FILE_TYPE::SCN): {
				if (!fs::is_directory(p) && fileName.substr(fileName.size() - 4) == ".scn") {
					if (ImGui::Button(fileName.data(), ImVec2(200, 20))) {
						std::string path = folderPath;
						loadScene(path.append("/").append(fileName), scene);
						CommandManager::reset();
						openFilePopupIsOpen = false;
					}
				}
				break;
			}
            case(FILE_TYPE::PNG): {
				if (!fs::is_directory(p) && fileName.substr(fileName.size() - 4) != ".scn") {
					if (ImGui::Button(fileName.data(), ImVec2(200, 20))) {
						loadLayerFromFile(folderPath, fileName, scene);
						openFilePopupIsOpen = false;
					}
				}
				break;
			}
			}

		}
		ImGui::Dummy(ImVec2(0, 30));
		ImGui::SameLine(ImGui::GetWindowSize().x - 60);
		if (ImGui::Button("Close", ImVec2(50, 20))) openFilePopupIsOpen = false;
		ImGui::EndPopup();
	}
}

void SaveFilePopup(const Scene& scene)
{
	ImGui::SetNextWindowSize(ImVec2(600, 500));
	ImGui::OpenPopup("Save File");
	if (ImGui::BeginPopupModal("Save File", &saveFilePopupIsOpen)) {
		std::string folderPath = SAVED_FILES_DIRECTORY;
		std::string tmpPath = TMP_DIRECTORY;

		for (auto p : fs::directory_iterator(folderPath)) {
			std::string fileName = p.path().string().substr(folderPath.size() + 1); //Remove folder name
			if (!fs::is_directory(p)) {
				if (ImGui::Button(fileName.data(), ImVec2(200, 20))) {
					input = fileName.substr(0, fileName.length() - 4).c_str(); //remove ending
				}
			}
		}

		ImGui::Dummy(ImVec2(0, 10));
		ImGui::DrawLine(sf::Vector2f(0, 0), sf::Vector2f(WINDOW_WIDTH, 0), sf::Color::White, 1.0f);
		ImGui::Dummy(ImVec2(0, 20));

		char* inputChar = (char*)input.data();
		if (ImGui::InputText("Press Enter", inputChar, 30, ImGuiInputTextFlags_EnterReturnsTrue)) {
			folderPath.append("/").append(inputChar);
			switch (file_type) {
            case(FILE_TYPE::JPEG):
			case(FILE_TYPE::SINGLE_JPEG):
				folderPath.append(".jpg");
				break;
			case(FILE_TYPE::PNG):
			case(FILE_TYPE::SINGLE_PNG):
				folderPath.append(".png");
				break;
			case(FILE_TYPE::SCN):
				tmpPath.append("/").append("tmp").append(".png");
				folderPath.append(".scn");
				break;
			}

			switch (file_type) {
			case(FILE_TYPE::JPEG): {
				saveSceneAsJPEG(scene);
				break;
			}
			case(FILE_TYPE::PNG): {
				saveSceneAsPNG(scene, folderPath);
				break;
			}
			case(FILE_TYPE::SINGLE_JPEG): {
				saveCurrentLayerAsJPEG(scene, folderPath);
				break;
			}
			case(FILE_TYPE::SINGLE_PNG): {
				saveCurrentLayerAsPNG(scene, folderPath);
				break;
			}
			case(FILE_TYPE::SCN): {
				saveScene(scene, tmpPath, folderPath);
			}
			}

			saveFilePopupIsOpen = false;
		}
		ImGui::Dummy(ImVec2(0, 30));
		ImGui::SameLine(ImGui::GetWindowSize().x - 60);
		if (ImGui::Button("Close", ImVec2(50, 20))) saveFilePopupIsOpen = false;
		ImGui::EndPopup();
	}
}

void DrawFileMenu()
{
	if (ImGui::MenuItem("Saving Scene"))
	{
		file_type = FILE_TYPE::SCN;
		saveFilePopupIsOpen = true;
		input = "Input FileName and";
	}
	if (ImGui::MenuItem("Open Scene"))
	{
		file_type = FILE_TYPE::SCN;
		openFilePopupIsOpen = true;
	}
	if (ImGui::MenuItem("Open Layer from File"))
	{
		file_type = FILE_TYPE::PNG;
		openFilePopupIsOpen = true;
	}
	if (ImGui::MenuItem("Save Scene as JPEG"))
	{
		file_type = FILE_TYPE::JPEG;
		saveFilePopupIsOpen = true;
		input = "Input FileName and";
	}
	if (ImGui::MenuItem("Save Scene as PNG"))
	{
		file_type = FILE_TYPE::PNG;
		saveFilePopupIsOpen = true;
		input = "Input FileName and";
	}
	if (ImGui::MenuItem("Save Layer as JPEG"))
	{
		file_type = FILE_TYPE::SINGLE_JPEG;
		saveFilePopupIsOpen = true;
		input = "Input FileName and";
	}
	if (ImGui::MenuItem("Save Layer as PNG"))
	{
		file_type = FILE_TYPE::SINGLE_PNG;
		saveFilePopupIsOpen = true;
		input = "Input FileName and";
	}
}

void loadLayerFromFile(const std::string& folderPath, const std::string &fileName, Scene& scene)
{
	std::string path = folderPath;
	path.append("/").append(fileName);

	sf::Texture tex;
	tex.loadFromFile(path);

	if (scene.lastActiveLayer < scene.layers.end()) {
		CommandManager::createLayer();
	}
	else {
		std::cout << "ERROR! Maxing number of Layers reached!!!" << std::endl;
		return;
	}
	scene.lastActiveLayer->name = "Layer";
	scene.lastActiveLayer->name.append(std::to_string(scene.getSize()));

	scene.lastActiveLayer->tex = tex;
	scene.lastActiveLayer->sprite.setTexture(scene.lastActiveLayer->tex);
}

void loadScene(const std::string& fileName, Scene & scene)
{
	std::ifstream newistrm(fileName, std::ios::out | std::ios::binary);

	int layerCount;
	int width;
	int height;

	extractMetaData(newistrm, layerCount, width, height);
	std::vector<std::string> layerNames = extractLayerNames(layerCount, newistrm);
	std::vector<char> buffer = extractImageData(newistrm);
	newistrm.close();

	scene.resetScene(width, height);
	sf::Texture tex = extractTextureFromBuffer(width, height, buffer, layerCount);
	loadDataIntoScene(layerCount, tex, scene, layerNames);
}

sf::Texture extractTextureFromBuffer(int width, int height, const std::vector<char>& buffer, int layerCount)
{
	sf::Texture tex;
	tex.create(width, height);
	tex.loadFromMemory(buffer.data(), width * height * (layerCount + 1));
	return tex;
}

void loadDataIntoScene(int layerCount, const sf::Texture& tex, Scene & scene, const std::vector<std::string>& layerNames)
{
	sf::RenderTexture rtex;
	sf::RenderStates rState;
	rState.blendMode = sf::BlendNone;
	rtex.create(scene.width, scene.height);
	sf::IntRect rect(0, 0, scene.width, scene.height);
	sf::Sprite sprite;

	for (int i = 0; i < layerCount + 1; i++) {
		rect.left = i * scene.width;
		sprite.setTexture(tex);
		sprite.setTextureRect(rect);
		rtex.draw(sprite, rState);
		rtex.display();

		if (i == 0) {
			scene.normalLayer.tex = rtex.getTexture();
			scene.normalLayer.sprite.setTexture(scene.normalLayer.tex);
		}
		else {
			scene.layers[i].name = layerNames[i - 1];
			scene.layers[i].tex = rtex.getTexture();
			scene.layers[i].sprite.setTexture(scene.layers[i].tex);
		}
	}

	scene.lastActiveLayer = scene.layers.begin() + layerCount - 1;
}

void extractMetaData(std::ifstream &newistrm, int &layerCount, int &width, int &height)
{
	newistrm.read((char*)&layerCount, sizeof(int));
	newistrm.read((char*)&width, sizeof(int));
	newistrm.read((char*)&height, sizeof(int));
}

std::vector<char> extractImageData(std::ifstream &newistrm)
{
	std::vector<char> buffer;

	std::streampos begin = newistrm.tellg();
	newistrm.seekg(0, std::ios::end);
	std::streampos end = newistrm.tellg();
	newistrm.seekg(begin);

	buffer.resize((end - begin) / sizeof(char));
	newistrm.read((char*)buffer.data(), (end - begin) / sizeof(char));

	return buffer;
}

std::vector<std::string> extractLayerNames(int layerCount, std::ifstream &newistrm)
{
	std::vector<std::string> layerNames;
	for (int i = 0; i < layerCount; i++) {
		std::string layerName;
		char newChar;
		newistrm.read((char*)&newChar, sizeof(char));
		while (newChar != '#') {
			layerName.push_back(newChar);
			newistrm.read((char*)&newChar, sizeof(char));
		}
		layerNames.push_back(layerName);
	}
	return layerNames;
}

void saveScene(const Scene& scene, const std::string& tmpPath, const std::string& folderPath)
{
	saveTemporaryTextureData(scene, tmpPath);

	std::ifstream istrm(tmpPath, std::ios::in | std::ios::binary);
	std::ofstream ostrm(folderPath, std::ios::out | std::ios::binary);

	saveMetaData(scene, ostrm);
	saveLayerNames(scene, ostrm);
	saveTemporaryTextureDataToFile(istrm, ostrm);

	istrm.close();
	ostrm.close();
}

void saveTemporaryTextureDataToFile(std::ifstream &istrm, std::ofstream &ostrm)
{
	istrm.seekg(0, std::ios::end);
	std::streampos end = istrm.tellg();
	istrm.seekg(0, std::ios::beg);

    std::vector<char> buffer(end);
	istrm.read(buffer.data(), end);
	ostrm.write(buffer.data(), end);
}

void saveLayerNames(const Scene& scene, std::ofstream &ostrm)
{
	for (int i = 0; i < scene.getSize() + 1; i++) {
		ostrm.write(scene.layers[i + 1].name.data(), scene.layers[i + 1].name.size());
		ostrm.put('#');
	}
}

void saveMetaData(const Scene& scene, std::ofstream &ostrm)
{
	unsigned int layerCount = scene.getSize() + 1;
	ostrm.write(reinterpret_cast<const char*>(&layerCount), sizeof(int));
	ostrm.write(reinterpret_cast<const char*>(&scene.width), sizeof(int));
	ostrm.write(reinterpret_cast<const char*>(&scene.height), sizeof(int));
}

void saveTemporaryTextureData(const Scene& scene, const std::string& tmpPath)
{
	size_t layerCount = scene.getSize() + 1;

	sf::Vector2f offset(scene.width, 0);
	sf::RenderStates rState;
	rState.blendMode = sf::BlendNone;

	sf::RenderTexture rTex;
	rTex.create(scene.width * (layerCount + 1), scene.height);
	rTex.clear(sf::Color(255, 255, 255, 0));

	rTex.draw(scene.normalLayer.sprite, rState);
	rState.transform.translate(offset);

	for (size_t i = 0; i < layerCount; i++) {
		rTex.draw(scene.layers[i + 1].sprite, rState);
		rState.transform.translate(offset);
	}

	rTex.display();
	rTex.getTexture().copyToImage().saveToFile(tmpPath);
}

void saveCurrentLayerAsPNG(const Scene& scene, const std::string& folderPath)
{
	scene.currentLayer->tex.copyToImage().saveToFile(folderPath);
}

void saveCurrentLayerAsJPEG(const Scene& scene, const std::string& folderPath)
{
	sf::RenderTexture rTex;
	rTex.create(scene.width, scene.height);
	rTex.clear(sf::Color::White);
	rTex.draw(scene.layers.begin()->sprite);
	rTex.draw(scene.currentLayer->sprite);
	rTex.display();
	rTex.getTexture().copyToImage().saveToFile(folderPath);
}

void saveSceneAsPNG(const Scene& scene, const std::string& folderPath)
{
	sf::RenderTexture rTex;
	rTex.create(scene.width, scene.height);
	rTex.clear(sf::Color(255, 255, 255, 0));
	for (auto layer = std::next(scene.layers.begin()); layer != scene.layers.end(); layer++) {
		rTex.draw(layer->sprite);
	}
	rTex.display();
	rTex.getTexture().copyToImage().saveToFile(folderPath);
}

void saveSceneAsJPEG(const Scene& scene)
{
	sf::RenderTexture rTex;
	rTex.create(scene.width, scene.height);
	rTex.clear(sf::Color::White);
	for (auto layer = scene.layers.begin(); layer != scene.layers.end(); layer++) {
		rTex.draw(layer->sprite);
	}
}
