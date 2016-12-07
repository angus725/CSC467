#include <string>
#include "common.h"

/* Macro for outputing ARB assembly code, TODO: find better place to put this*/
#define OUTPUT_ARB(...) {fprintf(outputFile,  __VA_ARGS__);}

class Register {
public:
	Register(const std::string &reg_name, int reg_id, std::string has_Component);
	Register(const std::string &reg_name, int reg_id);
	Register(const Register *orig);
	Register(); // empty register for copy-constrcutor

	std::string name;
	int id; //for possible reclaiming of registers
	bool hasComponent;
	std::string component;
protected:

private:

};

class RegAllocator
{

public:
	//****** for use with syntax tree generation ******//
	RegAllocator();

	Register *getNewReg();
protected:
	int current_reg_id; //Right now it's a trivial implementation

private:

};
