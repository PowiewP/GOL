#include "LsRendering.h"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

std::vector<LsLine> LsGraphic::getLines() const { return this->lines; }
LsColor LsGraphic::getColor(unsigned channelIndex) const { if(channelIndex >= this->colors.size()) { throw std::runtime_error("LsGraphic::getColor access out of bounds"); } return this->colors[channelIndex]; }
unsigned LsGraphic::getColorChannel(unsigned lineIndex) const { return this->colorChannelMap[lineIndex]; }
unsigned LsGraphic::getColorCount() const { return this->colors.size(); }
LsVec2F LsGraphic::getSize() const { return figureOutSize(); }

LsVec2F LsGraphic::figureOutSize() const {
    LsVec2F size = {0.f, 0.f};

    if(lines.empty()) {
        return size;
    }

    constexpr float inf = std::numeric_limits<float>::infinity();

    LsVec2F lo = {inf, inf}, hi = {-inf, -inf};

    for(const auto& l : lines) {
        lo.x = std::min({lo.x, l.start.x, l.end.x});
        lo.y = std::min({lo.y, l.start.y, l.end.y});
        hi.x = std::max({hi.x, l.start.x, l.end.x});
        hi.y = std::max({hi.y, l.start.y, l.end.y});
    }

    size = {hi.x - lo.x, hi.y - lo.y};

    return size;
}

void LsGraphic::addLine(const LsVec2F& start, const LsVec2F& end, const unsigned& colorChannel) {
    this->lines.push_back({start, end});
    this->colorChannelMap.push_back(colorChannel);
    if(colorChannel >= this->colors.size()) {
        this->colors.resize(colorChannel+1);
    }
}

void LsGraphic::addLine(const LsLine& line, const unsigned& colorChannel) {
    this->lines.emplace_back(line);
    this->colorChannelMap.push_back(colorChannel+1);
    if(colorChannel >= this->colors.size()) {
        this->colors.resize(colorChannel+1);
    }
}

void LsGraphic::setColor(const LsColor &color, const unsigned& colorChannel) {
    if(colorChannel >= this->getColorCount()) {
        this->colors.resize(colorChannel+1);
    }
    this->colors[colorChannel] = color;
}

//save and load are implemented in ILDAFormat.h

LsVec2F LsObject::getPosition() { return position; }
float LsObject::getRotation() { return rotation; }
LsVec2F LsObject::getScale() { return scale; }
LsColor LsObject::getColor(unsigned index) { return this->graphic.getColor(index); }
unsigned LsObject::getColorCount() { return this->graphic.getColorCount(); }
LsGraphic LsObject::getGraphic() { applyTransformation(); return graphic; }
LsGraphic LsObject::getSourceGraphic() { return sourceGraphic; }

LsVec2F LsObject::computeSourceCenter() {
    float left = std::numeric_limits<float>::infinity();
    float right = -std::numeric_limits<float>::infinity();
    float top = -std::numeric_limits<float>::infinity();
    float bottom = std::numeric_limits<float>::infinity();

    for (auto& line : this->sourceGraphic.getLines()) {
        left = std::min({left, line.start.x, line.end.x});
        right = std::max({right, line.start.x, line.end.x});
        bottom = std::min({bottom, line.start.y, line.end.y});
        top = std::max({top, line.start.y, line.end.y});
    }

    LsVec2F center = { left + (right - left) / 2.f, bottom + (top - bottom) / 2.f };

    return center;
}

LsObject::LsObject(const LsGraphic& lines) {
    this->sourceGraphic = lines;
    this->graphic = lines;
    sourceCenter = this->computeSourceCenter();
}

LsObject::LsObject(const LsVec2F& position, const LsVec2F& scale, const float& rotation, const LsGraphic& lines) {
    this->position = position;
    this->scale = scale;
    this->rotation = rotation;
    this->sourceGraphic = lines;
    this->graphic = lines;
    sourceCenter = this->computeSourceCenter();
    this->applyTransformation();
}

void LsObject::applyTransformation() {
    this->graphic.lines = this->sourceGraphic.lines; //reset the lines, don't reset colors

    auto& lines = graphic.lines;
    if (lines.empty())
        return;

    //rotation
    const float cosA = std::cos(rotation);
    const float sinA = std::sin(rotation);

    auto rotatePoint = [&](LsVec2F& p) {
        const float dx = p.x - sourceCenter.x;
        const float dy = p.y - sourceCenter.y;
        p.x = sourceCenter.x + dx * cosA - dy * sinA;
        p.y = sourceCenter.y + dx * sinA + dy * cosA;
    };

    for (auto& line : lines) {
        rotatePoint(line.start);
        rotatePoint(line.end);
    }

    //scale
    for(auto& line : lines) {
        line.start *= this->scale;
        line.end *= this->scale;
    }

    //position
    for(auto& line : lines) {
        line.start += this->position;
        line.end += this->position;
    }

    graphic.figureOutSize();
}

void LsObject::setPosition(LsVec2F newPosition) { this->position = newPosition; }
void LsObject::setScale(LsVec2F newScale) { this->scale = newScale; }
void LsObject::setRotation(const float& newRotation) { this->rotation = newRotation; }
void LsObject::setColor(const LsColor& color, const unsigned& colorID) { this->graphic.setColor(color, colorID); }
void LsObject::setSourceGraphic(const LsGraphic& sourceGraphic) { this->sourceGraphic = sourceGraphic; this->graphic = sourceGraphic; this->sourceCenter = computeSourceCenter(); }

void LsRenderer::draw(LsObject& object) {
    //object.applyTransformation(); not needed, getGraphic() already calls this by itself

    graphics.emplace_back(object.getGraphic());
}

//LsRenderer::buildOptimalOutput() implemented in isolated header-only implementation

void LsRenderer::pushFrame() {
    if(deviceID == UINT_MAX) {
        std::cerr << "Attempted to push frame with no valid device set\n";
        return;
    }

    //sophisticated bullshit to build the optimal points configuration
    std::vector<HeliosPoint> output = this->buildOptimalOutput();

    auto start = std::chrono::steady_clock::now();
    while(dac.GetStatus(deviceID) != 1) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        if (std::chrono::steady_clock::now() - start >= std::chrono::seconds(1)) {
            std::cerr << "LsRenderer::pushFrame no response from set device (" + std::to_string(deviceID) + ") within 1 second of waiting to send output\n";
            return;
        }
    }

    unsigned PPS = this->framerate * static_cast<unsigned>(output.size());
    if(PPS > maxPPS)
        PPS = maxPPS;

    dac.WriteFrame(deviceID, PPS, 0, output.data(), output.size());

    graphics.clear();
}

bool LsRenderer::deviceIsValid() { return deviceID == UINT_MAX; }
int LsRenderer::getDeviceID() { return this->deviceID; }
unsigned LsRenderer::getFramerate() { return this->framerate; }
unsigned LsRenderer::getMaxPPS() { return this->maxPPS; }
void LsRenderer::setFramerate(unsigned framerate) { this->framerate = framerate; }
void LsRenderer::setMaxPPS(unsigned maxPPS) { this->maxPPS = maxPPS; if (maxPPS > 30000)
    std::cout << "Warning: set maxPPS to above 30k, it's very rare for lasers to go this high, and requesting pointrates above the laser's limit can damage the hardware"; }

bool LsRenderer::setDevice(int deviceID) {
    if(validDevicesAmount >= deviceID+1)
        this->deviceID = deviceID;
    else
        this->deviceID = UINT_MAX;
    return this->deviceIsValid();
}

LsRenderer::LsRenderer(int deviceID) {
    validDevicesAmount = dac.OpenDevices();
    if(validDevicesAmount >= deviceID+1)
        this->deviceID = deviceID;
}


