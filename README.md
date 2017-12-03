# fostgres

*RESTful APIs for Postgres databases*

Postgres is an incredibly powerful database system. It seems a shame to hide it behind a cripplingly bad O/RM just because you want to use it for your web APIs.

Fostgres is a very small wrapper around the SQL interfaces already present in Postgres. Its intention is to enable as much of Postgres' power as possible to be delivered over an HTTP interface. To actually make you want to use it though it also needs to make those APIs great to consume.

Think of Fostgres as post-framework infrastructure.

Yes, [the Fostgres name is stupid](http://tvtropes.org/pmwiki/pmwiki.php/Main/LampshadeHanging). Once somebody suggests a better one it'll get changed.


# Central concepts

In Fostgres a URL represents some data in the database (-- who'd have thought?) This is going to be either:

1. A single database row where the primary key (or some other unique constraint) appears in the URL.
2. A set of database rows (either all for a given table, or some sub-set where a filter appears in the URL).

The behaviour between these two wants to be slightly different. In the first the `SELECT` used to get the data needs to return exactly one row. Too few and it's a 404 and too many and there's a mismatch between your database schema and the URL configuration (an application error).

The second form can return as many or as few rows as the `SELECT` likes. The only difference is how much data ends up in the response body.

To look at something concrete let's think about a database that holds films and allows them to be tagged. The first case wants a configuration something like this:

    {
        "return": "object",
        "path": ["/film", 1],
        "GET": "SELECT * FROM films WHERE slug = $1"
    }

This means:

* We are going to return a JSON object made up from the columns in the `SELECT`.
* The path is going to have the word "film" followed by what is going to become the first parameter in the `SELECT` statement. It should look something like `/film/t1`
* The GET will issue the specified SELECT and then return the object (or a 404 if nothing is matched).

The JSON object that is returned might look like this:

    {
        "slug": "t1",
        "title": "Terminator",
        "released": "1984-10-26"
    }


The other way we want to handle things is to return multiple rows. For example, fetching all of the tags in the database:

    {
        "path": ["/tags"],
        "GET": "SELECT * FROM tags"
    }

This means:

* We don't specify a "return" because this form is the default.
* The path just contains `/tags` and there are no parameters to the SQL.
* The `SELECT` statement that will be run for the GET request.

This will give you CSJ that describes the rows. The output might look like this:

    "tag"
    "action"
    "adventure"
    "sci-fi"
    "time-travel"
    "robots"
    "dystopian"

This should be enough to understand at least some of the [configuration](./Example/config/view.film-slug.json) found in the [films](./Example/schema/films.tables.sql) example. There is obviously a lot more going on in the configuration as well.

* [Tutorial on using the Fostgres view](./Example/tutorial/README.md)
* [Reference documentation about the main Fostgres view](./Cpp/fostgres/Fostgres-sql-view.md)
* Look at the basic Mengmom views -- TODO
* [Odin is a security system compatible with Fostgres](https://github.com/KayEss/odin/blob/master/README.md)

## Testing your views

Being able to write views is one thing, but we don't want to have to manually test that everything is working as we expect. Fostgres comes with a simple scripting language that knows enough about the database and the views to help you write tests that ensure that the APIs you build do what you expect.

For example, you can make sure that the first configuration above (in a `film.slug` view) returns with a 200 and contains a "title" field in the output with the right value like this:

    GET film.slug /film/terminator 200 {"title": "The Terminator"}

And we can make sure we get a 404 from an unknown tag

    GET film.slug /film/not-a-film 404

The script can be run using `fostgres-test`:

    fostgres-test test-dbname Example/schema/films.tables.sql \
        Example/config/view.film-slug.json Example/tests/film.t1.fg

The first argument is the name of the test database to use. This will be destroyed and re-created each time the tests are run so don't use a name you care about. The other arguments are one or more `.sql` files that set up the schema needed for testing, some `.json` file that contain the view configurations being tested and also a single `.fg` file that contains the test scripts.

This should be enough that the [real test script](./Example/tests/film.t1.fg) makes some sort of sense.


# Views and configuration

Most of Fostgres is implemented through a single view, `fostgres.sql` which is suitable for general interaction with the database.

## Configuration for running the `test`s target

You will need Postgres 9.5 or later installed. The database needs to be asccesible to your user account with super user permissions. Running `psql` on its own (with no command line options) will allow you to run commands like `CREATE DATABASE`.

