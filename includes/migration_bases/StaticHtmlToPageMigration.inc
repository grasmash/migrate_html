<?php

/**
 * @file
 * Defines StaticHtmlToPageMigration class.
 */

/**
 * Class StaticHtmlToPageMigration.
 *
 * Parent class for all migrations from static HTML to page content type.
 *
 * @package migrate_html
 */
abstract class StaticHtmlToPageMigration extends StaticHtmlToNodeMigration {

  /**
   * Define $this->source, $this->map, $this->destination, and simple mappings.
   *
   * @param array $arguments
   *   Arguments provided by migrate module.
   * @param array $source_dirs
   *   A flat array of the source directories containing html files.
   * @param string $regex
   *   The file mask. Only file names matching this regex will be migrated.
   *   This will default to '/.*\.htm(l)?$/' in StaticHtmlToNodeMigration
   *   if NULL is passed in.
   * @param array $options
   *   Options that will be passed on to file_scan_directory(). See docs of that
   *   core Drupal function for more information.
   */
  public function __construct($arguments, $source_dirs, $regex = NULL, $options = array()) {

    // Define the fields that will be derived from the static files.
    $source_fields = array(
      'title' => t('Title'),
      'body' => t('Body'),
    );

    // Calling parent constructor will set $this->source and $this->map.
    parent::__construct($arguments, $source_fields, $source_dirs, $regex, $options);

    // The destination is the page content type.
    $this->destination = new MigrateDestinationNode('page');

    // Define non-simple field mappings.
    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('body:format')->defaultValue('wysiwyg');
  }
}
