#include "LsRendering.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

std::vector<LsLine>& LsGraphic::getLines() { return this->lines; }
LsColor LsGraphic::getColor(unsigned index) { if(index >= this->colors.size()) { throw std::runtime_error("LsGraphic::getColor access out of bounds"); } return this->colors[index]; }
unsigned LsGraphic::getColorCount() { return this->colors.size(); }

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

void LsGraphic::addLine(const LsVec2D& start, const LsVec2D& end, const unsigned& colorChannel) {
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
        double startX, startY, endX, endY;
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

LsVec2D LsObject::getPosition() { return position; }
double LsObject::getRotation() { return rotation; }
LsVec2D LsObject::getScale() { return scale; }
LsColor LsObject::getColor(unsigned index) { return this->graphic.getColor(index); }
unsigned LsObject::getColorCount() { return this->graphic.getColorCount(); }
LsGraphic LsObject::getGraphic() { return graphic; }
LsGraphic LsObject::getSourceGraphic() { return sourceGraphic; }

LsVec2D LsObject::computeSourceCenter() {
    double left = std::numeric_limits<double>::infinity();
    double right = -std::numeric_limits<double>::infinity();
    double top = -std::numeric_limits<double>::infinity();
    double bottom = std::numeric_limits<double>::infinity();

    for (auto& line : this->sourceGraphic.getLines()) {
        left = std::min({left, line.start.x, line.end.x});
        right = std::max({right, line.start.x, line.end.x});
        bottom = std::min({bottom, line.start.y, line.end.y});
        top = std::max({top, line.start.y, line.end.y});
    }

    LsVec2D center = { left + (right - left) / 2.0, bottom + (top - bottom) / 2.0 };

    return center;
}

LsObject::LsObject(const LsGraphic& lines) {
    this->sourceGraphic = lines;
    this->graphic = lines;
    sourceCenter = this->computeSourceCenter();
}

LsObject::LsObject(const LsVec2D& position, const LsVec2D& scale, const double& rotation, const LsGraphic& lines) {
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
    const double cosA = std::cos(rotation);
    const double sinA = std::sin(rotation);

    auto rotatePoint = [&](LsVec2D& p) {
        const double dx = p.x - sourceCenter.x;
        const double dy = p.y - sourceCenter.y;
        p.x = sourceCenter.x + dx * cosA - dy * sinA;
        p.y = sourceCenter.y + dx * sinA + dy * cosA;
    };

    for (auto& line : lines) {
        rotatePoint(line.start);
        rotatePoint(line.end);
    }
}

void LsObject::setPosition(LsVec2D newPosition) { this->position = newPosition; }
void LsObject::setScale(LsVec2D newScale) { this->scale = newScale; }
void LsObject::setRotation(const double& newRotation) { this->rotation = newRotation; }
void LsObject::setColor(const LsColor &color, const unsigned &colorID) { this->graphic.setColor(color, colorID); }

void LsRenderer::add(LsObject& object) {
    object.applyTransformation();

    graphics.emplace_back(object.getGraphic());
}


