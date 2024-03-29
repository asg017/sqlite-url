# `sqlite-url` NPM Package

`sqlite-url` is distributed on `npm` for Node.js developers. To install on [supported platforms](#supported-platforms), simply run:

```
npm install sqlite-url
```

The `sqlite-url` package is meant to be used with Node SQLite clients like [`better-sqlite3`](https://github.com/WiseLibs/better-sqlite3) and [`node-sqlite3`](https://github.com/TryGhost/node-sqlite3). For `better-sqlite3`, call [`.loadExtension()`](https://github.com/WiseLibs/better-sqlite3/blob/master/docs/api.md#loadextensionurl-entrypoint---this) on your database object, passing in [`getLoadablePath()`](#getLoadablePath).

```js
import Database from "better-sqlite3";
import * as sqlite_url from "sqlite-url";

const db = new Database(":memory:");

db.loadExtension(sqlite_url.getLoadablePath());

const version = db.prepare("select url_version()").pluck().get();
console.log(version); // "v0.2.0"
```

For `node-sqlite3`, call the similarly named [`.loadExtension()`](https://github.com/TryGhost/node-sqlite3/wiki/API#loadextensionurl--callback) method on your database object, and pass in [`getLoadablePath()`](#getLoadablePath).

```js
import sqlite3 from "sqlite3";
import * as sqlite_url from "sqlite-url";

const db = new sqlite3.Database(":memory:");

db.loadExtension(sqlite_url.getLoadablePath());

db.get("select url_version()", (err, row) => {
  console.log(row); // {json_schema_version(): "v0.2.0"}
});
```

See [the full API Reference](#api-reference) for the Node API, and [`docs.md`](../../docs.md) for documentation on the `sqlite-url` SQL API.

## Supported Platforms

Since the underlying `url0` SQLite extension is pre-compiled, the `sqlite-url` NPM package only works on a few "platforms" (operating systems + CPU architectures). These platforms include:

- `darwin-x64` (MacOS x86_64)
- `win32-x64` (Windows x86_64)
- `linux-x64` (Linux x86_64)

To see which platform your machine is, check the [`process.arch`](https://nodejs.org/api/process.html#processarch) and [`process.platform`](https://nodejs.org/api/process.html#processplatform) values like so:

```bash
$ node -e 'console.log([process.platform, process.arch])'
[ 'darwin', 'x64' ]
```

When the `sqlite-url` NPM package is installed, the correct pre-compiled extension for your operating system and CPU architecture will be downloaded from the [optional dependencies](https://docs.npmjs.com/cli/v9/configuring-npm/package-json#optionaldependencies), with platform-specific packages like `sqlite-url-darwin-x64`. This will be done automatically, there's no need to directly install those packages.

More platforms may be supported in the future. Consider [supporting my work](https://github.com/sponsors/asg017/) if you'd like to see more operating systems and CPU architectures supported in `sqlite-url`.

## API Reference

<a href="#getLoadablePath" name="getLoadablePath">#</a> <b>getLoadablePath</b> [<>](https://github.com/asg017/sqlite-url/blob/main/npm/sqlite-url/src/index.js "Source")

Returns the full path to where the `sqlite-url` _should_ be installed, based on the `sqlite-url`'s `package.json` optional dependencies and the host's operating system and architecture.

This url can be directly passed into [`better-sqlite3`](https://github.com/WiseLibs/better-sqlite3)'s [`.loadExtension()`](https://github.com/WiseLibs/better-sqlite3/blob/master/docs/api.md#loadextensionurl-entrypoint---this).

```js
import Database from "better-sqlite3";
import * as sqlite_url from "sqlite-url";

const db = new Database(":memory:");
db.loadExtension(sqlite_url.getLoadablePath());
```

It can also be used in [`node-sqlite3`](https://github.com/TryGhost/node-sqlite3)'s [`.loadExtension()`](https://github.com/TryGhost/node-sqlite3/wiki/API#loadextensionurl--callback).

```js
import sqlite3 from "sqlite3";
import * as sqlite_url from "sqlite-url";

const db = new sqlite3.Database(":memory:");
db.loadExtension(sqlite_url.getLoadablePath());
```

This function throws an `Error` in two different cases. The first case is when `sqlite-url` is installed and run on an [unsupported platform](#supported-platforms). The second case is when the platform-specific optional dependency is not installed. If you reach this, ensure you aren't using `--no-optional` flag, and [file an issue](https://github.com/asg017/sqlite-url/issues/new) if you are stuck.

The `db.loadExtension()` function may also throw an Error if the compiled extension is incompatible with your SQLite connection for any reason, including missing system packages, outdated glib versions, or other misconfigurations. If you reach this, please [file an issue](https://github.com/asg017/sqlite-url/issues/new).
