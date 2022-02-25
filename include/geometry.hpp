#pragma once

#include <boost/geometry.hpp>

using point = boost::geometry::model::d2::point_xy<double>;
using line = boost::geometry::model::linestring<point>;
using box = boost::geometry::model::box<point>;
using ring = boost::geometry::model::ring<point>;
using boost_polygon = boost::geometry::model::polygon<point>;

using multipoint = boost::geometry::model::multi_point<point>;
using multiline = boost::geometry::model::multi_linestring<line>;
using multipolygon = boost::geometry::model::multi_polygon<boost_polygon>;
