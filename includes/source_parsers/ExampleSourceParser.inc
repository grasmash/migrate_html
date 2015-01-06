<?php
/**
 * @file
 * ExampleSourceParser.
 */

class ExampleSourceParser extends SourceParser {

  /**
   * {@inheritdoc}
   */
  public function setBody() {

    try {
      // Get rid of title banners.
      $this->queryPath->find(".top-bar")->remove();

      // Subtitle are inside of tables, Liberate them.
      foreach ($this->queryPath->find(".headline") as $headline) {
        $text = $headline->text();
        $headline->html("<h2>{$text}</h2>");
      }
    }
    catch(Exception $e) {

    }
    parent::setBody();
  }
}
