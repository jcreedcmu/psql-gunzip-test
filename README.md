gunzip function for postgresql
------------------------------

This is a little experiment to see if I could write an external
function for postgres that uncompresses gzip-compressed JSON data
stored in a database column.

Wouldn't be surprised if this is already a thing you can do with some
built-in or contrib code but I had fun figuring out how to do it from
scratch.

The example I started from for making external functions in C in the
first place was:
http://www.christian-rossow.de/articles/PostgreSQL_C_language_functions_with_bytea.php

How I built it was:

```shell
$ make
$ sudo cp func.so `pg_config --pkglibdir`
$ sudo su - the-name-of-the-db-user -c "psql the-name-of-the-db"
> CREATE FUNCTION gunzip(bytea) RETURNS text
  AS 'func', 'gunzip'
  LANGUAGE C STRICT;
> SELECT gunzip(compressed_json_column)::json->'foo'->'bar';
```
