#pragma once

#include "HeliosDac.h"

constexpr double epsi = 0.001;

template<typename T>
struct LsVec2 {
    T x, y;

    LsVec2& operator = (const LsVec2& other) = default;
    LsVec2 operator + (const LsVec2& other) { x += other.x; y += other.y; return *this; }
    LsVec2 operator - (const LsVec2& other) { x -= other.x; y -= other.y; return *this; }
    LsVec2 operator * (const LsVec2& other) { x *= other.x; y *= other.y; return *this; }
    LsVec2 operator / (const LsVec2& other) { x /= other.x; y /= other.y; return *this; }
    LsVec2 operator += (const LsVec2& other) { x += other.x; y += other.y; return *this; }
    LsVec2 operator -= (const LsVec2& other) { x -= other.x; y -= other.y; return *this; }
    LsVec2 operator *= (const LsVec2& other) { x *= other.x; y *= other.y; return *this; }
    LsVec2 operator /= (const LsVec2& other) { x /= other.x; y /= other.y; return *this; }
};
using LsVec2I = LsVec2<int>;
using LsVec2U = LsVec2<unsigned>;
using LsVec2F = LsVec2<float>;
using LsVec2D = LsVec2<double>;

struct LsColor {
    unsigned r = 255, g = 255, b = 255;
};

struct LsLine {
    LsVec2D start, end;
    [[nodiscard]] bool isPoint() const { return std::abs(end.x) - std::abs(start.x) < epsi && std::abs(end.y) - std::abs(start.y) < epsi; }
    LsLine(const LsVec2D& start, const LsVec2D& end) : start(start), end(end) {};
};

class LsGraphic {
private:
    std::vector<LsLine> lines;
    std::vector<unsigned> colorChannelMap;
    std::vector<LsColor> colors;

    void ensureColorIntegrity();
public:
    std::vector<LsLine>& getLines();
    unsigned getColorCount();
    LsColor getColor(unsigned index);

    void addLine(const LsVec2D& start, const LsVec2D& end, const unsigned& colorChannel);
    void addLine(const LsLine& line, const unsigned& colorChannel);
    void setColor(const LsColor& color, const unsigned& colorChannel);

    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath);
    LsGraphic() = default;
    LsGraphic(const std::string& filePath) { loadFromFile(filePath); };
};

class LsObject {
private:
    friend class LsRenderer;
    LsGraphic sourceGraphic{};
    LsGraphic graphic{};

    LsVec2D position = {0.0, 0.0};
    LsVec2D scale = {1.0, 1.0};
    double rotation = 0.0;

    LsVec2D sourceCenter;
    LsVec2D computeSourceCenter();
    void applyTransformation();
public:
    void setRotation(const double& newRotation);
    void setScale(LsVec2D newScale);
    void setPosition(LsVec2D newPosition);
    void setColor(const LsColor& color, const unsigned& colorID);

    LsVec2D getPosition();
    LsVec2D getScale();
    double getRotation();
    unsigned getColorCount();
    LsColor getColor(unsigned index);

    LsGraphic getGraphic();
    LsGraphic getSourceGraphic();

    LsObject() = default;
    LsObject(const LsGraphic& lines);
    LsObject(const LsVec2D& position, const LsVec2D& scale, const double& rotation, const LsGraphic& lines);
};

class LsRenderer{
private:
    HeliosDac dac;
    std::vector<LsGraphic> graphics;
    int totalLines = 0;
    int totalBlanks = 0;
public:
    unsigned maxPPS;
    LsVec2U canvasSize;
    LsVec2U colorRange;
    int deviceID = INT_MIN;

    void add(LsObject& object);
    void pushFrame();
    void setDevice(int deviceID);

    LsRenderer() = default;
    LsRenderer(int deviceID);
};


