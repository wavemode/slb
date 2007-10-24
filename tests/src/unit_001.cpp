#include <SLB/SLB.hpp>
#include "unit_001.hpp"
#include <SLB/Iterator.hpp>

namespace Unit_001 {

	Element::Element(const std::string &label) : _label(label) {}

	const std::string& Element::label() const
	{
		return _label;
	}

	void Element::setLabel(const std::string &label)
	{
		_label = label;
	}

	void wrapper()
	{
		SLB_DEBUG(1, "--> Loading unit_001 wrapper -->");
		
		SLB::Class<Element>("Unit_001::Element")
			.constructor<const std::string&>()
			.set( "label", &Element::label )
			.set( "setLabel", &Element::setLabel)
		;
		
		typedef std::vector<Element> Vector;
		SLB::Class< Vector >("Unit_001::Vector")
			.constructor()
			.set("push_back", &Vector::push_back)
			.nonconst_set("get", &Vector::operator[] )
			.const_set("get_const", &Vector::operator[] )
			.iterator("iterator", &Vector::begin, &Vector::end )
			.const_iterator("const_iterator", &Vector::begin, &Vector::end )
		;


		SLB_DEBUG(1, "<-- Loading unit_001 wrapper <--");
	}


	SLB_ON_LOAD(wrapper)

}