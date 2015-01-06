<?php
/**
 * @file
 * Helper function to clean up HTML.
 */

class HtmlCleanUp {

  /**
   * Removes legacy elements from HTML that are no longer needed.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   *
   * @param array $arguments
   *   (optional). An array of arbitrary arguments to be used by HtmlCleanUp
   *   methods. Defaults to empty array.
   */
  public static function stripOrFixLegacyElements($query_path, $arguments = array()) {

    // Strip comments.
    foreach ($query_path->top()->xpath('//comment()')->get() as $comment) {
      $comment->parentNode->removeChild($comment);
    }

    // Remove elements and their children.
    HTMLCleanUp::removeElements($query_path, array(
      'a[name="sitemap"]',
      'a[name="maincontent"]',
      'img[src="/gif/logo.gif"]',
      'div.skip',
      'div.hdrwrpr',
      'div.breadcrumbmenu',
      'div.footer',
      'div.lastupdate',
      'div.thick-bar',
      'div.rightcolumn',
      'div.leftcolmenu',
      'a[href="#top"]',
    ));

    // Remove extraneous html wrapping elements, leaving children intact.
    HTMLCleanUp::removeWrapperElements($query_path, array(
      'body > blockquote',
      '.bdywrpr',
      '.gridwrpr',
      '.leftcol-subpage',
      '.leftcol-subpage-content',
      '.bodytextbox',
      'body > div',
    ));

    // Remove style attribute from elements.
    $query_path->find('.narrow-bar')->removeAttr('style');

    // Remove matching elements containing only &nbsp; or nothing.
    HTMLCleanUp::removeEmptyElements($query_path, array(
      'div',
      'span',
      'p',
    ));

    // Empty anchors without name attribute will be stripped by ckEditor.
    HTMLCleanUp::fixNamedAnchors($query_path);
  }

  /**
   * Removes elements matching CSS selectors.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   *
   * @param array $selectors
   *   An array of selectors to remove.
   */
  public static function removeElements($query_path, array $selectors) {
    foreach ($selectors as $selector) {
      $query_path->find($selector)->remove();
    }
  }

  /**
   * Removes elements matching CSS selectors from html.
   *
   * @param string $html
   *   Html to get processed.
   * @param array $selectors
   *   An array of selectors to remove.
   *
   * @return string
   *   Processed html.
   */
  public static function removeElementsFromHtml($html, array $selectors) {
    // Put the shell on the html to extract with more certainty later.
    $html = '<div class="throw-away-parser-shell">' . $html . '</div>';
    $query_path = htmlqp($html, NULL, array());
    HTMLCleanUp::removeElements($query_path, $selectors);

    // Grab the html from the shell.
    $processed_html = $query_path->top('.throw-away-parser-shell')->innerHTML();
    return $processed_html;
  }

  /**
   * Get the first element matching the CSS selector from html.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   *
   * @param string $selector
   *   A css selector.
   *
   * @return string
   *   The text from the first matching selector, if matched.
   */
  public static function getFirstElement($query_path, $selector) {
    $text = "";

    // Put the shell on the html to extract with more certainty later.
    $items = $query_path->find($selector);
    foreach ($items as $item) {
      $text = $item->text();
      break;
    }

    return $text;
  }

  /**
   * Extract the first elements with the CSS selector from html.
   *
   * Extraction means that we return the match, but we also return the
   * original html without the element that matched the search.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   *
   * @param string $selector
   *   A CSS selector to extract.
   *
   * @return array
   *   The array contains the matched text, and the original html without the
   *   match.
   */
  public static function extractFirstElement($query_path, $selector) {

    $items = $query_path->find($selector);
    foreach ($items as $item) {
      $text = $item->text();
      $item->remove();
      break;
    }

    return $text;
  }

  /**
   * Removes a wrapping element, leaving child elements intact.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   * @param array $selectors
   *   An array of selectors for the wrapping element(s).
   */
  protected static function removeWrapperElements($query_path, array $selectors) {
    foreach ($selectors as $selector) {
      $children = $query_path->find($selector)->children();
      $children->unwrap();
    }
  }

  /**
   * Rewraps an element, leaving child elements intact.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   *
   * @param array $selectors
   *   An array of selectors for the wrapping element(s).
   *
   * @param string $new_wrapper
   *   A string of the leading wrapping element.
   *   - <h2 />
   *   - <h2 id="title" />
   *   - <div class="friends" />
   */
  public static function rewrapElements($query_path, array $selectors, $new_wrapper) {
    // There is something to wrap it in, so begin the hunt.
    foreach ($selectors as $selector) {
      $elements = $query_path->find($selector);
      foreach ($elements as $element) {
        $element->wrapInner($new_wrapper);
      }
    }
    HTMLCleanUp::removeWrapperElements($query_path, $selectors);
  }

  /**
   * Removes empty elements matching selectors.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   *
   * @param array $selectors
   *   An array of selectors to remove.
   */
  public static function removeEmptyElements($query_path, array $selectors) {
    foreach ($selectors as $selector) {
      $elements = $query_path->find($selector);
      foreach ($elements as $element) {
        $contents = StringCleanUp::superTrim($element->innerXHTML());
        $empty_values = array(
          '&nbsp;',
          '',
        );
        if (in_array($contents, $empty_values)) {
          $element->remove();
        }
      }
    }
  }

  /**
   * Empty anchors without name attribute will be stripped by ckEditor.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   */
  protected static function fixNamedAnchors($query_path) {
    $elements = $query_path->find('a');
    foreach ($elements as $element) {
      $contents = trim($element->innerXHTML());
      if ($contents == '') {
        if ($anchor_id = $element->attr('id')) {
          // Only set a name if there isn't one.
          if (!$element->hasAttr('name')) {
            $element->attr('name', $anchor_id);
          }
        }
      }
    }
  }

  /**
   * Makes relative sources values on <a> and <img> tags absolute.
   *
   * @param QueryPath $query_path
   *   The QueryPath object with HTML markup.
   *
   * @param string $file_id
   *   The full file path of the of the current file, used to determine
   *   location of relative links.
   */
  public static function convertRelativeSrcsToAbsolute($query_path, $file_id) {

    // A list of attributes to convert, keyed by HTML tag (NOT selector).
    $attributes = array(
      'img' => 'src',
      'a' => 'href',
    );

    $elements = $query_path->find(implode(', ', array_keys($attributes)));
    foreach ($elements as $element) {
      $attribute = $attributes[$element->tag()];

      $url = parse_url($element->attr($attribute));

      if ($url) {
        $is_relative = empty($url['scheme']) && !empty($url['path']) && substr($url['path'], 0, 1) !== '/';

        if ($is_relative) {
          $dir_path = dirname($file_id);
          $new_url = '/' . $dir_path . '/' . $url['path'];

          // We might get some double '//', let's clean them.
          $new_url = str_replace("//", "/", $new_url);

          $element->attr($attribute, $new_url);
        }
      }
    }
  }

  /**
   * General matching function.
   *
   * @param QueryPath $qp
   *   A QueryPath object.
   * @param string $selector
   *   The CSS selector for the element to be matched.
   * @param string $needle
   *   The text string for which to search.
   * @param string $function
   *   The function used to get the haystack. E.g., 'attr' if searching for
   *   a specific attribute value.
   * @param string $parameter
   *   A parameter to be passed into the defined $function.
   *
   * @return mixed
   *   The matched QueryPath element or FALSE.
   */
  private static function match($qp, $selector, $needle, $function, $parameter = NULL) {
    $elements = $qp->find($selector);
    foreach ($elements as $key => $elem) {
      $haystack = $elem->$function($parameter);
      if (substr_count($haystack, $needle) > 0) {
        return $elem;
      }
    }
    return FALSE;
  }

  /**
   * Return an element if the text in the attribute matches a search needle.
   *
   * @param QueryPath $qp
   *   QueryPath object.
   * @param string $selector
   *   The CSS selector for the element to be matched.
   * @param string $needle
   *   The text string for which to search.
   * @param string $attribute
   *   The HTML attribute whose value will be searched.
   *
   * @return mixed
   *   The matched QueryPath element or FALSE.
   */
  public static function matchAttribute($qp, $selector, $needle, $attribute) {
    return HtmlCleanUp::match($qp, $selector, $needle, "attr", $attribute);
  }

  /**
   * Return an element if the text that it contains matches a search needle.
   *
   * @param QueryPath $qp
   *   A QueryPath object.
   * @param string $selector
   *   The selector to look into.
   * @param string $needle
   *   The text string for which to search.
   *
   * @return mixed
   *   The matched QueryPath element or FALSE.
   */
  public static function matchText($qp, $selector, $needle) {
    return HtmlCleanUp::match($qp, $selector, $needle, "text");
  }

  /**
   * Return an element if the HMTL that it contains matches a search needle.
   *
   * @param QueryPath $qp
   *   A QueryPath object.
   * @param string $selector
   *   The selector to look into.
   * @param string $needle
   *   The text string for which to search.
   *
   * @return mixed
   *   The matched QueryPath element or FALSE.
   */
  public static function matchHtml($qp, $selector, $needle) {
    return HtmlCleanUp::match($qp, $selector, $needle, "html");
  }
}
