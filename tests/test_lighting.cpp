#include "lighting.h"
#include "gtest/gtest.h"

using namespace rt;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Base Point Lighting Class
////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(PointLighting, PointLightingHasPositionAndIntensity)
{
    Colour intensity{ 1, 1, 1 };
    Point position{ 0, 0, 0 };
    PointLight light{ position, intensity };
    EXPECT_EQ(light.position, position);
    EXPECT_EQ(light.colour, intensity);
}

TEST(PointLighting, DefaultLightIsWhite)
{
    Colour intensity{ 1, 1, 1 };
    Point position{ 1, 2, 3 };
    Light light{ position };
    EXPECT_EQ(light.position, position);
    EXPECT_EQ(light.colour, intensity);
}



