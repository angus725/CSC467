#include "regAlloc.h"
#include <sstream>


Register::Register()
{
	id = -1;
	hasComponent = false;
}

Register::Register(const std::string &reg_name, int reg_id) {
	this->name = reg_name;
	this->id = reg_id;
	this->hasComponent = false;
}

Register::Register(const Register *orig) {
	this->name = orig->name;
	this->id = orig->id;
	this->hasComponent = orig->hasComponent;
}

RegAllocator::RegAllocator() {
	this->current_reg_id = 0;
}

Register *RegAllocator::getNewReg() {
	std::ostringstream oss;
	Register *new_reg;

	oss << "R" << this->current_reg_id;
	new_reg = new Register(oss.str(), this->current_reg_id);
	oss.str("");
	this->current_reg_id++;

	oss << "TEMP " << new_reg->name << ";\n";

	OUTPUT_ARB("%s", oss.str().c_str())
	return new_reg;
}
