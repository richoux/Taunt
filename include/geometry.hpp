#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using multipoint = boost::geometry::model::multi_point<point>;
using line = boost::geometry::model::linestring<point>;
using segment = boost::geometry::model::segment<point>;
using contour = boost::geometry::model::ring<point>;
using boost_polygon = boost::geometry::model::polygon<point>;
using multi_polygon = boost::geometry::model::multi_polygon<boost_polygon>;