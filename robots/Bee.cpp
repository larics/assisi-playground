/*

 */

#include <cmath>

#include <boost/foreach.hpp>
#include <boost/math/constants/constants.hpp>

#include <PhysicalEngine.h>

#include "Bee.h"

const double pi = boost::math::constants::pi<double>();

// Signum function
template <typename T>
int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

namespace Enki
{
    Bee::Bee(void) :
        DifferentialWheeled(0.4, 2, 0.0),
        range_sensors(5)
    {
        // Set shape & color

        double len = 2; // Body length
        double w = 0.8; // Body width
        double h = 0.4; // Body height
        double m = 1;   // Body mass

        //setRectangular(len, w, h, 1);
        Polygone footprint;
        footprint.push_back(Point(len/2,w/4));
        footprint.push_back(Point(len/2-w/(4*sqrt(2)),w/2));
        footprint.push_back(Point(-len/2+w/(4*sqrt(2)),w/2));
        footprint.push_back(Point(-len/2,w/4));
        footprint.push_back(Point(-len/2,-w/4));
        footprint.push_back(Point(-len/2+w/(2*sqrt(2)),-w/2));
        footprint.push_back(Point(len/2-w/(2*sqrt(2)),-w/2));
        footprint.push_back(Point(len/2,-w/4));
        PhysicalObject::Hull hull(PhysicalObject::Part(footprint, h));
        setCustomHull(hull, m);
        setColor(Color(0.93,0.79,0,1));

        // Set other physical properties
        PhysicalObject::dryFrictionCoefficient = 2.5;

        int i = 0;
        for (double a = -pi/2; i < 5; i++, a += pi/4) 
        {
            range_sensors[i] = new IRSensor(this, 
                                            Vector(len/2-sin(a)*w/2, sin(a)*w/2),
                                            0, a, 10, 3731, 0, 0.7, 0);
            addLocalInteraction(range_sensors[i]);
        }
    }

    /* virtual */
    Bee::~Bee()
    {
        BOOST_FOREACH(IRSensor* p, range_sensors)
        {
            delete p;
        }
    }
}