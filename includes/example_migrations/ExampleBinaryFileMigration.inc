<?php

/**
 * Migrates files (*.pdf) from /example.
 *
 * @package migrate_html
 * @subpackage example
 */
class ExampleBinaryFileMigration extends BinaryFileMigration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    $this->description = t('Migrates non-image files from the example subdirectory.');

    // Define source directories, relative to directory defined in
    // migrate_html_base_dir variable.
    $source_dirs = array(
      'example',
    );

    $regex = '/.*\.(pdf|txt|rtf|doc|docx|xls|xlsx|csv|mp3|mp4|wpd|wp|qpw|xml|ppt|pptx)/';

    // Define a destination directory using a stream wrapper.
    $dest_dir = 'public:///example/docs';

    // Optionally set an array of options to be passed to Migrate.
    $options = array('recurse' => TRUE);

    // This will setup $this->map, $this->destination, and $this->source.
    // It will also add field mappings for file location and redirects, which
    // relies on prepareRow() defining $row->filepath and $row->legacy_path.
    parent::__construct($arguments, $source_dirs, $dest_dir, $regex, $options);
  }
}
