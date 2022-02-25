#include "model/region_builder.hpp"
#include "model/constraint_no_cuts.hpp"
#include "model/constraint_one_cluster_per_region.hpp"
//#include "model/constraint_minimal_region_area.hpp"
#include "model/objective_min_separation_width.hpp"
//#include "model/objective_same_area.hpp"

RegionBuilder::RegionBuilder( int number_separations,
                              const boost_polygon& polygon,
                              const std::vector<multipoint>& resources )
	: ghost::ModelBuilder( true ), // permutation problem
	  number_separations( number_separations ),
		polygon( polygon ),
	  resources( resources ),
	  separation_candidates{}
{	
	for( size_t p1 = 0 ; p1 < polygon.outer().size() - 2 ; ++p1 )
		for( size_t p2 = p1 + 2 ; p2 < polygon.outer().size() ; ++p2 )
		{
			if( !boost::geometry::equals( polygon.outer()[p1], polygon.outer()[p2] )
			    && ( p2 > p1 + 2 || ( polygon.outer()[p1].x() != polygon.outer()[p2].x() && polygon.outer()[p1].y() != polygon.outer()[p2].y() ) ) )
			{
				bool to_add = true;
				line line{ polygon.outer()[p1], polygon.outer()[p2] };
				ring ring_line{ polygon.outer()[p1], polygon.outer()[p2] };
				boost::geometry::correct( ring_line );
				
				for( auto& resource_cluster : resources )
					for( auto& resource_point : resource_cluster )
						if( boost::geometry::crosses( ring_line, resource_point ) )
						{
							to_add = false;
							break;
						}
			
				for( auto& inner : polygon.inners() )
					if( boost::geometry::covered_by( ring_line, inner ) || boost::geometry::crosses( inner, line ) )
					{
						to_add = false;
						break;
					}

				if( !boost::geometry::covered_by( line, polygon.outer() ) )
					to_add = false;
				
				if( to_add )
					separation_candidates.push_back( line );
			}
		}
}

void RegionBuilder::declare_variables()
{
	create_n_variables( separation_candidates.size(), 0, 2 );
	
	for( auto& var : variables )
		var.set_value( 0 );

	int index = 0;
	std::vector<int> indexes( variables.size() );
	std::iota( indexes.begin(), indexes.end(), 0 );
	for( int i = 0 ; i < number_separations ; ++i )
	{
		do
			index = _rng.pick( indexes );
		while( variables[index].get_value() != 0 );
		
		variables[index].set_value( 1 );
	}
}

void RegionBuilder::declare_constraints()
{
	//constraints.emplace_back( std::make_shared<WithinPolygon>( variables, auxiliary_data ) );
	constraints.emplace_back( std::make_shared<NoCuts>( variables, polygon, resources, separation_candidates ) );
	//constraints.emplace_back( std::make_shared<NumberSeparations>( variables, number_separations ) );

	// for( auto& var : variables )
	// {
	// 	std::vector<ghost::Variable> vec_var{ var };
	// 	constraints.emplace_back( std::make_shared<OneClusterPerRegion>( vec_var, auxiliary_data ) );
	// 	constraints.emplace_back( std::make_shared<MinRegionArea>( vec_var, auxiliary_data, number_separations ) );
	// }

	constraints.emplace_back( std::make_shared<OneClusterPerRegion>( variables, polygon, resources, separation_candidates ) );
	//constraints.emplace_back( std::make_shared<MinRegionArea>( variables, polygon, separation_candidates, number_separations ) );
}

void RegionBuilder::declare_objective()
{
	objective = std::make_shared<MinSeparationWidth>( variables, separation_candidates );
	//objective = std::make_shared<SameArea>( variables, polygon, separation_candidates );
}
