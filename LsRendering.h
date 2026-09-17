#pragma once

#include <cassert>

#include "HeliosDac.h"
#include "LsUtilities.h"

struct LsColor {
    unsigned r = 255, g = 255, b = 255;
};

struct LsLine {
    LsVec2F start, end;
    [[nodiscard]] bool isPoint() const { return pointsAreEqual(start, end); }
    LsLine(const LsVec2F& start, const LsVec2F& end) : start(start), end(end) { assert(start.x >= 0.f && start.y >= 0.f && end.x >= 0.f && end.y >= 0.f); };
};

class LsGraphic {
private:
    std::vector<LsLine> lines;
    std::vector<unsigned> colorChannelMap;
    std::vector<LsColor> colors;

    LsVec2F size = {-1.f, -1.f};

    void ensureColorIntegrity();
    void figureOutSize();
public:
    std::vector<LsLine>& getLines();
    unsigned getColorCount();
    LsColor getColor(unsigned index);
    LsVec2F getSize();

    void addLine(const LsVec2F& start, const LsVec2F& end, const unsigned& colorChannel);
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

    LsVec2F position = {0.0, 0.0};
    LsVec2F scale = {1.0, 1.0};
    float rotation = 0.0;

    LsVec2F sourceCenter;
    LsVec2F computeSourceCenter();
    void applyTransformation();
public:
    void setRotation(const float& newRotation);
    void setScale(LsVec2F newScale);
    void setPosition(LsVec2F newPosition);
    void setColor(const LsColor& color, const unsigned& colorID);
    void setSourceGraphic(const LsGraphic& sourceGraphic);

    LsVec2F getPosition();
    LsVec2F getScale();
    LsVec2F getSize();
    float getRotation();
    unsigned getColorCount();
    LsColor getColor(unsigned index);

    LsGraphic getGraphic();
    LsGraphic getSourceGraphic();

    LsObject() = default;
    LsObject(const LsGraphic& lines);
    LsObject(const LsVec2F& position, const LsVec2F& scale, const float& rotation, const LsGraphic& lines);
};

class LsRenderer{
private:
    HeliosDac dac;
    std::vector<LsGraphic> graphics;

    void buildOptimalOutput();
    std::vector<HeliosPoint> output;
    int totalLines = 0;
    int totalBlanks = 0;
    int dev;
public:
    unsigned maxPPS = 15000;
    LsVec2U canvasSize = {4095, 4095};
    LsVec2U colorRange = {0, 255};
    unsigned framerate = 60;
    unsigned deviceID = UINT_MAX;

    void draw(LsObject& object);
    void pushFrame();
    void setDevice(int deviceID);

    LsRenderer() = default;
    LsRenderer(int deviceID);
};


