#include <caml/mlvalues.h>
#include <caml/callback.h>

unsigned char* compile(char* v, int* size)
{
	value val = caml_callback(*caml_named_value("compile"), caml_copy_string(v));
	*size = string_length(val);
	return String_val( val );
}
