<?php

/**
 * @file
 * Class ObtainHtml
 *
 * Contains a collection of stackable finders that can be arranged
 * as needed to obtain a body or other long html content.
 */

/**
 * Obtains HTML using and stack of finder methods.
 */
class ObtainHtml extends Obtainer {

  /**
   * Get specific tds from a table, and lines it up to be removed.
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
  protected function getFromTable($table, $tr_target, $td_target) {
    $trcount = 1;
    $tdcount = 1;

    foreach ($table->find("tr") as $tr) {
      if ($trcount == $tr_target) {
        foreach ($tr->find("td") as $td) {
          if ($tdcount == $td_target) {
            $this->setElementToRemove($td);
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
   * Takes a string, returns anything before a <br> tag and its many variants.
   *
   * @param string $text
   *   The text to break at the first <br> variant.
   *
   * @return string
   *   The string appearing before the <br> or the full string if no <br>.
   */
  public static function trimAtBr($text = '') {
    // Replace variations of br tag.
    // @codingStandardsIgnoreStart
    $search = array(
      '<br>',
      '<br />',
      '<br/>',
    );
    $text = str_ireplace($search, '<br>', $text);
    $texts = explode('<br>', $text);
    // @codingStandardsIgnoreEnd

    return $texts[0];
  }

  /**
   * Takes a string, returns anything before a lone <br> tag and its variants.
   *
   * @param string $text
   *   The text to break at the first <br> variant.
   * @param object $qp_element
   *   The query path object that may need things removed from.
   * @param int $max_length
   *   The maximum length of the text to be considered valid.
   *
   * @return string
   *   The string appearing before the blank <br> or the full string if no <br>.
   */
  protected function trimAtBrBlank($text, $qp_element, $max_length = 0) {
    // Replace variations of br tag.
    // @codingStandardsIgnoreStart
    $search = array(
      '<br>',
      '<br />',
      '<br/>',
    );
    $text = str_ireplace($search, '<br>', $text);
    $texts = explode('<br>', $text);
    // @codingStandardsIgnoreEnd
    $trimmed = '';
    $lines_used = 0;
    foreach ($texts as $line_num => $line) {
      if (!empty($line)) {
        $lines_used = $line_num;
        $trimmed .= ' ' . $line;
      }
      else {
        break;
      }
    }
    // Clean string.
    $processed_text = $this->cleanString($trimmed);
    // Evaluate string.
    $valid = $this->validateString($processed_text);
    $length = drupal_strlen($processed_text);
    if ($valid && ($max_length == 0 || $max_length >= $length)) {
      // It was valid so strip out each line.
      foreach ($texts as $line_num => $line) {
        if ($line_num <= $lines_used) {
          $this->extractAndPutBack($line, $qp_element);
        }
      }
      return $trimmed;
    }
    else {
      return '';
    }
  }

  /**
   * Strips html, truncates to word boundary, and preserves what was left.
   *
   * @param string $text
   *   Html or plain text to be truncated.
   * @param int $length
   *   The number of characters to truncate to.
   * @param int $min_word_length
   *   Minimum number of characters to consider something as a word.
   *
   * @return array
   *   - truncated: Plain text that has been truncated.
   *   - remaining: Plain text that was left.
   */
  public static function truncateThisWithoutHTML($text = '', $length = 255, $min_word_length = 2) {
    $text = strip_tags($text);
    $trunc_text = truncate_utf8($text, $length, TRUE, FALSE, $min_word_length);
    // Check to see if any truncation is made.
    if (strcmp($text, $trunc_text) != 0) {
      // There was truncation, so process it differently.
      // Grab the remaining text by removing $trunc_test.
      $remaining_text = str_replace($trunc_text, '', $text);
    }
    $return = array(
      'truncated' => $trunc_text,
      'remaining' => (!empty($remaining_text)) ? $remaining_text : '',
    );

    return $return;
  }

  /**
   * Extracts, validates a string from html and puts remainder into the source.
   *
   * @param string $string
   *   The string of text to validated and remove.
   * @param object $qp_element
   *   The queryPath element to alter and put back.
   */
  protected function extractAndPutBack($string, $qp_element) {
    // Clean string.
    $processed_text = $this->cleanString($string);
    $valid = $this->validateStrong($processed_text);
    if ($valid) {
      // The string checks out, remove the original string from the element.
      $full_source = $qp_element->html();
      $new_source = str_replace($string, '', $full_source);
      $qp_element->html($new_source);
    }
  }
}
