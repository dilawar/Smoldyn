# Configuration file for the Sphinx documentation builder.
#
# DO NOT edit this file. Edit conf.py.in instead.
# This file is generated by CMake from conf.py.in
#

import subprocess
from pathlib import Path
import time
import sys
import os

#
# Modify os.path so that sphinx can locate smoldyn.
#
sdir_ = Path(__file__).parent.resolve()
sys.path.insert(0, str(sdir_ / ".." / "source" / "python"))
sys.path.insert(0, str(sdir_ / ".." / "examples" / "S15_python"))

assert Path(os.getcwd()) == sdir_, f"This script can only run {sdir_}"

print(sys.path)
import smoldyn

print(smoldyn.__version__)

# RTD doesn't uinderstand cmake. Implement the cmake flow here as well. Make
# sure that make does not replace
with (sdir_ / "Doxyfile.in").open("r") as f:
    txt = f.read()

assert txt

# make sure cmake does not replace these.
txt = txt.replace(r"@DOXYGEN_OUTPUT_DIR@", r"build_doxygen/doxygen")
txt = txt.replace(r"@DOXYGEN_INPUT_DIRS@", r"..")
with (sdir_ / "_Doxyfile").open("w") as f:
    f.write(txt)

# builds doxygen on READTHEDOCS
if os.environ.get("READTHEDOCS", None) is not None:
    time.sleep(0.1)
    subprocess.call("mkdir -p build_doxygen; doxygen _Doxyfile", shell=True)

# -- Project information -----------------------------------------------------

project = "Smoldyn"
copyright = "2020, Steven S. Andrews"
author = "Steven S. Andrews"

# The full version, including alpha/beta/rc tags
version = smoldyn.version()
release = "latest"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom ones.
extensions = [
    "sphinx.ext.autodoc",
    "sphinx_autodoc_typehints",
    "sphinx.ext.autosummary",
    "sphinx.ext.imgmath",
    "sphinx.ext.napoleon",
    "sphinx.ext.todo",
    "sphinx.ext.viewcode",
    "sphinxcontrib.programoutput",
    "sphinxcontrib.inlinesyntaxhighlight",
    "sphinx.ext.autosectionlabel",
    "sphinxcontrib.tikz",
    "sphinx_rtd_theme",
    "recommonmark",
    "breathe",
]

latex_engine = "xelatex"
imgmath_latex = "xelatex"
# imgmath_image_format = 'svg'

todo_include_todos = True

autosectionlabel_prefix_document = True
autosectionlabel_maxdepth = 2

autoclass_content = "both"
autodoc_default_options = {
    "member-order": "bysource",
    #  "special-members": "__init__",
    "undoc-members": True,
    "show_inheritance": True,
    "inherited-members": True,
}

autosummary_generate = True

breathe_default_project = "smoldyn"

tikz_tikzlibraries = "positioning,arrows.meta"

# generated by doxygen.
html_extra_path = ["build_doxygen"]

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", "*.md"]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "sphinx_rtd_theme"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]

source_suffix = {
    ".rst": "restructuredtext",
    ".txt": "markdown",
    ".md": "markdown",
}
