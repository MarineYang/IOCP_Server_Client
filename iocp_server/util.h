#pragma once

// delete object
#undef	SAFE_DELETE
#define SAFE_DELETE(obj)						\
{												\
	if ((obj)) delete(obj);		    			\
    (obj) = 0L;									\
}
// delete object array
#undef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(arr)					\
{												\
	if ((arr)) delete [] (arr);		    		\
    (arr) = 0L;									\
}

// delete gameObject
#define SAFE_FREE(obj)							\
{												\
	if ((obj)) obj->free();						\
    (obj) = 0L;									\
}

#define SAFE_RELEASE(obj)                       \
{                                               \
	if (obj) { obj.release(); }                 \
}
