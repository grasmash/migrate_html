<?php

/**
 * @file
 * Class ObtainBody
 *
 * Contains a collection of stackable finders that can be arranged
 * as needed to obtain a body or other long html content.
 */

/**
 * Class ObtainBody
 *
 * Obtains the HTML body.
 */
class ObtainBody extends ObtainHtml {

  /**
   * {@inheritdoc}
   */
  public function __construct($query_path, $method_stack) {
    parent::__construct($query_path, $method_stack);
    $this->processMethodStack($query_path, $method_stack, 'ObtainBody');
  }

  // **************** Begin finder target definitions *************************
  // To create a new finder, use this template and put them in alpha order.
  // @codingStandardsIgnoreStart
  /*
  protected function findMethod() {
    $this->setJustFound($this->queryPath->find("{SELECTOR}")->first());
    $text = $this->getJustFound()->text();
    return $text;
  }
  */
  // @codingStandardsIgnoreEnd

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
