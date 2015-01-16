<?php

/**
 * @file
 * Defines migration classes for Executive Office for Immigration Review.
 */

/**
 * Migrates .html files from /example directory to page nodes.
 *
 * @package migrate_html
 * @subpackage example
 */
class ExamplePageMigration extends StaticHtmlToPageMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {

    // Define source directories, relative to directory defined in
    // migrate_html_base_dir variable.
    $source_dirs = array(
      'example',
    );

    // Optionally defined a custom source parser class.
    // Defaults to SourceParser. The default SourceParser class will attempt
    // to clean the HTML markup. It also defines default methods for setting
    // and getting the title and body from markup. These methods can be
    // overridden by defining your own class here.
    // @see ExampleSimpleSourceParser
    // @see SourceParser
    $arguments['source_parser_class'] = "ExampleSimpleSourceParser";

    // Optionally set an array of options to be passed to Migrate.
    $options = array('recurse' => TRUE);

    // Parent constructor will set $this->source, $this->destination, and
    // $this->map.

    // Optionally define a regex for filtering source files. Defaults
    // to '/.*\.htm(l)?$/' in a parent class.
    $regex = NULL;

    parent::__construct($arguments, $source_dirs, $regex, $options);
    $this->description = t('Example page migration.');

    // Set your own field mappings. Maps $row->special_field_markup value in
    // prepareRow() to the Drupal field field_my_special_field.
    $this->addFieldMappings('field_my_special_field', 'special_field_markup');
  }

  /**
   * {@inheritdoc}
   */
  public function prepareRow($row) {

    // Optionally define an array of files that should specifically be skipped.
    // This is often simpler than creating a regex in the __contruct() method.
    $skip_these = array(
      'path/to/file/to/skip.html',
    );

    if (parent::prepareRow($row) === FALSE || migrate_html_skip_files($row->legacy_path, $skip_these)) {
      return FALSE;
    }

    // We can use a method defined by our custom source parser class to get
    // the markup for our special field.
    // @see ExampleSimpleSourceParser
    $row->special_field_markup = $this->SourceParser->getSpecialFieldMarkup();
  }
}
