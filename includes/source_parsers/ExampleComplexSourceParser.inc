<?php
/**
 * @file
 * ExampleSourceParser.
 */

class ExampleComplexSourceParser extends SourceParser {

  /**
   * Get the body from html and set the body var.
   *
   * @param string $body
   *   (optional) The body value to be set. Defaults to finding body via
   *   default method stack.
   */
  public function setBody() {
      // Default stack: Use this if none was defined in
      // $arguments['obtainer_methods'].
      $method_stack = array(
        'findTopBodyHtml',
        'findClassContentSub',
      );
      $this->body = $this->runObtainer('CustomBodyObtainer', 'body', $method_stack);
  }

  public function findSpecialMarkup() {
    // Default stack: Use this if none was defined in
    // $arguments['obtainer_methods'].
    $method_stack = array(
      'findByClass',
      'findByID',
      'findByRegex',
    );
    $this->body = $this->obtain('ObtainSpecialMarkup', $method_stack);
  }
}
