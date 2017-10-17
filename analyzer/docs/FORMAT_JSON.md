JSON Format
===========

This file describes the JSON export format.

The report is composed of a list of statement reports. Each statement report is a dictionary:

```json
{
    "status": "error",
    "checker": "boa",
    "short_msg": "buffer overflow",
    "long_msg": "buffer overflow, trying to access offset 99 (bytes)",
    "full_msg": "buffer overflow, trying to access offset 99 (bytes) of buffer 'str' of size 50 bytes",
    "extra_msgs": [
        ["expression well defined", "uva"],
        ["expression is non-null", "nullity"],
        ["pointer aligned to requirement", "upa"]
    ],
    "location": {
        "type": "stmt",
        "file": "/path/to/ikos-root/analyzer/tests/regression/boa/test-23-unsafe-1.c",
        "function": "main",
        "line": 18,
        "column": 3,
        "stmt_uid": 116
    },
    "all_contexts": true,
    "contexts": [
        []
    ]
},
```

Each statement report has the following key/values:

* A status (either "safe", "note", "warning", "error" or "unreachable");
* A checker (either "boa", "dbz", "nullity", "prover", "uva", "upa", "call", "special" or "unreachable");
* A short message `short_msg`, long message `long_msg` and full message `full_msg`, in increasing level of verbosity. It allows you to show a short message to a user first, and to allow it to get more information is needed;
* A list of extra messages `extra_msgs`. These are extra information that the analyzer was able to prove, and might be interesting for the user. Each message is a pair [string, checker]. The checker is either "boa", "dbz", "nullity", "uva" or "upa";
* A `location` (see below)
* A list of calling contexts `contexts` (see below)
* A boolean `all_contexts`, true if the report is true regardless of the calling context.

Note: `long_msg`, `full_msg` and `extra_msgs` might contain newlines. You should take this into account if you are designing a user interface.

Source code location
--------------------

There are three types of location: `source`, `stmt` and `range`.

A `source` location is a location to a specific file, function, line and column.
A `stmt` location is a location of an AR statement (the intermediate abstract representation). It has a file, function, line, column and the AR statement UID `stmt_uid`.
A `range` location refers to several lines and/or columns in a specific file. It has two key/values: `begin` and `end`. For instance:

```json
"location": {
    "type": "range",
    "begin": {
        "type": "source",
        "file": "/path/to/ikos-root/analyzer/tests/regression/boa/test-3-unsafe.c",
        "function": "main",
        "line": 16,
        "column": 3
    },
    "end": {
        "type": "source",
        "file": "/path/to/ikos-root/analyzer/tests/regression/boa/test-3-unsafe.c",
        "function": "main",
        "line": 17,
        "column": 3
    }
}
```

Note: The current version of ikos might generate invalid locations. To check if a source location is valid, check if:

* The file is different from "?";
* The line and column are greater than 0.

Calling contexts
----------------

Each report has a list of calling contexts. A calling context is a list of location of function calls leading to a specific function.

For instance, let's say we are analyzing the following code:

```c
 1	extern void __ikos_assert(int);
 2
 3	void foo() {
 4	    __ikos_assert(1);
 5	}
 6
 7	void f() {
 8	    foo();
 9	}
10
11	void g() {
12	    foo();
13	}
14
15	int main() {
16	    f();
17	    g();
18	    return 0;
19	}
```

The `contexts` value for the report on statement `__ikos_assert(1);` will be:

```json
[
    [
        {
            "type": "stmt",
            "file": "/tmp/test.c",
            "function": "main",
            "line": 16,
            "column": 5,
            "stmt_uid": 42
        },
        {
            "type": "stmt",
            "file": "/tmp/test.c",
            "function": "f",
            "line": 8,
            "column": 5,
            "stmt_uid": 22
        }
    ],
    [
        {
            "type": "stmt",
            "file": "/tmp/test.c",
            "function": "main",
            "line": 17,
            "column": 5,
            "stmt_uid": 47
        },
        {
            "type": "stmt",
            "file": "/tmp/test.c",
            "function": "g",
            "line": 12,
            "column": 5,
            "stmt_uid": 32
        }
    ]
]
```

It basically describes the two paths to the function `foo`:

* `main:16:5` -> `f:8:5` -> `foo`
* `main:17:5` -> `g:12:5` -> `foo`
