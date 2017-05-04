#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "model.h"
#include "vincenty.h"
#include "visualisation.h"


TEST_CASE( "01", "[vincenty]" ) {
    const auto L1 = model::Location{42.3541165, -71.0693514};
    const auto L2 = model::Location{40.7791472, -73.9680804};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 297842.96875f );
}

TEST_CASE( "02", "[vincenty]" ) {
    const auto L1 = model::Location{45.7597, 4.8422};
    const auto L2 = model::Location{48.8567, 2.3508};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 391998.09375f );
}

TEST_CASE( "03", "[vincenty]" ) {
    const auto L1 = model::Location{48.148636, 17.107558};
    const auto L2 = model::Location{48.208810, 16.372477};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 54881.52344f );
}

TEST_CASE( "04", "[vincenty]" ) {
    const auto L1 = model::Location{31.8300167,35.0662833};
    const auto L2 = model::Location{31.8300000,35.0708167};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 427.90118f );
}

TEST_CASE( "05", "[vincenty]" ) {
    const auto L1 = model::Location{46.49005, 30.74535};
    const auto L2 = model::Location{55.75372, 37.61991};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 1134385.75f );
}

TEST_CASE( "www.movable-type.co.uk/scripts/latlong-vincenty.html", "[vincenty]" ) {
    const auto L1 = model::Location{50.09861111, 5.74555556};
    const auto L2 = model::Location{58.98277778, 3.05083333};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 1002280.5f );
}

TEST_CASE( "pygeodesy/ellipsoidalVincenty.py Newport_RI Cleveland_OH", "[vincenty]" ) {
    const auto L1 = model::Location{41.49008, -71.312796};
    const auto L2 = model::Location{41.499498, -81.695391};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 863730.6875f );
}

TEST_CASE( "haskell", "[vincenty]" ) {
    const auto L1 = model::Location{-66.093, 12.84};
    const auto L2 = model::Location{27.812, 154.295};
    const auto r  = vincenty(L2, L1);
    REQUIRE( r == 15000939.0f );
}






TEST_CASE( "31℃", "[color]" ) {
    const auto t = 31.8300167f;
    const auto c = visualisation::interpolateColor(t);
    REQUIRE( c.red   == 255 );
    REQUIRE( c.green ==   2 );
    REQUIRE( c.blue  ==   0 );
}

TEST_CASE( "-11℃", "[color]" ) {
    const auto t = -11.f;
    const auto c = visualisation::interpolateColor(t);
    REQUIRE( c.red   ==   0 );
    REQUIRE( c.green ==  68 );
    REQUIRE( c.blue  == 255 );
}

TEST_CASE( "-50℃", "[color]" ) {
    const auto t = -50.f;
    const auto c = visualisation::interpolateColor(t);
    REQUIRE( c.red   ==  33 );
    REQUIRE( c.green ==   0 );
    REQUIRE( c.blue  == 107 );
}


TEST_CASE( "77℃", "[color]" ) {
    const auto t = 77.f;
    const auto c = visualisation::interpolateColor(t);
    REQUIRE( c.red   == 255 );
    REQUIRE( c.green == 255 );
    REQUIRE( c.blue  == 255 );
}

TEST_CASE( "sin-31", "[sincostab]" ) {
    const auto t = -3.1415;
    const auto c = test_me_sin(t);
    REQUIRE( c  == "sin(-3.1415)=-9.26574e-5, tab[10]=-9.26574e-5" );
}

TEST_CASE( "sin-01", "[sincostab]" ) {
    const auto t = -0.111111111f;
    const auto c = test_me_sin(t);
    REQUIRE( c  == "sin(-0.111111)=-0.110883, tab[303049]=-0.110882" );
}

TEST_CASE( "sin 00", "[sincostab]" ) {
    const auto t = 0.f;
    const auto c = test_me_sin(t);
    REQUIRE( c  == "sin(0)=0, tab[314160]=0" );
}

TEST_CASE( "sin 01", "[sincostab]" ) {
    const auto t = 0.1f;
    const auto c = test_me_sin(t);
    REQUIRE( c  == "sin(0.1)=0.0998334, tab[324160]=0.0998334" );
}

TEST_CASE( "sin 02", "[sincostab]" ) {
    const auto t = 0.2f;
    const auto c = test_me_sin(t);
    REQUIRE( c  == "sin(0.2)=0.198669, tab[334160]=0.198669" );
}

TEST_CASE( "sin 03", "[sincostab]" ) {
    const auto t = 0.3f;
    const auto c = test_me_sin(t);
    REQUIRE( c  == "sin(0.3)=0.29552, tab[344160]=0.29552" );
}

TEST_CASE( "sin 04", "[sincostab]" ) {
    const auto t = 0.44444444444f;
    const auto c = test_me_sin(t);
    REQUIRE( c  == "sin(0.444444)=0.429956, tab[358604]=0.429952" );
}



