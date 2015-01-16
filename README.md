## Overview

This module provides a robust library of migration classes that support the
migration of static HTML files into Drupal nodes.

The base classes included in this module allow for two types of custom 
migrations to be created:
* Static HTML to node
* Binary file to file entity

These base classes will do the following
* Scan the defined source directories for matching HTML files
* Instantiate QueryPath object for you source HTML, allowing you to manipulate
  DOM elements and set field values in migrate's prepareRow() method.
* Create a redirect from the legacy HTML path to the new Drupal node.

To implement these classes for your own migration, do the following:
* Enable the migrate_html module. `drush en migrate_html -y`
* Copy the source files (from your legacy, static HTML site) to your local
  machine
* Set the migrate_html_base_dir variable so that it points to the directory
  only your local machine containing the source files:
  `drush vset migrate_html_base_dir "/path/to/legacy/files"`
* To create a node migration class:
  * Create a new include file for your new class. E.g., 
    MyCustomNodeMigration.inc 
  * Add the new include file to the files[] array in your module's .info file
  * Create a new class in your include file by extending 
    StaticHtmlToPageMigration. See ExamplePageMigration in 
    ExamplePageMigration.inc for example implementation.
  * As per the instructions provided by the migrate module, implement 
    hook_migrate_api() in a *.migrate.inc file for your module to register 
    your migration class.
* To create a binary file migration class:
  * Create a new include file for your new class. E.g., 
    MyCustomFileMigration.inc 
  * Add the new include file to the files[] array in your module's .info file
  * Create a new class in your include file by extending 
    BinaryFileMigration. See ExampleBinaryFileMigration in 
    ExampleBinaryFileMigration.inc for example implementation.
  * As per the instructions provided by the migrate module, implement 
    hook_migrate_api() in a *.migrate.inc file for your module to register 
    your migration class.

Additionally, this module provides a strategy for handling 

## OOP Architecture

There are three types of classes that you may need to extend in order to
write a custom migration:
* Migration class. This class is primarily responsible for mapping data from
  source to destination, and for executing the migration. It also deals with all
  processing of Drupal entities.
* SourceParser class. The responsibility of this class is to accept an HTML 
  source file and to create a clean QueryPath object available for DOM 
  manipulation.
* Cleaner classes. These static classes contain functions that the SourceParser
  may use to clean strings or QueryPath object markup.
* Obtainer class. An obtainer class attempts to extract a specific snippet of 
  HTML from a QueryPath object using a stack of finder methods and return it
  to the SourceParser.


## Requirements
* Composer
* migrate module
* redirect module

## Installation

drush en migrate_html

