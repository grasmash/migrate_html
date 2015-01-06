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

    // Define source directories.
    $source_dirs = array(
      'example',
    );

    $arguments['source_parser_class'] = "ExampleSourceParser";

    $options = array('recurse' => TRUE);

    // Parent constructor will set $this->source, $this->destination, and
    // $this->map.
    parent::__construct($arguments, $source_dirs, NULL, $options);
    $this->description = t('Example page migration.');
  }

  /**
   * {@inheritdoc}
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}
