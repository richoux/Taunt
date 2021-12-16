#include "region_builder.hpp"
#include "constraint_no_cuts.hpp"
#include "constraint_one_cluster_per_region.hpp"
#include "constraint_minimal_region_area.hpp"
#include "objective_min_separation_width.hpp"

RegionBuilder::RegionBuilder( int number_separations,
                              const polygon& contour,
                              const std::vector<multi_point>& resources )
	: ghost::ModelBuilder( true ), // permutation problem
	  number_separations( number_separations ),
	  contour( contour ),
	  resources( resources ),
	  separation_candidates{}
{
#if defined MUF
	std::cout << "Contour.outer: " << boost::geometry::dsv( contour.outer() ) << "\n";
#endif
				
	for( size_t p1 = 0 ; p1 < contour.outer().size() - 2 ; ++p1 )
		for( size_t p2 = p1 + 2 ; p2 < contour.outer().size() ; ++p2 )
		{
#if defined MUF
			line line{{ contour.outer()[p1], contour.outer()[p2] }};
			std::cout << "Testing[" << p1 << "," << p2 << "] "<< boost::geometry::dsv( line )
			          << " equals:" << std::boolalpha << boost::geometry::equals( contour.outer()[p1], contour.outer()[p2] )
			          << " p2>p1+2:" << ( p2 > p1 + 2 )
			          << " p1.x!=p2.x:" << ( contour.outer()[p1].x() != contour.outer()[p2].x() )
			          << " p1.y!=p2.y:" << ( contour.outer()[p1].y() != contour.outer()[p2].y() ) << "\n";
#endif
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
#if defined MUF
					std::cout << "Don't add " << boost::geometry::dsv( ring_line ) << ": cross resources.\n";
#endif
							to_add = false;
							break;
						}
			
				for( auto& inner : contour.inners() )
					if( boost::geometry::covered_by( ring_line, inner ) || boost::geometry::crosses( inner, line ) )
					{
#if defined MUF
					std::cout << "Don't add " << boost::geometry::dsv( line ) << ": cross inners.\n";
#endif
						to_add = false;
						break;
					}

				if( !boost::geometry::covered_by( line, contour.outer() ) )
				{
#if defined MUF
					std::cout << "Don't add " << boost::geometry::dsv( line ) << ": out of zone.\n";
#endif
					to_add = false;
				}
				
				if( to_add )
					separation_candidates.push_back( line );
			}
		}
#if defined MUF
	for( size_t i = 0 ; i < separation_candidates.size() ; ++i )
		std::cout << "Var[" << i << "] = " << boost::geometry::dsv( separation_candidates[i] ) << "\n";
#endif
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
}

// void RegionBuilder::declare_auxiliary_data()
// {
// 	auxiliary_data = std::make_shared<DataZone>( variables, contour, resources, separation_candidates );
// }
