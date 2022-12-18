<?php

namespace Drupal\brapi;

use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\Core\Entity\EntityChangedInterface;

/**
 * Provides an interface defining a BrAPI Token entity.
 */
interface BrapiTokenInterface extends ContentEntityInterface, EntityChangedInterface {

  /**
   * Tells if the token expired.
   *
   * @return bool
   *   TRUE if expired, FALSE otherwise.
   */
  public function isExpired() :bool;

  /**
   * Renew current token and extend its expiration time.
   */
  public function renew();

}
