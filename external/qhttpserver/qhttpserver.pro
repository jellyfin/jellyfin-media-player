CONFIG += ordered

TEMPLATE = subdirs

SUBDIRS += src \
           examples

examples.depends = src
