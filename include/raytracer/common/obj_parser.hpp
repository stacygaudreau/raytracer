////////////////////////////////////////////////////////////////////////////////////////////////////
///     Raytracer Libs: OBJ Parser
///     OBJ format file parser
///     Stacy Gaudreau
///     11.23.2022
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <map>

#include "raytracer/shapes/group.hpp"
#include "raytracer/shapes/triangle.hpp"
#include "raytracer/math/tuples.hpp"

namespace rt
{
////////////////////////////////////////////////////////////////////////////////////////////////////
class ParserOBJ
{
  public:
    ParserOBJ();
    /// @brief Parse a given .OBJ file, returning its Group() geometry.
    Group& parseToGroup(const std::string& filename);

    /// @brief Open .OBJ file for reading.
    static std::unique_ptr<std::ifstream> openFile(std::string fileName);
    /// @brief Parse a given OBJ file stream, returning the number of ignored statements.
    size_t parseFile(std::unique_ptr<std::ifstream> file);
    /// @brief Get the geometry Group() which has been parsed.
    inline Group& getGroup() { return *geometry; }
    /// @brief Get vertex at index number given by OBJ file (ie: 1-indexed!!)
    inline Tuple getVertex(size_t n) { return vertices.at(n-1); }

    /// The supported .obj file statement types
    enum class StatementType{
        vertex,
        triangle,
        polygon,
        group,
        illegal
    };

    /// @brief Parse a given line, returning the given statement type it represents.
    StatementType parseStatement(std::string tokens);
    /// @brief Validates if the given tokens are a vertex statement.
    static bool isValidVertex(const std::vector<std::string>& tokens);
    /// @brief Validates if the given tokens are a valid triangle statement.
    static bool isValidTriangle(const std::vector<std::string>& tokens);
    /// @brief Validates whether a given Face statement (tri or polygon) has the correct vertices
    /// declared before it.
    bool isValidFace(const std::vector<std::string>& tokens);
    /// @brief Validates whether a given statement is an OBJ Group statement.
    static bool isValidGroup(const std::vector<std::string>& tokens);

    /// @brief Parses and adds a valid vertex line to the vector of vertices.
    void parseVertex(const std::vector<std::string>& tokens);
    /// @brief Parses and adds a valid triangle Shape() to the geometry grouping.
    void parseTriangle(const std::vector<std::string>& tokens);
    /// @brief Parses and adds a valid polygon into triangles, adding them all to the geometry group.
    void parsePolygon(const std::vector<std::string>& tokens);

    /// @brief Check whether a given index actually contains a vertex.
    inline bool vertexExistsAtIndex(size_t n) { return 0 < (n-1) <= vertices.size(); }
    /// @brief Tokenise a given line string, removing any newlines that may exist in it.
    static inline std::vector<std::string> splitLineToTokens(std::string line) {
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.cend()); // remove any \n
        return Utils::split(line, ' ');
    }

  private:
    std::unique_ptr<Group> geometry;    /// the group containing all the generated geometry
    std::vector<Tuple> vertices;    /// all of the vertices parsed from the file *vertex indices start at 1!!*
    bool isParsingGroup{ false };   /// true when the parser is in the process of parsing grouped geometry
    Group* currentGroup{ nullptr }; /// the current group we are parsing geometry to (if any)
};
}