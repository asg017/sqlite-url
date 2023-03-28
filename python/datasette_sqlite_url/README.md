# The `datasette-sqlite-url` Datasette Plugin

`datasette-sqlite-url` is a [Datasette plugin](https://docs.datasette.io/en/stable/plugins.html) that loads the [`sqlite-url`](https://github.com/asg017/sqlite-url) extension in Datasette instances, allowing you to generate and work with [TODO](https://github.com/url/spec) in SQL.

```
datasette install datasette-sqlite-url
```

See [`docs.md`](../../docs.md) for a full API reference for the all url SQL functions.

Alternatively, when publishing Datasette instances, you can use the `--install` option to install the plugin.

```
datasette publish cloudrun data.db --service=my-service --install=datasette-sqlite-url

```
