<?php

/**
 * @file
 * Class ObtainSpecialMarkup
 *
 * A collection of finders methods used for finding a string within a QueryPath
 * object.
 */

/**
 * Class ObtainSpecialMarkup
 *
 * Obtains the HTML body.
 */
class ObtainSpecialMarkup extends ObtainHtml {

  /**
   * {@inheritdoc}
   */
  public function __construct($query_path, $method_stack) {
    parent::__construct($query_path, $method_stack);
    $this->processMethodStack($query_path, $method_stack, 'ObtainBody');
  }

  /**
   * Finder method to find the top body.
   *
   * @return string
   *   The string that was found
   */
  protected function findTopBodyHtml() {
    $this->setJustFound($this->queryPath->top('body'));
    $string = $this->getJustFound()->innerHTML();
    // This is essentially everything. so lets not remove it.
    $this->removeMeNot();
    return $string;
  }

  /**
   * Finder method to find the body in .contentSub.
   *
   * @return string
   *   The string that was found
   */
  protected function findClassContentSub() {
    $this->setJustFound($this->queryPath->top('.contentSub'));
    $string = $this->getJustFound()->innerHTML();
    // This is essentially everything. so lets not remove it.
    $this->removeMeNot();
    return $string;
  }
}
