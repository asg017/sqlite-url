from setuptools import setup

version = {}
with open("datasette_sqlite_url/version.py") as fp:
    exec(fp.read(), version)

VERSION = version['__version__']

setup(
    name="datasette-sqlite-url",
    description="",
    long_description="",
    long_description_content_type="text/markdown",
    author="Alex Garcia",
    url="https://github.com/asg017/sqlite-url",
    project_urls={
        "Issues": "https://github.com/asg017/sqlite-url/issues",
        "CI": "https://github.com/asg017/sqlite-url/actions",
        "Changelog": "https://github.com/asg017/sqlite-url/releases",
    },
    license="MIT License, Apache License, Version 2.0",
    version=VERSION,
    packages=["datasette_sqlite_url"],
    entry_points={"datasette": ["sqlite_url = datasette_sqlite_url"]},
    install_requires=["datasette", "sqlite-url"],
    extras_require={"test": ["pytest"]},
    python_requires=">=3.7",
)