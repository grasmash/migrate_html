<?php

/**
 * @file
 * Contains classes for chunking HTML files.
 */

/**
 * Class StaticHtmlChunkParser
 *
 * Parses a single HTML file into many chunks, each of which is mapped to a
 * separate destination node. Used for 'many to many' migrations where
 * each file contains content for more than one destination.
 *
 * @package migrate_html
 */
abstract class StaticHtmlChunkParser extends MigrateContentParser {
  /**
   * $this->setContent() will be called by the migrate module. Other methods
   * can rely on this being set.
   */
  protected $content;
  protected $sourceParser;
  protected $chunks;

  /**
   * {@inheritdoc}
   */
  public function setContent($content) {
    parent::setContent($content);
    // Create a new SourceParser to handle HTML content.
    // We don't need the full ID now. Migrate will generate it and make it
    // available in prepareRow().
    $this->sourceParser = new SourceParser('placeholder', $this->content);
    $this->setChunks();
  }

  /**
   * Gets $this->chunks.
   *
   * @return QueryPath
   *   A QueryPath object containing one or more elements.
   */
  public function getChunks() {
    if (!isset($this->chunks)) {
      $this->setChunks();
    }
    return $this->chunks;
  }

  /**
   * {@inheritdoc}
   */
  public function getChunkCount() {
    $chunks = $this->getChunks();
    return count($chunks);
  }

  /**
   * Sets $this->chunks.
   */
  abstract public function setChunks();
}
