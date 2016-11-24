{
	int var_int = 1;
	float var_float = 2.5;
	bool var_bool = true;
	
	ivec2 var_ivec2 = ivec2();
	vec3 var_vec3 = vec3( 1.1, 2.2, 3.3);
	bvec4 var_bvec4 = bvec4(true, false, true, false);

	vec3 var_mul_result;
	const int var_cnst_int = 1234;

	if (var_int < 5) {
		var_mul_result = var_float * var_vec3;
	} else if (var_bvec4[2]) {
		var_float = var_float * var_float;
	}
}
