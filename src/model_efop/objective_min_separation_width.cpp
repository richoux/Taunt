#include "objective_min_separation_width.hpp"

MinSeparationWidth::MinSeparationWidth( const std::vector<ghost::Variable>& variables, const polygon& contour )
	: Minimize( variables ),
	  _contour( contour )
{}

double MinSeparationWidth::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	std::vector< bool > processed( variables.size(), false );
	double total_width = 0.0;
	
	for( size_t i = 0 ; i < variables.size() - 1 ; ++i )
	{
		if( variables[i]->get_value() > 0 && !processed[i] )
		{
			processed[i] = true;
			for( size_t j = i + 1 ; j < variables.size() ; ++j )
				if( variables[j]->get_value() == variables[i]->get_value() && !processed[j] )
				{
					processed[j] = true;
					segment segment{ _contour.outer()[i], _contour.outer()[j] };
					total_width += boost::geometry::length( segment );
				}
		}
	}

	return total_width;
}
