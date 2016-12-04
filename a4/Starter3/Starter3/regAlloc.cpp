#include "regAlloc.h"
#include <sstream>


Register::Register(const std::string &reg_name, int reg_id) {
	this->name = reg_name;
	this->id = reg_id;
}

RegAllocator::RegAllocator() {
	this->current_reg_id = 0;
}

Register *RegAllocator::getNewReg() {
	std::ostringstream oss;
	oss << "TEMP" << this->current_reg_id;
	this->current_reg_id++;
	return new Register(oss.str(), this->current_reg_id);
}
