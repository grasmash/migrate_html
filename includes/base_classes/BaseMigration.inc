<?php

/**
 * @file
 * Contains base migration class for all other migrations.
 */

/**
 * The default system path for the legacy webroot. Can be overridden via
 * migrate_html_base_dir variable.
 */
define('LEGACY_WEBROOT', DRUPAL_ROOT . '/../../legacy-site');

/**
 * Abstract intermediate class holding common settings.
 *
 * @package migrate_html
 */
abstract class BaseMigration extends Migration {

  /**
   * {@inheritdoc}
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->mergeArguments($arguments);

    $this->team = array(
      new MigrateTeamMember('Matthew Grasmick', 'matthew.grasmick@acquia.com', t('Architect')),
    );
  }

  /**
   * Add multiple field mappings at once.
   *
   * @param array $mappings
   *   An array of field mappings in the form of source_key => dest_key.
   *
   * @param bool $warn_on_override
   *   Set to FALSE to prevent warnings when there's an existing mapping.
   */
  public function addFieldMappings(array $mappings, $warn_on_override = TRUE) {
    foreach ($mappings as $source => $destination) {
      $this->addFieldMapping($source, $destination, $warn_on_override);
    }
  }

  /**
   * Arguments in keyed array passed among migration classes and parsers.
   *
   * @var array $arguments
   */
  protected $arguments = array();

  /**
   * Basic getter for $arguments.
   *
   * @return array
   *   Whatever has been stored in $this->arguments.
   */
  public function getArguments() {
    return $this->arguments;
  }
  /**
   * Merges an array into the existing arguments array.
   *
   * @param array $new_args
   *   An array matching the format of the arguments array, to be merged.
   *
   * @return array
   *   Array of the complete arguments array with the new material.
   */
  protected function mergeArguments($new_args) {
    return array_merge($this->getArguments(), $new_args);
  }

  /**
   * Gets a single argument from the arguments array.
   *
   * @param string $arg_key
   *   The key of the item to return from the Arguments array.
   *
   * @return mixed
   *   Whatever is stored in the $keys's value, or NULL if not in the arguments.
   */
  protected function getArgument($arg_key = '') {
    if (!empty($arg_key)) {
      $args = $this->getArguments();
      if (array_key_exists($arg_key, $args)) {
        return $args[$arg_key];
      }
    }
    return NULL;
  }

  /**
   * Unsets NULL properties on a single dimensional object.
   *
   * @param obj $row
   *   The object to iterate over.
   */
  public function removeEmptyProperties(&$row) {
    foreach ($row as $key => $property) {
      if (is_null($property)) {
        unset($row->$key);
      }
      elseif (is_string($property) && !$property) {
        unset($row->$key);
      }
    }
  }

  /**
   * Adds path and path auto field mapping to 'Do not migrate' issue group.
   */
  public function addPathDnmMapping() {
    // Map path and pathauto fields to NULL, assigning to DNM.
    if (module_exists('path')) {
      $this->addFieldMapping('path')
        ->issueGroup(t('DNM'));
      if (module_exists('pathauto')) {
        $this->addFieldMapping('pathauto')
          ->issueGroup(t('DNM'));
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function prepareRow($row) {
    parent::prepareRow($row);

    // Prepare title property by trimming to 255 chars and fixing encoding.
    if (!empty($row->title)) {
      $row->title = mb_strimwidth($row->title, 0, 255, "...");
      $row->title = html_entity_decode($row->title, ENT_QUOTES, 'UTF-8');
    }
  }

  /**
   * Geocode a string using geocoder module.
   *
   * @param string $string
   *   A location string.
   *
   * @return array
   *   An array with location information extracted from the string.
   */
  public function geoCodeString($string) {

    // Geocode the location and parse into structured data for migration.
    // Geocoder module is not an explicit dependency because most migrations
    // do not rely on it. It should be disabled after use.
    if (!empty($string)) {
      if ($string == 'Washington, D.C.') {
        // The most common entry, so skip geocoding.
        $address['locality'] = 'Washington';
        $address['administrative_area_level_1'] = 'DC';
        $address['country'] = "US";
      }
      elseif (module_exists('geocoder')) {
        // Note that calling this too many times (as in very large migrations)
        // may exceed the API request limit for geocoder's source data.
        $point = geocoder('google', $string);
        module_load_include('inc', 'migrate_html', 'includes/migrate_html');
        $address = migrate_html_convert_geocoded_point_to_address($point);

        if (!$address) {
          $address['locality'] = '';
          $address['administrative_area_level_1'] = '';
          $address['country'] = '';
          $this->queueMessage(t('Could not look up speech location because geocoder returned nothing The API request limit may have been exceeded.'));
        }
      }
      else {
        $this->queueMessage(t('Could not look up speech location because geocoder is not installed.'));
      }

      return $address;
    }
  }

  /**
   * Extracts metadata from pdf file.
   *
   * @param string $pdf_file_path
   *   The absolute file path of the pdf on the local system.
   *
   * @return array
   *   An associative array of pdf metadata.
   */
  public function getPdfMetadata($pdf_file_path) {
    try {
      $pdfinfo_bin = variable_get('pdfinfo_binary', '/home/user/xpdf/bin64/pdfinfo');
      $file_arg = escapeshellarg($pdf_file_path);
      $command = "$pdfinfo_bin $file_arg";
      $output = shell_exec($command);
      $pdf_info_rows = explode("\n", $output);
      $pdf_metadata = array();
      foreach ($pdf_info_rows as $row) {
        $columns = explode(':', $row);
        $pdf_metadata[$columns[0]] = $columns[1];
      }

      return $pdf_metadata;
    }
    catch (Exception $e) {
      $message = t('Could not parse attached PDF for @fileid. Exception thrown: @exception', array('@fileid' => $pdf_file_path, '@exception' => $e->getMessage()));
      $this->queueMessage($message);
      watchdog('migrate_html', $message);
    }
  }

  /**
   * Extracts the contents of a pdf file.
   *
   * @param string $pdf_file_path
   *   The absolute file path of the pdf on the local system.
   *
   * @return string
   *   The text content of the pdf.
   *
   * @throws \XPDF\Exception\BinaryNotFoundException
   */
  public function getPdfContents($pdf_file_path) {
    try {
      // Get PDF contents.
      $pdftotext_bin = variable_get('pdftotext_binary', '/home/user/xpdf/bin64/pdftotext');
      $pdf_parser = XPDF\PdfToText::create(array('pdftotext.binaries' => $pdftotext_bin));
      $pdf_contents = $pdf_parser->getText($pdf_file_path);

      return $pdf_contents;
    }
    catch (Exception $e) {
      $message = t('Could not parse attached PDF for @fileid. Exception thrown: @exception', array('@fileid' => $pdf_file_path, '@exception' => $e->getMessage()));
      $this->queueMessage($message);
      watchdog('migrate_html', $message);
    }
  }

  /**
   * Creates redirects for files attached to a given entity's field field.
   *
   * @param object $entity
   *   The fully loaded entity.
   *
   * @param array $source_urls
   *   A flat array of source urls that should redirect to the attachments
   *   on this entity. $source_urls[0] will redirect to the first attachment,
   *   $entity->$field_name[$language][0], and so on.
   *
   * @param string $field_name
   *   The machine name of the attachment field.
   *
   * @param string $language
   *   Optional. Defaults to LANGUAGE_NONE.
   */
  public static function createAttachmentRedirect($entity, $source_urls, $field_name, $language = LANGUAGE_NONE) {
    if (empty($source_urls)) {
      // Nothing to be done here.
      $json_entity = json_encode($entity);
      watchdog("migrate_html", "redirect was not created for attachment in entity {$json_entity}");
      return;
    }

    $field = $entity->$field_name;
    if (!empty($field[$language])) {
      foreach ($field[$language] as $delta => $item) {
        $file = file_load($item['fid']);
        $destination = file_create_url($file->uri);
        $source = $source_urls[$delta];

        // Create redirect.
        $redirect = redirect_load_by_source($source);
        if (!$redirect) {
          $redirect = new stdClass();
          redirect_object_prepare($redirect);
          $redirect->source = $source;
          $redirect->redirect = $destination;
          redirect_save($redirect);
        }
      }
    }
  }

  /**
   * Deletes any redirects associated files attached to an entity's file field.
   *
   * @param object $entity
   *   The fully loaded entity.
   *
   * @param string $field_name
   *   The machine name of the attachment field.
   *
   * @param string $language
   *   Optional. Defaults to LANGUAGE_NONE.
   */
  public static function rollbackAttachmentRedirect($entity, $field_name, $language = LANGUAGE_NONE) {
    $field = $entity->$field_name;
    if (!empty($field[$language])) {
      foreach ($field[$language] as $delta => $item) {
        $file = file_load($item['fid']);
        $destination = file_create_url($file->uri);
        redirect_delete_by_path($destination);
      }
    }
  }
}
