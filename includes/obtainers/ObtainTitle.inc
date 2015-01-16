<?php

/**
 * @file
 * Class ObtainTitle
 *
 * Contains a collection of stackable finders that can be arranged
 * as needed to obtain a title/heading and possible subtitle/subheading.
 */

/**
 * {@inheritdoc}
 */
class ObtainTitle extends ObtainHtml {

  /**
   * {@inheritdoc}
   */
  protected function processString($string) {
    return $this->truncateString($string);
  }

  /**
   * Truncates and sets the discarded if there is a remainder.
   */
  protected function truncateString($string) {
    $split = $this->truncateThisWithoutHTML($string, 255, 2);

    // @todo Add debugging to display $split['remaining'].
    // $this->setTextDiscarded($split['remaining']);

    return $split['truncated'];
  }

  /**
   * Finder method to Loop through all h1 first H1 to evaluate gets it.
   * @return string
   *   The text found.
   */
  protected function findH1Any() {

    // Check all h1
    foreach ($this->queryPath->find("h1") as $key => $h1) {
      $this->setElementToRemove($h1);
      $text = $h1->text();
      $text = $this->cleanString($text);
      if ($this->validateString($text)) {
        // @todo Add debug message.
        // $this->setCurrentFindMethod("findAnyH1-i={$key}");
        // Return the original string to avoid double cleanup causing issues.
        return $text;
      }
    }
    // If it made it this far, nothing was found.
    return '';
  }

  /**
   * Finder method to find the content of the first H1 on the page.
   * @return string
   *   The text found.
   */
  protected function findH1First() {
    $element = $this->queryPath->find("h1")->first();
    $this->setElementToRemove($element);

    return $element->text();
  }

  /**
   * Finder method to find the content of the first H2 on the page.
   * @return string
   *   The text found.
   */
  protected function findH2First() {
    $element = $this->queryPath->find("h2")->first();
    $this->setElementToRemove($element);

    return $element->text();
  }

  /**
   * Finder method to find the content of the first #headline on the page.
   * @return string
   *   The text found.
   */
  protected function findIdHeadline() {
    $element = $this->queryPath->find("#headline")->first();
    $this->setElementToRemove($element);

    return $element->text();
  }

  /**
   * Find  the content of the first  "p > strong > em" on the page.
   * @return string
   *   The text found.
   */
  protected function findPStrongEm() {
    $element = $this->queryPath->find("p > strong > em")->first();
    $this->setElementToRemove($element);

    return $element->text();
  }

  /**
   * Finder method to find the content of the title.
   * @return string
   *   The text found.
   */
  protected function findTitleTag() {
    $element = $this->queryPath->find("title");
    $this->setElementToRemove($element);

    return $element->text();
  }

  /**
   * Overrides parent::cleanString().
   *
   * {@inheritdoc}
   */
  public static function cleanString($text) {
    $text = strip_tags($text);
    // Titles can not have html entities.
    $text = html_entity_decode($text, ENT_COMPAT, 'UTF-8');

    // There are also numeric html special chars, let's change those.
    module_load_include('inc', 'doj_migration', 'includes/doj_migration');
    $text = doj_migration_html_entity_decode_numeric($text);

    // We want out titles to be only digits and ascii chars so we can produce
    // clean aliases.
    $text = StringCleanUp::convertNonASCIItoASCII($text);

    // Remove white space-like things from the ends and decodes html entities.
    $text = StringCleanUp::superTrim($text);
    // Remove multiple spaces.
    $text = preg_replace('!\s+!', ' ', $text);
    // Convert to ucwords If the entire thing is caps. Otherwise leave it alone
    // for preservation of acronyms.
    // Caveat: will obliterate acronyms if the entire title is caps.
    $uppercase_version = strtoupper($text);
    if (strcmp($uppercase_version, $text) == 0) {
      // The entire thing is caps.
      $text = ucwords(strtolower($text));
    }

    // Remove undesirable chars.
    $text = str_replace('»', '', $text);

    return $text;
  }
}
