#include "TileMapSerializer.hpp"
#include <fstream>
#include <cctype>
#include <sstream>

#include "../Logger.hpp"

namespace {
int directionToInt(Direction direction) {
    return static_cast<int>(direction);
}

Direction intToDirection(int value) {
    switch (value) {
        case 0:
            return Direction::DOWN;
        case 1:
            return Direction::UP;
        case 2:
            return Direction::LEFT;
        case 3:
            return Direction::RIGHT;
        default:
            return Direction::RIGHT;
    }
}

bool isIntegerToken(const std::string& token) {
    if (token.empty()) {
        return false;
    }

    size_t index = (token[0] == '-' || token[0] == '+') ? 1 : 0;
    if (index >= token.size()) {
        return false;
    }

    for (; index < token.size(); index++) {
        if (!std::isdigit(static_cast<unsigned char>(token[index]))) {
            return false;
        }
    }

    return true;
}
}

bool TileMapSerializer::save(const World& world, const std::string &filePath) {
    std::ofstream file(filePath);

    if (!file.is_open()) {
        return false;
    }

    const TileMap& tileMap = world.getTileMap();
    const auto& layers = tileMap.getLayers();
    const auto conveyors = world.getConveyorBeltData();
    const auto machines = world.getMachinePlacementData();
    std::vector<std::tuple<int, int, int, std::string, int, int, bool>> placeables;

    file << "TILEMAP_V1\n";
    file << "LAYERS " << layers.size() << "\n";

    for (size_t layerIndex = 0; layerIndex < layers.size(); layerIndex++) {
        const TileMapLayer& layer = layers[layerIndex];

        file << "LAYER "
                << layerIndex << " "
                << layer.getWidth() << " "
                << layer.getHeight() << " "
                << layer.getCellWidth() << " "
                << layer.getCellHeight() << "\n";

        for (const Tile& tile : layer.getTiles()) {
            if (!tile.shouldRender) continue;

            if (tile.isPlaceableRoot) {
                placeables.emplace_back(
                    static_cast<int>(layerIndex),
                    tile.x,
                    tile.y,
                    tile.placeableItemName,
                    tile.widthTiles,
                    tile.heightTiles,
                    tile.isBlocking
                );
                continue;
            }

            file << "TILE "
                 << layerIndex << " "
                 << tile.x << " "
                 << tile.y << " "
                 << tile.paletteName << " "
                 << tile.atlasX << " "
                 << tile.atlasY << " "
                 << (tile.isBlocking ? 1 : 0)
                 << "\n";
        }
    }

    file << "PLACEABLES " << placeables.size() << "\n";
    for (const auto& [layerIndex, tileX, tileY, itemName, widthTiles, heightTiles, isBlocking] : placeables) {
        file << "PLACEABLE "
             << layerIndex << " "
             << tileX << " "
             << tileY << " "
             << itemName << " "
             << widthTiles << " "
             << heightTiles << " "
             << (isBlocking ? 1 : 0)
             << "\n";
    }

    file << "CONVEYORS " << conveyors.size() << "\n";
    for (const auto& [tileX, tileY, direction] : conveyors) {
        file << "CONVEYOR "
             << tileX << " "
             << tileY << " "
             << directionToInt(direction)
             << "\n";
    }

    file << "MACHINES " << machines.size() << "\n";
    for (const auto& [machineUniqueName, tileX, tileY] : machines) {
        file << "MACHINE "
             << machineUniqueName << " "
             << tileX << " "
             << tileY << "\n";
    }

    return true;
}

bool TileMapSerializer::load(World& world, const std::string &filePath) {
    std::ifstream file(filePath);
    LOG_INFO("Loading map {}", filePath);

    if (!file.is_open())
        return false;

    TileMap& tileMap = world.getTileMap();

    tileMap.clear();
    world.clearConveyorBelts();
    world.clearMachines();

    std::string line;
    std::getline(file, line);

    if (line != "TILEMAP_V1")
        return false;


    while (std::getline(file, line))
    {
        std::stringstream ss(line);

        std::string type;
        ss >> type;

        if (type == "LAYERS")
        {
            // Kannst du erstmal ignorieren
        }
        else if (type == "CONVEYORS")
        {
            // Count is optional metadata for now.
        }
        else if (type == "PLACEABLES")
        {
            // Count is optional metadata for now.
        }
        else if (type == "MACHINES")
        {
            // Count is optional metadata for now.
        }
        else if (type == "LAYER")
        {
            int layerIndex;
            int width;
            int height;
            int cellWidth;
            int cellHeight;

            ss >> layerIndex >> width >> height >> cellWidth >> cellHeight;

            TileMapLayer layer;
            layer.createLayer(width, height, cellWidth, cellHeight);

            tileMap.addLayer(layer);
        }
        else if (type == "TILE")
        {
            int layerIndex;
            int x;
            int y;
            int atlasX;
            int atlasY;
            int blockingInt;
            std::vector<std::string> tokens;

            ss >> layerIndex >> x >> y;

            std::string token;
            while (ss >> token) {
                tokens.push_back(token);
            }

            if (tokens.size() < 3) {
                continue;
            }

            std::string paletteName = "Overworld";
            const size_t atlasStartIndex = tokens.size() - 3;
            atlasX = std::stoi(tokens[atlasStartIndex]);
            atlasY = std::stoi(tokens[atlasStartIndex + 1]);
            blockingInt = std::stoi(tokens[atlasStartIndex + 2]);

            if (atlasStartIndex > 0) {
                paletteName = tokens[0];
                for (size_t i = 1; i < atlasStartIndex; i++) {
                    paletteName += " " + tokens[i];
                }
            }

            SpriteAtlas* atlas = world.getTilePalette(paletteName);
            if (!atlas) {
                atlas = world.getTilePalette("Overworld");
            }
            if (!atlas) {
                continue;
            }

            Sprite sprite = atlas->getSprite(atlasX, atlasY);

            tileMap.setTile(
                x,
                y,
                sprite,
                layerIndex,
                paletteName,
                atlasX,
                atlasY,
                blockingInt != 0
            );
        }
        else if (type == "CONVEYOR")
        {
            int tileX;
            int tileY;
            int directionValue;

            ss >> tileX >> tileY >> directionValue;
            world.placeConveyorBelt(tileX, tileY, intToDirection(directionValue));
        }
        else if (type == "PLACEABLE")
        {
            int layerIndex;
            int tileX;
            int tileY;
            std::string itemName;
            int widthTiles;
            int heightTiles;
            int blockingInt;

            ss >> layerIndex >> tileX >> tileY >> itemName >> widthTiles >> heightTiles >> blockingInt;

            const ItemDefinition* item = world.getItemDatabase().getItem(itemName);
            if (!item || !item->isPlaceable || !item->placeableSprite.texture) {
                continue;
            }

            world.getTileMap().setTileObject(
                tileX,
                tileY,
                item->placeableSprite,
                layerIndex,
                widthTiles,
                heightTiles,
                blockingInt != 0,
                itemName
            );
        }
        else if (type == "MACHINE")
        {
            std::string machineUniqueName;
            int tileX;
            int tileY;

            ss >> machineUniqueName >> tileX >> tileY;
            world.placeMachine(machineUniqueName, tileX, tileY);
        }
    }

    LOG_INFO("Map loaded");

    return true;
}
