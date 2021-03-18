/*
Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>

#ifdef __GNUC__
#pragma GCC visibility pop
#endif

#include "cjson.h"
#include "base_api.h"

/************************begin*****************************/
/*json格式检验接口相关*/
#define __   -1     /* the universal error code */

/*
    Characters are mapped into these 31 character classes. This allows for
    a significant reduction in the size of the state transition table.
*/

enum classes {
    C_SPACE,  /* space */
    C_WHITE,  /* other whitespace */
    C_LCURB,  /* {  */
    C_RCURB,  /* } */
    C_LSQRB,  /* [ */
    C_RSQRB,  /* ] */
    C_COLON,  /* : */
    C_COMMA,  /* , */
    C_QUOTE,  /* " */
    C_BACKS,  /* \ */
    C_SLASH,  /* / */
    C_PLUS,   /* + */
    C_MINUS,  /* - */
    C_POINT,  /* . */
    C_ZERO ,  /* 0 */
    C_DIGIT,  /* 123456789 */
    C_LOW_A,  /* a */
    C_LOW_B,  /* b */
    C_LOW_C,  /* c */
    C_LOW_D,  /* d */
    C_LOW_E,  /* e */
    C_LOW_F,  /* f */
    C_LOW_L,  /* l */
    C_LOW_N,  /* n */
    C_LOW_R,  /* r */
    C_LOW_S,  /* s */
    C_LOW_T,  /* t */
    C_LOW_U,  /* u */
    C_ABCDF,  /* ABCDF */
    C_E,      /* E */
    C_ETC,    /* everything else */
    NR_CLASSES
};

static int ascii_class[128] = {
/*
    This array maps the 128 ASCII characters into character classes.
    The remaining Unicode characters should be mapped to C_ETC.
    Non-whitespace control characters are errors.
*/
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      C_WHITE, C_WHITE, __,      __,      C_WHITE, __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,

    C_SPACE, C_ETC,   C_QUOTE, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_PLUS,  C_COMMA, C_MINUS, C_POINT, C_SLASH,
    C_ZERO,  C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
    C_DIGIT, C_DIGIT, C_COLON, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

    C_ETC,   C_ABCDF, C_ABCDF, C_ABCDF, C_ABCDF, C_E,     C_ABCDF, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LSQRB, C_BACKS, C_RSQRB, C_ETC,   C_ETC,

    C_ETC,   C_LOW_A, C_LOW_B, C_LOW_C, C_LOW_D, C_LOW_E, C_LOW_F, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_LOW_L, C_ETC,   C_LOW_N, C_ETC,
    C_ETC,   C_ETC,   C_LOW_R, C_LOW_S, C_LOW_T, C_LOW_U, C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LCURB, C_ETC,   C_RCURB, C_ETC,   C_ETC
};


/*
    The state codes.
*/
enum states {
    GO,  /* start    */
    OK,  /* ok       */
    OB,  /* object   */
    KE,  /* key      */
    CO,  /* colon    */
    VA,  /* value    */
    AR,  /* array    */
    ST,  /* string   */
    ES,  /* escape   */
    U1,  /* u1       */
    U2,  /* u2       */
    U3,  /* u3       */
    U4,  /* u4       */
    MI,  /* minus    */
    ZE,  /* zero     */
    IN,  /* integer  */
    FR,  /* fraction */
    E1,  /* e        */
    E2,  /* ex       */
    E3,  /* exp      */
    T1,  /* tr       */
    T2,  /* tru      */
    T3,  /* true     */
    F1,  /* fa       */
    F2,  /* fal      */
    F3,  /* fals     */
    F4,  /* false    */
    N1,  /* nu       */
    N2,  /* nul      */
    N3,  /* null     */
    NR_STATES
};


static int state_transition_table[NR_STATES][NR_CLASSES] = {
/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. An action is represented as a
    negative number. A JSON text is accepted if at the end of the text the
    state is OK and if the mode is MODE_DONE.

                 white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ {GO,GO,-6,__,-5,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ok     OK*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*object OB*/ {OB,OB,__,-9,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*key    KE*/ {KE,KE,__,__,__,__,__,__,ST,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*colon  CO*/ {CO,CO,__,__,__,__,-2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*value  VA*/ {VA,VA,-6,__,-5,__,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*array  AR*/ {AR,AR,-6,__,-5,-7,__,__,ST,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*string ST*/ {ST,__,ST,ST,ST,ST,ST,ST,-4,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST},
/*escape ES*/ {__,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__},
/*u1     U1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__},
/*u2     U2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__},
/*u3     U3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__},
/*u4     U4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__},
/*minus  MI*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*zero   ZE*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*int    IN*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*frac   FR*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,FR,FR,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*e      E1*/ {__,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ex     E2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*exp    E3*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*tr     T1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__},
/*tru    T2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__},
/*true   T3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*fa     F1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fal    F2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__},
/*fals   F3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__},
/*false  F4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*nu     N1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__},
/*nul    N2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__},
/*null   N3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__},
};


/*
    These modes can be pushed on the stack.
*/
enum modes {
    MODE_ARRAY, 
    MODE_DONE,  
    MODE_KEY,   
    MODE_OBJECT,
};
/***********************end******************************/


/* define our own boolean type */
#define true ((cJSON_bool)1)
#define false ((cJSON_bool)0)

typedef struct {
	const unsigned char *json;
	size_t position;
} error;
static error global_error = { NULL, 0 };

CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void)
{
	return (const char*)(global_error.json + global_error.position);
}

/* This is a safeguard to prevent copy-pasters from using incompatible C and header files */
#if (CJSON_VERSION_MAJOR != 1) || (CJSON_VERSION_MINOR != 5) || (CJSON_VERSION_PATCH != 3)
#error cJSON.h and cJSON.c have different versions. Make sure that both have the same.
#endif

CJSON_PUBLIC(const char*) cJSON_Version(void)
{
	static char version[15];
	sprintf(version, "%i.%i.%i", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH);

	return version;
}

/* Case insensitive string comparison, doesn't consider two NULL pointers equal though */
static int case_insensitive_strcmp(const unsigned char *string1, const unsigned char *string2)
{
	if ((string1 == NULL) || (string2 == NULL))
	{
		return 1;
	}

	if (string1 == string2)
	{
		return 0;
	}

	for (; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
	{
		if (*string1 == '\0')
		{
			return 0;
		}
	}

	return tolower(*string1) - tolower(*string2);
}

typedef struct internal_hooks
{
	void *(*allocate)(unsigned int size, const char *pfile, const char *pfun, int line);
	void(*deallocate)(void *pointer, const char *pfile, const char *pfun, int line);
	void *(*reallocate)(void *pointer, unsigned int size, const char *pfile, const char *pfun, int line);
} internal_hooks;

static internal_hooks global_hooks = { _base_malloc, _base_free, _base_realloc };

static unsigned char* cJSON_strdup(const unsigned char* string, const internal_hooks * const hooks)
{
	size_t length = 0;
	unsigned char *copy = NULL;

	if (string == NULL)
	{
		return NULL;
	}

	length = strlen((const char*)string) + sizeof("");
	if (!(copy = (unsigned char*)hooks->allocate(length, __FILE__, __FUNCTION__, __LINE__)))
	{
		return NULL;
	}
	memcpy(copy, string, length);

	return copy;
}

CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks)
{
	if (hooks == NULL)
	{
		/* Reset hooks */
		global_hooks.allocate = _base_malloc;
		global_hooks.deallocate = _base_free;
		global_hooks.reallocate = _base_realloc;
		return;
	}

	global_hooks.allocate = _base_malloc;
	if (hooks->malloc_fn != NULL)
	{
		global_hooks.allocate = hooks->malloc_fn;
	}

	global_hooks.deallocate = _base_free;
	if (hooks->free_fn != NULL)
	{
		global_hooks.deallocate = hooks->free_fn;
	}

	/* use realloc only if both free and malloc are used */
	global_hooks.reallocate = NULL;
	if ((global_hooks.allocate == _base_malloc) && (global_hooks.deallocate == _base_free))
	{
		global_hooks.reallocate = _base_realloc;
	}
}

/* Internal constructor. */
static cJSON *cJSON_New_Item(const internal_hooks * const hooks)
{
	cJSON* node = (cJSON*)hooks->allocate(sizeof(cJSON), __FILE__, __FUNCTION__, __LINE__);
	if (node)
	{
		memset(node, '\0', sizeof(cJSON));
	}

	return node;
}

/* Delete a cJSON structure. */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item)
{
	cJSON *next = NULL;
	while (item != NULL)
	{
		next = item->next;
		if (!(item->type & cJSON_IsReference) && (item->child != NULL))
		{
			cJSON_Delete(item->child);
		}
		if (!(item->type & cJSON_IsReference) && (item->valuestring != NULL))
		{
			global_hooks.deallocate(item->valuestring, __FILE__, __FUNCTION__, __LINE__);
		}
		if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
		{
			global_hooks.deallocate(item->string, __FILE__, __FUNCTION__, __LINE__);
		}
		global_hooks.deallocate(item, __FILE__, __FUNCTION__, __LINE__);
		item = next;
	}
}

/* get the decimal point character of the current locale */
static unsigned char get_decimal_point(void)
{
	struct lconv *lconv = localeconv();
	return (unsigned char)lconv->decimal_point[0];
}

typedef struct
{
	const unsigned char *content;
	size_t length;
	size_t offset;
	size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
	internal_hooks hooks;
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
#define cannot_read(buffer, size) (!can_read(buffer, size))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
static cJSON_bool parse_number(cJSON * const item, parse_buffer * const input_buffer)
{
	double number = 0;
	unsigned char *after_end = NULL;
	unsigned char number_c_string[64];
	unsigned char decimal_point = get_decimal_point();
	size_t i = 0;

	if ((input_buffer == NULL) || (input_buffer->content == NULL))
	{
		return false;
	}

	/* copy the number into a temporary buffer and replace '.' with the decimal point
	* of the current locale (for strtod)
	* This also takes care of '\0' not necessarily being available for marking the end of the input */
	for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
	{
		switch (buffer_at_offset(input_buffer)[i])
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '+':
		case '-':
		case 'e':
		case 'E':
			number_c_string[i] = buffer_at_offset(input_buffer)[i];
			break;

		case '.':
			number_c_string[i] = decimal_point;
			break;

		default:
			goto loop_end;
		}
	}
loop_end:
	number_c_string[i] = '\0';

	number = strtod((const char*)number_c_string, (char**)&after_end);
	if (number_c_string == after_end)
	{
		return false; /* parse_error */
	}

	item->valuedouble = number;

	/* use saturation in case of overflow */
	if (number >= INT_MAX)
	{
		item->valueint = INT_MAX;
	}
	else if (number <= INT_MIN)
	{
		item->valueint = INT_MIN;
	}
	else
	{
		item->valueint = (int)number;
	}

	item->type = cJSON_Number;

	input_buffer->offset += (size_t)(after_end - number_c_string);
	return true;
}

/* don't ask me, but the original cJSON_SetNumberValue returns an integer or double */
CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON *object, double number)
{
	if (number >= INT_MAX)
	{
		object->valueint = INT_MAX;
	}
	else if (number <= INT_MIN)
	{
		object->valueint = INT_MIN;
	}
	else
	{
		object->valueint = (int)number;
	}

	return object->valuedouble = number;
}

typedef struct
{
	unsigned char *buffer;
	size_t length;
	size_t offset;
	size_t depth; /* current nesting depth (for formatted printing) */
	cJSON_bool noalloc;
	cJSON_bool format; /* is this print a formatted print */
	internal_hooks hooks;
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static unsigned char* ensure(printbuffer * const p, size_t needed)
{
	unsigned char *newbuffer = NULL;
	size_t newsize = 0;

	if ((p == NULL) || (p->buffer == NULL))
	{
		return NULL;
	}

	if ((p->length > 0) && (p->offset >= p->length))
	{
		/* make sure that offset is valid */
		return NULL;
	}

	if (needed > INT_MAX)
	{
		/* sizes bigger than INT_MAX are currently not supported */
		return NULL;
	}

	needed += p->offset + 1;
	if (needed <= p->length)
	{
		return p->buffer + p->offset;
	}

	if (p->noalloc) {
		return NULL;
	}

	/* calculate new buffer size */
	if (needed > (INT_MAX / 2))
	{
		/* overflow of int, use INT_MAX if possible */
		if (needed <= INT_MAX)
		{
			newsize = INT_MAX;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		newsize = needed * 2;
	}

	if (p->hooks.reallocate != NULL)
	{
		/* reallocate with realloc if available */
		newbuffer = (unsigned char*)p->hooks.reallocate(p->buffer, newsize, __FILE__, __FUNCTION__, __LINE__);
	}
	else
	{
		/* otherwise reallocate manually */
		newbuffer = (unsigned char*)p->hooks.allocate(newsize, __FILE__, __FUNCTION__, __LINE__);
		if (!newbuffer)
		{
			p->hooks.deallocate(p->buffer, __FILE__, __FUNCTION__, __LINE__);
			p->length = 0;
			p->buffer = NULL;

			return NULL;
		}
		if (newbuffer)
		{
			memcpy(newbuffer, p->buffer, p->offset + 1);
		}
		p->hooks.deallocate(p->buffer, __FILE__, __FUNCTION__, __LINE__);
	}
	p->length = newsize;
	p->buffer = newbuffer;

	return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer and update the offset */
static void update_offset(printbuffer * const buffer)
{
	const unsigned char *buffer_pointer = NULL;
	if ((buffer == NULL) || (buffer->buffer == NULL))
	{
		return;
	}
	buffer_pointer = buffer->buffer + buffer->offset;

	buffer->offset += strlen((const char*)buffer_pointer);
}

/* Render the number nicely from the given item into a string. */
static cJSON_bool print_number(const cJSON * const item, printbuffer * const output_buffer)
{
	unsigned char *output_pointer = NULL;
	double d = item->valuedouble;
	int length = 0;
	size_t i = 0;
	unsigned char number_buffer[26]; /* temporary buffer to print the number into */
	unsigned char decimal_point = get_decimal_point();
	double test;

	if (output_buffer == NULL)
	{
		return false;
	}

	/* This checks for NaN and Infinity */
	if ((d * 0) != 0)
	{
		length = sprintf((char*)number_buffer, "null");
	}
	else
	{
		/* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
		length = sprintf((char*)number_buffer, "%1.15g", d);

		/* Check whether the original double can be recovered */
		if ((sscanf((char*)number_buffer, "%lg", &test) != 1) || ((double)test != d))
		{
			/* If not, print with 17 decimal places of precision */
			length = sprintf((char*)number_buffer, "%1.17g", d);
		}
	}

	/* sprintf failed or buffer overrun occured */
	if ((length < 0) || (length >(int)(sizeof(number_buffer) - 1)))
	{
		return false;
	}

	/* reserve appropriate space in the output */
	output_pointer = ensure(output_buffer, (size_t)length);
	if (output_pointer == NULL)
	{
		return false;
	}

	/* copy the printed number to the output and replace locale
	* dependent decimal point with '.' */
	for (i = 0; i < ((size_t)length); i++)
	{
		if (number_buffer[i] == decimal_point)
		{
			output_pointer[i] = '.';
			continue;
		}

		output_pointer[i] = number_buffer[i];
	}
	output_pointer[i] = '\0';

	output_buffer->offset += (size_t)length;

	return true;
}

/* parse 4 digit hexadecimal number */
static unsigned parse_hex4(const unsigned char * const input)
{
	unsigned int h = 0;
	size_t i = 0;

	for (i = 0; i < 4; i++)
	{
		/* parse digit */
		if ((input[i] >= '0') && (input[i] <= '9'))
		{
			h += (unsigned int)input[i] - '0';
		}
		else if ((input[i] >= 'A') && (input[i] <= 'F'))
		{
			h += (unsigned int)10 + input[i] - 'A';
		}
		else if ((input[i] >= 'a') && (input[i] <= 'f'))
		{
			h += (unsigned int)10 + input[i] - 'a';
		}
		else /* invalid */
		{
			return 0;
		}

		if (i < 3)
		{
			/* shift left to make place for the next nibble */
			h = h << 4;
		}
	}

	return h;
}

/* converts a UTF-16 literal to UTF-8
* A literal can be one or two sequences of the form \uXXXX */
static unsigned char utf16_literal_to_utf8(const unsigned char * const input_pointer, const unsigned char * const input_end, unsigned char **output_pointer)
{
	long unsigned int codepoint = 0;
	unsigned int first_code = 0;
	const unsigned char *first_sequence = input_pointer;
	unsigned char utf8_length = 0;
	unsigned char utf8_position = 0;
	unsigned char sequence_length = 0;
	unsigned char first_byte_mark = 0;

	if ((input_end - first_sequence) < 6)
	{
		/* input ends unexpectedly */
		goto fail;
	}

	/* get the first utf16 sequence */
	first_code = parse_hex4(first_sequence + 2);

	/* check that the code is valid */
	if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
	{
		goto fail;
	}

	/* UTF16 surrogate pair */
	if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
	{
		const unsigned char *second_sequence = first_sequence + 6;
		unsigned int second_code = 0;
		sequence_length = 12; /* \uXXXX\uXXXX */

		if ((input_end - second_sequence) < 6)
		{
			/* input ends unexpectedly */
			goto fail;
		}

		if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
		{
			/* missing second half of the surrogate pair */
			goto fail;
		}

		/* get the second utf16 sequence */
		second_code = parse_hex4(second_sequence + 2);
		/* check that the code is valid */
		if ((second_code < 0xDC00) || (second_code > 0xDFFF))
		{
			/* invalid second half of the surrogate pair */
			goto fail;
		}


		/* calculate the unicode codepoint from the surrogate pair */
		codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
	}
	else
	{
		sequence_length = 6; /* \uXXXX */
		codepoint = first_code;
	}

	/* encode as UTF-8
	* takes at maximum 4 bytes to encode:
	* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
	if (codepoint < 0x80)
	{
		/* normal ascii, encoding 0xxxxxxx */
		utf8_length = 1;
	}
	else if (codepoint < 0x800)
	{
		/* two bytes, encoding 110xxxxx 10xxxxxx */
		utf8_length = 2;
		first_byte_mark = 0xC0; /* 11000000 */
	}
	else if (codepoint < 0x10000)
	{
		/* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
		utf8_length = 3;
		first_byte_mark = 0xE0; /* 11100000 */
	}
	else if (codepoint <= 0x10FFFF)
	{
		/* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
		utf8_length = 4;
		first_byte_mark = 0xF0; /* 11110000 */
	}
	else
	{
		/* invalid unicode codepoint */
		goto fail;
	}

	/* encode as utf8 */
	for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
	{
		/* 10xxxxxx */
		(*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
		codepoint >>= 6;
	}
	/* encode first byte */
	if (utf8_length > 1)
	{
		(*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
	}
	else
	{
		(*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
	}

	*output_pointer += utf8_length;

	return sequence_length;

fail:
	return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static cJSON_bool parse_string(cJSON * const item, parse_buffer * const input_buffer)
{
	const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
	const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
	unsigned char *output_pointer = NULL;
	unsigned char *output = NULL;

	/* not a string */
	if (buffer_at_offset(input_buffer)[0] != '\"')
	{
		goto fail;
	}

	{
		/* calculate approximate size of the output (overestimate) */
		size_t allocation_length = 0;
		size_t skipped_bytes = 0;
		while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
		{
			/* is escape sequence */
			if (input_end[0] == '\\')
			{
				if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
				{
					/* prevent buffer overflow when last input character is a backslash */
					goto fail;
				}
				skipped_bytes++;
				input_end++;
			}
			input_end++;
		}
		if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
		{
			goto fail; /* string ended unexpectedly */
		}

		/* This is at most how much we need for the output */
		allocation_length = (size_t)(input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
		output = (unsigned char*)input_buffer->hooks.allocate(allocation_length + sizeof(""), __FILE__, __FUNCTION__, __LINE__);
		if (output == NULL)
		{
			goto fail; /* allocation failure */
		}
	}

	output_pointer = output;
	/* loop through the string literal */
	while (input_pointer < input_end)
	{
		if (*input_pointer != '\\')
		{
			*output_pointer++ = *input_pointer++;
		}
		/* escape sequence */
		else
		{
			unsigned char sequence_length = 2;
			if ((input_end - input_pointer) < 1)
			{
				goto fail;
			}

			switch (input_pointer[1])
			{
			case 'b':
				*output_pointer++ = '\b';
				break;
			case 'f':
				*output_pointer++ = '\f';
				break;
			case 'n':
				*output_pointer++ = '\n';
				break;
			case 'r':
				*output_pointer++ = '\r';
				break;
			case 't':
				*output_pointer++ = '\t';
				break;
			case '\"':
			case '\\':
			case '/':
				*output_pointer++ = input_pointer[1];
				break;

				/* UTF-16 literal */
			case 'u':
				sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
				if (sequence_length == 0)
				{
					/* failed to convert UTF16-literal to UTF-8 */
					goto fail;
				}
				break;

			default:
				goto fail;
			}
			input_pointer += sequence_length;
		}
	}

	/* zero terminate the output */
	*output_pointer = '\0';

	item->type = cJSON_String;
	item->valuestring = (char*)output;

	input_buffer->offset = (size_t)(input_end - input_buffer->content);
	input_buffer->offset++;

	return true;

fail:
	if (output != NULL)
	{
		input_buffer->hooks.deallocate(output, __FILE__, __FUNCTION__, __LINE__);
	}

	if (input_pointer != NULL)
	{
		input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
	}

	return false;
}

/* Render the cstring provided to an escaped version that can be printed. */
static cJSON_bool print_string_ptr(const unsigned char * const input, printbuffer * const output_buffer)
{
	const unsigned char *input_pointer = NULL;
	unsigned char *output = NULL;
	unsigned char *output_pointer = NULL;
	size_t output_length = 0;
	/* numbers of additional characters needed for escaping */
	size_t escape_characters = 0;

	if (output_buffer == NULL)
	{
		return false;
	}

	/* empty string */
	if (input == NULL)
	{
		output = ensure(output_buffer, sizeof("\"\""));
		if (output == NULL)
		{
			return false;
		}
		strcpy((char*)output, "\"\"");

		return true;
	}

	/* set "flag" to 1 if something needs to be escaped */
	for (input_pointer = input; *input_pointer; input_pointer++)
	{
		switch (*input_pointer)
		{
		case '\"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			/* one character escape sequence */
			escape_characters++;
			break;
		default:
			if (*input_pointer < 32)
			{
				/* UTF-16 escape sequence uXXXX */
				escape_characters += 5;
			}
			break;
		}
	}
	output_length = (size_t)(input_pointer - input) + escape_characters;

	output = ensure(output_buffer, output_length + sizeof("\"\""));
	if (output == NULL)
	{
		return false;
	}

	/* no characters have to be escaped */
	if (escape_characters == 0)
	{
		output[0] = '\"';
		memcpy(output + 1, input, output_length);
		output[output_length + 1] = '\"';
		output[output_length + 2] = '\0';

		return true;
	}

	output[0] = '\"';
	output_pointer = output + 1;
	/* copy the string */
	for (input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
	{
		if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
		{
			/* normal character, copy */
			*output_pointer = *input_pointer;
		}
		else
		{
			/* character needs to be escaped */
			*output_pointer++ = '\\';
			switch (*input_pointer)
			{
			case '\\':
				*output_pointer = '\\';
				break;
			case '\"':
				*output_pointer = '\"';
				break;
			case '\b':
				*output_pointer = 'b';
				break;
			case '\f':
				*output_pointer = 'f';
				break;
			case '\n':
				*output_pointer = 'n';
				break;
			case '\r':
				*output_pointer = 'r';
				break;
			case '\t':
				*output_pointer = 't';
				break;
			default:
				/* escape and print as unicode codepoint */
				sprintf((char*)output_pointer, "u%04x", *input_pointer);
				output_pointer += 4;
				break;
			}
		}
	}
	output[output_length + 1] = '\"';
	output[output_length + 2] = '\0';

	return true;
}

/* Invoke print_string_ptr (which is useful) on an item. */
static cJSON_bool print_string(const cJSON * const item, printbuffer * const p)
{
	return print_string_ptr((unsigned char*)item->valuestring, p);
}

/* Predeclare these prototypes. */
static cJSON_bool parse_value(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_value(const cJSON * const item, printbuffer * const output_buffer);
static cJSON_bool parse_array(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer);
static cJSON_bool parse_object(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_object(const cJSON * const item, printbuffer * const output_buffer);

/* Utility to jump whitespace and cr/lf */
static parse_buffer *buffer_skip_whitespace(parse_buffer * const buffer)
{
	if ((buffer == NULL) || (buffer->content == NULL))
	{
		return NULL;
	}

	while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
	{
		buffer->offset++;
	}

	if (buffer->offset == buffer->length)
	{
		buffer->offset--;
	}

	return buffer;
}

/* Parse an object - create a new root, and populate. */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated)
{
	parse_buffer buffer = { 0, 0, 0, 0,{ 0, 0, 0 } };
	cJSON *item = NULL;

	/* reset error position */
	global_error.json = NULL;
	global_error.position = 0;

	if (value == NULL)
	{
		goto fail;
	}

	buffer.content = (const unsigned char*)value;
	buffer.length = strlen((const char*)value) + sizeof("");
	buffer.offset = 0;
	buffer.hooks = global_hooks;

	item = cJSON_New_Item(&global_hooks);
	if (item == NULL) /* memory fail */
	{
		goto fail;
	}

	if (!parse_value(item, buffer_skip_whitespace(&buffer)))
	{
		/* parse failure. ep is set. */
		goto fail;
	}

	/* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
	if (require_null_terminated)
	{
		buffer_skip_whitespace(&buffer);
		if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
		{
			goto fail;
		}
	}
	if (return_parse_end)
	{
		*return_parse_end = (const char*)buffer_at_offset(&buffer);
	}

	return item;

fail:
	if (item != NULL)
	{
		cJSON_Delete(item);
	}

	if (value != NULL)
	{
		error local_error;
		local_error.json = (const unsigned char*)value;
		local_error.position = 0;

		if (buffer.offset < buffer.length)
		{
			local_error.position = buffer.offset;
		}
		else if (buffer.length > 0)
		{
			local_error.position = buffer.length - 1;
		}

		if (return_parse_end != NULL)
		{
			*return_parse_end = (const char*)local_error.json + local_error.position;
		}
		else
		{
			global_error = local_error;
		}
	}

	return NULL;
}

/* Default options for cJSON_Parse */
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value)
{
	return cJSON_ParseWithOpts(value, 0, 0);
}

#define cjson_min(a, b) ((a < b) ? a : b)

static unsigned char *print(const cJSON * const item, cJSON_bool format, const internal_hooks * const hooks)
{
	printbuffer buffer[1];
	unsigned char *printed = NULL;

	memset(buffer, 0, sizeof(buffer));

	/* create buffer */
	buffer->buffer = (unsigned char*)hooks->allocate(256, __FILE__, __FUNCTION__, __LINE__);
	buffer->format = format;
	buffer->hooks = *hooks;
	if (buffer->buffer == NULL)
	{
		goto fail;
	}

	/* print the value */
	if (!print_value(item, buffer))
	{
		goto fail;
	}
	update_offset(buffer);

	/* check if reallocate is available */
	if (hooks->reallocate != NULL)
	{
		printed = (unsigned char*)hooks->reallocate(buffer->buffer, buffer->length, __FILE__, __FUNCTION__, __LINE__);
		buffer->buffer = NULL;
		if (printed == NULL) {
			goto fail;
		}
	}
	else /* otherwise copy the JSON over to a new buffer */
	{
		printed = (unsigned char*)hooks->allocate(buffer->offset + 1, __FILE__, __FUNCTION__, __LINE__);
		if (printed == NULL)
		{
			goto fail;
		}
		memcpy(printed, buffer->buffer, cjson_min(buffer->length, buffer->offset + 1));
		printed[buffer->offset] = '\0'; /* just to be sure */

										/* free the buffer */
		hooks->deallocate(buffer->buffer, __FILE__, __FUNCTION__, __LINE__);
	}

	return printed;

fail:
	if (buffer->buffer != NULL)
	{
		hooks->deallocate(buffer->buffer, __FILE__, __FUNCTION__, __LINE__);
	}

	if (printed != NULL)
	{
		hooks->deallocate(printed, __FILE__, __FUNCTION__, __LINE__);
	}

	return NULL;
}

/* Render a cJSON item/entity/structure to text. */
CJSON_PUBLIC(char *) cJSON_Print(const cJSON *item)
{
	return (char*)print(item, true, &global_hooks);
}

CJSON_PUBLIC(char *) cJSON_PrintUnformatted(const cJSON *item)
{
	return (char*)print(item, false, &global_hooks);
}

CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt)
{
	printbuffer p = { 0, 0, 0, 0, 0, 0,{ 0, 0, 0 } };

	if (prebuffer < 0)
	{
		return NULL;
	}

	p.buffer = (unsigned char*)global_hooks.allocate((size_t)prebuffer, __FILE__, __FUNCTION__, __LINE__);
	if (!p.buffer)
	{
		return NULL;
	}

	p.length = (size_t)prebuffer;
	p.offset = 0;
	p.noalloc = false;
	p.format = fmt;
	p.hooks = global_hooks;

	if (!print_value(item, &p))
	{
		return NULL;
	}

	return (char*)p.buffer;
}

CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buf, const int len, const cJSON_bool fmt)
{
	printbuffer p = { 0, 0, 0, 0, 0, 0,{ 0, 0, 0 } };

	if (len < 0)
	{
		return false;
	}

	p.buffer = (unsigned char*)buf;
	p.length = (size_t)len;
	p.offset = 0;
	p.noalloc = true;
	p.format = fmt;
	p.hooks = global_hooks;

	return print_value(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
static cJSON_bool parse_value(cJSON * const item, parse_buffer * const input_buffer)
{
	if ((input_buffer == NULL) || (input_buffer->content == NULL))
	{
		return false; /* no input */
	}

	/* parse the different types of values */
	/* null */
	if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
	{
		item->type = cJSON_NULL;
		input_buffer->offset += 4;
		return true;
	}
	/* false */
	if (can_read(input_buffer, 5) && (strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
	{
		item->type = cJSON_False;
		input_buffer->offset += 5;
		return true;
	}
	/* true */
	if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
	{
		item->type = cJSON_True;
		item->valueint = 1;
		input_buffer->offset += 4;
		return true;
	}
	/* string */
	if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
	{
		return parse_string(item, input_buffer);
	}
	/* number */
	if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
	{
		return parse_number(item, input_buffer);
	}
	/* array */
	if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
	{
		return parse_array(item, input_buffer);
	}
	/* object */
	if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
	{
		return parse_object(item, input_buffer);
	}


	return false;
}

/* Render a value to text. */
static cJSON_bool print_value(const cJSON * const item, printbuffer * const output_buffer)
{
	unsigned char *output = NULL;

	if ((item == NULL) || (output_buffer == NULL))
	{
		return false;
	}

	switch ((item->type) & 0xFF)
	{
	case cJSON_NULL:
		output = ensure(output_buffer, 5);
		if (output == NULL)
		{
			return false;
		}
		strcpy((char*)output, "null");
		return true;

	case cJSON_False:
		output = ensure(output_buffer, 6);
		if (output == NULL)
		{
			return false;
		}
		strcpy((char*)output, "false");
		return true;

	case cJSON_True:
		output = ensure(output_buffer, 5);
		if (output == NULL)
		{
			return false;
		}
		strcpy((char*)output, "true");
		return true;

	case cJSON_Number:
		return print_number(item, output_buffer);

	case cJSON_Raw:
	{
		size_t raw_length = 0;
		if (item->valuestring == NULL)
		{
			if (!output_buffer->noalloc)
			{
				output_buffer->hooks.deallocate(output_buffer->buffer, __FILE__, __FUNCTION__, __LINE__);
			}
			return false;
		}

		raw_length = strlen(item->valuestring) + sizeof("");
		output = ensure(output_buffer, raw_length);
		if (output == NULL)
		{
			return false;
		}
		memcpy(output, item->valuestring, raw_length);
		return true;
	}

	case cJSON_String:
		return print_string(item, output_buffer);

	case cJSON_Array:
		return print_array(item, output_buffer);

	case cJSON_Object:
		return print_object(item, output_buffer);

	default:
		return false;
	}
}

/* Build an array from input text. */
static cJSON_bool parse_array(cJSON * const item, parse_buffer * const input_buffer)
{
	cJSON *head = NULL; /* head of the linked list */
	cJSON *current_item = NULL;

	if (input_buffer->depth >= CJSON_NESTING_LIMIT)
	{
		return false; /* to deeply nested */
	}
	input_buffer->depth++;

	if (buffer_at_offset(input_buffer)[0] != '[')
	{
		/* not an array */
		goto fail;
	}

	input_buffer->offset++;
	buffer_skip_whitespace(input_buffer);
	if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
	{
		/* empty array */
		goto success;
	}

	/* check if we skipped to the end of the buffer */
	if (cannot_access_at_index(input_buffer, 0))
	{
		input_buffer->offset--;
		goto fail;
	}

	/* step back to character in front of the first element */
	input_buffer->offset--;
	/* loop through the comma separated array elements */
	do
	{
		/* allocate next item */
		cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
		if (new_item == NULL)
		{
			goto fail; /* allocation failure */
		}

		/* attach next item to list */
		if (head == NULL)
		{
			/* start the linked list */
			current_item = head = new_item;
		}
		else
		{
			/* add to the end and advance */
			current_item->next = new_item;
			new_item->prev = current_item;
			current_item = new_item;
		}

		/* parse next value */
		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if (!parse_value(current_item, input_buffer))
		{
			goto fail; /* failed to parse value */
		}
		buffer_skip_whitespace(input_buffer);
	} while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

	if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
	{
		goto fail; /* expected end of array */
	}

success:
	input_buffer->depth--;

	item->type = cJSON_Array;
	item->child = head;

	input_buffer->offset++;

	return true;

fail:
	if (head != NULL)
	{
		cJSON_Delete(head);
	}

	return false;
}

/* Render an array to text */
static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer)
{
	unsigned char *output_pointer = NULL;
	size_t length = 0;
	cJSON *current_element = item->child;

	if (output_buffer == NULL)
	{
		return false;
	}

	/* Compose the output array. */
	/* opening square bracket */
	output_pointer = ensure(output_buffer, 1);
	if (output_pointer == NULL)
	{
		return false;
	}

	*output_pointer = '[';
	output_buffer->offset++;
	output_buffer->depth++;

	while (current_element != NULL)
	{
		if (!print_value(current_element, output_buffer))
		{
			return false;
		}
		update_offset(output_buffer);
		if (current_element->next)
		{
			length = (size_t)(output_buffer->format ? 2 : 1);
			output_pointer = ensure(output_buffer, length + 1);
			if (output_pointer == NULL)
			{
				return false;
			}
			*output_pointer++ = ',';
			if (output_buffer->format)
			{
				*output_pointer++ = ' ';
			}
			*output_pointer = '\0';
			output_buffer->offset += length;
		}
		current_element = current_element->next;
	}

	output_pointer = ensure(output_buffer, 2);
	if (output_pointer == NULL)
	{
		return false;
	}
	*output_pointer++ = ']';
	*output_pointer = '\0';
	output_buffer->depth--;

	return true;
}

/* Build an object from the text. */
static cJSON_bool parse_object(cJSON * const item, parse_buffer * const input_buffer)
{
	cJSON *head = NULL; /* linked list head */
	cJSON *current_item = NULL;

	if (input_buffer->depth >= CJSON_NESTING_LIMIT)
	{
		return false; /* to deeply nested */
	}
	input_buffer->depth++;

	if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
	{
		goto fail; /* not an object */
	}

	input_buffer->offset++;
	buffer_skip_whitespace(input_buffer);
	if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
	{
		goto success; /* empty object */
	}

	/* check if we skipped to the end of the buffer */
	if (cannot_access_at_index(input_buffer, 0))
	{
		input_buffer->offset--;
		goto fail;
	}

	/* step back to character in front of the first element */
	input_buffer->offset--;
	/* loop through the comma separated array elements */
	do
	{
		/* allocate next item */
		cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
		if (new_item == NULL)
		{
			goto fail; /* allocation failure */
		}

		/* attach next item to list */
		if (head == NULL)
		{
			/* start the linked list */
			current_item = head = new_item;
		}
		else
		{
			/* add to the end and advance */
			current_item->next = new_item;
			new_item->prev = current_item;
			current_item = new_item;
		}

		/* parse the name of the child */
		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if (!parse_string(current_item, input_buffer))
		{
			goto fail; /* faile to parse name */
		}
		buffer_skip_whitespace(input_buffer);

		/* swap valuestring and string, because we parsed the name */
		current_item->string = current_item->valuestring;
		current_item->valuestring = NULL;

		if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
		{
			goto fail; /* invalid object */
		}

		/* parse the value */
		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if (!parse_value(current_item, input_buffer))
		{
			goto fail; /* failed to parse value */
		}
		buffer_skip_whitespace(input_buffer);
	} while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

	if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
	{
		goto fail; /* expected end of object */
	}

success:
	input_buffer->depth--;

	item->type = cJSON_Object;
	item->child = head;

	input_buffer->offset++;
	return true;

fail:
	if (head != NULL)
	{
		cJSON_Delete(head);
	}

	return false;
}

/* Render an object to text. */
static cJSON_bool print_object(const cJSON * const item, printbuffer * const output_buffer)
{
	unsigned char *output_pointer = NULL;
	size_t length = 0;
	cJSON *current_item = item->child;

	if (output_buffer == NULL)
	{
		return false;
	}

	/* Compose the output: */
	length = (size_t)(output_buffer->format ? 2 : 1); /* fmt: {\n */
	output_pointer = ensure(output_buffer, length + 1);
	if (output_pointer == NULL)
	{
		return false;
	}

	*output_pointer++ = '{';
	output_buffer->depth++;
	if (output_buffer->format)
	{
		*output_pointer++ = '\n';
	}
	output_buffer->offset += length;

	while (current_item)
	{
		if (output_buffer->format)
		{
			size_t i;
			output_pointer = ensure(output_buffer, output_buffer->depth);
			if (output_pointer == NULL)
			{
				return false;
			}
			for (i = 0; i < output_buffer->depth; i++)
			{
				*output_pointer++ = '\t';
			}
			output_buffer->offset += output_buffer->depth;
		}

		/* print key */
		if (!print_string_ptr((unsigned char*)current_item->string, output_buffer))
		{
			return false;
		}
		update_offset(output_buffer);

		length = (size_t)(output_buffer->format ? 2 : 1);
		output_pointer = ensure(output_buffer, length);
		if (output_pointer == NULL)
		{
			return false;
		}
		*output_pointer++ = ':';
		if (output_buffer->format)
		{
			*output_pointer++ = '\t';
		}
		output_buffer->offset += length;

		/* print value */
		if (!print_value(current_item, output_buffer))
		{
			return false;
		}
		update_offset(output_buffer);

		/* print comma if not last */
		length = (size_t)((output_buffer->format ? 1 : 0) + (current_item->next ? 1 : 0));
		output_pointer = ensure(output_buffer, length + 1);
		if (output_pointer == NULL)
		{
			return false;
		}
		if (current_item->next)
		{
			*output_pointer++ = ',';
		}

		if (output_buffer->format)
		{
			*output_pointer++ = '\n';
		}
		*output_pointer = '\0';
		output_buffer->offset += length;

		current_item = current_item->next;
	}

	output_pointer = ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
	if (output_pointer == NULL)
	{
		return false;
	}
	if (output_buffer->format)
	{
		size_t i;
		for (i = 0; i < (output_buffer->depth - 1); i++)
		{
			*output_pointer++ = '\t';
		}
	}
	*output_pointer++ = '}';
	*output_pointer = '\0';
	output_buffer->depth--;

	return true;
}

/* Get Array size/item / object item. */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array)
{
	cJSON *c = array->child;
	size_t i = 0;
	while (c)
	{
		i++;
		c = c->next;
	}

	/* FIXME: Can overflow here. Cannot be fixed without breaking the API */

	return (int)i;
}

static cJSON* get_array_item(const cJSON *array, size_t index)
{
	cJSON *current_child = NULL;

	if (array == NULL)
	{
		return NULL;
	}

	current_child = array->child;
	while ((current_child != NULL) && (index > 0))
	{
		index--;
		current_child = current_child->next;
	}

	return current_child;
}

CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index)
{
	if (index < 0)
	{
		return NULL;
	}

	return get_array_item(array, (size_t)index);
}

static cJSON *get_object_item(const cJSON * const object, const char * const name, const cJSON_bool case_sensitive)
{
	cJSON *current_element = NULL;

	if ((object == NULL) || (name == NULL))
	{
		return NULL;
	}

	current_element = object->child;
	if (case_sensitive)
	{
		while ((current_element != NULL) && (strcmp(name, current_element->string) != 0))
		{
			current_element = current_element->next;
		}
	}
	else
	{
		while ((current_element != NULL) && (case_insensitive_strcmp((const unsigned char*)name, (const unsigned char*)(current_element->string)) != 0))
		{
			current_element = current_element->next;
		}
	}

	return current_element;
}

CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string)
{
	return get_object_item(object, string, false);
}

CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string)
{
	return get_object_item(object, string, true);
}

CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object, const char *string)
{
	return cJSON_GetObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffix_object(cJSON *prev, cJSON *item)
{
	prev->next = item;
	item->prev = prev;
}

/* Utility for handling references. */
static cJSON *create_reference(const cJSON *item, const internal_hooks * const hooks)
{
	cJSON *ref = cJSON_New_Item(hooks);
	if (!ref)
	{
		return NULL;
	}
	memcpy(ref, item, sizeof(cJSON));
	ref->string = NULL;
	ref->type |= cJSON_IsReference;
	ref->next = ref->prev = NULL;
	return ref;
}

/* Add item to array/object. */
CJSON_PUBLIC(void) cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
	cJSON *child = NULL;

	if ((item == NULL) || (array == NULL))
	{
		return;
	}

	child = array->child;

	if (child == NULL)
	{
		/* list is empty, start new one */
		array->child = item;
	}
	else
	{
		/* append to the end */
		while (child->next)
		{
			child = child->next;
		}
		suffix_object(child, item);
	}
}

CJSON_PUBLIC(void) cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
	/* call cJSON_AddItemToObjectCS for code reuse */
	cJSON_AddItemToObjectCS(object, (char*)cJSON_strdup((const unsigned char*)string, &global_hooks), item);
	/* remove cJSON_StringIsConst flag */
	item->type &= ~cJSON_StringIsConst;
}

#if defined (__clang__) || ((__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

/* Add an item to an object with constant string as key */
CJSON_PUBLIC(void) cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item)
{
	if (!item)
	{
		return;
	}
	if (!(item->type & cJSON_StringIsConst) && item->string)
	{
		global_hooks.deallocate(item->string, __FILE__, __FUNCTION__, __LINE__);
	}
	item->string = (char*)string;
	item->type |= cJSON_StringIsConst;
	cJSON_AddItemToArray(object, item);
}
#if defined (__clang__) || ((__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic pop
#endif

CJSON_PUBLIC(void) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item)
{
	cJSON_AddItemToArray(array, create_reference(item, &global_hooks));
}

CJSON_PUBLIC(void) cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item)
{
	cJSON_AddItemToObject(object, string, create_reference(item, &global_hooks));
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item)
{
	if ((parent == NULL) || (item == NULL))
	{
		return NULL;
	}

	if (item->prev != NULL)
	{
		/* not the first element */
		item->prev->next = item->next;
	}
	if (item->next != NULL)
	{
		/* not the last element */
		item->next->prev = item->prev;
	}

	if (item == parent->child)
	{
		/* first element */
		parent->child = item->next;
	}
	/* make sure the detached item doesn't point anywhere anymore */
	item->prev = NULL;
	item->next = NULL;

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which)
{
	if (which < 0)
	{
		return NULL;
	}

	return cJSON_DetachItemViaPointer(array, get_array_item(array, (size_t)which));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which)
{
	cJSON_Delete(cJSON_DetachItemFromArray(array, which));
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string)
{
	cJSON *to_detach = cJSON_GetObjectItem(object, string);

	return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string)
{
	cJSON *to_detach = cJSON_GetObjectItemCaseSensitive(object, string);

	return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string)
{
	cJSON_Delete(cJSON_DetachItemFromObject(object, string));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string)
{
	cJSON_Delete(cJSON_DetachItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
CJSON_PUBLIC(void) cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem)
{
	cJSON *after_inserted = NULL;

	if (which < 0)
	{
		return;
	}

	after_inserted = get_array_item(array, (size_t)which);
	if (after_inserted == NULL)
	{
		cJSON_AddItemToArray(array, newitem);
		return;
	}

	newitem->next = after_inserted;
	newitem->prev = after_inserted->prev;
	after_inserted->prev = newitem;
	if (after_inserted == array->child)
	{
		array->child = newitem;
	}
	else
	{
		newitem->prev->next = newitem;
	}
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement)
{
	if ((parent == NULL) || (replacement == NULL))
	{
		return false;
	}

	if (replacement == item)
	{
		return true;
	}

	replacement->next = item->next;
	replacement->prev = item->prev;

	if (replacement->next != NULL)
	{
		replacement->next->prev = replacement;
	}
	if (replacement->prev != NULL)
	{
		replacement->prev->next = replacement;
	}
	if (parent->child == item)
	{
		parent->child = replacement;
	}

	item->next = NULL;
	item->prev = NULL;
	cJSON_Delete(item);

	return true;
}

CJSON_PUBLIC(void) cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem)
{
	if (which < 0)
	{
		return;
	}

	cJSON_ReplaceItemViaPointer(array, get_array_item(array, (size_t)which), newitem);
}

static cJSON_bool replace_item_in_object(cJSON *object, const char *string, cJSON *replacement, cJSON_bool case_sensitive)
{
	if (replacement == NULL)
	{
		return false;
	}

	/* replace the name in the replacement */
	if (!(replacement->type & cJSON_StringIsConst) && (replacement->string != NULL))
	{
		cJSON_free(replacement->string);
	}
	replacement->string = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
	replacement->type &= ~cJSON_StringIsConst;

	cJSON_ReplaceItemViaPointer(object, get_object_item(object, string, case_sensitive), replacement);

	return true;
}

CJSON_PUBLIC(void) cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem)
{
	replace_item_in_object(object, string, newitem, false);
}

CJSON_PUBLIC(void) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object, const char *string, cJSON *newitem)
{
	replace_item_in_object(object, string, newitem, true);
}

/* Create basic types: */
CJSON_PUBLIC(cJSON *) cJSON_CreateNull(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_NULL;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateTrue(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_True;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateFalse(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_False;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateBool(cJSON_bool b)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = b ? cJSON_True : cJSON_False;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateNumber(double num)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_Number;
		item->valuedouble = num;

		/* use saturation in case of overflow */
		if (num >= INT_MAX)
		{
			item->valueint = INT_MAX;
		}
		else if (num <= INT_MIN)
		{
			item->valueint = INT_MIN;
		}
		else
		{
			item->valueint = (int)num;
		}
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_String;
		item->valuestring = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
		if (!item->valuestring)
		{
			cJSON_Delete(item);
			return NULL;
		}
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateRaw(const char *raw)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_Raw;
		item->valuestring = (char*)cJSON_strdup((const unsigned char*)raw, &global_hooks);
		if (!item->valuestring)
		{
			cJSON_Delete(item);
			return NULL;
		}
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_Array;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_Object;
	}

	return item;
}

/* Create Arrays: */
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	if (count < 0)
	{
		return NULL;
	}

	a = cJSON_CreateArray();
	for (i = 0; a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if (!i)
		{
			a->child = n;
		}
		else
		{
			suffix_object(p, n);
		}
		p = n;
	}

	return a;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateFloatArray(const float *numbers, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	if (count < 0)
	{
		return NULL;
	}

	a = cJSON_CreateArray();

	for (i = 0; a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateNumber((double)numbers[i]);
		if (!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if (!i)
		{
			a->child = n;
		}
		else
		{
			suffix_object(p, n);
		}
		p = n;
	}

	return a;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateDoubleArray(const double *numbers, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	if (count < 0)
	{
		return NULL;
	}

	a = cJSON_CreateArray();

	for (i = 0; a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if (!i)
		{
			a->child = n;
		}
		else
		{
			suffix_object(p, n);
		}
		p = n;
	}

	return a;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateStringArray(const char **strings, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	if (count < 0)
	{
		return NULL;
	}

	a = cJSON_CreateArray();

	for (i = 0; a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateString(strings[i]);
		if (!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if (!i)
		{
			a->child = n;
		}
		else
		{
			suffix_object(p, n);
		}
		p = n;
	}

	return a;
}

/* Duplication */
CJSON_PUBLIC(cJSON *) cJSON_Duplicate(const cJSON *item, cJSON_bool recurse)
{
	cJSON *newitem = NULL;
	cJSON *child = NULL;
	cJSON *next = NULL;
	cJSON *newchild = NULL;

	/* Bail on bad ptr */
	if (!item)
	{
		goto fail;
	}
	/* Create new item */
	newitem = cJSON_New_Item(&global_hooks);
	if (!newitem)
	{
		goto fail;
	}
	/* Copy over all vars */
	newitem->type = item->type & (~cJSON_IsReference);
	newitem->valueint = item->valueint;
	newitem->valuedouble = item->valuedouble;
	if (item->valuestring)
	{
		newitem->valuestring = (char*)cJSON_strdup((unsigned char*)item->valuestring, &global_hooks);
		if (!newitem->valuestring)
		{
			goto fail;
		}
	}
	if (item->string)
	{
		newitem->string = (item->type&cJSON_StringIsConst) ? item->string : (char*)cJSON_strdup((unsigned char*)item->string, &global_hooks);
		if (!newitem->string)
		{
			goto fail;
		}
	}
	/* If non-recursive, then we're done! */
	if (!recurse)
	{
		return newitem;
	}
	/* Walk the ->next chain for the child. */
	child = item->child;
	while (child != NULL)
	{
		newchild = cJSON_Duplicate(child, true); /* Duplicate (with recurse) each item in the ->next chain */
		if (!newchild)
		{
			goto fail;
		}
		if (next != NULL)
		{
			/* If newitem->child already set, then crosswire ->prev and ->next and move on */
			next->next = newchild;
			newchild->prev = next;
			next = newchild;
		}
		else
		{
			/* Set newitem->child and move to it */
			newitem->child = newchild;
			next = newchild;
		}
		child = child->next;
	}

	return newitem;

fail:
	if (newitem != NULL)
	{
		cJSON_Delete(newitem);
	}

	return NULL;
}

CJSON_PUBLIC(void) cJSON_Minify(char *json)
{
	unsigned char *into = (unsigned char*)json;
	while (*json)
	{
		if (*json == ' ')
		{
			json++;
		}
		else if (*json == '\t')
		{
			/* Whitespace characters. */
			json++;
		}
		else if (*json == '\r')
		{
			json++;
		}
		else if (*json == '\n')
		{
			json++;
		}
		else if ((*json == '/') && (json[1] == '/'))
		{
			/* double-slash comments, to end of line. */
			while (*json && (*json != '\n'))
			{
				json++;
			}
		}
		else if ((*json == '/') && (json[1] == '*'))
		{
			/* multiline comments. */
			while (*json && !((*json == '*') && (json[1] == '/')))
			{
				json++;
			}
			json += 2;
		}
		else if (*json == '\"')
		{
			/* string literals, which are \" sensitive. */
			*into++ = (unsigned char)*json++;
			while (*json && (*json != '\"'))
			{
				if (*json == '\\')
				{
					*into++ = (unsigned char)*json++;
				}
				*into++ = (unsigned char)*json++;
			}
			*into++ = (unsigned char)*json++;
		}
		else
		{
			/* All other characters. */
			*into++ = (unsigned char)*json++;
		}
	}

	/* and null-terminate. */
	*into = '\0';
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_Invalid;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_False;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xff) == cJSON_True;
}


CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & (cJSON_True | cJSON_False)) != 0;
}
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_NULL;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_Number;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_String;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_Array;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_Object;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item)
{
	if (item == NULL)
	{
		return false;
	}

	return (item->type & 0xFF) == cJSON_Raw;
}

CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON * const a, const cJSON * const b, const cJSON_bool case_sensitive)
{
	if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)) || cJSON_IsInvalid(a))
	{
		return false;
	}

	/* check if type is valid */
	switch (a->type & 0xFF)
	{
	case cJSON_False:
	case cJSON_True:
	case cJSON_NULL:
	case cJSON_Number:
	case cJSON_String:
	case cJSON_Raw:
	case cJSON_Array:
	case cJSON_Object:
		break;

	default:
		return false;
	}

	/* identical objects are equal */
	if (a == b)
	{
		return true;
	}

	switch (a->type & 0xFF)
	{
		/* in these cases and equal type is enough */
	case cJSON_False:
	case cJSON_True:
	case cJSON_NULL:
		return true;

	case cJSON_Number:
		if (a->valuedouble == b->valuedouble)
		{
			return true;
		}
		return false;

	case cJSON_String:
	case cJSON_Raw:
		if ((a->valuestring == NULL) || (b->valuestring == NULL))
		{
			return false;
		}
		if (strcmp(a->valuestring, b->valuestring) == 0)
		{
			return true;
		}

		return false;

	case cJSON_Array:
	{
		cJSON *a_element = a->child;
		cJSON *b_element = b->child;

		for (; (a_element != NULL) && (b_element != NULL);)
		{
			if (!cJSON_Compare(a_element, b_element, case_sensitive))
			{
				return false;
			}

			a_element = a_element->next;
			b_element = b_element->next;
		}

		return true;
	}

	case cJSON_Object:
	{
		cJSON *a_element = NULL;
		cJSON_ArrayForEach(a_element, a)
		{
			/* TODO This has O(n^2) runtime, which is horrible! */
			cJSON *b_element = get_object_item(b, a_element->string, case_sensitive);
			if (b_element == NULL)
			{
				return false;
			}

			if (!cJSON_Compare(a_element, b_element, case_sensitive))
			{
				return false;
			}
		}

		return true;
	}

	default:
		return false;
	}
}

CJSON_PUBLIC(void *) cJSON_malloc(size_t size)
{
	return global_hooks.allocate(size, __FILE__, __FUNCTION__, __LINE__);
}

CJSON_PUBLIC(void) cJSON_free(void *object)
{
	global_hooks.deallocate(object, __FILE__, __FUNCTION__, __LINE__);
}

/*******************************************************/
/*************json格式校验相关接口**********************/
/*******************************************************/

static int
reject(JSON_checker jc)
{
/*
    Delete the JSON_checker object.
*/
    free((void*)jc->stack);
    free((void*)jc);
    return false;
}


static int
push(JSON_checker jc, int mode)
{
/*
    Push a mode onto the stack. Return false if there is overflow.
*/
    jc->top += 1;
    if (jc->top >= jc->depth) {
        return false;
    }
    jc->stack[jc->top] = mode;
    return true;
}


static int
pop(JSON_checker jc, int mode)
{
/*
    Pop the stack, assuring that the current mode matches the expectation.
    Return false if there is underflow or if the modes mismatch.
*/
    if (jc->top < 0 || jc->stack[jc->top] != mode) {
        return false;
    }
    jc->top -= 1;
    return true;
}


static JSON_checker
new_JSON_checker(int depth)
{
/*
    new_JSON_checker starts the checking process by constructing a JSON_checker
    object. It takes a depth parameter that restricts the level of maximum
    nesting.

    To continue the process, call JSON_checker_char for each character in the
    JSON text, and then call JSON_checker_done to obtain the final result.
    These functions are fully reentrant.

    The JSON_checker object will be deleted by JSON_checker_done.
    JSON_checker_char will delete the JSON_checker object if it sees an error.
*/
    JSON_checker jc = (JSON_checker)malloc(sizeof(struct JSON_checker_struct));
    jc->state = GO;
    jc->depth = depth;
    jc->top = -1;
    jc->stack = (int*)calloc(depth, sizeof(int));
    push(jc, MODE_DONE);
    return jc;
}


static int
JSON_checker_char(JSON_checker jc, int next_char)
{
/*
    After calling new_JSON_checker, call this function for each character (or
    partial character) in your JSON text. It can accept UTF-8, UTF-16, or
    UTF-32. It returns true if things are looking ok so far. If it rejects the
    text, it deletes the JSON_checker object and returns false.
*/
    int next_class, next_state;
/*
    Determine the character's class.
*/
    if (next_char < 0) {
        return reject(jc);
    }
    if (next_char >= 128) {
        next_class = C_ETC;
    } else {
        next_class = ascii_class[next_char];
        if (next_class <= __) {
            return reject(jc);
        }
    }
/*
    Get the next state from the state transition table.
*/
    next_state = state_transition_table[jc->state][next_class];
    if (next_state >= 0) {
/*
    Change the state.
*/
        jc->state = next_state;
    } else {
/*
    Or perform one of the actions.
*/
        switch (next_state) {
/* empty } */
        case -9:
            if (!pop(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

/* } */ case -8:
            if (!pop(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

/* ] */ case -7:
            if (!pop(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = OK;
            break;

/* { */ case -6:
            if (!push(jc, MODE_KEY)) {
                return reject(jc);
            }
            jc->state = OB;
            break;

/* [ */ case -5:
            if (!push(jc, MODE_ARRAY)) {
                return reject(jc);
            }
            jc->state = AR;
            break;

/* " */ case -4:
            switch (jc->stack[jc->top]) {
            case MODE_KEY:
                jc->state = CO;
                break;
            case MODE_ARRAY:
            case MODE_OBJECT:
                jc->state = OK;
                break;
            default:
                return reject(jc);
            }
            break;

/* , */ case -3:
            switch (jc->stack[jc->top]) {
            case MODE_OBJECT:
/*
    A comma causes a flip from object mode to key mode.
*/
                if (!pop(jc, MODE_OBJECT) || !push(jc, MODE_KEY)) {
                    return reject(jc);
                }
                jc->state = KE;
                break;
            case MODE_ARRAY:
                jc->state = VA;
                break;
            default:
                return reject(jc);
            }
            break;

/* : */ case -2:
/*
    A colon causes a flip from key mode to object mode.
*/
            if (!pop(jc, MODE_KEY) || !push(jc, MODE_OBJECT)) {
                return reject(jc);
            }
            jc->state = VA;
            break;
/*
    Bad action.
*/
        default:
            return reject(jc);
        }
    }
    return true;
}


static int
JSON_checker_done(JSON_checker jc)
{
/*
    The JSON_checker_done function should be called after all of the characters
    have been processed, but only if every call to JSON_checker_char returned
    true. This function deletes the JSON_checker and returns true if the JSON
    text was accepted.
*/
    int result = jc->state == OK && pop(jc, MODE_DONE);
    reject(jc);
    return result;
}

static char*
JSON_print_type(int type)
{
	switch(type)
	{
        case 0:
		case 1:
            return "Boolean";
		case 2:
            return "Empty";
		case 3:
            return "Number";
		case 4:
            return "String";
		case 5:
            return "Array";
		case 6:
            return "Object";
		/*
			Bad action.
		*/
        default:
            return "Unknown";
    }
}

/*************************************************************
*函数:	JSON_check
*参数:	JSON_str
*返回值:true(1)表示json，false(0)表示非json
*描述:	判断该字符串是否是json
*************************************************************/
extern int
JSON_check(char *JSON_str)
{
	int iFlag = true;
	char *pChr = JSON_str;
	
	if(NULL == JSON_str)
	{
		return false;
	}
	
	JSON_checker jc = new_JSON_checker(JSON_DEPTH);
	
	while(*pChr)
	{
		int next_char = *pChr++;
		if (!JSON_checker_char(jc, next_char)) {
            iFlag = false;
			return iFlag;
        }
	}

    if (!JSON_checker_done(jc)) {
        iFlag = false;
    }
	
	return iFlag;
	
}


/*************************************************************
*函数:	JSON_is_boolean
*参数:	pstData:json结构体
*返回值:true(1)表示是，false(0)表示否
*描述:	判断该json是否是布尔类型
*************************************************************/
extern int
JSON_is_boolean(cJSON *pstData)
{
	if(pstData && (cJSON_False == pstData->type || cJSON_True == pstData->type))
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*************************************************************
*函数:	JSON_is_empty
*参数:	pstData:json结构体
*返回值:true(1)表示是，false(0)表示否
*描述:	判断该json是否是空类型
*************************************************************/
extern int
JSON_is_empty(cJSON *pstData)
{
	if(pstData && cJSON_NULL == pstData->type)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*************************************************************
*函数:	JSON_is_number
*参数:	pstData:json结构体
*返回值:true(1)表示是，false(0)表示否
*描述:	判断该json是否是数值类型
*************************************************************/
extern int
JSON_is_number(cJSON *pstData)
{
	if(pstData && cJSON_Number == pstData->type)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*************************************************************
*函数:	JSON_is_array
*参数:	pstData:json结构体
*返回值:true(1)表示是，false(0)表示否
*描述:	判断该json是否是数组类型
*************************************************************/
extern int
JSON_is_array(cJSON *pstData)
{
	if(pstData && cJSON_Array == pstData->type)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*************************************************************
*函数:	JSON_is_object
*参数:	pstData:json结构体
*返回值:true(1)表示是，false(0)表示否
*描述:	判断该json是否是对象类型
*************************************************************/
extern int
JSON_is_object(cJSON *pstData)
{
	if(pstData && cJSON_Object == pstData->type)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*************************************************************
*函数:	JSON_is_string
*参数:	pstData:json结构体
*返回值:true(1)表示是，false(0)表示否
*描述:	判断该json是否是字符串类型
*************************************************************/
extern int
JSON_is_string(cJSON *pstData)
{
	if(pstData && cJSON_String == pstData->type)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*************************************************************
*函数:	JSON_str_value_get
*参数:	pcKey:键值，键值可以为NULL
*		pcValue:值
*		iValueLen:值缓存的长度
*		pstData:json结构体
*返回值:0表示获取成功，非0表示获取失败
*描述:	从json中获取指定key的值，该值必须是字符串类型
*************************************************************/
extern int
JSON_str_value_get(char *pcKey, char *pcValue, int iValueLen, cJSON *pstData)
{
	char *pch = NULL;
	cJSON *pstItem = NULL;
	
	if(NULL == pcValue || NULL == pstData)
	{
		return -1;
	}
	
	if(NULL == pcKey || !strcmp(pcKey, ""))
	{
		if(pstData && cJSON_String == pstData->type)
		{
			pch = pstData->valuestring;
			strncpy(pcValue, pch, iValueLen);
			//printf("%s\n", pcValue);
			return 0;
		}
		else
		{
			printf("Value is not a string type! This type is '%s'\n", JSON_print_type(pstData->type));
			return -1;
		}
	}
	else
	{
		pstItem = cJSON_GetObjectItem(pstData, pcKey);
		if(pstItem && cJSON_String == pstItem->type)
		{
			pch = pstItem->valuestring;
			strncpy(pcValue, pch, iValueLen);
			//printf("%s = %s\n", pcKey, pcValue);
			return 0;
		}
		else
		{
			if(pstItem)
			{
				printf("%s value is not a string type! This type is '%s'\n", pcKey, JSON_print_type(pstItem->type));
				return -1;	
			}
			else
			{
				printf("the %s doesn't exist\n", pcKey);
				return -1;
			}
			
		}
	}
	return 0;
}

/*************************************************************
*函数:	JSON_number_value_get
*参数:	pcKey:键值，键值可以为NULL
*		pcValue:值
*		iValueLen:值缓存的长度
*		pstData:json结构体
*返回值:0表示获取成功，非0表示获取失败
*描述:	从json中获取指定key的值，该值必须是数值类型
*************************************************************/
extern int
JSON_number_value_get(char *pcKey, char *pcValue, int iValueLen, cJSON *pstData)
{
	double dValue = 0.0;
	cJSON *pstItem = NULL;
	
	if(NULL == pcValue || NULL == pstData)
	{
		return -1;
	}
	
	if(NULL == pcKey || !strcmp(pcKey, ""))
	{
		if(pstData && cJSON_Number == pstData->type)
		{
			dValue = pstData->valuedouble;
			if (0 == dValue)
			{
				snprintf(pcValue, iValueLen, "%d", 0);
			}
			else if (fabs(((double)pstData->valueint)-dValue) <= DBL_EPSILON && dValue <= INT_MAX && dValue >= INT_MIN)
			{
				snprintf(pcValue, iValueLen, "%d", pstData->valueint);
			}
			else
			{
				if(fabs(floor(dValue)-dValue) <= DBL_EPSILON && fabs(dValue) < 1.0e60)
				{
					snprintf(pcValue, iValueLen, "%.0f", dValue);
				}
				else if(fabs(dValue) < 1.0e-6 || fabs(dValue) > 1.0e9)
				{
					snprintf(pcValue, iValueLen, "%e", dValue);
				}
				else
				{
					snprintf(pcValue, iValueLen, "%f", dValue);
				}
			}
			return 0;
		}
		else
		{
			printf("Value is not a number type! This type is '%s'\n", JSON_print_type(pstData->type));
			return -1;
		}
	}
	else
	{
		pstItem = cJSON_GetObjectItem(pstData, pcKey);
		if(pstItem && cJSON_Number == pstItem->type)
		{
			dValue = pstData->valuedouble;
			if (0 == dValue)
			{
				snprintf(pcValue, iValueLen, "%d", 0);
			}
			else if (fabs(((double)pstData->valueint)-dValue) <= DBL_EPSILON && dValue <= INT_MAX && dValue >= INT_MIN)
			{
				snprintf(pcValue, iValueLen, "%d", pstData->valueint);
			}
			else
			{
				if(fabs(floor(dValue)-dValue) <= DBL_EPSILON && fabs(dValue) < 1.0e60)
				{
					snprintf(pcValue, iValueLen, "%.0f", dValue);
				}
				else if(fabs(dValue) < 1.0e-6 || fabs(dValue) > 1.0e9)
				{
					snprintf(pcValue, iValueLen, "%e", dValue);
				}
				else
				{
					snprintf(pcValue, iValueLen, "%f", dValue);
				}
			}
			return 0;
		}
		else
		{
			if(pstItem)
			{
				printf("%s value is not a number type! This type is '%s'\n", pcKey, JSON_print_type(pstItem->type));
				return -1;	
			}
			else
			{
				printf("the %s doesn't exist\n", pcKey);
				return -1;
			}
			
		}
	}
	return 0;
}

/*************************************************************
*函数:	JSON_number_value_get
*参数:	pcKey:键值，键值可以为NULL
*		pcValue:值
*		iValueLen:值缓存的长度
*		pstData:json结构体
*返回值:0表示获取成功，非0表示获取失败
*描述:	从json中获取指定key的值，该值必须是数值类型
*************************************************************/
extern int
JSON_bool_value_get(char *pcKey, char *pcValue, int iValueLen, cJSON *pstData)
{
	cJSON *pstItem = NULL;
	
	if(NULL == pcValue || NULL == pstData)
	{
		return -1;
	}
	
	if(NULL == pcKey || !strcmp(pcKey, ""))
	{
		if(cJSON_False == pstData->type)
		{
			snprintf(pcValue, iValueLen, "%d", 0);
			return 0;
		}
		else if(cJSON_True == pstData->type)
		{
			snprintf(pcValue, iValueLen, "%d", 1);
			return 0;
		}
		else
		{
			printf("Value is not a bool type! This type is '%s'\n", JSON_print_type(pstData->type));
			return -1;
		}
	}
	else
	{
		pstItem = cJSON_GetObjectItem(pstData, pcKey);
		if(pstItem && cJSON_False == pstItem->type)
		{
			snprintf(pcValue, iValueLen, "%d", 0);
			return 0;
		}
		else if(pstItem && cJSON_True == pstItem->type)
		{
			snprintf(pcValue, iValueLen, "%d", 1);
			return 0;
		}
		else
		{
			if(pstItem)
			{
				printf("%s value is not a bool type! This type is '%s'\n", pcKey, JSON_print_type(pstItem->type));
				return -1;	
			}
			else
			{
				printf("the %s doesn't exist\n", pcKey);
				return -1;
			}
			
		}
	}
	return 0;
}
/*************************************************************
*函数:	JSON_array_value_get
*参数:	pcKey:键值，键值可以为NULL
*		pstArray:返回json数组指针
*		pstData:json结构体
*返回值:0表示获取成功，非0表示获取失败
*描述:	从json中获取指定key的值，该值必须是数组类型
*************************************************************/
extern int
JSON_array_value_get(char *pcKey, cJSON **pstArray, cJSON *pstData)
{
	if(NULL == pstArray || NULL == pstData)
	{
		return -1;
	}
	if(NULL == pcKey || !strcmp(pcKey, ""))
	{
		if(cJSON_Array == pstData->type)
		{
			*pstArray = pstData;
			return 0;
		}
		else
		{
			printf("Value is not a array type! This type is '%s'\n", JSON_print_type(pstData->type));
			*pstArray = NULL;
			return -1;
		}
	}
	else
	{
		*pstArray = cJSON_GetObjectItem(pstData, pcKey);
		if(*pstArray && cJSON_Array == (*pstArray)->type)
		{
			return 0;
		}
		else
		{
			printf("the %s doesn't exist\n", pcKey);
			*pstArray = NULL;
			return -1;
		}
	}
	return 0;
}
/*************************************************************
*函数:	JSON_array_value_get
*参数:	pcKey:键值，键值可以为NULL
*		pstObject:返回json数组指针
*		pstData:json结构体
*返回值:0表示获取成功，非0表示获取失败
*描述:	从json中获取指定key的值，该值必须是对象类型
*************************************************************/
extern int
JSON_object_value_get(char *pcKey, cJSON **pstObject, cJSON *pstData)
{
	if(NULL == pstObject || NULL == pstData)
	{
		return -1;
	}
	if(NULL == pcKey || !strcmp(pcKey, ""))
	{
		if(cJSON_Object == pstData->type)
		{
			*pstObject = pstData;
			return 0;
		}
		else
		{
			printf("Value is not a array type! This type is '%s'\n", JSON_print_type(pstData->type));
			*pstObject = NULL;
			return -1;
		}
	}
	else
	{
		*pstObject = cJSON_GetObjectItem(pstData, pcKey);
		if(*pstObject && cJSON_Object == (*pstObject)->type)
		{
			return 0;
		}
		else
		{
			printf("the %s doesn't exist\n", pcKey);
			*pstObject = NULL;
			return -1;
		}
	}
	
	return 0;
}

/*************************************************************
*函数:	JSON_value_get
*参数:	pcKey:键值，键值可以为NULL
*		pcValue:返回布尔、数值以及字符串类型，可以为NULL
*		iValueLen:值缓存的长度
*		pstValue:返回数组或对象类型， 可以为NULL
*		pstData:json结构体
*返回值:0表示获取成功，非0表示获取失败
*描述:	从json中获取指定key的值
*************************************************************/
extern int
JSON_value_get(char *pcKey, char *pcValue, int iValueLen, cJSON **pstValue, int *piValueType, cJSON *pstData)
{
	if(NULL == piValueType || NULL == pstData)
	{
		return -1;
	}

	int iRet = 0;
	cJSON *pstItem = NULL;
	
	if(NULL == pcKey || !strcmp(pcKey, ""))
	{	
		pstItem = pstData;
	}
	else
	{
		pstItem = cJSON_GetObjectItem(pstData, pcKey);
	}
	if(NULL!= pstItem)
	{
		*piValueType = pstItem->type;
		switch(pstItem->type)
		{
			case cJSON_False:
			case cJSON_True:
				iRet = JSON_bool_value_get(NULL, pcValue, iValueLen, pstItem);
				break;
			case cJSON_NULL:
				
				break;
			case cJSON_Number:
				iRet = JSON_number_value_get(NULL, pcValue, iValueLen, pstItem);
				break;
			case cJSON_String:
				iRet = JSON_str_value_get(NULL, pcValue, iValueLen, pstItem);
				break;
			case cJSON_Array:
				iRet = JSON_array_value_get(NULL, pstValue, pstItem);
				/*pcValue如果非空，则把数组类型转变为字符串类型，并输出*/
				if(0 == iRet && NULL != pcValue)
				{
					char *pstr = NULL;
					pstr = cJSON_Print(*pstValue);
					strncpy(pcValue, pstr, iValueLen);
					free(pstr);
					cJSON_Minify(pcValue);
				}
				break;
			case cJSON_Object:
				iRet = JSON_object_value_get(NULL, pstValue, pstItem);
				/*pcValue如果非空，则把对象类型转变为字符串类型，并输出*/
				if(0 == iRet && NULL != pcValue)
				{
					char *pstr = NULL;
					pstr = cJSON_Print(*pstValue);
					strncpy(pcValue, pstr, iValueLen);
					free(pstr);
					cJSON_Minify(pcValue);
				}
				break;

			default:
				printf("The Value Type error.\n");
				iRet = -1;
				break;
		}
		return iRet;
	}
	else
	{
		if(pcKey)
		{
			printf("the %s doesn't exist\n", pcKey);
		}
		return -1;
	}
	return 0;
}


