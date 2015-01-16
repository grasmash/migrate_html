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
  protected $fileId;
  protected $id;
  public $queryPath;

  /**
   * Constructor.
   *
   * @param string $file_id
   *   The file id, e.g. careers/legal/pm7205.html
   * @param string $html
   *   The full HTML data as loaded from the file.
   * @param array $qp_options
   *   An associative array of options to be passed to the html_qp() function.
   * @param array $arguments
   *   An associative array arguments passed up from the migration class.
   */
  public function __construct($file_id, $html, $qp_options = array(), $arguments = array()) {
    $this->mergeArguments((array) $arguments);

    $html = StringCleanUp::fixEncoding($html);
    $html = StringCleanUp::stripWindowsCRChars($html);
    $html = StringCleanUp::fixWindowSpecificChars($html);

    $this->initQueryPath($html, $qp_options);
    $this->fileId = $file_id;

    // Runs the methods defined in getParseOrder to set Title, date, body, etc.
    $this->drushPrintSeparator();
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
      HtmlCleanUp::stripOrFixLegacyElements($this->queryPath, $this->arguments);
    }
    catch (Exception $e) {
      $this->sourceParserMessage('Failed to clean the html, Exception: @error_message', array('@error_message' => $e->getMessage()), WATCHDOG_ERROR);
    }
  }

  /**
   * Returns and removes last updated date from markup.
   *
   * @return string
   *   The update date.
   */
  public function extractUpdatedDate() {
    $method_stack = array(
      'findClassLastupdate',
    );
    $date = $this->runObtainer('ObtainDate', 'date_updated', $method_stack);

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
  protected function setObtainerMethods(array $obtainer_methods) {
    $args = $this->getArguments();
    // Loop through new array of obtainer methods.
    foreach ($obtainer_methods as $obtainer_methods_key => $stack) {
      // Put the new into the original.
      $args['obtainer_methods'][$obtainer_methods_key] = $stack;
    }
    // Put the newly formed args back.
    $this->mergeArguments($args);
  }

  /**
   * Defines and returns an array of parsing methods to call in order.
   *
   * @return array
   *   An array of parsing methods to call in order.
   */
  public function getParseOrder() {
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
   * Use an Obtainer class to obtain some markup.
   *
   * @param string $obtainer_class
   *   The name of the obtainer class to use.
   * @param object $query_path
   *   The query path object to use as the source of possible content.
   * @param array $method_stack
   *   The stack of findMethods to use.
   */
  public function obtain($obtainer_class, $query_path, $method_stack) {
    $obtainer = new $obtainer_class($query_path, $method_stack);

    return $obtainer->obtain();
  }

  /**
   * Runs an obtainer and returns the text it found.
   *
   * @param string $obtainer_class
   *   The name of the obtainer class to use.
   * @param string $obtainer_methods_key
   *   The key for the obtainer arguments array to see which findMethods to run.
   * @param array $method_stack
   *   (optional) The stack of findMethods to use. Defaults to values from
   *   $this->getObtainerMethods().
   *
   * @return string
   *   The string retrieved by executing the obtainer.
   */
  public function runObtainer($obtainer_class, $obtainer_methods_key, $method_stack = array()) {
    $text = '';

    try {
      if (empty($method_stack)) {
        $method_stack = $this->getObtainerMethods($obtainer_methods_key);
      }

      $this->sourceParserMessage("Obtaining @key via @obtainer_class", array('@key' => $obtainer_methods_key, '@obtainer_class' => $obtainer_class));
      $text = $this->obtain($obtainer_class, $this->queryPath, $method_stack);
    }
    catch (Exception $e) {
      $this->sourceParserMessage("Failed to set @key, Exception: @error_message", array(
        '@key' => $obtainer_methods_key,
        '@error_message' => $e->getMessage(),
      ), WATCHDOG_ERROR);
    }

    return $text;
  }

  /**
   * Runs the parse methods defined in by getParseOrder().
   */
  protected function runParserOrder() {
    $parse_methods = $this->getParseOrder();
    foreach ($parse_methods as $method) {
      $this->$method();
    }
  }

  /**
   * Prints a log message separator to drush.
   */
  protected function drushPrintSeparator() {
    if (drupal_is_cli() && variable_get('migrate_html_drush_debug', FALSE)) {
      drush_print(str_repeat('-', 40));
      $this->sourceParserMessage('@class: @file_id:', array('@class' => get_class($this), '@file_id' => $this->fileId), WATCHDOG_DEBUG, 0);
    }
  }

  /**
   * Logs a system message.
   *
   * @param string $message
   *   The message to store in the log. Keep $message translatable
   *   by not concatenating dynamic values into it! Variables in the
   *   message should be added by using placeholder strings alongside
   *   the variables argument to declare the value of the placeholders.
   *   See t() for documentation on how $message and $variables interact.
   * @param array $variables
   *   Array of variables to replace in the message on display or
   *   NULL if message is already translated or not possible to
   *   translate.
   * @param int $severity
   *   The severity of the message; one of the following values as defined in
   *   - WATCHDOG_EMERGENCY: Emergency, system is unusable.
   *   - WATCHDOG_ALERT: Alert, action must be taken immediately.
   *   - WATCHDOG_CRITICAL: Critical conditions.
   *   - WATCHDOG_ERROR: Error conditions.
   *   - WATCHDOG_WARNING: Warning conditions.
   *   - WATCHDOG_NOTICE: (default) Normal but significant conditions.
   *   - WATCHDOG_INFO: Informational messages.
   *   - WATCHDOG_DEBUG: Debug-level messages.
   *
   * @param int $indent
   *   (optional). Sets indentation for drush output. Defaults to 1.
   *
   * @link http://www.faqs.org/rfcs/rfc3164.html RFC 3164: @endlink
   */
  protected function sourceParserMessage($message, $variables = array(), $severity = WATCHDOG_NOTICE, $indent = 1) {
    $type = get_class($this);
    watchdog($type, $message, $variables, $severity);

    if (drupal_is_cli() && variable_get('migrate_html_drush_debug', FALSE)) {
      $formatted_message = format_string($message, $variables);
      drush_print($formatted_message, $indent);
    }
  }
}
