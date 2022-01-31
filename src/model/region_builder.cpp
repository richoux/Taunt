#include "model/region_builder.hpp"
#include "model/constraint_no_cuts.hpp"
#include "model/constraint_one_cluster_per_region.hpp"
//#include "model/constraint_minimal_region_area.hpp"
#include "model/objective_min_separation_width.hpp"
//#include "model/objective_same_area.hpp"

RegionBuilder::RegionBuilder( int number_separations,
                              const polygon& contour,
                              const std::vector<multi_point>& resources )
	: ghost::ModelBuilder( true ), // permutation problem
	  number_separations( number_separations ),
	  contour( contour ),
	  resources( resources ),
	  separation_candidates{}
{	
	for( size_t p1 = 0 ; p1 < contour.outer().size() - 2 ; ++p1 )
		for( size_t p2 = p1 + 2 ; p2 < contour.outer().size() ; ++p2 )
		{
			if( !boost::geometry::equals( contour.outer()[p1], contour.outer()[p2] )
			    && ( p2 > p1 + 2 || ( contour.outer()[p1].x() != contour.outer()[p2].x() && contour.outer()[p1].y() != contour.outer()[p2].y() ) ) )
			{
				bool to_add = true;
				line line{{ contour.outer()[p1], contour.outer()[p2] }};
				ring ring_line{{ contour.outer()[p1], contour.outer()[p2] }};
				boost::geometry::correct( ring_line );
				
				for( auto& resource_cluster : resources )
					for( auto& resource_point : resource_cluster )
						if( boost::geometry::crosses( ring_line, resource_point ) )
						{
							to_add = false;
							break;
						}
			
				for( auto& inner : contour.inners() )
					if( boost::geometry::covered_by( ring_line, inner ) || boost::geometry::crosses( inner, line ) )
					{
						to_add = false;
						break;
					}

				if( !boost::geometry::covered_by( line, contour.outer() ) )
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
	constraints.emplace_back( std::make_shared<NoCuts>( variables, contour, resources, separation_candidates ) );
	//constraints.emplace_back( std::make_shared<NumberSeparations>( variables, number_separations ) );

	// for( auto& var : variables )
	// {
	// 	std::vector<ghost::Variable> vec_var{ var };
	// 	constraints.emplace_back( std::make_shared<OneClusterPerRegion>( vec_var, auxiliary_data ) );
	// 	constraints.emplace_back( std::make_shared<MinRegionArea>( vec_var, auxiliary_data, number_separations ) );
	// }

	constraints.emplace_back( std::make_shared<OneClusterPerRegion>( variables, contour, resources, separation_candidates ) );
	//constraints.emplace_back( std::make_shared<MinRegionArea>( variables, contour, separation_candidates, number_separations ) );
}

void RegionBuilder::declare_objective()
{
	objective = std::make_shared<MinSeparationWidth>( variables, separation_candidates );
	//objective = std::make_shared<SameArea>( variables, contour, separation_candidates );
}
