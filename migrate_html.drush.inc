<?php

/**
 * @file
 * Drush commands Migrate HTML.
 */

/**
 * Implements hook_drush_command().
 */
function migrate_html_drush_command() {
  $items = array();

  $items['migrate-images'] = array(
    'description' => "Migrate all image files from a source directory to the public files directory.",
    'aliases' => array('dmi'),
    'arguments' => array(
      'organization' => 'The organization to migrate',
    ),
  );

  $items['migrate-find-html-folders'] = array(
    'description' => "Find all folders that contain html files.",
    'aliases' => array('dmhf'),
    'arguments' => array(
      'organization' => 'The organization to migrate',
    ),
    'options' => array(
      'ignore' => 'Comma delimited list of strings in folder to ignore',
    ),
  );

  $items['migrate-find-file-folders'] = array(
    'description' => "Find all folders that contain binary/not-image files.",
    'aliases' => array('dmff'),
    'arguments' => array(
      'organization' => 'The organization to migrate',
    ),
    'options' => array(
      'ignore' => 'Comma delimited list of strings in folder to ignore',
    ),
  );

  return $items;
}

/**
 * Drush command callback.
 */
function drush_migrate_html_migrate_images($organization) {
  // Let's get our source directory, and our output directory.
  $source_directory = variable_get("migrate_html_base_dir");
  if (!$source_directory) {
    throw new Exception("the migrate_html_base_dir variable has not been defined");
  }

  if ($wrapper = file_stream_wrapper_get_instance_by_uri('public://')) {
    $output_directory = $wrapper->realpath();
  }
  else {
    throw new Exception("We couldn't get the absolute path for public://");
  }

  migrate_html_move_images($source_directory, $organization, $output_directory);
}

/**
 * Drush command callback.
 */
function drush_migrate_html_migrate_find_html_folders($organization) {
  $ignore = drush_get_option('ignore');
  $ignore = explode(",", $ignore);
  drush_print_r(migrate_html_find_folders_with_files($organization, array("htm", "html"), $ignore));
}

/**
 * Drush command callback.
 */
function drush_migrate_html_migrate_find_file_folders($organization) {
  $ignore = drush_get_option('ignore');
  $ignore = explode(",", $ignore);
  drush_print_r(migrate_html_find_folders_with_files($organization,
    array("pdf", "txt", "rtf", "doc", "docx", "xls", "xlsx", "csv", "mp3",
      "mp4", "wpd", "wp", "qpw", "xml", "ppt", "pptx"), $ignore));
}

/**
 * Find all the folders inside of org migraton souce that match the extensions.
 *
 * @param string $organization
 *   The org abbreviations for the org we want to process.
 * @param array $exts
 *   The extension of the type of files that we want to find.
 * @param array $ignore
 *   A list of words that if found in a folder path will ignore it from the
 *   list.
 *
 * @return string
 *   A JSON string of an array containing all of the folders that contain files
 *   of the given extensions.
 *
 * @throws Exception
 *   If migrate_html_base_dir variable is not set.
 */
function migrate_html_find_folders_with_files($organization, $exts, $ignore = array()) {
  $source_directory = variable_get("migrate_html_base_dir");
  if (!$source_directory) {
    throw new Exception("the migrate_html_base_dir variable has not been defined");
  }
  $organization_dir = $source_directory . "/{$organization}";

  $file_types = $exts;

  $files = array();

  foreach ($file_types as $ft) {
    $output = shell_exec("find {$organization_dir} -type f -name '*.{$ft}'");
    $files = array_merge($files, explode("\n", $output));
  }

  $folders = array();
  // Let's get all the folders.
  foreach ($files as $f) {
    $file = str_replace($source_directory . "/", "", $f);
    $pieces = explode("/", $file);

    // Remove the file part.
    $pieces = array_slice($pieces, 0, count($pieces) - 1);
    $key = implode("/", $pieces);
    $ignore_folder = migrate_html_ignore_folder($key, $ignore);
    if (!empty($key) && !$ignore_folder) {
      $folders[$key] = TRUE;
    }
  }

  return var_export(array_keys($folders));
}

/**
 * Ignore this folder, or not.
 *
 * @param string $folder
 *   a string representing a folder.
 * @param array $ignore
 *   A list of string to fine in folders to ignore.
 *
 * @return bool
 *   Should this folder be ignored.
 */
function migrate_html_ignore_folder($folder, $ignore = array()) {
  $ignore_it = FALSE;

  foreach ($ignore as $string) {
    if (!empty($string) && substr_count($folder, $string) > 0) {
      $ignore_it = TRUE;
      break;
    }
  }

  return $ignore_it;
}

/**
 * Move all the images from a directory tree to an output directory.
 *
 * The directory structure in the output directory will be an exact match of
 * the directory structure from the source directory.
 *
 * @param string $parent_input_directory
 *   The parent directory where another directory with images is located.
 * @param string $directory
 *   The directory that contains images.
 * @param string $parent_output_directory
 *   The directory where we want to save all the images.
 */
function migrate_html_move_images($parent_input_directory, $directory, $parent_output_directory) {

  // Make the directory where our files will go.
  $final_output_directory = "$parent_output_directory/$directory";

  // Get all files from the source directory.
  $final_input_directory = "$parent_input_directory/$directory";
  $files = scandir($final_input_directory);
  $count = 0;
  foreach ($files as $file) {
    $ext = pathinfo("$final_input_directory/$file", PATHINFO_EXTENSION);
    $ext = strtoupper($ext);
    // If any of the files are images, copy them.
    if ($ext == "GIF" || $ext == "PNG" || $ext == "JPG" || $ext == "JPEG") {
      // We only want to create the output directory if we have images.
      if (!file_exists($final_output_directory)) {
        mkdir($final_output_directory, 0777, TRUE);
      }

      watchdog("migrate_html", "FILE: @file EXT: @ext \n", array('@file' => $file, 'ext' => $ext));

      if (copy("$final_input_directory/$file", "$final_output_directory/$file")) {
        $count++;
        watchdog("migrate_html", "File @file was copied to $final_output_directory \n", array('@file' => $file));
      }
      else {
        watchdog("migrate_html", "There was an error copying @file \n", array('@file' => $file), WATCHDOG_ERROR);
      }
    }
    // If we are dealing with a directory, let's recurse.
    elseif (is_dir("$final_input_directory/$file") && "{$file}" != "." && "{$file}" != "..") {
      watchdog("migrate_html", "Found a directory {$file} \n");
      migrate_html_move_images($final_input_directory, $file, $final_output_directory);
    }
  }
  if ($count > 0) {
    $message = dt('Moved @count images into -> @final_output_directory.', array('@count' => $count, '@final_output_directory' => $final_output_directory));
    drush_log($message, 'success');
  }
}

/**
 * Wrapper function to output info if migrate_html_drush_debug = TRUE.
 *
 * @param mixed $output
 *   Thing to pass to drush_print_r().
 */
function drush_migrate_html_debug_output($output = '') {
  if (variable_get('migrate_html_drush_debug', FALSE)) {
    drush_print_r($output);
  }
}
