<?php

/**
 * @file
 * Defines FileSourceMigration class.
 */

/**
 * Class FileSourceMigration.
 *
 * Parent class for all migrations with source data stored in files.
 *
 * @package migrate_html
 */
abstract class FileSourceMigration extends BaseMigration {

  // Stores base directory for legacy files.
  public $baseDir;

  /**
   * Prepare the file source.
   *
   * This will setup $this->source.
   * It will also add field mappings for redirects.
   *
   * All derived classes should define 'fileid' as the source key in
   * MigrateSQLMap(), as it is used to create redirects.
   *
   * @param array $arguments
   *   Arguments provided by migrate module.
   * @param array $source_fields
   *   An associative array of fields contained in the source data, in the form
   *   of array('machine_name' => t('Human Readable')).
   * @param array $source_dirs
   *   An array of source directories, relative to $this->baseDir.
   * @param string $regex
   *   The file mask. Only filenames matching this regex will be migrated.
   * @param array $options
   *   Options that will be passed on to file_scan_directory(). See docs of that
   *   core Drupal function for more information.
   * @param MigrateContentParser $parser
   *   A parser class that will provide file chunk ids. Defaults to
   *   MigrateSimpleContentParser in parent constructor.
   */
  public function __construct($arguments, $source_fields, $source_dirs, $regex, $options = array(), MigrateContentParser $parser = NULL) {
    parent::__construct($arguments);

    $this->baseDir = variable_get('migrate_html_base_dir', LEGACY_WEBROOT);

    // Define the directories containing files to be migrated.
    $absolute_dirs = $this->prependLegacyFilepath($source_dirs);

    // $list_files will provide migrate with a list of all files to be migrated.
    // $item_file provides methods for retrieving a file given an identifier.
    // $this->source define the essential data source from which to migrate.
    $list_files = new MigrateListFiles($absolute_dirs, $this->baseDir, $regex, $options, $parser);
    $item_file = new MigrateItemFile($this->baseDir, TRUE, $parser);
    $this->source = new MigrateSourceList($list_files, $item_file, $source_fields);
  }

  /**
   * Makes relative file paths absolute based on $this->baseDir.
   *
   * @param array $relative_paths
   *   A flat array of relative directory paths.
   *
   * @return array
   *   An array of absolute directory paths, based on $this->base_path.
   */
  public function prependLegacyFilepath(array $relative_paths) {

    $absolute_paths = array();
    foreach ($relative_paths as $key => $relative_path) {
      $absolute_paths[$key] = $this->baseDir . '/' . $relative_path;
    }

    return $absolute_paths;
  }

  /**
   * Generates a legacy file path based on a row's file id.
   *
   * @param object $row
   *   The row being imported.
   */
  public function generateLegacyPath($row) {
    // $row->url_path can be used as an identifier, whereas $row->legacy_path
    // may have multiple values.
    $row->url_path = substr($row->fileid, 1);
    $row->legacy_path = $row->url_path;

    $this->handleIndexRedirect($row);
  }

  /**
   * Handles generation of redirects for index.php or index.html pages.
   */
  public function handleIndexRedirect($row) {
    // Assume not an index until proven otherwise.
    $row->isIndex = FALSE;
    // Check to see if $row->fileid == 'index.html' or 'index.php'.
    if (preg_match('/index\.(htm(l?)|php)/', $row->fileid, $matches)) {
      $row->isIndex = TRUE;
      // Determine root path. E.g., oip/index.html would have the root path
      // /oip.
      $root_path = ltrim(rtrim(str_replace($matches[0], '', $row->fileid), '/'), '/');

      // Check to see if this alias already exists. E.g., if we are migrating
      // oip/index.html, see if an alias exists for /oip.
      if ($system_path = drupal_lookup_path('source', $root_path)) {
        // Continuing /oip example: if an alias does exist for /oip, see if
        // the system path points to an organization node.
        $node = menu_get_object($type = 'node', $position = 1, $system_path);
        if ($node->type == 'organization') {
          // Check to see if a redirect already exists for this path.
          $redirect = redirect_load_by_source($row->legacy_path);
          if (!$redirect) {
            // Create a redirect from /oip/index.html to the organization node.
            $redirect = new stdClass();
            redirect_object_prepare($redirect);
            $redirect->source = $row->legacy_path;
            $redirect->redirect = $system_path;

            redirect_save($redirect);

            // Remove legacy_patch from this node.
            unset($row->legacy_path);
          }
        }
      }
      else {
        $row->legacy_path .= '|' . $root_path;
      }
    }
  }
}
