import re

from setuptools import find_namespace_packages, setup

with open("redisim/__init__.py") as f:
    version = re.findall("__version__.*(\d+\.\d+\.\d+).*", f.read())[0]

with open("README.md") as f:
    LONG_DESCRIPTION = f.read()

setup(
    name="redisim",  # package name
    version=version,  # package version
    description="redisim",  # package description
    long_description=LONG_DESCRIPTION,
    long_description_content_type="text/markdown",
    project_urls={
        "Documentation": "http://github.com/lloydzhou/redisim",
        "Code": "http://github.com/lloydzhou/redisim",
        "Issue tracker": "http://github.com/lloydzhou/redisim/issues",
    },
    author="lloydzhou@gmail.com",
    license="MIT",
    keywords=["Redis", "IM", "Websocket"],
    packages=find_namespace_packages(),
    zip_safe=False,
    install_requires=["redis>=4.2.0"],
    python_requires=">=3.8",
)
