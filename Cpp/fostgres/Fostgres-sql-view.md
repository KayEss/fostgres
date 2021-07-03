# `fostgres.sql` view reference

* For details about file uploads see [File uploads](./File-uploads.md)


## Configuration

* `sql` -- The individual end points, see "End point configuration" below.

The view first goes through the list of end point configurations looking for the first path match.


### End point configuration

Each end point configuration consists of a number of keys:

* `response` -- The type of API. The URL to database mapping essentially must describe a resource that comprised either one or multiple rows in the relation/table.
    * `object` -- The URL describes a single row in the database.
    * `csj` (default) -- The URL describes multiple rows in the database.
* `path` -- The path configuration to match against
* `response` -- The type of API. The URL to database mapping essentially must describe a resource that comprised either one or multiple rows in the relation/table.
    * `object` -- The URL describes a single row in the database.
    * `csj` (default) -- The URL describes multiple rows in the database.
* `precondition` -- A precondition expression that must be true.
* `GET` -- Used for `GET` requests.
* `PUT` -- Used for `PUT` requests.
* `PATCH` -- Used for `PATCH` requests.
* `POST` -- Used for `POST` requests.
* `DELETE` -- Used for `DELETE` requests.


#### Path configuration (match expressions)

The path is a JSON array of path elements that are to be matched against. Two options exist:

* A number. This then becomes that source number in the SQL. For example, `1` would become the `$1` argument in the `GET` request SQL.
* A string starting with a forward slash. This describes fixed text in the URL.

For example:

    ["/location", 1]

Would match the following URLs:

    /location/one
    /location/foo
    /location/bar.txt

But would not match:

    /location/one/two # Contains an extra path element on the end
    /api/location/one # Doesn't have `location` as the first path element

To match the first one use one of:

    ["/location", 1, 2] # for two arguments
    ["/location", 1, "/two"] # for one argument

And again, the second could have a one or two argument form:

    ["/api", "/location", 1] # for one argument
    [2, "/location", 1] # for two arguments


#### Preconditions

Preconditions are used to describe relationships between the data that must be true. For example, if you want the match captures `1` and `2` to be equal you can use the following:

    "precondition": ["eq", ["match", 1], ["match", 2]]

If they are not equal then a 422 response will be generated. The logs will contain details of the failed expression, but this detail will not be returned to the client.

Headers can be examined by using a `jcursor` to fetch the wanted field:

    "precondition": ["eq", ["match", 1], [ "header", "__user"]]

This checks that the matched URL part `1` is the same as the `__user` header (as would be set by Odin).

The `eq` operator itself is variadic and can be given many items that must all be equal to each other:

    "precondition": ["eq",
        ["match", 1], ["match", 2], ["header", "__user"]]

This means that the matches `1` and `2` must be equal to each other, and be equal to the `__user` header value.

The operators `or`(WIP) and `and`(WIP) will be availabled in the future. Again, they are both variaded, with `or` returning true if any of the sub-expressions are true, and `and` returns true so long as all sub-expresssions are true.

    "precondition": ["or",
        ["eq", ["match", 1], ["match", 2]],
        ["eq", 1, ["header", "__user"]]]

This precondition passes if either `1` and `2` are equal or if  `1` is equal to the `__user` header.
