#include "LsRendering.h"
#include <iostream>
#include <fstream>

bool LsGraphic::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if(!file.is_open()) {
        std::cerr << "Error opening LsGraphic source file: " << filePath << " - Failed to open\n";
        return false;
    }

    LsGraphic graphic;

    auto readBE16 = [&file]() -> uint16_t {
        uint8_t bytes[2] = {};
        file.read(reinterpret_cast<char*>(bytes), 2);
        return static_cast<uint16_t>((bytes[0] << 8) | bytes[1]);
    };

    auto readColorPalette = [&file, &graphic, readBE16]() {
        unsigned channel = 0;
        while (file.peek() != 'I') {
            LsColor color;
            color.r = static_cast<uint8_t>(file.get());
            color.g = static_cast<uint8_t>(file.get());
            color.b = static_cast<uint8_t>(file.get());
            graphic.setColor(color, channel);
            channel++;
        }
    };

    auto readPoints = [&file, &graphic, readBE16]() {
        while (file.peek() != 'I' && !file.eof()) {
            LsLine line;
            uint8_t colorChannel;
            line.start.x = static_cast<float>(readBE16());
            line.start.y = static_cast<float>(readBE16());
            file >> colorChannel;

            uint8_t status;
            status = static_cast<uint8_t>(file.get()); //status doesnt have any useful info

            if (file.peek() == 'I' || file.eof())
                return;

            line.end.x = static_cast<float>(readBE16());
            line.end.y = static_cast<float>(readBE16());
            status = static_cast<uint8_t>(file.get()); status = static_cast<uint8_t>(file.get()); //ignore 2 bytes

            graphic.addLine(line, static_cast<unsigned>(colorChannel));
        }
    };

    char* jump = nullptr;
    bool end = false;

    while (!end && !file.eof()) {
        file.ignore(7);
        uint8_t format;
        format = static_cast<uint8_t>(file.get());

        if (format == 2) {
            file.ignore(24);
            readColorPalette();
        }
        else if (format == 1) {
            file.ignore(16);
            if (readBE16() == 0)
                end = true;
            else
                readPoints();
        }
    }

    *this = graphic;
    return true;
}

bool LsGraphic::saveToFile(const std::string& filePath)  const{
    std::ofstream file(filePath, std::ios::binary);
    if(!file.is_open()) {
        std::cerr << "Couldn't create file " << filePath << " - The location is likely read-only or the program has no access priveleges for it\n";
        return false;
    }
    if(this->lines.size()*2 > 65535) {
        std::cerr << "The graphic has too many points to save into an ILDA file";
        return false;
    }

    auto writeBE16 = [&file](uint16_t v) { //big endian required
        const char bytes[2] = {
            static_cast<char>((v >> 8) & 0xFF),
            static_cast<char>(v & 0xFF)
        };
        file.write(bytes, 2);
    };

    //writing color header
    auto writeILDAColorHeader = [this, &file, writeBE16]() {
        constexpr const char* magic = "ILDA";
        constexpr uint8_t zero8 = 0;
        constexpr uint8_t two8 = 2;
        constexpr const char* paletteName = "Graphic ";
        constexpr const char* companyName = "GOL Lib ";
        uint16_t totalColors = this->getColorCount();
        uint16_t zero16 = 0;

        file.write(magic, 4);
        file << zero8 << zero8 << zero8;
        file << two8; //format code
        file.write(paletteName, 8);
        file.write(companyName, 8);
        writeBE16(totalColors);
        writeBE16(zero16);
        writeBE16(zero16);
        file << zero8;
        file << zero8;
    };

    writeILDAColorHeader();
    for (int i = 0; i < this->getColorCount(); i++) {
        const auto& color = this->getColor(i);
        file << static_cast<uint8_t>(color.r) << static_cast<uint8_t>(color.g) << static_cast<uint8_t>(color.b);
    }



    //writing coords ILDA header
    auto writeILDAHeader = [this, &file, writeBE16](bool end = false){
        constexpr const char* magic = "ILDA";
        constexpr uint8_t zero8 = 0;
        constexpr uint8_t one8 = 1;
        constexpr uint16_t one16 = 1;
        constexpr const char* frameName = "Graphic ";
        constexpr const char* companyName = "GOL Lib ";
        uint16_t totalPoints = this->lines.size()*2;
        if(end)
            totalPoints = 0;

        file.write(magic, 4);
        file << zero8 << zero8 << zero8;
        file << one8; //format code
        file.write(frameName, 8);
        file.write(companyName, 8);
        writeBE16(totalPoints);
        file << zero8 << zero8; //meant to be a 2-byte zero, but since zero is just all zeros in binary it is fine if I chain two 1-byte zeros
        writeBE16(one16);
        file << zero8;
        file << zero8;
    };

    writeILDAHeader();

    for(int i = 0; i < this->lines.size(); i++) {
        const auto& line = this->lines[i];

        int16_t x, y;
        uint8_t color = this->getColorChannel(i);
        uint8_t status = 0;

        x = static_cast<int16_t>(std::round(std::clamp(line.start.x, static_cast<float>(INT16_MIN), static_cast<float>(INT16_MAX))));
        y = static_cast<int16_t>(std::round(std::clamp(line.start.y, static_cast<float>(INT16_MIN), static_cast<float>(INT16_MAX))));
        writeBE16(x);
        writeBE16(y);

        bool lastPoint = false;
        bool blanked = this->getColor(this->getColorChannel(i)).a == 0;
        status |= (blanked << 6);
        status |= (lastPoint << 7);

        file << status; //reversed order because of big endian
        file << color; //

        x = static_cast<int16_t>(std::round(std::clamp(line.end.x, static_cast<float>(INT16_MIN), static_cast<float>(INT16_MAX))));
        y = static_cast<int16_t>(std::round(std::clamp(line.end.y, static_cast<float>(INT16_MIN), static_cast<float>(INT16_MAX))));
        writeBE16(x);
        writeBE16(y);

        lastPoint = i == this->lines.size() - 1;
        status |= (blanked << 6);
        status |= (lastPoint << 7);
        file << status;
        file << color;
    }

    writeILDAHeader(true);

    file.close();

    return true;
}