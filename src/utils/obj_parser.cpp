#include "raytracer/utils/obj_parser.hpp"
#include "raytracer/utils/utils.hpp"

#include <iostream>
#include <cstdint>
#include <string>
#include <utility>


namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
// ParserOBJ
////////////////////////////////////////////////////////////////////////////////////////////////////
ParserOBJ::ParserOBJ()
:   geometry(std::make_unique<Group>())
{}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<std::ifstream> ParserOBJ::openFile(std::string fileName)
{
    auto file = std::make_unique<std::ifstream>(fileName);
    if (!file->is_open())
    {
        std::cout << "<OBJParser>: Error! OBJ file '" << fileName << "' cannot be opened.\n";
        return nullptr;
    }
    else
        return file;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t ParserOBJ::parseFile(std::unique_ptr<std::ifstream> file)
{
    size_t nLinesIgnored{};
    std::string line;
    if (file != nullptr)
    {
        while (std::getline(*file, line))
        {
            auto type = parseStatement(line);
            if (type == StatementType::illegal)
                nLinesIgnored ++;
        }
        file->close();
    }
    std::cout << "<OBJParser>: Parsing complete. Ignored " << nLinesIgnored
              << " lines of unsupported statements.\n";
    return nLinesIgnored;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ParserOBJ::StatementType ParserOBJ::parseStatement(std::string line)
{
    auto tokens = splitLineToTokens(std::move(line));
    auto type = StatementType::illegal;
    if (isValidVertex(tokens))
    {
        type = StatementType::vertex;
        parseVertex(tokens);
    }
    else if (isValidFace(tokens))
    {
        if (isValidTriangle(tokens))
        {
            type = StatementType::triangle;
            parseTriangle(tokens);
        }
        else
        {
            type = StatementType::polygon;
            parsePolygon(tokens);
        }
    }
    else if (isValidGroup(tokens))
    {
        type = StatementType::group;
        // subsequently parsed geometry will now be grouped
        currentGroup = new Group{};
        geometry->addChild(currentGroup);
        isParsingGroup = true;
    }
    return type;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParserOBJ::isValidVertex(const std::vector<std::string>& tokens)
{
    if (tokens[0] != "v" || tokens.size() != 4)
        return false;
    for (size_t n = 1; n < 4; ++n)
        if (!Utils::isDouble(tokens[n]))
            return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ParserOBJ::parseVertex(const std::vector<std::string>& tokens)
{
    vertices.push_back(Point{
        std::stod(tokens[1]),
        std::stod(tokens[2]),
        std::stod(tokens[3])
    });
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ParserOBJ::parsePolygon(const std::vector<std::string>& tokens)
{
    // triangulate polygon into a series of triangles by using fan
    //  triangulation, pushing each of them into our geometry group
    for (size_t n = 2; n < tokens.size()-1; ++n)
    {
        auto t = new Triangle(
            getVertex(std::stoi(tokens[1])),
            getVertex(std::stoi(tokens[n])),
            getVertex(std::stoi(tokens[n+1]))
        );
        if (isParsingGroup && currentGroup != nullptr)
            currentGroup->addChild(t);
        else
            geometry->addChild(t);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ParserOBJ::parseTriangle(const std::vector<std::string>& tokens)
{
    auto t = new Triangle(getVertex(std::stoi(tokens[1])),
                          getVertex(std::stoi(tokens[2])),
                          getVertex(std::stoi(tokens[3])));
    if (isParsingGroup && currentGroup != nullptr)
        currentGroup->addChild(t);
    else
        geometry->addChild(t);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParserOBJ::isValidFace(const std::vector<std::string>& tokens)
{
    if (tokens[0] != "f" || tokens.size() < 4)
        return false;
    // check that the vertex indices given by the Face statement
    //  are indeed within the bounds of the vertices array (which in a valid
    //  statement, should have been populated before encountering this face)
    for (size_t n = 1; n < 4; ++n)
        if (!vertexExistsAtIndex(std::stoi(tokens[n])))
            return false;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParserOBJ::isValidTriangle(const std::vector<std::string>& tokens)
{
    if (tokens[0] != "f" || tokens.size() != 4)
        return false;
    for (size_t n = 1; n < 4; ++n)
    {
        if (!Utils::isDouble(tokens[n]))
            return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ParserOBJ::isValidGroup(const std::vector<std::string>& tokens)
{
    // groups are just a 'g' followed by a name string
    // also, the group name cannot be a number
    if (tokens[0] != "g" || tokens.size() != 2 || Utils::isDouble(tokens[1]))
        return false;
    else
        return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Group& ParserOBJ::parseToGroup(const std::string& filename)
{
    auto file = openFile(filename);
    std::cout << "<OBJParser>: Loading triangles from OBJ file...\n";
    parseFile(std::move(file));
    return getGroup();
}
}