#include <caml/mlvalues.h>
#include <caml/callback.h>

int compile_stage;

unsigned char* compile(char* v, int* size)
{
	compile_stage = 1;
	value val = caml_callback(*caml_named_value("compile"), caml_copy_string(v));
	compile_stage = 0;
	*size = string_length(val);
	return String_val( val );
}
