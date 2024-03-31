# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import sphinx_fontawesome, textwrap

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'jcc1 Compiler Framework'
copyright = '2024, Kevin Dai, Larry Wu, Owen Zhu'
author = 'Kevin Dai, Larry Wu, Owen Zhu'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx_fontawesome',
    'sphinxawesome_theme.highlighting'
]
templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_static_path = ['_static']
html_css_files = [
    'http://netdna.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css',
    'custom.css'
]
html_js_files = [
    'replaceDxRefs.js'
]
html_theme = "sphinxawesome_theme"

# Tell sphinx what the primary language being documented is.
primary_domain = 'cpp'

# Tell sphinx what the pygments highlight language should be.
highlight_language = 'cpp'
