#include "LsRendering.h"
#include "LsUtilities.h"

HeliosPoint LsRenderer::makeHeliosPoint(LsVec2F pos, LsColor color) {
    HeliosPoint p;

    p.x = static_cast<uint16_t>(std::clamp(std::round(pos.x), 0.f, static_cast<float>(this->canvasSize.x)));
    p.y = static_cast<uint16_t>(std::clamp(std::round(pos.y), 0.f, static_cast<float>(this->canvasSize.y)));

    unsigned cmax =  UINT8_MAX;
    p.r = static_cast<uint8_t>(std::min(color.r, cmax));
    p.g = static_cast<uint8_t>(std::min(color.g, cmax));
    p.b = static_cast<uint8_t>(std::min(color.b, cmax));
    p.i = static_cast<uint8_t>(std::min(color.a, cmax));

    return p;
}

std::vector<HeliosPoint> LsRenderer::buildOptimalOutput() {
    std::vector<HeliosPoint> output;
    std::vector<LsLine> lines;
    std::vector<LsColor> colors;

    const LsVec2F canvasSizeF = {static_cast<float>(this->canvasSize.x), static_cast<float>(this->canvasSize.y)};

    // build a collection of lines, throwing away all lines invisible to the canvas, cropping the lines that are partially visible to the canvas
    for(auto& graphic : this->graphics) {
        auto glines = graphic.getLines();

        for(int i = 0; i < glines.size(); i++) {
            auto& line = glines[i];

            if ((line.start.x < 0.f && line.end.x < 0.f) || (line.start.x > canvasSizeF.x && line.end.x > canvasSizeF.x)) continue;
            if ((line.start.y < 0.f && line.end.y < 0.f) || (line.start.y > canvasSizeF.y && line.end.y > canvasSizeF.y)) continue;

            line.start.x = std::clamp(line.start.x, 0.f, canvasSizeF.x);
            line.end.x = std::clamp(line.end.x, 0.f, canvasSizeF.x);
            line.start.y = std::clamp(line.start.y, 0.f, canvasSizeF.y);
            line.end.y = std::clamp(line.end.y, 0.f, canvasSizeF.y);

            lines.emplace_back(glines[i]);
            colors.emplace_back(graphic.getColor(graphic.getColorChannel(i)));
        }
    }

    float totalDistance = distance(this->laserPoint.currentPos, lines.front().start); //first distance

    for(int i = 0; i < lines.size()-1; i++) {
        totalDistance += distance(lines[i].start, lines[i].end);
        totalDistance += distance(lines[i].end, lines[i+1].start);
    }
    totalDistance += distance(lines.back().start, lines.back().end); //last iter

    float distancePerPoint = totalDistance / static_cast<float>(this->maxPPS);

    // generate output

    return output;
}

/*inline std::vector<std::vector<LsLine>> groupConnectedLines(std::vector<LsGraphic>& graphics) {
    std::vector<std::vector<LsLine>> groups;

    for(auto& graphic : graphics) {
        auto lines = graphic.getLines();

        while(!lines.empty()) {



        }
    }

    return groups;
}

inline std::vector<HeliosPoint> LsRenderer::buildOptimalOutput() {

    struct ConnectedComponent {
        std::vector<LsLine> lines;
        std::vector<LsColor> linesColors;
        std::vector<LsVec2F> edges;
    };

    std::vector<ConnectedComponent> connectedComponents;
    std::vector<ConnectedComponent> connectedGroups;
    std::vector<HeliosPoint> output;

    // Step 1: create Connected Components
    for(auto& graphic : this->graphics) {
        auto lines = graphic.getLines();

        while(!lines.empty()) {
            ConnectedComponent newCC;

            //start from line 0, travel until reaching one end of the connected component
            bool keepsGoing = true;

            while(keepsGoing) {
                keepsGoing = false;
                for(int i = 0; i < lines.size(); i++) {
                    if (pointsAreEqual(lines[0].end, lines[i].start) || pointsAreEqual(lines[0].start, lines[i].end)) {
                        keepsGoing = true;
                        break;
                    }
                }

                newCC.lines.emplace_back(lines[0]);
                newCC.linesColors.emplace_back();
                lines.erase(lines.begin());
            }

            connectedComponents.emplace_back(newCC);
        }

        for(auto& comp : connectedComponents) {
            std::cout << "\nEdges:\n";
            for(auto& edge : comp.edges) {
                std::cout << edge << ", \n";
            }
            std::cout << "\nLines:\n";
            for(auto& line : comp.lines) {
                std::cout << line.start << " - " << line.end << "\n";
            }
        }

    }

    return output;
}*/
