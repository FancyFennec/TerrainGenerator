#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <numeric>


class Brush
{
public:
	std::string name = "Brush";
	sf::Texture tex;
	sf::Sprite sprite;

	static float guiBrushColor[3]; //Currently needed to pass to the imgui color picker

	static sf::Color currentColor; //This is the color that is used to draw things
	static sf::Color previousColor; //We can switch to this color by pressing X
	static sf::Color currentNormal;

	//Brush settings#
	bool useSizePress = false;
	bool useFlowPress = false;

	float stepSize = 33.0f;
	int opacity = 170;
	float maxSize = 0.3f;
	float minSize = 0.0f;
	int maxFlow = 70;
	int minFlow = 0;
	float pressure = 1.0f;

	float sizeRatio = minSize / maxSize;
	float flowRatio = float(minFlow) / float(maxFlow);

	//Flags that allow switching scattering on/off
	bool useSScatter = true;
	bool usePScatter = true;
	bool useAScatter = true;

	//Scatter values
	float scatterScale = 0.0f;
	float scatterPos = 0.0f;
	float scatterAngle = 0.0f;

	Brush(const char* filePath)
	{
		if (!tex.loadFromFile(filePath)) {
			std::cout << "ERROR! unable to find path to brush file!" << std::endl;
		}
		initialize();
	}

	float computeRelativeStepSize() const
    {
		return stepSize * (getMinSize() + getMaxSize());
	}

	float getMinSize() const
    {
		return useSizePress ? minSize : 0.0f;
	}

	float getMaxSize() const
    {
		return useSizePress ? pressure * (maxSize - minSize) : maxSize;
	}

	float getMinSizeScale() const
    {
		return getMinSize() / maxSize;
	}

	float getMaxSizeScale() const
    {
		return getMaxSize() / maxSize;
	}

	ImVec4 getCurrentImColorRGBA() const
    {
		return ImVec4(
			currentColor.r / 255.0f,
			currentColor.g / 255.0f,
			currentColor.b / 255.0f,
			currentColor.a / 255.0f
		);
	}

	ImVec4 getCurrentImColorRGB() const
    {
		return ImVec4(
			currentColor.r / 255.0f,
			currentColor.g / 255.0f,
			currentColor.b / 255.0f,
			1.0f
		);
	}

	ImVec4 getPreviousImColorRGBA() const
    {
		return ImVec4(
			previousColor.r / 255.0f,
			previousColor.g / 255.0f,
			previousColor.b / 255.0f,
			previousColor.a / 255.0f
		);
	}

	ImVec4 getPreviousImColorRGB() const
    {
		return ImVec4(
			previousColor.r / 255.0f,
			previousColor.g / 255.0f,
			currentColor.b / 255.0f,
			1.0f
		);
	}

	ImVec4 getCurrentImNormalRGB() const
    {
		return ImVec4(
			currentNormal.r / 255.0f,
			currentNormal.g / 255.0f,
			currentNormal.b / 255.0f,
			1.0f
		);
	}

	void synchronizeGuiBrushColor() {
		guiBrushColor[0] = currentColor.r / 255.0f;
		guiBrushColor[1] = currentColor.g / 255.0f;
		guiBrushColor[2] = currentColor.b / 255.0f;
	}

	//TODO: Not the nicest way we are doingt his now, maybe there is a better way of doing things
	void synchronizeBrushColor() {
		currentColor.r = sf::Uint8(guiBrushColor[0] * 255);
		currentColor.g = sf::Uint8(guiBrushColor[1] * 255);
		currentColor.b = sf::Uint8(guiBrushColor[2] * 255);
	}

	void setSpriteSize() {
		sprite.setScale(sf::Vector2f(this->maxSize, this->maxSize));
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	}

	void setSpriteColor(){
		switch (DRAWING_STATE)
		{
		case DrawingState::ALPHA:
			sprite.setColor(sf::Color(
				currentColor.r,
				currentColor.g,
				currentColor.b,
				useFlowPress ? minFlow + pressure * (maxFlow - minFlow) : maxFlow));
			break;
		case DrawingState::NORMAL:
			sprite.setColor(sf::Color(
				currentNormal.r,
				currentNormal.g,
				currentNormal.b,
				useFlowPress ? minFlow + pressure * (maxFlow - minFlow) : maxFlow));
			break;
		default:
			break;
		}
	}

	void resetSpriteColor() {
		sprite.setColor(currentColor);
	}

private:
	void initialize();
};

float Brush::guiBrushColor[3] = { 0.5f,0.0f,0.5f };
sf::Color Brush::currentColor = sf::Color::Black;
sf::Color Brush::previousColor = sf::Color::Black;
sf::Color Brush::currentNormal = sf::Color(127, 127, 255, 255);

inline void Brush::initialize() {
	sprite.setTexture(tex);
	sprite.setScale(sf::Vector2f(maxSize, maxSize));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
}
