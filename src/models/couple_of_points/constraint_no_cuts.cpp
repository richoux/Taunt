#include "constraint_no_cuts.hpp"

NoCuts::NoCuts( const std::vector<ghost::Variable>& variables,
                const polygon& contour,
                const std::vector<multi_point>& resources )
	: Constraint( variables ),
	  _contour( contour ),
	  _resources( resources )
{ }

double NoCuts::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	std::vector< bool > processed( variables.size(), false );
	int error = 0;

	std::vector<line> vec_lines;
	
	for( size_t i = 0 ; i < variables.size() - 1 ; ++i )
	{
		if( variables[i]->get_value() > 0 && !processed[i] )
		{
			processed[i] = true;
			for( size_t j = i + 1 ; j < variables.size() ; ++j )
				if( variables[j]->get_value() == variables[i]->get_value() && !processed[j] )
				{
					processed[j] = true;
					ring ring_line{{ _contour.outer()[i], _contour.outer()[j] }};
					line line{{ _contour.outer()[i], _contour.outer()[j] }};

					for( auto& resource_cluster : _resources )
						for( auto& resource_point : resource_cluster )
							if( boost::geometry::crosses( ring_line, resource_point ) )
								++error;

					for( auto& inner : _contour.inners() )
						if( boost::geometry::covered_by( ring_line, inner ) || boost::geometry::crosses( inner, line ) )
							++error;

					for( auto& segment : vec_lines )
						if( boost::geometry::crosses( segment, line ) )
							++error;

					vec_lines.push_back( line );
				}
		}
	}

	return error;
}
