#include "gtest/gtest.h"
#include "raytracer/utils/obj_parser.hpp"
#include "raytracer/shapes/triangle.hpp"
#include "raytracer/logging/logging.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
// OBJ Files
////////////////////////////////////////////////////////////////////////////////////////////////////
class OBJFileSupport: public ::testing::Test
{
  protected:
    // create a simple test obj file with the given contents
    void makeTestFile(std::string name, std::string contents)
    {
        (void)name;
        std::ofstream testFile("test.obj");
        if (!testFile.is_open())
            throw std::runtime_error("File opening error.");
        else
        {
            testFile << contents;
            testFile.close();
        }
    };
    std::string filename{ "test.obj" };
};


TEST_F(OBJFileSupport, IgnoresUnrecognisedLines)
{
    // we only handle a subset of the .obj format, so show
    // we can gracefully ignore lines and count how many were ignored

    // 1. create gibberish file for testing
    std::string gibberish{
        "There was a young lady named Bright\n"
        "who traveled much faster than light.\n"
        "She set out one day\n"
        "in a relative way,\n"
        "and came back the previous night.\n"
    };
    makeTestFile(filename, gibberish);
    // 2. open gibberish file
    auto file = ParserOBJ::openFile(filename);
    // 3. parse it
    ParserOBJ parsedObj{};
    auto nSkipped = parsedObj.parseFile(std::move(file));

    EXPECT_EQ(nSkipped, 5);
}

TEST_F(OBJFileSupport, IdentifiesIllegalStatement)
{
    std::string str{ "v -1.2424 1.25 abc.00\n"};
    auto obj = ParserOBJ{};
    auto type = obj.parseStatement(str);
    EXPECT_EQ(type, ParserOBJ::StatementType::illegal);
}

TEST_F(OBJFileSupport, VerifiesVertex)
{
    std::string str{ "v -1.0000 0.5000 0.0000\n"};
    auto tokens = ParserOBJ::splitLineToTokens(str);
    EXPECT_TRUE(ParserOBJ::isValidVertex(tokens));
}

TEST_F(OBJFileSupport, VerifiesTriangle)
{
    std::string str{ "f 1 2 3\n"};
    auto tokens = ParserOBJ::splitLineToTokens(str);
    EXPECT_TRUE(ParserOBJ::isValidTriangle(tokens));
}

TEST_F(OBJFileSupport, IdentifiesGroup)
{
    std::string str{ "g GroupName"};
    auto obj = ParserOBJ{};
    auto type = obj.parseStatement(str);
    EXPECT_EQ(type, ParserOBJ::StatementType::group);
}


TEST_F(OBJFileSupport, ProcessesVertexData)
{
    // four valid vertex statements are parsed
    std::string testData{
        "v -1 1 0\n"
        "v -1.0000 0.5000 0.0000\n"
        "v 1 0 0\n"
        "v 1 1 0"
    };
    makeTestFile(filename, testData);
    auto file = ParserOBJ::openFile(filename);

    ParserOBJ obj{};
    auto nSkipped = obj.parseFile(std::move(file));

    ASSERT_EQ(nSkipped, 0);
    EXPECT_EQ(obj.getVertex(1), Point( -1, 1, 0 ));
    EXPECT_EQ(obj.getVertex(2), Point( -1, 0.5, 0 ));
    EXPECT_EQ(obj.getVertex(3), Point( 1, 0, 0 ));
    EXPECT_EQ(obj.getVertex(4), Point( 1, 1, 0 ));
}

TEST_F(OBJFileSupport, ProcessesTriangleData)
{
    // two triangular faces are parsed into shapes added
    // to the OBJ's geometry group member
    std::string testData{
        "v -1 1 0\n"
        "v -1 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "f 1 2 3\n"
        "f 1 3 4\n"
    };
    makeTestFile(filename, testData);
    auto file = ParserOBJ::openFile(filename);

    ParserOBJ obj{};
    auto nSkipped = obj.parseFile(std::move(file));

    auto g = obj.getGroup();
    auto* t1 = dynamic_cast<Triangle*>(&g.getChild(0));
    auto* t2 = dynamic_cast<Triangle*>(&g.getChild(1));
    ASSERT_EQ(nSkipped, 0);
    EXPECT_EQ(t1->getP1(), obj.getVertex(1));
    EXPECT_EQ(t1->getP2(), obj.getVertex(2));
    EXPECT_EQ(t1->getP3(), obj.getVertex(3));
    EXPECT_EQ(t2->getP1(), obj.getVertex(1));
    EXPECT_EQ(t2->getP2(), obj.getVertex(3));
    EXPECT_EQ(t2->getP3(), obj.getVertex(4));
}

TEST_F(OBJFileSupport, ProcessesPolygonalData)
{
    // parses and triangulates arbitrary polygon data, converting the polygon
    // into triangle data our raytracer can interpret
    std::string testData{
        "v -1 1 0\n"
        "v -1 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 2 0\n"
        "f 1 2 3 4 5\n"
    };
    makeTestFile(filename, testData);
    auto file = ParserOBJ::openFile(filename);

    ParserOBJ obj{};
    auto nSkipped = obj.parseFile(std::move(file));

    auto g = obj.getGroup();
    Triangle* t1 = dynamic_cast<Triangle*>(&g.getChild(0));
    Triangle* t2 = dynamic_cast<Triangle*>(&g.getChild(1));
    Triangle* t3 = dynamic_cast<Triangle*>(&g.getChild(2));
    ASSERT_EQ(nSkipped, 0);
    EXPECT_EQ(t1->getP1(), obj.getVertex(1));
    EXPECT_EQ(t1->getP2(), obj.getVertex(2));
    EXPECT_EQ(t1->getP3(), obj.getVertex(3));
    EXPECT_EQ(t2->getP1(), obj.getVertex(1));
    EXPECT_EQ(t2->getP2(), obj.getVertex(3));
    EXPECT_EQ(t2->getP3(), obj.getVertex(4));
    EXPECT_EQ(t3->getP1(), obj.getVertex(1));
    EXPECT_EQ(t3->getP2(), obj.getVertex(4));
    EXPECT_EQ(t3->getP3(), obj.getVertex(5));
}

TEST_F(OBJFileSupport, ProcessesGroupData)
{
    // parses and triangulates an OBJ format named group
    // the group might only be triangles, or it may be polygons, too
    std::string testData{
        "v -1 1 0\n"
        "v -1 0 0\n"
        "v 1 0 0\n"
        "v -1 1 0\n"
        "v -1 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 2 0\n"
        "g TriGroup\n"
        "f 1 2 3\n"
        "g PolygonalGroup\n"
        "f 4 5 6 7 8\n"
    };

    makeTestFile(filename, testData);
    auto file = ParserOBJ::openFile(filename);

    ParserOBJ obj{};
    auto nSkipped = obj.parseFile(std::move(file));

    auto g = obj.getGroup();
    Group* g1 = dynamic_cast<Group*>(&g.getChild(0));
    Group* g2 = dynamic_cast<Group*>(&g.getChild(1));
    ASSERT_EQ(nSkipped, 0);
    // triangle group
    Triangle* t1 = dynamic_cast<Triangle*>(&g1->getChild(0));
    EXPECT_EQ(t1->getP1(), obj.getVertex(1));
    EXPECT_EQ(t1->getP2(), obj.getVertex(2));
    EXPECT_EQ(t1->getP3(), obj.getVertex(3));
    // polygon group
    Triangle* t2 = dynamic_cast<Triangle*>(&g2->getChild(0));
    Triangle* t3 = dynamic_cast<Triangle*>(&g2->getChild(1));
    Triangle* t4 = dynamic_cast<Triangle*>(&g2->getChild(2));
    EXPECT_EQ(t2->getP1(), obj.getVertex(4));
    EXPECT_EQ(t2->getP2(), obj.getVertex(5));
    EXPECT_EQ(t2->getP3(), obj.getVertex(6));
    EXPECT_EQ(t3->getP1(), obj.getVertex(4));
    EXPECT_EQ(t3->getP2(), obj.getVertex(6));
    EXPECT_EQ(t3->getP3(), obj.getVertex(7));
    EXPECT_EQ(t4->getP1(), obj.getVertex(4));
    EXPECT_EQ(t4->getP2(), obj.getVertex(7));
    EXPECT_EQ(t4->getP3(), obj.getVertex(8));
}

TEST_F(OBJFileSupport, ParsesToGroupShape)
{
    std::string testData{
        "v -1 1 0\n"
        "v -1 0 0\n"
        "v 1 0 0\n"
        "g TriGroup\n"
        "f 1 2 3\n"
    };
    makeTestFile(filename, testData);

    ParserOBJ obj{};
    auto g = obj.parseToGroup(filename);
    auto g1 = dynamic_cast<Group*>(&g.getChild(0));
    Triangle* t1 = dynamic_cast<Triangle*>(&g1->getChild(0));
    EXPECT_EQ(t1->getP1(), obj.getVertex(1));
    EXPECT_EQ(t1->getP2(), obj.getVertex(2));
    EXPECT_EQ(t1->getP3(), obj.getVertex(3));
}