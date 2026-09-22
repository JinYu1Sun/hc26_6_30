//          Copyright Erik Lundin 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Version: 1.0.0

#ifndef GAUSSKRUGER_H
#define GAUSSKRUGER_H

namespace gausskruger {


class Projection
{
public:
    Projection() {}
    virtual ~Projection() {}
    virtual double centralMeridian() = 0;
    virtual double flattening() = 0;
    virtual double equatorialRadius() = 0;
    virtual double scale() = 0;
    virtual double falseNorthing() = 0;
    virtual double falseEasting() = 0;
    void geodeticToGrid(double latitude, double longitude, double& northing, double& easting);
    void gridToGeodetic(double northing, double easting, double& latitude, double& longitude);

    double longitude_ = 0;
};

class WGS84Projection : public Projection
{
public:

    double flattening() { return 1/298.257223563; }
    double equatorialRadius() { return 6378137.0; }
    double centralMeridian() { return 3 * (int) ((longitude_+1.5) / 3);} // zone wide = 3
    double scale() { return 1.0; }
    double falseNorthing() { return 0; }
    double falseEasting() { return 0; }
};
} // namespace gausskruger

#endif // GAUSSKRUGER_H
