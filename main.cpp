#include <SFML/Graphics.hpp>
#include <map>
#include <cmath>
#include <random>
#include <fstream>
#include <iostream>

double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
double lerp(double t, double a, double b) { return a + t * (b - a); }
double grad(int hash, double x, double y)
{
    int h = hash & 15;
    double u = h < 8 ? x : y,
        v = h < 4 ? y : h == 12 || h == 14 ? x
        : 0;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double perlinNoise(double x, double y, int octaves, double persistence, double scale)
{
    double total = 0;
    double frequency = scale;
    double amplitude = 1;
    double maxValue = 0;

    for (int i = 0; i < octaves; i++)
    {
        int xi = static_cast<int>(floor(x / frequency));
        int yi = static_cast<int>(floor(y / frequency));

        double value = lerp(lerp(fade(x / frequency - xi),
            grad(xi, x - xi, y - yi),
            grad(xi + 1, x - xi - 1, y - yi)),
            lerp(fade(y / frequency - yi),
                grad(xi, x - xi, y - yi - 1),
                grad(xi + 1, x - xi - 1, y - yi - 1)),
            fade(y / frequency - yi));

        total += value * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2;
    }

    return total / maxValue;
}

/**
 * Generates a height map using Perlin noise algorithm.
 *
 * @param width the width of the height map
 * @param height the height of the height map
 * @param scale the scale of the noise
 * @param numOctaves the number of octaves for noise generation
 * @param persistence the persistence value for noise generation
 * @param offsetX the offset on the x-axis
 * @param offsetY the offset on the y-axis
 *
 * @return a vector of integers representing the generated height map
 *
 * @throws None
 */
std::vector<int> generateHeightMap(int width, int height, double scale, int numOctaves, double persistence, double offsetX, double offsetY)
{
    std::vector<int> heightMap(width);

    for (int i = 0; i < width; i++)
    {
        double noiseValue = perlinNoise((i + offsetX) / static_cast<double>(width), (offsetY) / static_cast<double>(width), numOctaves, persistence, scale);
        heightMap[i] = static_cast<int>((noiseValue + 1) * 0.5 * height);
    }

    return heightMap;
}

std::vector<std::vector<int>> generateTerrain(int width, int height, int seed, std::vector<int> heightMap)
{
    std::vector<std::vector<int>> tileMap(height, std::vector<int>(width));
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // Определить тип тайла в зависимости от высоты
            if (y < heightMap[x])
            {
                tileMap[y][x] = 1; // Небо
            }
            else if (y == heightMap[x] || y == heightMap[x] + 1)
            {
                tileMap[y][x] = 0; // Земля с травой
            }
            else
            {
                tileMap[y][x] = 2; // Камень
            }
        }
    }
    return tileMap;
}

int calculatePercentage(int value, int start, int end, int startPercent, int endPercent)
{
    if (value < start || value > end)
    {
        return 0;
    }

    int range = end - start;
    int difference = endPercent - startPercent;
    int step = difference / range;

    return startPercent + (value - start) * step;
}

void generateUndergroundResources(std::vector<std::vector<int>>& tileMap, int NumTilesX, int NumTilesY)
{

    std::vector<std::pair<int, std::pair<double, double>>> resources =
    {
        {2, {0.0, 25.0}},   // Олово
        {3, {17.5, 35.0}},  // Медь
        {4, {30.0, 80.0}},  // Железо
        {5, {50.0, 75.0}},  // Серебро
        {6, {65.0, 100.0}}, // Золото
        {7, {75.0, 100.0}}  // Мифрил
    };

    std::vector<std::pair<double, double>> resourceChances =
    {
        {15.0, 1.8}, // Олово
        {15.0, 2.0}, // Медь
        {10.0, 1.2}, // Железо
        {10.0, 1.6}, // Серебро
        {5.0, 0.71}, // Золото
        {7.0, 1.32}  // Мифрил
    };

    // Цикл по ресурсам
    for (size_t i = 0; i < resources.size(); ++i)
    {
        double startHeightPercent = resources[i].second.first;
        double endHeightPercent = resources[i].second.second;
        double startChance = resourceChances[i].first;
        double chanceStep = resourceChances[i].second;

        // Цикл по y
        for (size_t y = (int)NumTilesY * 0.3; y < NumTilesY; ++y)
        {
            double heightPercent = static_cast<double>(y) / static_cast<double>(NumTilesY);

            if (heightPercent >= startHeightPercent / 100.0 && heightPercent <= endHeightPercent / 100.0)
            {
                double currentChance = startChance;
                for (size_t x = 0; x < NumTilesX; ++x)
                {
                    if (static_cast<double>(rand()) / RAND_MAX < currentChance / 100.0)
                    {
                        if (tileMap[y][x] == 2) // Камень
                        {
                            tileMap[y][x] = resources[i].first; // Заменить на ресурс
                            //break;
                        }
                    }
                }
                if (startHeightPercent < endHeightPercent)
                {
                    double chanceFactor = (heightPercent - startHeightPercent / 100.0) / ((endHeightPercent - startHeightPercent) / 100.0);
                    currentChance += chanceStep * chanceFactor;
                }
            }
        }
    }
}

// void generateTrees(std::vector<std::vector<int>>& terrain, const std::vector<std::vector<int>>& treeTemplates)
// {
//     std::default_random_engine generator;
//     std::uniform_int_distribution<int> distribution(0, 99);

//     for (size_t y = 0; y < terrain.size(); ++y)
//     {
//         for (size_t x = 0; x < terrain[y].size(); ++x)
//         {
//             if (terrain[y][x] == 2) // Ground
//             {
//                 if (distribution(generator) < 5) // 5% chance to generate a tree
//                 {
//                     bool canPlaceTree = true;

//                     for (const auto& treeTemplate : treeTemplates)
//                     {
//                         for (size_t ty = 0; ty < treeTemplate.size(); ++ty)
//                         {
//                             for (size_t tx = 0; tx < treeTemplate[ty].size(); ++tx)
//                             {
//                                 if (treeTemplate[ty][tx] != 0 &&
//                                     (x + tx < 0 || x + tx >= terrain[0].size() ||
//                                      y + ty < 0 || y + ty >= terrain.size() ||
//                                      terrain[y + ty][x + tx] != 2))
//                                 {
//                                     canPlaceTree = false;
//                                     break;
//                                 }
//                             }
//                             if (!canPlaceTree)
//                             {
//                                 break;
//                             }
//                         }
//                         if (canPlaceTree)
//                         {
//                             for (size_t ty = 0; ty < treeTemplate.size(); ++ty)
//                             {
//                                 for (size_t tx = 0; tx < treeTemplate[ty].size(); ++tx)
//                                 {
//                                     if (treeTemplate[ty][tx] != 0)
//                                     {
//                                         terrain[y + ty][x + tx] = 10; // Tree
//                                     }
//                                 }
//                             }
//                             break;
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }

// ------------------------------------------------------------------

// Класс для управления ресурсами (текстурами)
class Resources
{
public:
    // Получить текстуру по имени
    sf::Texture& getTexture(const std::string& name);
    // Загрузить текстуру из файла
    void loadTexture(const std::string& name, const std::string& filePath);

private:
    std::map<std::string, sf::Texture> m_textures; // Хранилище текстур
};

// Получить текстуру по имени
sf::Texture& Resources::getTexture(const std::string& name)
{
    return m_textures.at(name);
}

// Загрузить текстуру из файла
void Resources::loadTexture(const std::string& name, const std::string& filePath)
{
    sf::Texture texture;
    if (!texture.loadFromFile(filePath))
    {
        throw std::runtime_error("Failed to load texture: " + filePath);
    }
    m_textures[name] = texture;
}

// Класс для представления тайла (плитки)
class Tile
{
public:
    // Создать тайл из текстуры и указать размер тайла
    Tile(const sf::Texture& texture, int tileSize);
    // Нарисовать тайл на экране в указанной позиции
    void draw(sf::RenderTarget& target, sf::Vector2f position);

private:
    sf::Sprite m_sprite; // Спрайт (графическое представление тайла)
    int m_tileSize;      // Размер тайла
};

// Создать тайл из текстуры и указать размер тайла
Tile::Tile(const sf::Texture& texture, int tileSize) : m_tileSize(tileSize)
{
    m_sprite.setTexture(texture);
    m_sprite.setTextureRect(sf::IntRect(0, 0, tileSize, tileSize));
}

// Нарисовать тайл на экране в указанной позиции
void Tile::draw(sf::RenderTarget& target, sf::Vector2f position)
{
    m_sprite.setPosition(position);
    target.draw(m_sprite);
}

int main()
{
    // Получить разрешение экрана
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    int screenWidth = desktopMode.width;
    int screenHeight = desktopMode.height;

    // Создать окно приложения на весь экран
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "SFML Application");

    // Создать менеджер ресурсов и загрузить текстуры
    Resources resources;
    resources.loadTexture("ground_with_grass", "textures/ground_with_grass.png");
    resources.loadTexture("sky", "textures/sky.png");
    resources.loadTexture("rock", "textures/rock.png");
    resources.loadTexture("tin", "textures/tin.png");
    resources.loadTexture("Iron", "textures/Iron.png");
    resources.loadTexture("mithril", "textures/mithril.png");
    resources.loadTexture("silver", "textures/silver.png");
    resources.loadTexture("gold", "textures/gold.png");
    resources.loadTexture("copper", "textures/copper.png");
    resources.loadTexture("wood_tree", "textures/wood_tree.png");
    resources.loadTexture("leaves", "textures/leaves.png");
    resources.loadTexture("grass", "textures/grass.jpg");

    int tileSize = 16; // Размер тайла

    // Создать тайлы для каждой текстуры
    Tile groundWithGrassTile(resources.getTexture("ground_with_grass"), tileSize);
    Tile skyTile(resources.getTexture("sky"), tileSize);
    Tile rockTile(resources.getTexture("rock"), tileSize);
    Tile copperTile(resources.getTexture("copper"), tileSize);
    Tile ironTile(resources.getTexture("Iron"), tileSize);
    Tile tinTile(resources.getTexture("tin"), tileSize);
    Tile silverTile(resources.getTexture("silver"), tileSize);
    Tile goldTile(resources.getTexture("gold"), tileSize);
    Tile mithrilTile(resources.getTexture("mithril"), tileSize);
    Tile woodTreeTile(resources.getTexture("wood_tree"), tileSize);
    Tile leavesTile(resources.getTexture("leaves"), tileSize);
    Tile grassTile(resources.getTexture("grass"), tileSize);

    // Создать словарь тайлов, где ключ - индекс тайла, значение - указатель на тайл
    std::map<int, Tile*> tileDictionary;

    tileDictionary[0] = &groundWithGrassTile; // земля с травой
    tileDictionary[1] = &skyTile;             // небо
    tileDictionary[2] = &rockTile;            // камень
    tileDictionary[3] = &tinTile;             // олово
    tileDictionary[4] = &copperTile;          // медь
    tileDictionary[5] = &ironTile;            // железо
    tileDictionary[6] = &silverTile;          // серебро
    tileDictionary[7] = &goldTile;            // золото
    tileDictionary[8] = &mithrilTile;         // мифрил
    tileDictionary[9] = &woodTreeTile;        // дерево
    tileDictionary[10] = &leavesTile;         // листва
    tileDictionary[11] = &grassTile;          // трава

    // Рассчитать количество тайлов по горизонтали и вертикали для покрытия всего экрана
    // Рассчитать количество тайлов по горизонтали и вертикали для покрытия всего экрана
    int numTilesX = screenWidth / tileSize;
    int numTilesY = screenHeight / tileSize;

    std::cout << numTilesX << " " << numTilesY << std::endl;
    srand(time(NULL));

    int seed = rand();
    int offsetX = rand() % numTilesX;
    int offsetY = rand() % numTilesY;
    std::vector<int> heightMap = generateHeightMap(numTilesX, numTilesY, 0.01, 28, 4, offsetX, offsetY);

    // Создать карту тайлов (вектор векторов индексов тайлов)
    std::vector<std::vector<int>> tileMap = generateTerrain(numTilesX, numTilesY, seed, heightMap);

    // Генерировать минералы на карте тайлов
    generateUndergroundResources(tileMap, numTilesX, numTilesY);

    // Создать малые подкарты тайлов для деревьев
    // std::vector<std::vector<int>> treeTemplates = createTreeTemplates();

    // Генерировать деревья на карте тайлов
    // generateTrees(tileMap, treeTemplates);

    std::ofstream file("output.txt");
    for (const auto& row : tileMap)
    {
        for (const auto& tileId : row)
        {
            file << tileId << " ";
        }
        file << std::endl;
    }

    // Главный цикл приложения
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        // Нарисовать карту тайлов
        for (size_t y = 0; y < tileMap.size(); ++y)
        {
            for (size_t x = 0; x < tileMap[y].size(); ++x)
            {
                int tileIndex = tileMap[y][x];
                Tile* tile = tileDictionary[tileIndex];
                if (tile)
                    tile->draw(window, sf::Vector2f(x * tileSize, y * tileSize));
            }
        }

        window.display();
    }

    return 0;
}
