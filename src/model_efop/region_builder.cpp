#include "region_builder.hpp"
#include "constraint_within_polygon.hpp"
#include "constraint_no_cuts.hpp"
//#include "constraint_number_of_separations.hpp"
#include "constraint_one_cluster_per_region.hpp"
//#include "constraint_minimal_region_area.hpp"
#include "objective_min_separation_width.hpp"

RegionBuilder::RegionBuilder( int number_separations,
                              const polygon& contour,
                              const std::vector<multi_point>& resources )
	: ghost::ModelBuilder( true ), // permutation problem
	  _number_separations( number_separations ),
	  _contour( contour ),
	  _resources( resources )
{ }

void RegionBuilder::declare_variables()
{
	create_n_variables( _contour.outer().size(), 0, _number_separations + 1 ); // + 1 because we need a value to express the non-selection (here, 0),
	                                                                           // ie, variables that do not participate to a separation.

	for( auto& var : variables )
		var.set_value( 0 );

	int index = 0;
	std::vector<int> indexes( variables.size() );
	std::iota( indexes.begin(), indexes.end(), 0 );
	for( int i = 0 ; i < _number_separations ; ++i )
		for( int j = 0 ; j < 2 ; ++j )
		{
			do
			{
				index = _rng.pick( indexes );
			}
			while( variables[index].get_value() != 0 );
			variables[index].set_value( i + 1 );
		}
}

void RegionBuilder::declare_constraints()
{
	constraints.emplace_back( std::make_shared<WithinPolygon>( variables, _contour ) );
	constraints.emplace_back( std::make_shared<NoCuts>( variables, _contour, _resources ) );
	//constraints.emplace_back( std::make_shared<NumberSeparations>( variables, _number_separations ) );
	constraints.emplace_back( std::make_shared<OneClusterPerRegion>( variables, _contour, _resources, _number_separations ) );
	//constraints.emplace_back( std::make_shared<MinRegionArea>( variables, _contour, _number_separations ) );
}

void RegionBuilder::declare_objective()
{
	objective = std::make_shared<MinSeparationWidth>( variables, _contour );
}
