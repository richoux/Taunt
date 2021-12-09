#pragma once

#include <ghost/constraint.hpp>

class NumberSeparations : public ghost::Constraint
{
	int _number_separations;

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
	
public:
	NumberSeparations( const std::vector<ghost::Variable>& variables, int number_separations );
};
