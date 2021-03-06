#pragma once

#include <vector>
#include <algorithm>
#include "Scene.h"
#include "Settings.h"
#include "Structs.h"

struct command {
	command(COMMAND_TYPE type, int layerPos, sf::Texture newTexure, sf::Texture oldTexure) :
		type(type),
		layerPos(layerPos),
		newTexure(newTexure.copyToImage()),
		oldTexure(oldTexure.copyToImage()) {}

	COMMAND_TYPE type;
	int layerPos;
	sf::Image newTexure;
	sf::Image oldTexure;
};

struct create_command : command{
	create_command() :
		command(COMMAND_TYPE::CREATE_LAYER, 0, sf::Texture(), sf::Texture()) {}
};

struct update_command : command {
	update_command(int layerPos, sf::Texture newTexure, sf::Texture oldTexure) :
		command(COMMAND_TYPE::UPDATE_LAYER, layerPos, newTexure, oldTexure) {}
};

struct delete_command : command {
	delete_command(int layerPos, sf::Texture texture) :
		command(COMMAND_TYPE::DELETE_LAYER, layerPos, sf::Texture(), texture) {}
};

class CommandManager
{
public:

	static void createLayer();
	static void deleteLayer(std::vector<Layer>::iterator iter);
	static void updateLayer(sf::Texture& oldTexture);

	static void moveForward();
	static void moveBackward();
	static void clearActions();

	static void initialize(Scene& scn) {
		scene = &scn;
		actions.reserve(MAX_ACTIONS);
		actionIter = std::prev(actions.end());
	}

	static void reset() {
		actionIter = actions.begin();
		clearActions();
	}

private:
	static Scene* scene;
	static std::vector<command> actions;
	static std::vector<command>::iterator actionIter;

	static void delteLayerAt(std::vector<Layer>::iterator iter);
};

Scene* CommandManager::scene = nullptr;
std::vector<command> CommandManager::actions = { create_command() }; // We create a layer when we create the scene
std::vector<command>::iterator CommandManager::actionIter = actions.begin();

inline void CommandManager::createLayer() {
	if (actions.size() == MAX_ACTIONS) {
		actions.erase(actions.begin());
	}
	actions.push_back(create_command());
	actionIter = std::prev(actions.end());

	std::advance(scene->lastActiveLayer, 1);
	scene->lastActiveLayer->clearLayer();
}

inline void CommandManager::deleteLayer(std::vector<Layer>::iterator iter) {
	if (actions.size() == MAX_ACTIONS) {
		actions.erase(actions.begin());
	}
	actions.push_back(delete_command(scene->getLayerDistance(iter), iter->tex));
	actionIter = std::prev(actions.end());

	delteLayerAt(iter);
}

inline void CommandManager::delteLayerAt(std::vector<Layer>::iterator iter)
{
	if (std::prev(scene->lastActiveLayer) == scene->layers.begin()) { // Create a new Layer if There are none left
		scene->lastActiveLayer->clearLayer();
	}
	else { // This makes sure that we continue working on the layer we were before deleting
		auto iterDist = std::distance(scene->layers.begin(), scene->currentLayer);
		scene->currentLayer = scene->layers.begin();

		iter->clearLayer();
		std::advance(scene->lastActiveLayer, -1);
		

		bool isCurrentLayerBelowIter = scene->layers.begin() + iterDist < iter;
		std::rotate(iter, iter + 1, scene->layers.end());
		if (isCurrentLayerBelowIter) {
			std::advance(scene->currentLayer, iterDist);
		}
		else {
			std::advance(scene->currentLayer, iterDist - 1);
		}
	}
	if (scene->currentLayer == scene->layers.begin()) std::advance(scene->currentLayer, 1);

	scene->reloadLayerSprites();
}

inline void CommandManager::updateLayer(sf::Texture& oldTexture) {
	if (actions.size() == MAX_ACTIONS) {
		actions.erase(actions.begin());
	}
	switch (DRAWING_STATE) {
	case(DrawingState::ALPHA): {
		actions.push_back(update_command(scene->getLayerDistance(), scene->currentLayer->tex, oldTexture));
		break;
	}
	case(DrawingState::NORMAL): {
		actions.push_back(update_command(0, scene->layers.begin()->tex, oldTexture));
		break;
	}
	}
	actionIter = std::prev(actions.end());
}

inline void CommandManager::moveForward()
{
	if (std::next(actionIter) != actions.end()) {
		std::advance(actionIter, 1);
		switch (actionIter->type) {
		case(COMMAND_TYPE::CREATE_LAYER): { //Increment the lastActiveLayer iterator
			std::advance(scene->lastActiveLayer, 1);
			scene->lastActiveLayer->clearLayer();
			break;
		}
		case(COMMAND_TYPE::DELETE_LAYER): { // Delete the layer at the position of the iterator
			if (std::prev(scene->lastActiveLayer) == scene->layers.begin()) { // Create a new Layer if There are none left
				scene->lastActiveLayer->clearLayer();
			}
			else { // This makes sure that we continue working on the layer we were before deleting
				auto iter = scene->layers.begin() + actionIter->layerPos;
				auto currentLayerOffset = std::distance(scene->layers.begin(), scene->currentLayer);
				scene->currentLayer = scene->layers.begin();
				
				iter->clearLayer();
				std::advance(scene->lastActiveLayer, -1);

				bool isCurrentLayerBelowIter = scene->layers.begin() + currentLayerOffset < iter;
				std::rotate(iter, iter + 1, scene->layers.end());
				if (isCurrentLayerBelowIter) {
					std::advance(scene->currentLayer, currentLayerOffset);
				}
				else {
					std::advance(scene->currentLayer, currentLayerOffset - 1);
				}
			}
			if (scene->currentLayer == scene->layers.begin()) std::advance(scene->currentLayer, 1);

			scene->reloadLayerSprites();
			break;
		}
		case(COMMAND_TYPE::UPDATE_LAYER): { //Set the layer texture to the new texture
			auto iter = (scene->layers.begin() + actionIter->layerPos);
			iter->tex.update(actionIter->newTexure);
			iter->sprite.setTexture(iter->tex);
			break;
		}
		}
	}
}

inline void CommandManager::moveBackward()
{
	if (actionIter != actions.begin()) {
		switch (actionIter->type) {
		case(COMMAND_TYPE::CREATE_LAYER): { //Decrement the lastActiveLayer iterator
			std::advance(scene->lastActiveLayer, -1);
			break;
		}
		case(COMMAND_TYPE::DELETE_LAYER): { // Create a new layer at the position of the iterator
			auto iter = scene->layers.begin() + actionIter->layerPos;
			auto currentLayerOffset = std::distance(scene->layers.begin(), scene->currentLayer);
			scene->currentLayer = scene->layers.begin();

			if (std::next(iter) == scene->layers.begin()) { // Create a new Layer if There are none left
				scene->lastActiveLayer->tex.update(actionIter->oldTexure);
				scene->lastActiveLayer->sprite.setTexture(scene->lastActiveLayer->tex);
			}
			else { // This makes sure that we continue working on the layer we were before deleting
				bool isCurrentLayerBelowIter = scene->layers.begin() + currentLayerOffset < iter;
				std::advance(scene->lastActiveLayer, 1);
				std::rotate(iter, scene->lastActiveLayer, std::next(scene->lastActiveLayer));
				iter->tex.update(actionIter->oldTexure);
				iter->sprite.setTexture(iter->tex);

				if (isCurrentLayerBelowIter) {
					std::advance(scene->currentLayer, currentLayerOffset);
				}
				else {
					std::advance(scene->currentLayer, currentLayerOffset + 1);
				}
			}
			if (scene->currentLayer == scene->layers.begin()) std::advance(scene->currentLayer, 1);

			scene->reloadLayerSprites();
			break;
		}
		case(COMMAND_TYPE::UPDATE_LAYER): { //Set the layer texture to the old texture
			auto iter = (scene->layers.begin() + actionIter->layerPos);
			iter->tex.update(actionIter->oldTexure);
			iter->sprite.setTexture(iter->tex);
			break;
		}
		}
		std::advance(actionIter, -1);
	}
}

inline void CommandManager::clearActions()
{
	if (std::next(actionIter) != actions.end()) {
		auto iter = std::prev(actions.end());
		while (iter != actionIter) {
			actions.erase(iter);
			iter = std::prev(actions.end());
		}
	}
	
	actionIter = std::prev(actions.end());
}