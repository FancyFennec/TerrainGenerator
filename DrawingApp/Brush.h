#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <numeric>


class Brush
{
public:
	std::string brushName = "Brush";

	sf::Image image;
	sf::Texture tex;
	sf::Sprite sprite;
	static sf::Color currentColor;
	static sf::Color previousColor;

	static float guiBrushColor[3]; //Currently needed to pass to the imgui color picker

	float stepSize = 33.0f;
	float brushSize = 0.3f;
	int opacity = 170;
	int flow = 70;
	float pressure = 1.0f;

	bool useSScatter = true;
	bool usePScatter = true;
	bool useAScatter = true;

	float scaterScale = 0.0f;
	float scaterPos = 0.0f;
	float scaterAngle = 0.0f;

	Brush(int brush_width, const char* filePath)
	{
		image.create(brush_width, brush_width);
		if (!image.loadFromFile(filePath)) {
			std::cout << "ERROR! unable to find path to file!" << std::endl;
		}
		initialize();
	}

	float computeRelativeStepSize() {
		return stepSize * brushSize * pressure;
	}

	ImVec4 getCurrentImColorRGBA() {
		return ImVec4(
			currentColor.r / 255.0f,
			currentColor.g / 255.0f,
			currentColor.b / 255.0f,
			currentColor.a / 255.0f
		);
	}

	ImVec4 getCurrentImColorRGB() {
		return ImVec4(
			currentColor.r / 255.0f,
			currentColor.g / 255.0f,
			currentColor.b / 255.0f,
			1.0f
		);
	}

	ImVec4 getPreviousImColorRGBA() {
		return ImVec4(
			previousColor.r / 255.0f,
			previousColor.g / 255.0f,
			previousColor.b / 255.0f,
			previousColor.a / 255.0f
		);
	}

	ImVec4 getPreviousImColorRGB() {
		return ImVec4(
			previousColor.r / 255.0f,
			previousColor.g / 255.0f,
			currentColor.b / 255.0f,
			1.0f
		);
	}

	void updateGuiColor() {
		guiBrushColor[0] = currentColor.r / 255.0f;
		guiBrushColor[1] = currentColor.g / 255.0f;
		guiBrushColor[2] = currentColor.b / 255.0f;
	}

	//TODO: Not the nicest way we are doingt his now, maybe there is a better way of doing things
	void synchronizeColors() {
		currentColor.r = (sf::Uint8)(guiBrushColor[0] * 255);
		currentColor.g = (sf::Uint8)(guiBrushColor[1] * 255);
		currentColor.b = (sf::Uint8)(guiBrushColor[2] * 255);
	}

	void setSize(float brushSize);
	void setSpriteColor(){
		sprite.setColor(sf::Color(
			currentColor.r,
			currentColor.g,
			currentColor.b,
			pressure * flow));
	}

	void resetBrushColor() {
		sprite.setColor(currentColor);
	}

	~Brush() {};

private:
	void initialize();
};

sf::Color Brush::currentColor = sf::Color::Black;
sf::Color Brush::previousColor = sf::Color::Black;
float Brush::guiBrushColor[3] = { 0.5f,0.0f,0.5f };

inline void Brush::initialize() {
	tex.create(image.getSize().x, image.getSize().y);
	tex.update(image);
	sprite.setTexture(tex);
	sprite.setScale(sf::Vector2f(brushSize, brushSize));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}

inline void Brush::setSize(float brushSize) {
	this->brushSize = brushSize;
	sprite.setScale(sf::Vector2f(brushSize, brushSize));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}
