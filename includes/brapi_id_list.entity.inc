<?php

/**
 * @file
 * BrAPI ID List.
 */

/**
 * The class used for BrAPI ID list entities.
 */
class BrapiIdList extends Entity {

  /**
   * Constructs a BrAPI entity.
   */
  public function __construct($values = array()) {
    parent::__construct($values, 'brapi_id_list');
  }

  /**
   * Returns BrAPI entity label.
   *
   * @return string
   *   The BrAPI ID list label.
   */
  protected function defaultLabel() {
    return $this->title;
  }

  /**
   * Returns BrAPI entity URI.
   *
   * @return string
   *   The BrAPI ID list URL.
   */
  protected function defaultUri() {
    return array('path' => 'brapi_id_list/' . $this->blid);
  }

}
