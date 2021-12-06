#include "region_builder.hpp"
#include "constraint_within_polygon.hpp"
#include "constraint_no_cuts.hpp"
#include "constraint_number_of_separations.hpp"
#include "objective_min_separation_width.hpp"

RegionBuilder::RegionBuilder( int number_separations,
                              const polygon& contour,
                              const std::vector<multi_point>& resources )
	: ghost::ModelBuilder(),
	  _number_separations( number_separations ),
	  _contour( contour ),
	  _resources( resources )
{ }

void RegionBuilder::declare_variables()
{
	create_n_variables( _contour.outer().size(), 0, _number_separations + 1 ); // + 1 because we need a value to express the non-selection (here, 0),
	                                                                           // ie, variables that do not participate to a separation.
}

void RegionBuilder::declare_constraints()
{
	constraints.emplace_back( std::make_shared<WithinPolygon>( variables, _contour ) );
	constraints.emplace_back( std::make_shared<NoCuts>( variables, _contour, _resources ) );
	constraints.emplace_back( std::make_shared<NumberSeparations>( variables, _number_separations ) );
}

void RegionBuilder::declare_objective()
{
	objective = std::make_shared<MinSeparationWidth>( variables, _contour );
}
