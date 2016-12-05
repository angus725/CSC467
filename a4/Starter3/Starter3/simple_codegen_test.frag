{
	int x;
	float x1 = 4.0;
	bool y = false;
	vec3 z = vec3(1.0, 2.0, x1);
	vec3 w = vec3(1.0, z[1], 3.0);
        int var_int = 1;
        const int var_in2 = 2;
        float var_float = 2.5;
        bool var_bool = true;

        vec4    var_vec4;
        const ivec2 var_ivec2 = ivec2(2, 3);
        vec3 var_vec3 = vec3( 1.1, 2.2, 3.3);
        bvec4 var_bvec4 = bvec4(true, false, true, false);

        float func_return = dp3(var_vec3, var_vec3);
        float rsq_return = rsq(var_float);
        float rsq_return2 = rsq(var_int);
        vec4 lit_return = lit(var_vec4);

	
	x1 = z[2] / x1;
	x = -4;
	
}
