#include <string>

class Register {
public:
	Register(const std::string &reg_name, int reg_id);
	std::string name;
	int id; //for possible reclaiming of registers
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
