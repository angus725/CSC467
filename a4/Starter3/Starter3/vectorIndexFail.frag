{
	int var_int = 1;
	int var_in2 = 2;	
float var_float = 2.5;
	bool var_bool = true;
	
	vec4	var_vec4;
	ivec2 var_ivec2 = ivec2(var_int, 2);
	vec3 var_vec3 = vec3( 1.1, 2.2, 3.3);
	bvec4 var_bvec4 = bvec4(true, false, true, false);
	
	float func_return = dp3(var_vec3, var_vec3);
	float rsq_return = rsq(var_float);
	float rsq_return2 = rsq(var_int);
	vec4 lit_return = lit(var_vec4);
	
	vec3 var_mul_result;
	/*const int var_cnst_int = 1234*/
	gl_FragDepth = true;
	
	var_int = var_ivec2[2];
	var_int = var_ivec2[3];

	if (var_int < 5) {
		var_mul_result = var_float * var_vec3;
	} else if (var_bvec4[2]) {
		var_float = var_float * var_float;
	}
}
