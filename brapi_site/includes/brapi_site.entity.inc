<?php

/**
 * @file
 * BrAPI site entity.
 */

/**
 * The class used for BrAPI site entities.
 */
class BrapiSite extends Entity {

  /**
   * Constructs a BrAPI entity.
   */
  public function __construct($values = array()) {
    parent::__construct($values, 'brapi_site');
  }

  /**
   * Returns BrAPI entity label.
   *
   * @return string
   *   The BrAPI site label.
   */
  protected function defaultLabel() {
    return $this->title;
  }

  /**
   * Returns BrAPI entity URI.
   *
   * @return string
   *   The BrAPI site URI.
   */
  protected function defaultUri() {
    return array('path' => 'brapi_site/' . $this->bsid);
  }

}
