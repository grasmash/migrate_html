<?php

/**
 * @file
 * Includes SourceParser class, which parses static HTML files via queryPath.
 */

// composer_manager is supposed to take care of including this library, but
// it doesn't seem to be working.
require DRUPAL_ROOT . '/sites/all/vendor/querypath/querypath/src/qp.php';

/**
 * Class SourceParser.
 *
 * @package migrate_html
 */
class SourceParser {

  /**
   * @var array $args
   *   Arguments array passed from migration class.
   */
  protected $arguments = array();
  protected $body;
  protected $date;
  protected $fileId;
  protected $id;
  public $queryPath;
  protected $subTitle = '';
  protected $title;

  /**
   * Constructor.
   *
   * @param string $file_id
   *   The file id, e.g. careers/legal/pm7205.html
   * @param string $html
   *   The full HTML data as loaded from the file.
   * @param bool $fragment
   *   Set to TRUE if there are no <html>,<head>, or <body> tags in the HTML.
   * @param array $qp_options
   *   An associative array of options to be passed to the html_qp() function.
   * @param array $arguments
   *   An associative array arguments passed up from the migration class.
   */
  public function __construct($file_id, $html, $fragment = FALSE, $qp_options = array(), $arguments = array()) {
    $this->mergeArguments((array) $arguments);

    $html = StringCleanUp::fixEncoding($html);

    // Strip Windows' CR chars.
    $html = StringCleanUp::stripWindowsCRChars($html);
    $html = StringCleanUp::fixWindowSpecificChars($html);

    $this->initQueryPath($html, $qp_options);

    $this->fileId = $file_id;
    // Runs the methods defined in getParseOrder to set Title, date, body, etc.
    $this->runParserOrder($this->getParseOrder());
  }

  /**
   * Create the queryPath object.
   */
  protected function initQueryPath($html, $qp_options) {
    $type_detect = array(
      'UTF-8',
      'ASCII',
      'ISO-8859-1',
      'ISO-8859-2',
      'ISO-8859-3',
      'ISO-8859-4',
      'ISO-8859-5',
      'ISO-8859-6',
      'ISO-8859-7',
      'ISO-8859-8',
      'ISO-8859-9',
      'ISO-8859-10',
      'ISO-8859-13',
      'ISO-8859-14',
      'ISO-8859-15',
      'ISO-8859-16',
      'Windows-1251',
      'Windows-1252',
      'Windows-1254',
    );
    $convert_from = mb_detect_encoding($html, $type_detect);

    if (!$qp_options) {
      $qp_options = array(
        'convert_to_encoding' => 'UTF-8',
        'convert_from_encoding' => $convert_from,
      );
    }

    // Create query path object.
    $this->queryPath = htmlqp($html, NULL, $qp_options);
  }

  /**
   * Set the html var after some cleaning.
   */
  protected function cleanHtml() {
    try {
      HtmlCleanUp::convertRelativeSrcsToAbsolute($this->queryPath, $this->fileId);

      // Clean up specific to the example site.
      HtmlCleanUp::stripOrFixLegacyElements($this->queryPath, $this->arguments);
    }
    catch (Exception $e) {
      watchdog('migrate_html', '%file: failed to clean the html, Error: %error', array(
        '%file' => $this->fileId,
        '%error' => $e->getMessage(),
      ), WATCHDOG_ERROR);
    }
  }

  /**
   * Getter.
   */
  public function getDate() {
    return $this->date;
  }

  /**
   * Setter.
   */
  protected function setDate($override = '') {
    if (empty($override)) {
      // Default stack: Use this if none was defined in
      // $arguments['obtainer_methods'].
      $default_target_stack = array();

      $om = $this->getObtainerMethods('date');
      $date_find_stack = (!empty($om)) ? $om : $default_target_stack;
      $obtained_date = new ObtainDate($this->queryPath, $date_find_stack);
      $date = $obtained_date->formatDate('n/d/Y');
      $date_string_raw = $obtained_date->getText();
    }
    else {
      // The override was invoked, so use it.
      $date = $override;
      $date_string_raw = $override;
    }

    $this->date = $date;
    // Output to show progress to aid debugging.
    drush_migrate_html_debug_output("--date--> Raw:{$date_string_raw}  Formatted:{$this->getDate()}");
  }


  /**
   * Getter.
   */
  public function getID() {
    return $this->id;
  }


  /**
   * Setter.
   */
  protected function setID($override = '') {
    if (empty($override)) {
      // Default stack: Use this if none was defined in
      // $arguments['obtainer_methods'].
      $default_target_stack = array();

      $id = $this->runObtainer('ObtainID', 'id', $default_target_stack);
    }
    else {
      // The override was invoked, so use it.
      $id = $override;
    }

    $this->id = $id;
    // Output to show progress to aid debugging.
    drush_migrate_html_debug_output("--id-->  {$this->getID()}");
  }


  /**
   * Sets $this->title using breadcrumb or <title>.
   */
  protected function setTitle($override = '') {

    // QueryPathing our way through things can fail.
    // In that case let's still set things and inform people about the issue.
    try {
      if (empty($override)) {
        // Default stack: Use this if none was defined in
        // $arguments['obtainer_methods'].
        $default_target_stack = array(
          'findClassBreadcrumbMenuContentLast',
          'findTitleTag',
          'findH1First',
        );

        $target_stack = $this->getObtainerMethods('title');
        $title_find_stack = (!empty($target_stack)) ? $target_stack : $default_target_stack;
        $obtained_title = new ObtainTitle($this->queryPath, $title_find_stack);
        $title = $obtained_title->getText();

        // Check for discarded text and for a setSubTitle method.
        // Some extensions may have them, but the base class does not.
        $discarded = $obtained_title->getTextDiscarded();
        if (!empty($discarded) && method_exists($this, 'setSubTitle')) {
          // Put the discarded text into the subtitle.  It might not be right,
          // but at least it is not lost.
          $this->setSubTitle($discarded);
        }
      }
      else {
        // The override was invoked, so use it.
        $title = $override;
      }

      $this->title = $title;
      // Output to show progress to aid debugging.
      drush_migrate_html_debug_output("{$this->fileId}  --->  {$this->title}");
    }
    catch (Exception $e) {
      $this->title = "";
      watchdog('migrate_html', '%file: failed to set the title', array('%file' => $this->fileId), WATCHDOG_ALERT);
      drush_migrate_html_debug_output("ERROR: {$this->fileId} failed to set title.");
    }
  }


  /**
   * Return the title for this content.
   */
  public function getTitle() {
    // The title gets set in the constructor, no need to check here.
    return $this->title;
  }


  /**
   * Getter.
   */
  public function getSubTitle() {
    return $this->subTitle;
  }


  /**
   * Get the body from html and set the body var.
   *
   * @param string $body
   *   (optional) The body value to be set. Defaults to finding body via
   *   default method stack.
   */
  public function setBody($body = '') {
    if (empty($body)) {
      // Default stack: Use this if none was defined in
      // $arguments['obtainer_methods'].
      $default_method_stack = array(
        'findTopBodyHtml',
        'findClassContentSub',
      );
      $body = $this->runObtainer('ObtainBody', 'body', $default_method_stack);
    }

    $this->body = $body;

    // Output to show progress to aid debugging.
    if (!empty($this->body)) {
      drush_migrate_html_debug_output("--body-->  found something");
    }
    else {
      drush_migrate_html_debug_output("--body-->  FOUND NOTHING");
    }
  }


  /**
   * Return content of <body> element.
   */
  public function getBody() {
    // The body gets set in the constructor, no need to check for existence.
    return $this->body;
  }

  /**
   * Returns and removes last updated date from markup.
   *
   * @return string
   *   The update date.
   */
  public function extractUpdatedDate() {
    // Default stack: Use this if none was defined in
    // $arguements['obtainer_methods'].
    $default_target_stack = array(
      'findClassLastupdate',
    );

    $method = $this->getObtainerMethods('date_updated');
    $date_find_stack = (!empty($method)) ? $method : $default_target_stack;
    $obtained_date = new ObtainDate($this->queryPath, $date_find_stack);
    $date = $obtained_date->getText();

    return $date;
  }

  /**
   * Returns the contents of <a href="mailto:*" /> elements in <body>.
   *
   * @return null|string
   *   A string of email addresses separated by pipes.
   */
  public function getEmailAddresses() {
    $query_path = $this->queryPath;
    $anchors = $query_path->find('a[href^="mailto:"]');
    if ($anchors) {
      $email_addresses = array();
      foreach ($anchors as $anchor) {
        $email_addresses[] = $anchor->text();
      }
      $email_addresses = implode('|', $email_addresses);
      return $email_addresses;
    }

    return NULL;
  }

  /**
   * Crude search for strings matching US States.
   */
  public function getUsState() {
    $query_path = $this->queryPath;
    $states_blob = trim(file_get_contents(drupal_get_path('module', 'migrate_html') . '/sources/us-states.txt'));
    $states = explode("\n", $states_blob);
    $elements = $query_path->find('p');
    foreach ($elements as $element) {
      foreach ($states as $state) {
        list($abbreviation, $state_title) = explode('|', $state);
        if (strpos(strtolower($element->text()), strtolower($state_title)) !== FALSE) {
          return $abbreviation;
        }
      }
    }
    return NULL;
  }

  /**
   * Swaps one element tag for another.
   *
   * @param array $selectors
   *   Associative array keyed by original value. E.g., changing all h4 tags
   *   for strong tags would require array('h4' => 'strong').
   */
  public function changeElementTag($selectors) {
    foreach ($selectors as $old_selector => $new_selector) {
      $elements = $this->queryPath->find($old_selector);
      foreach ($elements as $element) {
        $element->wrapInner('<' . $new_selector . '></' . $new_selector . '>');
        $element->children($new_selector)->first()->unwrap($old_selector);
      }
    }
  }

  /**
   * Get specific tds from a table.
   *
   * @param object $table
   *   A query path object with a table as the root.
   * @param int $tr_target
   *   Which tr do you want. Starting the count from 1.
   * @param int $td_target
   *   Which td do you want. Starting the count from 1.
   *
   * @return string
   *   The text inside of the wanted tr and td.
   */
  public function getFromTable($table, $tr_target, $td_target) {
    $trcount = 1;
    $tdcount = 1;

    foreach ($table->find("tr") as $tr) {
      if ($trcount == $tr_target) {
        foreach ($tr->find("td") as $td) {
          if ($tdcount == $td_target) {
            return $td->text();
          }
          $tdcount++;
        }
      }
      $trcount++;
    }

    return "";
  }


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
   * Merges an array into the arguments array.
   *
   * @param array $new_args
   *   An array matching the format of the arguments array, to be merged.
   */
  protected function mergeArguments($new_args) {
    if (!empty($new_args) && is_array($new_args)) {
      $this->arguments = array_merge($this->getArguments(), $new_args);
    }
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
  protected function getArgument($arg_key) {
    if (!empty($arg_key)) {
      $args = $this->getArguments();
      if (array_key_exists($arg_key, $args)) {
        return $args[$arg_key];
      }
    }
    return array();
  }


  /**
   * Gets the specified obtainer methods from the arguments.
   *
   * @param string $obtainer_methods_key
   *   The key for the obtainer methods (ex: title, body, date).
   *
   * @return array
   *   Returns the array of obtainer methods for a specific key or empty array.
   */
  protected function getObtainerMethods($obtainer_methods_key) {
    $obtainer = $this->getArgument('obtainer_methods');
    if (array_key_exists($obtainer_methods_key, $obtainer)) {
      return $obtainer[$obtainer_methods_key];
    }

    return array();
  }


  /**
   * Adds an array of obtainer method arrays overwriting existing methods.
   *
   * @param array $obtainer_methods
   *   An array of obtainer method arrays.
   */
  protected function setObtainerMethods($obtainer_methods = array()) {
    if ((!empty($obtainer_methods)) && (is_array($obtainer_methods))) {
      $args = $this->getArguments();
      // Loop through new array of obtainer methods.
      foreach ($obtainer_methods as $obtainer_methods_key => $stack) {
        // Put the new into the original.
        $args['obtainer_methods'][$obtainer_methods_key] = $stack;
      }
      // Put the newly formed args back.
      $this->mergeArguments($args);
    }
  }

  /**
   * Defines and returns an array of parsing methods to call in order.
   *
   * @return array
   *   An array of parssing methods to call in order.
   */
  public function getParseOrder() {
    // Specify the parsing methods that should run in order.
    return array(
      // Getting the title relies on html that could be wiped during clean up
      // so let's get it before we clean things up.
      'setTitle',
      // The title is set, so let's clean our html up.
      'cleanHtml',
      // With clean html we can get the body out.
      'setBody',
    );
  }

  /**
   * Runs an obtainer and returns the text it found.
   *
   * @param string $obtainer_class
   *   The name of the obtainer class to use.
   * @param string $obtainer_methods_key
   *   The key for the obtainer arguments array to see which findMethods to run.
   * @param array $default_method_stack
   *   (optional) The default stack of findMethods to use if none is available
   *   from the arguments.
   *
   * @return string
   *   The string retrieved by executing the obtainer.
   */
  public function runObtainer($obtainer_class, $obtainer_methods_key, $default_method_stack = array()) {
    $text = '';

    try {
      $method_stack = $this->getObtainerMethods($obtainer_methods_key);
      if (empty($method_stack)) {
        $method_stack = $default_method_stack;
      }
      $obtained = new $obtainer_class($this->queryPath, $method_stack);
      $text = $obtained->getText();
    }
    catch (Exception $e) {
      watchdog('migrate_html', '%file: failed to set the %obtainer_methods_key because: %message', array(
        '%obtainer_methods_key' => $obtainer_methods_key,
        '%file' => $this->fileId,
        '%message' => $e->getMessage(),
      ), WATCHDOG_ALERT);
      drush_migrate_html_debug_output("ERROR: {$this->fileId} failed to set $obtainer_methods_key Exception: {$e->getMessage()}");
    }

    return $text;
  }


  /**
   * Runs the parse methods prescribed by getParseOrder().
   */
  protected function runParserOrder() {
    drush_migrate_html_debug_output('----------------------row-------------------');
    $parse_methods = $this->getParseOrder();
    foreach (is_array($parse_methods) ? $parse_methods : array() as $method) {
      // Check to see if this $method is an available method.
      if (method_exists($this, $method)) {
        // Run the method to do its job.
        $this->$method();
      }
      else {
        // Output a message that the method does not exist.
        drush_migrate_html_debug_output("The parser method '{$method}' in SourceParser and its children does not exist and was skipped.");
      }
    }
  }
}
