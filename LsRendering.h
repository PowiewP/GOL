#pragma once

#include "HeliosDac.h"
#include "LsUtilities.h"

struct LsLine {
    LsVec2F start, end;
    [[nodiscard]] bool isPoint() const { return pointsAreEqual(start, end); }
    LsLine(const LsVec2F& start, const LsVec2F& end) : start(start), end(end) {};
    LsLine(const LsVec2F& point) : start(point), end(point) {};
    LsLine() = default;
};

/// The fundamental class for storing pictures to use in LsObjects.
///
/// An LsGraphic contains an array of lines and an array of color channels. Each line maps to a single color channel and derives its color value from it. Its contents can be saved and loaded using ILDA files.
class LsGraphic {
private:
    friend class LsObject;
    std::vector<LsLine> lines;
    std::vector<unsigned> colorChannelMap; //index matches an index of a line. Value matches the index of a color channel.
    std::vector<LsColor> colors;

    LsVec2F figureOutSize() const;
public:
    /// @return A copy of the lines inside this graphic
    std::vector<LsLine> getLines() const;

    /// @return The amount of color channels this graphic contains
    unsigned getColorCount() const;

    /// @return The color stored inside a specific color channel within this graphic
    /// @param channelIndex Index of a color channel
    LsColor getColor(unsigned channelIndex) const;

    /// @return Index of a color channel where the specific line derives a color from
    /// @param lineIndex Index of a line within this graphic
    unsigned getColorChannel(unsigned lineIndex) const;

    /// @return The rectangular size of this graphic
    LsVec2F getSize() const;

    /// Adds a line to this graphic
    /// @param colorChannel The index of a color channel within this graphic. If the channel doesn't exist, new ones are created until reaching the requested index, with the default value of white.
    void addLine(const LsVec2F& start, const LsVec2F& end, const unsigned& colorChannel);

    /// Adds a line to this graphic
    /// @param colorChannel The index of a color channel within this graphic. If the channel doesn't exist, new ones are created until reaching the requested index, with the default value of white.
    void addLine(const LsLine& line, const unsigned& colorChannel);

    /// Modifies the color value of a specific color channel within this graphic
    /// @param colorChannel The index of a color channel within this graphic. If the channel doesn't exist, new ones are created until reaching the requested index.
    /// The specified channel is created with the color value that was passed, the ones in between will have the default color of white.
    void setColor(const LsColor& color, const unsigned& colorChannel);

    /// Loads the graphic from an ILDA file (.ILD)
    /// @return True on success, False on failure.
    bool loadFromFile(const std::string& filePath);

    /// Saves the graphic into an ILDA file (.ILD)
    /// @return True on success, False on failure.
    bool saveToFile(const std::string& filePath) const;

    void logContents();

    LsGraphic() = default;
};

class LsObject {
private:
    friend class LsRenderer;
    LsGraphic sourceGraphic{};
    LsGraphic graphic{};

    LsVec2F position = {0.f, 0.f};
    LsVec2F scale = {1.f, 1.f};
    float rotation = 0.f;

    LsVec2F sourceCenter;
    LsVec2F computeSourceCenter();
    void applyTransformation();
public:
    void setRotation(const float& newRotation);
    void setScale(LsVec2F newScale);
    void setPosition(LsVec2F newPosition);
    void setSourceGraphic(const LsGraphic& sourceGraphic);

    LsVec2F getPosition();
    LsVec2F getScale();
    float getRotation();

    LsGraphic getTransformedGraphic();
    LsGraphic& getSourceGraphic();

    LsObject() = default;
    LsObject(const LsGraphic& lines);
    LsObject(const LsVec2F& position, const LsVec2F& scale, const float& rotation, const LsGraphic& lines);
};

class LsRenderer{
private:
    HeliosDac dac;
    std::vector<LsGraphic> graphics;

    HeliosPoint makeHeliosPoint(LsVec2F pos, LsColor color);
    std::vector<HeliosPoint> buildOptimalOutput();
    struct LaserPoint {
        LsVec2F currentPos;
        float angleMoving; // degrees
    };
    LaserPoint laserPoint = {{0, 0}, 0};

    LsVec2U canvasSize = {4095, 4095};
    LsVec2U colorRange = {0, 255};

    int validDevicesAmount;
    unsigned maxPPS = 15000;
    unsigned framerate = 60;
    unsigned deviceID = UINT_MAX;
public:
    bool setDevice(int deviceID);
    void setFramerate(unsigned framerate);
    void setMaxPPS(unsigned maxPPS);

    int getDeviceID();
    bool deviceIsValid();
    unsigned getMaxPPS();
    unsigned getFramerate();

    void draw(LsObject& object);
    void pushFrame();
    void DEBUG_optimal_output() {
        buildOptimalOutput();
    }

    LsRenderer();
};


