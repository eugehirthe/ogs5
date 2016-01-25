// Solution.cxx: implementation of the cxxSolution class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma warning(disable : 4786)	// disable truncation warning (Only used by debugger)
#endif

#include <set>
#include <cassert>				// assert
#include <algorithm>			// std::sort
#include "Utils.h"				// define first
#include "Phreeqc.h"
#include "Solution.h"
#include "cxxMix.h"
#include "phqalloc.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


cxxSolution::cxxSolution(PHRQ_io * io)
	//
	// default constructor for cxxSolution 
	//
:	cxxNumKeyword(io)
{
	this->io = io;
	this->new_def = false;
	this->patm = 1.0;
	this->tc = 25.0;
	this->ph = 7.0;
	this->pe = 4.0;
	this->mu = 1e-7;
	this->ah2o = 1.0;
	this->total_h = 111.1;
	this->total_o = 55.55;
	this->cb = 0.0;
	this->density = 1.0;
	this->mass_water = 1.0;
	this->soln_vol = 1.0;
	this->total_alkalinity = 0.0;
	this->totals.type = cxxNameDouble::ND_ELT_MOLES;
	this->master_activity.type = cxxNameDouble::ND_SPECIES_LA;
	this->species_gamma.type = cxxNameDouble::ND_SPECIES_GAMMA;
	this->initial_data = NULL;
}
cxxSolution::cxxSolution(const cxxSolution &old_sol)
:	initial_data(NULL)
{
	*this = old_sol;
}
const cxxSolution &
cxxSolution::operator =(const cxxSolution &rhs)
{
	if (this != &rhs)
	{
		this->io                         = rhs.io;
		this->n_user                     = rhs.n_user;
		this->n_user_end                 = rhs.n_user_end;
		this->description                = rhs.description;
		this->new_def                    = rhs.new_def;
		this->patm                       = rhs.patm;
		this->tc                         = rhs.tc;
		this->ph                         = rhs.ph;
		this->pe                         = rhs.pe;
		this->mu                         = rhs.mu;
		this->ah2o                       = rhs.ah2o;
		this->total_h                    = rhs.total_h;
		this->total_o                    = rhs.total_o;
		this->density                    = rhs.density;
		this->cb                         = rhs.cb;
		this->mass_water                 = rhs.mass_water;
		this->soln_vol                   = rhs.soln_vol;
		this->total_alkalinity           = rhs.total_alkalinity;
		this->totals		             = rhs.totals;
		this->master_activity            = rhs.master_activity;
		this->species_gamma              = rhs.species_gamma;
		this->isotopes                   = rhs.isotopes;
		if (this->initial_data)
			delete initial_data;
		if (rhs.initial_data != NULL)
			this->initial_data           = new cxxISolution(*rhs.initial_data);
		else
			this->initial_data           = NULL;
	}
	return *this;
}
cxxSolution::cxxSolution(std::map < int, cxxSolution > &solutions,
						 cxxMix & mix, int l_n_user, PHRQ_io * io)
//
// constructor for cxxSolution from mixture of solutions
//
: cxxNumKeyword(io)
{
//
//   Zero out solution data
//
	this->zero();
	this->n_user = this->n_user_end = l_n_user;
	this->new_def = false;
	this->ah2o = 0;
//
//   Mix solutions
//
	const std::map < int, LDBLE >&mixcomps = mix.Get_mixComps();
	std::map < int, LDBLE >::const_iterator it;
	for (it = mixcomps.begin(); it != mixcomps.end(); it++)
	{
		std::map < int, cxxSolution >::const_iterator sol =
			solutions.find(it->first);
		if (sol == solutions.end())
		{
			std::ostringstream msg;
			msg << "Solution " << it->first << " not found in mix_cxxSolutions.";
			error_msg(msg.str(), CONTINUE);
		}
		else
		{
			const cxxSolution *cxxsoln_ptr1 = &(sol->second);
			this->add(*cxxsoln_ptr1, it->second);
		}
	}

}
cxxSolution::~cxxSolution()
{
	delete this->initial_data;
}

void
cxxSolution::dump_xml(std::ostream & s_oss, unsigned int indent) const
{
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);

	// Solution element and attributes
	s_oss << indent0;
	s_oss << "<solution " << "\n";

	s_oss << indent1;
	s_oss << "soln_n_user=\"" << this->n_user << "\" " << "\n";

	s_oss << indent1;
	s_oss << "soln_description=\"" << this->description << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_tc=\"" << this->tc << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_ph=\"" << this->ph << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_solution_pe=\"" << this->pe << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_mu=\"" << this->mu << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_ah2o=\"" << this->ah2o << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_total_h=\"" << this->total_h << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_total_o=\"" << this->total_o << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_cb=\"" << this->cb << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_mass_water=\"" << this->mass_water << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_vol=\"" << this->soln_vol << "\"" << "\n";

	s_oss << indent1;
	s_oss << "soln_total_alkalinity=\"" << this->
		total_alkalinity << "\"" << "\n";

	s_oss << indent1;
	s_oss << "\">" << "\n";

	// soln_total conc structures
	this->totals.dump_xml(s_oss, indent + 1);

	// master_activity map
	this->master_activity.dump_xml(s_oss, indent + 1);

	// species_gamma map
	this->species_gamma.dump_xml(s_oss, indent + 1);

	// End of solution
	s_oss << indent0;
	s_oss << "</solution>" << "\n";

	return;
}

void
cxxSolution::dump_raw(std::ostream & s_oss, unsigned int indent, int *n_out) const
{
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Solution element and attributes
	s_oss << indent0;
	int n_user_local = (n_out != NULL) ? *n_out : this->n_user;
	s_oss << "SOLUTION_RAW                 " << n_user_local << " " << this->description << "\n";

	s_oss << indent1;
	s_oss << "-temp                      " << this->tc << "\n";

	s_oss << indent1;
	s_oss << "-pressure                  " << this->patm << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-total_h                   " << this->total_h << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-total_o                   " << this->total_o << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-cb                        " << this->cb << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-density                   " << this->density << "\n";

	// soln_total conc structures
	s_oss << indent1;
	s_oss << "-totals" << "\n";
	this->totals.dump_raw(s_oss, indent + 2);

	// Isotopes
	{
		for (std::map < std::string, cxxSolutionIsotope >::const_iterator it =
			this->isotopes.begin(); it != isotopes.end(); ++it)
		{
			s_oss << indent1 << "-Isotope" << "\n";
			it->second.dump_raw(s_oss, indent + 2);
		}
	}

	s_oss << indent1;
	s_oss << "-pH                        " << this->ph << "\n";

	s_oss << indent1;
	s_oss << "-pe                        " << this->pe << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-mu                        " << this->mu << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-ah2o                      " << this->ah2o << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-mass_water                " << this->mass_water << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-soln_vol                  " << this->soln_vol << "\n";

	// new identifier
	s_oss << indent1;
	s_oss << "-total_alkalinity          " << this->total_alkalinity << "\n";

	// master_activity map
	s_oss << indent1;
	s_oss << "-activities" << "\n";
	this->master_activity.dump_raw(s_oss, indent + 2);

	// species_gamma map
	s_oss << indent1;
	s_oss << "-gammas" << "\n";
	this->species_gamma.dump_raw(s_oss, indent + 2);

	return;
}

#ifdef USE_REVISED_READ_RAW
void
cxxSolution::read_raw(CParser & parser, bool check)
{

	// Used if it is modify
	cxxNameDouble original_totals = this->totals;
	cxxNameDouble original_activities(this->master_activity);

	this->master_activity.clear();

	std::istream::pos_type ptr;
	std::istream::pos_type next_char;
	std::string token;
	int opt_save;

	// Read solution number and description
	this->read_number_description(parser.line());
	this->Set_new_def(false);

	opt_save = CParser::OPT_ERROR;
	bool tc_defined(false);
	bool ph_defined(false);
	bool pe_defined(false);
	bool mu_defined(false);
	bool ah2o_defined(false);
	bool total_h_defined(false);
	bool total_o_defined(false);
	bool cb_defined(false);
	bool mass_water_defined(false);
	bool total_alkalinity_defined(false);

	for (;;)
	{
		int opt = parser.get_option(vopts, next_char);
		if (opt == CParser::OPT_DEFAULT)
		{
			opt = opt_save;
		}

		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_EOF;
			parser.error_msg("Unknown input in SOLUTION_RAW keyword.",
							 PHRQ_io::OT_CONTINUE);
			parser.error_msg(parser.line().c_str(), PHRQ_io::OT_CONTINUE);
			continue;

		case 0:				// totals
			{
				cxxNameDouble temp_totals;
				if (temp_totals.read_raw(parser, next_char) !=	CParser::PARSER_OK)
				{
					parser.incr_input_error();
					parser.
						error_msg("Expected element name and moles for totals.",
						PHRQ_io::OT_CONTINUE);
				}
				else
				{
					this->totals.merge_redox(temp_totals);
				}
			}
			opt_save = 0;
			break;

		case 1:				// activities
			if (this->master_activity.read_raw(parser, next_char) !=
				CParser::PARSER_OK)
			{
				parser.incr_input_error();
				parser.
					error_msg
					("Expected species name and log activity for activities.",
					 PHRQ_io::OT_CONTINUE);
			}
			opt_save = 1;
			break;

		case 2:				// gammas
			if (this->species_gamma.read_raw(parser, next_char) !=
				CParser::PARSER_OK)
			{
				parser.incr_input_error();
				parser.
					error_msg
					("Expected species name and activity coefficient for gammas.",
					 PHRQ_io::OT_CONTINUE);
			}
			opt_save = 2;
			break;

		case 3:				// isotope
			{
				std::string name;
				if (!(parser.get_iss() >> name))
				{
					parser.incr_input_error();
					parser.error_msg("Expected character value for isotope name.",
									 PHRQ_io::OT_CONTINUE);
				}
				else
				{
					cxxSolutionIsotope iso(this->Get_io());
					iso.Set_isotope_name(name.c_str());
					iso.read_raw(parser, check);
					this->isotopes[name] = iso;
				}
			}
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 4:				// temp
		case 5:				// tc_avoid_conflict_with_technetium
		case 6:				// temperature                  
			if (!(parser.get_iss() >> this->tc))
			{
				this->tc = 25.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for temperature.",
								 PHRQ_io::OT_CONTINUE);
			}
			tc_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 7:				// ph
			if (!(parser.get_iss() >> this->ph))
			{
				this->ph = 7.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for pH.",
								 PHRQ_io::OT_CONTINUE);
			}
			ph_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 8:				// pe
			if (!(parser.get_iss() >> this->pe))
			{
				this->pe = 4.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for pe.",
								 PHRQ_io::OT_CONTINUE);
			}
			pe_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 9:				// mu
		case 10:				// ionic_strength
			if (!(parser.get_iss() >> this->mu))
			{
				this->mu = 1e-7;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for ionic strength.",
								 PHRQ_io::OT_CONTINUE);
			}
			mu_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 11:				// ah2o
		case 12:				// activity_water
			if (!(parser.get_iss() >> this->ah2o))
			{
				this->ah2o = 1.0;
				parser.incr_input_error();
				parser.
					error_msg("Expected numeric value for activity of water.",
							  PHRQ_io::OT_CONTINUE);
			}
			ah2o_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 13:				// total_h
			if (!(parser.get_iss() >> this->total_h))
			{
				this->total_h = 111.1;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for total hydrogen.",
								 PHRQ_io::OT_CONTINUE);
			}
			total_h_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 14:				// total_o
			if (!(parser.get_iss() >> this->total_o))
			{
				this->total_o = 55.55;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for total oxygen.",
								 PHRQ_io::OT_CONTINUE);
			}
			total_o_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 15:				// mass_water
		case 16:				// mass_h2o
			if (!(parser.get_iss() >> this->mass_water))
			{
				this->mass_water = 1.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for mass of water.",
								 PHRQ_io::OT_CONTINUE);
			}
			mass_water_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 17:				// total_alkalinity
		case 18:				// total_alk
			if (!(parser.get_iss() >> this->total_alkalinity))
			{
				this->total_alkalinity = 0;
				parser.incr_input_error();
				parser.
					error_msg("Expected numeric value for total_alkalinity.",
							  PHRQ_io::OT_CONTINUE);
			}
			total_alkalinity_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 19:				// cb
		case 20:				// charge_balance
			if (!(parser.get_iss() >> this->cb))
			{
				this->cb = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for charge balance.",
								 PHRQ_io::OT_CONTINUE);
			}
			cb_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;
		case 21:				// density
			if (!(parser.get_iss() >> this->density))
			{
				this->density = 1.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for density.",
								 PHRQ_io::OT_CONTINUE);
			}
			opt_save = CParser::OPT_DEFAULT;
			break;
		}

		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	if (check)
	{
		// all members must be defined
		if (tc_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Temp not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (ph_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("pH not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (pe_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("pe not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (mu_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Ionic strength not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (ah2o_defined == false)
		{
			parser.incr_input_error();
			parser.
				error_msg("Activity of water not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (total_h_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Total hydrogen not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (total_o_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Total oxygen not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (cb_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Charge balance not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (mass_water_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Temp not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (total_alkalinity_defined == false)
		{
			parser.incr_input_error();
			parser.
				error_msg("Total alkalinity not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
	}

	// Update activities
	
	if (original_activities.size() > 0)
	{
		cxxNameDouble new_activities = this->master_activity;
		this->master_activity = original_activities;
		this->Update_activities(original_totals);
		cxxNameDouble::iterator it = new_activities.begin();
		for ( ; it != new_activities.end(); it++)
		{
			this->master_activity[it->first] = it->second;
		}
	}
	return;
}
#endif
void
cxxSolution::read_raw(CParser & parser, bool check)
{

	// Used if it is modify
	cxxNameDouble simple_original_totals = this->totals.Simplify_redox();
	cxxNameDouble original_activities(this->master_activity);

	this->master_activity.clear();

	std::istream::pos_type ptr;
	std::istream::pos_type next_char;
	std::string token;
	int opt_save;

	// Read solution number and description
	this->read_number_description(parser.line());

	opt_save = CParser::OPT_ERROR;
	bool tc_defined(false);
	bool ph_defined(false);
	bool pe_defined(false);
	bool mu_defined(false);
	bool ah2o_defined(false);
	bool total_h_defined(false);
	bool total_o_defined(false);
	bool cb_defined(false);
	bool mass_water_defined(false);
	bool total_alkalinity_defined(false);

	for (;;)
	{
		int opt = parser.get_option(vopts, next_char);
		if (opt == CParser::OPT_DEFAULT)
		{
			opt = opt_save;
		}

		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_EOF;
			parser.error_msg("Unknown input in SOLUTION_RAW keyword.",
							 PHRQ_io::OT_CONTINUE);
			parser.error_msg(parser.line().c_str(), PHRQ_io::OT_CONTINUE);
			continue;

		case 0:				// totals
			{
				cxxNameDouble temp_totals;
				if (temp_totals.read_raw(parser, next_char) !=	CParser::PARSER_OK)
				{
					parser.incr_input_error();
					parser.
						error_msg("Expected element name and moles for totals.",
						PHRQ_io::OT_CONTINUE);
				}
				else
				{
					this->totals.merge_redox(temp_totals);
				}
			}
			opt_save = 0;
			break;

		case 1:				// activities
			if (this->master_activity.read_raw(parser, next_char) !=
				CParser::PARSER_OK)
			{
				parser.incr_input_error();
				parser.
					error_msg
					("Expected species name and log activity for activities.",
					 PHRQ_io::OT_CONTINUE);
			}
			opt_save = 1;
			break;

		case 2:				// gammas
			if (this->species_gamma.read_raw(parser, next_char) !=
				CParser::PARSER_OK)
			{
				parser.incr_input_error();
				parser.
					error_msg
					("Expected species name and activity coefficient for gammas.",
					 PHRQ_io::OT_CONTINUE);
			}
			opt_save = 2;
			break;

		case 3:				// isotope
			{
				std::string name;
				if (!(parser.get_iss() >> name))
				{
					parser.incr_input_error();
					parser.error_msg("Expected character value for isotope name.",
									 PHRQ_io::OT_CONTINUE);
				}
				else
				{
					cxxSolutionIsotope iso(this->Get_io());
					iso.Set_isotope_name(name.c_str());
					iso.read_raw(parser, check);
					this->isotopes[name] = iso;
				}
			}
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 4:				// temp
		case 5:				// tc_avoid_conflict_with_technetium
		case 6:				// temperature                  
			if (!(parser.get_iss() >> this->tc))
			{
				this->tc = 25.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for temperature.",
								 PHRQ_io::OT_CONTINUE);
			}
			tc_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 7:				// ph
			if (!(parser.get_iss() >> this->ph))
			{
				this->ph = 7.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for pH.",
								 PHRQ_io::OT_CONTINUE);
			}
			ph_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 8:				// pe
			if (!(parser.get_iss() >> this->pe))
			{
				this->pe = 4.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for pe.",
								 PHRQ_io::OT_CONTINUE);
			}
			pe_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 9:				// mu
		case 10:				// ionic_strength
			if (!(parser.get_iss() >> this->mu))
			{
				this->mu = 1e-7;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for ionic strength.",
								 PHRQ_io::OT_CONTINUE);
			}
			mu_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 11:				// ah2o
		case 12:				// activity_water
			if (!(parser.get_iss() >> this->ah2o))
			{
				this->ah2o = 1.0;
				parser.incr_input_error();
				parser.
					error_msg("Expected numeric value for activity of water.",
							  PHRQ_io::OT_CONTINUE);
			}
			ah2o_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 13:				// total_h
			if (!(parser.get_iss() >> this->total_h))
			{
				this->total_h = 111.1;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for total hydrogen.",
								 PHRQ_io::OT_CONTINUE);
			}
			total_h_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 14:				// total_o
			if (!(parser.get_iss() >> this->total_o))
			{
				this->total_o = 55.55;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for total oxygen.",
								 PHRQ_io::OT_CONTINUE);
			}
			total_o_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 15:				// mass_water
		case 16:				// mass_h2o
			if (!(parser.get_iss() >> this->mass_water))
			{
				this->mass_water = 1.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for mass of water.",
								 PHRQ_io::OT_CONTINUE);
			}
			mass_water_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 17:				// total_alkalinity
		case 18:				// total_alk
			if (!(parser.get_iss() >> this->total_alkalinity))
			{
				this->total_alkalinity = 0;
				parser.incr_input_error();
				parser.
					error_msg("Expected numeric value for total_alkalinity.",
							  PHRQ_io::OT_CONTINUE);
			}
			total_alkalinity_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 19:				// cb
		case 20:				// charge_balance
			if (!(parser.get_iss() >> this->cb))
			{
				this->cb = 0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for charge balance.",
								 PHRQ_io::OT_CONTINUE);
			}
			cb_defined = true;
			opt_save = CParser::OPT_DEFAULT;
			break;
		case 21:				// density
			if (!(parser.get_iss() >> this->density))
			{
				this->density = 1.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for density.",
					PHRQ_io::OT_CONTINUE);
			}
			opt_save = CParser::OPT_DEFAULT;
			break;
		case 22:				// pressure
			if (!(parser.get_iss() >> this->patm))
			{
				this->patm = 1.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for pressure.",
					PHRQ_io::OT_CONTINUE);
			}
			opt_save = CParser::OPT_DEFAULT;
			break;

		case 23:				// soln_vol
			if (!(parser.get_iss() >> this->soln_vol))
			{
				this->soln_vol = 1.0;
				parser.incr_input_error();
				parser.error_msg("Expected numeric value for solution volume.",
								 PHRQ_io::OT_CONTINUE);
			}
			opt_save = CParser::OPT_DEFAULT;
			break;
		}
		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	if (check)
	{
		// all members must be defined
		if (tc_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Temp not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (ph_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("pH not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (pe_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("pe not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (mu_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Ionic strength not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (ah2o_defined == false)
		{
			parser.incr_input_error();
			parser.
				error_msg("Activity of water not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (total_h_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Total hydrogen not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (total_o_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Total oxygen not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (cb_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Charge balance not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (mass_water_defined == false)
		{
			parser.incr_input_error();
			parser.error_msg("Temp not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
		if (total_alkalinity_defined == false)
		{
			parser.incr_input_error();
			parser.
				error_msg("Total alkalinity not defined for SOLUTION_RAW input.",
				PHRQ_io::OT_CONTINUE);
		}
	}

	// Update activities
	if (original_activities.size() > 0)
	{
		
		cxxNameDouble simple_this_totals = this->totals.Simplify_redox();
		cxxNameDouble::iterator it = simple_original_totals.begin();
		for ( ; it != simple_original_totals.end(); it++)
		{
			cxxNameDouble::iterator jit = simple_this_totals.find(it->first);
			if (jit != simple_this_totals.end())
			{
				if (it->second > 0 && jit->second > 0.0)
				{
					LDBLE f = jit->second / it->second;
					if (f != 1)
					{
						original_activities.Multiply_activities_redox(it->first, f);
					}
				}
			}
		}
		original_activities.merge_redox(this->master_activity);
		this->master_activity = original_activities;
	}

	return;
}

void
cxxSolution::Update(LDBLE h_tot, LDBLE o_tot, LDBLE charge, const cxxNameDouble &const_nd)
{
	// H, O, charge, totals, and activities of solution are updated
	this->total_h = h_tot;
	this->total_o = o_tot;
	this->cb = charge;

	// Don`t bother to update activities?
	//this->Update(const_nd);
	this->totals = const_nd;
}
void
cxxSolution::Update_activities(const cxxNameDouble &original_tot)
{
	// Totals and activities of solution are updated
	// nd does not have H, O, charge
	cxxNameDouble simple_original = original_tot.Simplify_redox();
	// totals after read
	cxxNameDouble simple_new = this->totals.Simplify_redox();

	// make factors 
	cxxNameDouble factors;
	{
		cxxNameDouble::iterator it = simple_new.begin();
		cxxNameDouble::iterator jit = simple_original.begin();
		while (it != simple_new.end() && jit != simple_original.end())
		{
			int j = strcmp(it->first.c_str(), jit->first.c_str());
			if (j < 0)
			{
				it++;
			}
			else if (j == 0)
			{
				if (jit->second != it->second)
				{
					if (it->second > 0 && jit->second > 0)
					{
						factors[it->first] = log10(jit->second / it->second);
					}
				}
				it++;
				jit++;
			}
			else
			{
				jit++;
			}
		}


		// simple_new now has factors for master activities
		// Now add factors to activities
		{
			cxxNameDouble::iterator activity_it = this->master_activity.begin();
			cxxNameDouble::iterator factors_it = factors.begin();
			std::string activity_ename;
			std::basic_string < char >::size_type indexCh;
			while (activity_it != master_activity.end() && factors_it != factors.end())
			{
				activity_ename = activity_it->first;
				if (activity_ename.size() > 3)
				{
					indexCh = activity_ename.find("(");
					if (indexCh != std::string::npos)
					{
						activity_ename = activity_ename.substr(0, indexCh);
					}
				}
				int j = strcmp(factors_it->first.c_str(), activity_ename.c_str());
				if (j < 0)
				{
					factors_it++;
				}
				else if (j == 0)
				{
					activity_it->second += factors_it->second;
					activity_it++;
				}
				else 
				{
					activity_it++;
				}
			}
		}
	}
}
void
cxxSolution::Update(const cxxNameDouble &const_nd)
{
	// const_nd is a list of new totals, assumed to be inclusive of all elements
	// Totals and activities of solution are updated
	// nd does not have H, O, charge
	cxxNameDouble simple_original = this->totals.Simplify_redox();
	cxxNameDouble simple_new = const_nd.Simplify_redox();

	cxxNameDouble factors;
	{
		// make factors 
		cxxNameDouble::iterator it = simple_new.begin();
		cxxNameDouble::iterator jit = simple_original.begin();
		while (it != simple_new.end() && jit != simple_original.end())
		{
			int j = strcmp(it->first.c_str(), jit->first.c_str());
			if (j < 0)
			{
				it++;
			}
			else if (j == 0)
			{
				if (jit->second != it->second)
				{
					if (it->second > 0 && jit->second > 0)
					{
						factors[it->first] = log10(it->second / jit->second);
					}
				}
				it++;
				jit++;
			}
			else
			{
				jit++;
			}
		}
		// simple_new now has factors for master activities
		// Now add log factors to log activities
		{
			cxxNameDouble::iterator activity_it = this->master_activity.begin();
			cxxNameDouble::iterator factors_it = factors.begin();
			std::string activity_ename;
			std::basic_string < char >::size_type indexCh;
			while (activity_it != master_activity.end() && factors_it != factors.end())
			{
				activity_ename = activity_it->first;
				if (factors_it->first[0] < activity_ename[0])
				{
					factors_it++;
					continue;
				}
				else if (factors_it->first[0] > activity_ename[0])
				{
					activity_it++;
					continue;
				}
				if (activity_ename.size() > 3)
				{
					indexCh = activity_ename.find("(");
					if (indexCh != std::string::npos)
					{
						activity_ename = activity_ename.substr(0, indexCh);
					}
				}
				int j = strcmp(factors_it->first.c_str(), activity_ename.c_str());
				if (j < 0)
				{
					factors_it++;
				}
				else if (j == 0)
				{
					activity_it->second += factors_it->second;
					activity_it++;
				}
				else 
				{
					activity_it++;
				}
			}
		}
	}

	// update totals
	this->totals = simple_new;
}
#ifdef SKIP
void
cxxSolution::Update(const cxxNameDouble &const_nd)
{
	// const_nd is updated totals
	cxxNameDouble simple_original_totals = this->totals.Simplify_redox();
	cxxNameDouble original_activities(this->master_activity);

	this->master_activity.clear();

	// Update activities
	if (original_activities.size() > 0)
	{
		cxxNameDouble nd = const_nd;
		cxxNameDouble simple_this_totals = nd.Simplify_redox();
		cxxNameDouble::iterator it = simple_original_totals.begin();
		for ( ; it != simple_original_totals.end(); it++)
		{
			cxxNameDouble::iterator jit = simple_this_totals.find(it->first);
			if (jit != simple_this_totals.end())
			{
				if (it->second != 0)
				{
					LDBLE f = jit->second / it->second;
					if (f != 1)
					{
						original_activities.Multiply_activities_redox(it->first, f);
					}
				}
			}
		}
		original_activities.merge_redox(this->master_activity);
		this->master_activity = original_activities;
	}

	return;
}
#endif
void
cxxSolution::zero()
{
	this->tc = 0.0;
	this->ph = 0.0;
	this->pe = 0.0;
	this->mu = 0.0;
	this->ah2o = 0.0;
	this->total_h = 0.0;
	this->total_o = 0.0;
	this->cb = 0.0;
	this->density = 1.0;
	this->mass_water = 0.0;
	this->soln_vol = 0.0;
	this->total_alkalinity = 0.0;
	this->totals.type = cxxNameDouble::ND_ELT_MOLES;
	this->master_activity.type = cxxNameDouble::ND_SPECIES_LA;
	this->species_gamma.type = cxxNameDouble::ND_SPECIES_GAMMA;
	this->patm = 1.0;
	this->initial_data = NULL;
}

void
cxxSolution::add(const cxxSolution & addee, LDBLE extensive)
		//
		// Add existing solution to "this" solution
		//
{
	if (extensive == 0.0)
		return;
	LDBLE ext1 = this->mass_water;
	LDBLE ext2 = addee.mass_water * extensive;
	LDBLE f1 = ext1 / (ext1 + ext2);
	LDBLE f2 = ext2 / (ext1 + ext2);
	this->tc = f1 * this->tc + f2 * addee.tc;
	this->ph = f1 * this->ph + f2 * addee.ph;
	this->pe = f1 * this->pe + f2 * addee.pe;
	this->mu = f1 * this->mu + f2 * addee.mu;
	this->ah2o = f1 * this->mu + f2 * addee.ah2o;
	this->total_h += addee.total_h * extensive;
	this->total_o += addee.total_o * extensive;
	this->cb += addee.cb * extensive;
	this->density = f1 * this->density + f2 * addee.density;
	this->patm = f1 * this->patm + f2 * addee.patm;
	this->mass_water += addee.mass_water * extensive;
	this->soln_vol += addee.soln_vol * extensive;
	this->total_alkalinity += addee.total_alkalinity * extensive;
	this->totals.add_extensive(addee.totals, extensive);
	this->master_activity.add_log_activities(addee.master_activity, f1, f2);
	this->species_gamma.add_intensive(addee.species_gamma, f1, f2);
	this->Add_isotopes(addee.isotopes, f2, extensive);
}

void
cxxSolution::multiply(LDBLE extensive)
		//
		// Multiply existing solution by extensive
		//
{
	if (extensive == 0.0 || extensive == 1.0)
		return;
	this->total_h *= extensive;
	this->total_o *= extensive;
	this->cb *= extensive;
	this->mass_water *= extensive;
	this->soln_vol *= extensive;
	this->total_alkalinity *= extensive;
	this->totals.multiply(extensive);
	this->Multiply_isotopes(extensive);
}

LDBLE
cxxSolution::Get_total(const char *string) const
{
	cxxNameDouble::const_iterator it = this->totals.find(string);
	if (it == this->totals.end())
	{
		return (0.0);
	}
	else
	{
		return (it->second);
	}
}
#ifdef SKIP
LDBLE
cxxSolution::Get_total_element(const char *string) const
{
	cxxNameDouble::const_iterator it;
	LDBLE d = 0.0;
	for (it = this->totals.begin(); it != this->totals.end(); ++it)
	{
		// C++ way to do it
		std::string ename(string);
		std::string current_ename(it->first);
		std::basic_string < char >::size_type indexCh;
		indexCh = current_ename.find("(");
		if (indexCh != std::string::npos)
		{
			current_ename = current_ename.substr(0, indexCh);
		}
		if (current_ename == ename)
		{
			d += it->second;
		}
	}
	return (d);
}
#endif

void
cxxSolution::Set_total(char *string, LDBLE d)
{
	this->totals[string] = d;
}

LDBLE
cxxSolution::Get_master_activity(char *string) const
{
	cxxNameDouble::const_iterator it = this->master_activity.find(string);
	if (it == this->master_activity.end())
	{
		return (0.0);
	}
	else
	{
		return (it->second);
	}
}

void
cxxSolution::Set_master_activity(char *string, LDBLE d)
{
	this->master_activity[string] = d;
}
void
cxxSolution::Add_isotopes(const std::map < std::string, cxxSolutionIsotope > & old, LDBLE intensive, LDBLE extensive)
{
	for (std::map < std::string, cxxSolutionIsotope >::const_iterator itold = old.begin(); itold != old.end(); ++itold)
	{
		std::map < std::string, cxxSolutionIsotope >::iterator it_this;
		it_this = this->isotopes.find(itold->first);
		if (it_this != this->isotopes.end())
		{
			LDBLE t = it_this->second.Get_total();
			t += itold->second.Get_total() * extensive;
			it_this->second.Set_total(t);

			t = it_this->second.Get_ratio();
			t += itold->second.Get_ratio() * intensive;
			it_this->second.Set_ratio(t);

			t = it_this->second.Get_ratio_uncertainty();
			t += itold->second.Get_ratio_uncertainty() * intensive;
			it_this->second.Set_ratio_uncertainty(t);
			it_this->second.Set_ratio_uncertainty_defined(it_this->second.Get_ratio_uncertainty_defined()
												 || itold->second.Get_ratio_uncertainty_defined());
		}
		else
		{
			cxxSolutionIsotope iso(itold->second);
			iso.Set_total(itold->second.Get_total() * extensive);
			this->Get_isotopes()[iso.Get_isotope_name()] = iso;
		}
	}
}
void
cxxSolution::Multiply_isotopes(LDBLE extensive)
{
	std::map < std::string, cxxSolutionIsotope>::iterator it;
	for (it = this->isotopes.begin(); it != this->isotopes.end(); it++)
	{
		LDBLE total = it->second.Get_total();
		total *= extensive;
		it->second.Set_total(total);
	}
}
const std::vector< std::string >::value_type temp_vopts[] = {
	std::vector< std::string >::value_type("totals"),	                            // 0 
	std::vector< std::string >::value_type("activities"),	                        // 1 
	std::vector< std::string >::value_type("gammas"),	                            // 2 
	std::vector< std::string >::value_type("isotopes"),	                            // 3 
	std::vector< std::string >::value_type("temp"),	                                // 4 
	std::vector< std::string >::value_type("tc_avoid_conflict_with_technetium"),	// 5 
	std::vector< std::string >::value_type("temperature"),	                        // 6 
	std::vector< std::string >::value_type("ph"),	                                // 7 
	std::vector< std::string >::value_type("pe"),	                                // 8 
	std::vector< std::string >::value_type("mu"),	                                // 9 
	std::vector< std::string >::value_type("ionic_strength"),	                    // 10
	std::vector< std::string >::value_type("ah2o"),	                                // 11
	std::vector< std::string >::value_type("activity_water"),	                    // 12
	std::vector< std::string >::value_type("total_h"),	                            // 13
	std::vector< std::string >::value_type("total_o"),	                            // 14
	std::vector< std::string >::value_type("mass_water"),	                        // 15
	std::vector< std::string >::value_type("mass_h2o"),	                            // 16
	std::vector< std::string >::value_type("total_alkalinity"),	                    // 17
	std::vector< std::string >::value_type("total_alk"),	                        // 18
	std::vector< std::string >::value_type("cb"),	                                // 19
	std::vector< std::string >::value_type("charge_balance"),	                    // 20
	std::vector< std::string >::value_type("density"),	                            // 21
	std::vector< std::string >::value_type("pressure"),	                            // 22
	std::vector< std::string >::value_type("soln_vol") 	                            // 23
};									   
const std::vector< std::string > cxxSolution::vopts(temp_vopts, temp_vopts + sizeof temp_vopts / sizeof temp_vopts[0]);	