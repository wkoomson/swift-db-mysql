// vim:ts=4:sts=4:sw=4:expandtab

// (c) Bharanee Rathna 2012

#include "common.h"
#include "typecast.h"
#include <uuid/uuid.h>

VALUE db_mysql_adapter_escape(VALUE, VALUE);

VALUE rb_uuid_string() {
    size_t n;
    uuid_t uuid;
    char uuid_hex[sizeof(uuid_t) * 2 + 1];

    uuid_generate(uuid);
    for (n = 0; n < sizeof(uuid_t); n++)
        sprintf(uuid_hex + n * 2 + 1, "%02x", uuid[n]);

    uuid_hex[0] = 'u';
    return rb_str_new(uuid_hex, sizeof(uuid_t) * 2 + 1);
}

/* NOTE: very naive, no regex etc. */
VALUE db_mysql_bind_sql(VALUE adapter, VALUE sql, VALUE bind) {
    VALUE value;
    size_t i = 0, j = 0, n = 0;
    size_t size = 4096, slen;
    char *ptr, *buffer, *data;

    buffer = (char *)malloc(size);
    ptr    = RSTRING_PTR(sql);

    while (i < (size_t)RSTRING_LEN(sql)) {
        if (*ptr == '?') {
            if (n < (size_t)RARRAY_LEN(bind)) {
                value = rb_ary_entry(bind, n++);
                data  = NIL_P(value) ? 0 : CSTRING(db_mysql_adapter_escape(adapter, typecast_to_string(value)));
                slen  = data ? strlen(data) : 4;
                if (slen > size - j)
                    buffer = realloc(buffer, size += (slen > 4096 ? slen + 4096 : 4096));

                if (data)
                    j += sprintf(buffer + j, "'%s'", data);
                else
                    j += sprintf(buffer + j, "NULL");
            }
            else {
                buffer[j++] = *ptr;
                n++;
            }
        }
        else {
            buffer[j++] = *ptr;
        }

        i++;
        ptr++;

        if (j >= size)
            buffer = realloc(buffer, size += 4096);
    }

    sql = rb_str_new(buffer, j);
    free(buffer);

    if (n != (size_t)RARRAY_LEN(bind))
        rb_raise(eSwiftArgumentError, "expected %d arguments got %d instead", n, RARRAY_LEN(bind));
    return sql;
}
