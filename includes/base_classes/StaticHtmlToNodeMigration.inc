<?php

/**
 * @file
 * Defines StaticHtmlToNodeMigration class.
 */

/**
 * Class StaticHtmlToNodeMigration.
 *
 * Parent class for all migrations from static HTML to any content type.
 *
 * @package migrate_html
 */
abstract class StaticHtmlToNodeMigration extends FileSourceMigration {

  private $sourceParserClass;

  /**
   * @var SourceParser $source_parser
   *   The source parser object for a given row.
   */
  protected $sourceParser;

  /**
   * Define $this->source, $this->map, and simple mappings.
   *
   * @param array $arguments
   *   Arguments provided by migrate module.
   * @param array $source_fields
   *   An associative array of fields contained in the source data, in the form
   *   of array('machine_name' => t('Human Readable')).
   * @param array $source_dirs
   *   A flat array of the source directories containing html files.
   * @param string $regex
   *   The file mask. Only file names matching this regex will be migrated.
   *   This will default to '/.*\.htm(l)?$/' if NULL is passed in.
   * @param array $options
   *   Options that will be passed on to file_scan_directory(). See docs of that
   *   core Drupal function for more information.
   * @param MigrateContentParser $parser
   *   A parser class that will provide file chunk ids. Defaults to
   *   MigrateSimpleContentParser in parent constructor.
   */
  public function __construct($arguments, $source_fields, $source_dirs, $regex = NULL, $options = array(), MigrateContentParser $parser = NULL) {
    if ($regex === NULL) {
      $regex = '/.*\.htm(l)?$/';
    }

    // Calling parent constructor will set $this->source and $this->map.
    parent::__construct($arguments, $source_fields, $source_dirs, $regex, $options, $parser);

    // A child class might provide it's own source parser that offer specific
    // cleanup.
    if (array_key_exists('source_parser_class', $arguments)) {
      $this->sourceParserClass = $arguments['source_parser_class'];
    }
    else {
      $this->sourceParserClass = "SourceParser";
    }

    // The destination is the page content type.
    $this->destination = new MigrateDestinationNode('page');

    // Map fields where source field name is same as destination field name.
    $this->addSimpleMappings(array(
      'uid',
      'title',
    ));

    // A map of source HTML filename -> destination node id.
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'fileid' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
        ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Define non-simple field mappings.
    $this->addFieldMapping('workbench_moderation_state_new')->defaultValue('published');

    // Create redirects using relative, legacy path of static file.
    $this->addFieldMapping('migrate_redirects', 'legacy_path')->separator('|');
  }

  /**
   * {@inheritdoc}
   */
  public function prepareRow($row) {
    parent::prepareRow($row);

    // Sets $row->legacy_path and $row->url_path.
    if (empty($row->legacy_path) || empty($row->url_path)) {
      $this->generateLegacyPath($row);
    }

    // Check whether a redirect to this page already exists, if so, do not
    // migrate the "duplicate" content.
    if ($this->isDuplicateByRedirect($row)) {
      return FALSE;
    }

    // Set to admin for now.
    $row->uid = 1;

    // Create a new SourceParser to handle HTML content.
    $this->sourceParser = new $this->sourceParserClass($row->url_path, $row->filedata, FALSE, array(), $this->getArguments());

    // We check that everything has been initialized correctly to keep children
    // classes from having to do all this checks.
    if (!is_object($this->sourceParser->queryPath)) {
      $this->queueMessage(t("@fileid failed to initialize QueryPath", array('@fileid' => $row->fileid)));
      return FALSE;
    }

    $row->title = $this->sourceParser->getTitle();
    $row->body = $this->sourceParser->getBody();
  }

  /**
   * {@inheritdoc}
   */
  public function prepare($entity, $row) {
    migrate_html_prepare_workbench_settings($entity, $row);
  }

  /**
   * {@inheritdoc}
   */
  public function complete($entity, $row) {
    if (isset($entity->nid) && $entity->nid) {
      migrate_html_complete_workbench_settings($entity, $row);
    }
  }

  /**
   * Is this document a duplicate? Determine this by using redirect objects.
   *
   * Legacy paths should not be pointing to more than one node,
   * If this is happening, it is a good sign that we are bringing in duplicate
   * content.
   *
   * @param object $row
   *   A row object as delivered by migrate.
   *
   * @return bool
   *   Whether this row is a duplicate or not.
   */
  protected function isDuplicateByRedirect($row) {
    $parsed = redirect_parse_url($row->legacy_path);
    $source = isset($parsed['path']) ? ltrim($parsed['path'], '/') : '';
    $redirect = redirect_load_by_source($source);

    if ($redirect) {
      $message = "- @source  -> Skipped: Already redirected to '@redirect'.";
      watchdog('migrate_html', $message, array('@source' => $source, '@redirect' => $redirect->redirect), WATCHDOG_WARNING);
      return TRUE;
    }
    return FALSE;
  }
}
