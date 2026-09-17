#include "LsRendering.h"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

std::vector<LsLine>& LsGraphic::getLines() { return this->lines; }
LsColor LsGraphic::getColor(unsigned index) { if(index >= this->colors.size()) { throw std::runtime_error("LsGraphic::getColor access out of bounds"); } return this->colors[index]; }
unsigned LsGraphic::getColorCount() { return this->colors.size(); }
LsVec2F LsGraphic::getSize() { figureOutSize(); return this->size; }

void LsGraphic::figureOutSize() {
    this->size = {0.f, 0.f};
    for(const auto& line : this->lines) {
        if(line.start.x > size.x)
            size.x = line.start.x;
        if(line.start.y > size.y)
            size.y = line.start.y;
        if(line.end.x > size.x)
            size.x = line.end.x;
        if(line.end.y > size.y)
            size.y = line.end.y;
    }
}

void LsGraphic::ensureColorIntegrity() {
    unsigned min = INT_MAX;
    unsigned max = 0;
    for(auto& idx : this->colorChannelMap) {
        if(idx < min)
            min = idx;
        if(idx > max)
            max = idx;
    }

    if(min > 0) {
        for(auto& idx : this->colorChannelMap) {
            idx -= min;
        }
    }
    if(max > colors.size()) {
        colors.resize(max);
    }
}

void LsGraphic::addLine(const LsVec2F& start, const LsVec2F& end, const unsigned& colorChannel) {
    assert(start.x >= 0.f && start.y >= 0.f && end.x >= 0.f && end.y >= 0.f);
    this->lines.push_back({start, end});
    this->colorChannelMap.push_back(colorChannel);
    ensureColorIntegrity();
}

void LsGraphic::addLine(const LsLine& line, const unsigned& colorChannel) {
    this->lines.emplace_back(line);
    this->colorChannelMap.push_back(colorChannel);
    ensureColorIntegrity();
}

void LsGraphic::setColor(const LsColor &color, const unsigned& colorChannel) {
    if(colorChannel > this->getColorCount()) {
        this->colors.resize(colorChannel);
    }
    this->colors[colorChannel] = color;
}

bool LsGraphic::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if(!file.is_open()) {
        std::cerr << "Error opening LsGraphic source file: " << filePath << " - Failed to open\n";
        return false;
    }

    LsGraphic graphic;

    int numOfLines;
    if(!(file >> numOfLines)) {
        std::cerr << "Error reading LsGraphic source file: " << filePath << " - The file is likely corrupted\n";
        return false;
    }

    for(int i = 0; i < numOfLines; i++) {
        float startX, startY, endX, endY;
        unsigned colorID;
        if(file >> startX >> startY >> endX >> endY >> colorID) {
            graphic.addLine({startX, startY}, {endX, endY}, colorID);
        }
        else {
            std::cerr << "Error reading LsGraphic source file: " << filePath << " - The file is likely corrupted\n";
            return false;
        }
    }

    int numOfColors;
    if(!(file >> numOfColors)) {
        std::cerr << "Error reading LsGraphic source file: " << filePath << " - The file is likely corrupted\n";
        return false;
    }

    for(int i = 0; i < numOfColors; i++) {
        unsigned r, g, b;
        if(file >> r >> g >> b) {
            if(i > graphic.getColorCount()) {
                std::cerr << "Error reading LsGraphic source file: " << filePath << " - The file is likely corrupted\n";
                return false;
            }
            this->setColor({r, g, b}, i);
        }
        else {
            std::cerr << "Error reading LsGraphic source file: " << filePath << " - The file is likely corrupted\n";
            return false;
        }
    }

    *this = graphic;
    return true;
}

bool LsGraphic::saveToFile(const std::string &filePath) {
    std::ofstream file(filePath);
    if(!file.is_open()) {
        std::cerr << "Couldn't create file " << filePath << " - The location is likely read-only or the program has no access priveleges for it\n";
        return false;
    }

    file << this->lines.size() << "\n\n";
    for(const auto& line : this->lines) {
        file << line.start.x << " " << line.start.y << " " << line.end.x << " " << line.end.y << "\n";
    }

    file << "\n" << this->getColorCount() << "\n\n";
    for(const auto& color : this->colors) {
        file << color.r << " " << color.g << " " << color.b << "\n";
    }

    return true;
}

LsVec2F LsObject::getPosition() { return position; }
float LsObject::getRotation() { return rotation; }
LsVec2F LsObject::getScale() { return scale; }
LsVec2F LsObject::getSize() { return this->graphic.getSize(); }
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
    this->graphic.getLines() = this->sourceGraphic.getLines(); //reset the lines, don't reset colors

    auto& lines = graphic.getLines();
    if (lines.empty())
        return;

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
}

void LsObject::setPosition(LsVec2F newPosition) { this->position = newPosition; }
void LsObject::setScale(LsVec2F newScale) { this->scale = newScale; }
void LsObject::setRotation(const float& newRotation) { this->rotation = newRotation; }
void LsObject::setColor(const LsColor &color, const unsigned &colorID) { this->graphic.setColor(color, colorID); }
void LsObject::setSourceGraphic(const LsGraphic &sourceGraphic) { this->sourceGraphic = sourceGraphic; this->graphic = sourceGraphic; }

void LsRenderer::setDevice(int deviceID) {
    if(dev >= deviceID+1)
        this->deviceID = deviceID;
    else
        this->deviceID = UINT_MAX;
}

LsRenderer::LsRenderer(int deviceID) {
    dev = dac.OpenDevices();
    if(dev >= deviceID+1)
        this->deviceID = deviceID;
}

void LsRenderer::draw(LsObject& object) {
    object.applyTransformation();

    graphics.emplace_back(object.getGraphic());
}

void LsRenderer::buildOptimalOutput() {

}

void LsRenderer::pushFrame() {
    if(deviceID == UINT_MAX) {
        std::cerr << "Attempted to push frame with no valid device set\n";
        return;
    }

    //sophisticated bullshit to build the optimal points configuration
    this->buildOptimalOutput();

    auto start = std::chrono::steady_clock::now();
    while(dac.GetStatus(deviceID) != 1) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        if (std::chrono::steady_clock::now() - start >= std::chrono::seconds(1)) {
            std::cerr << "LsRenderer::pushFrame no response from set device (" + std::to_string(deviceID) + ") within 1 second of waiting to send output\n";
            return;
        }
    }

    int PPS = this->framerate * static_cast<int>(output.size());
    if(PPS > maxPPS)
        PPS = maxPPS;

    dac.WriteFrame(deviceID, PPS, 0, output.data(), output.size());

    graphics.clear();
    output.clear();
    totalLines = 0;
    totalBlanks = 0;
}
